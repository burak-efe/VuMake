#include "VuRenderer.h"

bool VuRenderer::ShouldWindowClose() {
    return Vu::sdlEvent.type == SDL_EVENT_QUIT;
}

void VuRenderer::WaitIdle() {
    vkDeviceWaitIdle(Vu::Device);
}

void VuRenderer::BeginFrame() {
    SDL_PollEvent(&Vu::sdlEvent);

    vkWaitForFences(Vu::Device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    VkResult result = vkAcquireNextImageKHR(
        Vu::Device, swapChain.swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentFrameImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        ResetSwapChain();
        std::cerr << "[INFO]: SwapChain Recreated because of VK_ERROR_OUT_OF_DATE_KHR" << "\n";
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    vkResetFences(Vu::Device, 1, &inFlightFences[currentFrame]);
    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    BeginRecordCommandBuffer(commandBuffers[currentFrame], currentFrameImageIndex);
}

void VuRenderer::BeginRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    swapChain.BeginRenderPass(commandBuffer, imageIndex);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float) swapChain.SwapChainExtent.height;
    viewport.width = (float) swapChain.SwapChainExtent.width;
    viewport.height = -(float) swapChain.SwapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.SwapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

}

void VuRenderer::EndRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) {
    swapChain.EndRenderPass(commandBuffer);
    VK_CHECK(vkEndCommandBuffer(commandBuffer));

}

void VuRenderer::BindMesh(const Mesh& mesh) {
    auto commandBuffer = commandBuffers[currentFrame];

    std::array vertexBuffers = {mesh.vertexBuffer.buffer, mesh.normalBuffer.buffer, mesh.uvBuffer.buffer};
    VkDeviceSize offsets[] = {0, 0, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets);

    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
}

void VuRenderer::BindMaterial(const VuMaterial& material, VuPushConstant pushConstant) {
    auto commandBuffer = commandBuffers[currentFrame];
    material.bindFrameConstants(commandBuffer, currentFrame);
    material.PushConstants(commandBuffer, pushConstant);
    material.bindPipeline(commandBuffer);
}

void VuRenderer::DrawIndexed(uint32 indexCount) {
    auto commandBuffer = commandBuffers[currentFrame];
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}

void VuRenderer::BeginImgui() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui_ImplSDL3_ProcessEvent(&Vu::sdlEvent);
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

    VK_CHECK(vkQueueSubmit(Vu::graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &currentFrameImageIndex;

    auto result = vkQueuePresentKHR(Vu::presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        ResetSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % Vu::MAX_FRAMES_IN_FLIGHT;
}


void VuRenderer::UpdateUniformBuffer(FrameUBO ubo) {
    uniformBuffers[currentFrame].SetData(&ubo, sizeof(ubo));
}

// void VuRenderer::PushConstants(VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* pValues) {
//     vkCmdPushConstants(commandBuffers[currentFrame], debugPipeline.PipelineLayout, stage, offset, size, pValues);
// }

void VuRenderer::ResetSwapChain() {
    SDL_Event event;

    int width = 0;
    int height = 0;
    SDL_GetWindowSize(Vu::Window, &width, &height);
    auto minimized = (SDL_GetWindowFlags(Vu::Window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;

    while (width <= 0 || height <= 0 || minimized) {
        SDL_GetWindowSize(Vu::Window, &width, &height);
        minimized = (SDL_GetWindowFlags(Vu::Window) & SDL_WINDOW_MINIMIZED) == SDL_WINDOW_MINIMIZED;
        SDL_WaitEvent(&event);
    }
    vkDeviceWaitIdle(Vu::Device);
    swapChain.ResetSwapChain(surface);
}
