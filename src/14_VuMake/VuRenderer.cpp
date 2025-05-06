#include "VuRenderer.h"

#include <iostream>

#include "11_Config/VuConfig.h"
#include "11_Config/VuCtx.h"

#include "VuDevice.h"

namespace Vu
{
    bool VuRenderer::shouldWindowClose()
    {
        return ctx::sdlEvent.type == SDL_EVENT_QUIT;
    }

    void VuRenderer::waitIdle()
    {
        vuDevice.waitIdle();
    }

    void VuRenderer::waitForFences()
    {
        vkWaitForFences(vuDevice.device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);
    }

    void VuRenderer::beginFrame()
    {
        waitForFences();
        VkResult result = vkAcquireNextImageKHR(vuDevice.device,
                                                swapChain.swapChain,
                                                UINT64_MAX,
                                                imageAvailableSemaphores[currentFrame],
                                                VK_NULL_HANDLE,
                                                &currentFrameImageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            resetSwapChain();
            std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        vkResetFences(vuDevice.device, 1, &inFlightFences[currentFrame]);
        vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        VkCheck(vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo));
        swapChain.beginGBufferPass(commandBuffers[currentFrame], currentFrameImageIndex);

        VkViewport viewport{};
        viewport.x        = 0.0f;
        viewport.y        = (float)swapChain.swapChainExtent.height;
        viewport.width    = (float)swapChain.swapChainExtent.width;
        viewport.height   = -(float)swapChain.swapChainExtent.height;
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

        VkRect2D scissor{};
        scissor.offset = {0, 0};
        scissor.extent = swapChain.swapChainExtent;
        vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
        bindGlobalBindlessSet(commandBuffers[currentFrame]);
    }

    void VuRenderer::beginLightningPass()
    {
        VkCommandBuffer cb = commandBuffers[currentFrame];
        swapChain.endRenderPass(cb);

        VkPipelineStageFlags srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                        VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
        VkPipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask   = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
                                      VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
        memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(cb, srcStage, dstStage, 0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);

        //lightning pass
        swapChain.beginLightningPass(cb, currentFrameImageIndex);
    }

    void VuRenderer::endFrame()
    {
        VkCommandBuffer cb = commandBuffers[currentFrame];
        swapChain.endRenderPass(cb);
        VkCheck(vkEndCommandBuffer(cb));

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

        VkSemaphore          waitSemaphores[]   = {imageAvailableSemaphores[currentFrame]};
        VkPipelineStageFlags waitStages[]       = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
        VkSemaphore          signalSemaphores[] = {renderFinishedSemaphores[currentFrame]};

        submitInfo.waitSemaphoreCount   = 1;
        submitInfo.pWaitSemaphores      = waitSemaphores;
        submitInfo.pWaitDstStageMask    = waitStages;
        submitInfo.commandBufferCount   = 1;
        submitInfo.pCommandBuffers      = &commandBuffers[currentFrame];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores    = signalSemaphores;

        VkCheck(vkQueueSubmit(vuDevice.graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

        VkPresentInfoKHR presentInfo{};
        VkSwapchainKHR   swapChains[] = {swapChain.swapChain};

        presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores    = signalSemaphores;
        presentInfo.swapchainCount     = 1;
        presentInfo.pSwapchains        = swapChains;
        presentInfo.pImageIndices      = &currentFrameImageIndex;


        auto result = vkQueuePresentKHR(vuDevice.presentQueue, &presentInfo);

        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR)
        {
            resetSwapChain();
        }
        else if (result != VK_SUCCESS)
        {
            throw std::runtime_error("failed to present swap chain image!");
        }
        currentFrame = (currentFrame + 1) % config::MAX_FRAMES_IN_FLIGHT;
        FrameMark;
    }

    void VuRenderer::bindMesh(VuMesh& mesh)
    {
        //we are using vertex pulling, so only index buffers we need to bind
        auto commandBuffer = commandBuffers[currentFrame];
        auto indexBuffer   = mesh.indexBuffer.get();
        vkCmdBindIndexBuffer(commandBuffer, indexBuffer->buffer, 0, VK_INDEX_TYPE_UINT32);
    }

    void VuRenderer::bindMaterial(std::shared_ptr<VuMaterial>& material)
    {
        auto commandBuffer = commandBuffers[currentFrame];
        vuDevice.bindMaterial(commandBuffer, material);
    }

    void VuRenderer::drawIndexed(u32 indexCount)
    {
        auto commandBuffer = commandBuffers[currentFrame];
        vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
    }

    void VuRenderer::pushConstants(const GPU_PushConstant& pushConstant)
    {
        auto commandBuffer = commandBuffers[currentFrame];
        vkCmdPushConstants(commandBuffer, vuDevice.globalPipelineLayout, VK_SHADER_STAGE_ALL, 0,
                           config::PUSH_CONST_SIZE,
                           &pushConstant);
    }

    void VuRenderer::beginImgui()
    {
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDL3_ProcessEvent(&ctx::sdlEvent);
        ImGui::NewFrame();
    }

    void VuRenderer::endImgui()
    {
        auto commandBuffer = commandBuffers[currentFrame];

        ImGui::Render();
        ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    }


    void VuRenderer::updateFrameConstantBuffer(GPU_FrameConst ubo)
    {
        VkCheck(uniformBuffers[currentFrame].setData(&ubo, sizeof(ubo)));
    }


    void VuRenderer::resetSwapChain()
    {
        ZoneScoped;
        SDL_Event event;

        int width  = 0;
        int height = 0;
        SDL_GetWindowSize(ctx::window, &width, &height);
        auto minimized = (SDL_GetWindowFlags(ctx::window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;

        while (width <= 0 || height <= 0 || minimized)
        {
            SDL_GetWindowSize(ctx::window, &width, &height);
            minimized = (SDL_GetWindowFlags(ctx::window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
            SDL_WaitEvent(&event);
        }
        vkDeviceWaitIdle(vuDevice.device);
        swapChain.resetSwapChain(surface);
    }
}
