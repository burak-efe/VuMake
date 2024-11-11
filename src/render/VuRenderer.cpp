#include "VuRenderer.h"

namespace Vu {
    bool VuRenderer::ShouldWindowClose() {
        return ctx::sdlEvent.type == SDL_EVENT_QUIT;
    }

    void VuRenderer::WaitIdle() {
        vkDeviceWaitIdle(ctx::device);
    }

    void VuRenderer::BeginFrame() {
        SDL_PollEvent(&ctx::sdlEvent);

        vkWaitForFences(ctx::device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

        VkResult result = vkAcquireNextImageKHR(
            ctx::device, swapChain.swapChain, UINT64_MAX,
            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentFrameImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            ResetSwapChain();
            std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
        } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(ctx::device, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
        BeginRecordCommandBuffer(commandBuffers[currentFrame], currentFrameImageIndex);
    }

    void VuRenderer::BeginRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkCheck(vkBeginCommandBuffer(commandBuffer, &beginInfo));
        swapChain.BeginRenderPass(commandBuffer, imageIndex);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = (float) swapChain.swapChainExtent.height;
        viewport.width = (float) swapChain.swapChainExtent.width;
        viewport.height = -(float) swapChain.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChain.swapChainExtent;
        vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

        bindGlobalSet(commandBuffer);

    }

    void VuRenderer::EndRecordCommandBuffer(const VkCommandBuffer& commandBuffer, uint32 imageIndex) {
        swapChain.EndRenderPass(commandBuffer);
        VkCheck(vkEndCommandBuffer(commandBuffer));

    }

    void VuRenderer::BindMesh(const VuMesh& mesh) {
        auto commandBuffer = commandBuffers[currentFrame];

        std::array vertexBuffers = {
            mesh.vertexBuffer.buffer,
            mesh.vertexBuffer.buffer,
            mesh.vertexBuffer.buffer,
            mesh.vertexBuffer.buffer
        };

        std::array<VkDeviceSize, 4> offsets = {0, mesh.getNormalOffsetAsByte(), mesh.getTangentOffsetAsByte(), mesh.getUV_OffsetAsByte()};
        vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets.data());

        vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VuRenderer::BindMaterial(const VuMaterial& material, VuPushConstant pushConstant) {
        auto commandBuffer = commandBuffers[currentFrame];
        //material.bindFrameConstants(commandBuffer, currentFrame);
        material.bindPipeline(commandBuffer);
        material.pushConstants(commandBuffer, pushConstant);
    }

    void VuRenderer::DrawIndexed(uint32 indexCount) {
        auto commandBuffer = commandBuffers[currentFrame];
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    void VuRenderer::BeginImgui() {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDL3_ProcessEvent(&ctx::sdlEvent);
        ImGui::NewFrame();
    }

    void VuRenderer::EndImgui() {
        auto commandBuffer = commandBuffers[currentFrame];
        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }

    void VuRenderer::EndFrame() {
        EndRecordCommandBuffer(commandBuffers[currentFrame], currentFrameImageIndex);
        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore waitSemaphores[] = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = waitSemaphores;
        submitInfo.pWaitDstStageMask = waitStages;

        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffers[currentFrame];

        VkSemaphore signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = signalSemaphores;

        VkCheck(vkQueueSubmit(ctx::graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = signalSemaphores;

        VkSwapchainKHR swapChains[] = {swapChain.swapChain};
        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = swapChains;

        presentInfo.pImageIndices = &currentFrameImageIndex;

        auto result = vkQueuePresentKHR(ctx::presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
            ResetSwapChain();
        } else if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to present swap chain image!");
        }

        currentFrame = (currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
    }


    void VuRenderer::UpdateUniformBuffer(VuFrameConst ubo) {



        uniformBuffers[currentFrame].SetData(&ubo, sizeof(ubo));
    }

    // void VuRenderer::PushConstants(VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* pValues) {
    //     vkCmdPushConstants(commandBuffers[currentFrame], debugPipeline.PipelineLayout, stage, offset, size, pValues);
    // }

    void VuRenderer::ResetSwapChain() {
        SDL_Event event;

        int width = 0;
        int height = 0;
        SDL_GetWindowSize(ctx::window, &width, &height);
        auto minimized = (SDL_GetWindowFlags(ctx::window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;

        while (width <= 0 || height <= 0 || minimized) {
            SDL_GetWindowSize(ctx::window, &width, &height);
            minimized = (SDL_GetWindowFlags(ctx::window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
            SDL_WaitEvent(&event);
        }
        vkDeviceWaitIdle(ctx::device);
        swapChain.ResetSwapChain(surface);
    }
}
