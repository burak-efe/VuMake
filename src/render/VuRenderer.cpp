#include "VuRenderer.h"


#include "VuSync.h"

bool VuRenderer::ShouldWindowClose() {
    return glfwWindowShouldClose(Vu::window);
}

void VuRenderer::WaitIdle() {
    vkDeviceWaitIdle(Vu::Device);
}

void VuRenderer::BeginFrame() {
    glfwPollEvents();

    vkWaitForFences(Vu::Device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);


    VkResult result = vkAcquireNextImageKHR(
        Vu::Device, SwapChain.swapChain, UINT64_MAX,
        imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &currentFrameImageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
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

    // With dynamic rendering there are no subpass dependencies,
    // so we need to take care of proper layout transitions by using barriers
    // This set of barriers prepares the color and depth images for output
    VuSync::InsertImageMemoryBarrier2(
        commandBuffer,
        SwapChain.swapChainImages[imageIndex],
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    VuSync::InsertImageMemoryBarrier2(
        commandBuffer,
        DepthStencil.Image,
        0,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VkImageSubresourceRange{
            VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT,
            0, 1, 0, 1
        });


    // New structures are used to define the attachments used in dynamic rendering
    VkRenderingAttachmentInfoKHR colorAttachment{};
    colorAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    colorAttachment.imageView = SwapChain.swapChainImageViews[imageIndex];
    colorAttachment.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.clearValue.color = {0.0f, 0.0f, 0.0f, 0.0f};


    // A single depth stencil attachment info can be used, but they can also be specified separately.
    // When both are specified separately, the only requirement is that the image view is identical.
    VkRenderingAttachmentInfoKHR depthStencilAttachment{};
    depthStencilAttachment.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO_KHR;
    depthStencilAttachment.imageView = DepthStencil.View;
    depthStencilAttachment.imageLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    depthStencilAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthStencilAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    depthStencilAttachment.clearValue.depthStencil = {1.0f, 0};


    VkRenderingInfoKHR renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO_KHR,
        .flags = 0,
        .renderArea = {0, 0, WIDTH, HEIGHT},
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachment,
        .pDepthAttachment = &depthStencilAttachment,
        .pStencilAttachment = &depthStencilAttachment,
    };
    renderingInfo.renderArea = VkRect2D{.offset = {0, 0}, .extent = SwapChain.swapChainExtent};

    vkCmdBeginRendering(commandBuffer, &renderingInfo);


    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = (float) SwapChain.swapChainExtent.height;
    viewport.width = (float) SwapChain.swapChainExtent.width;
    viewport.height = -(float) SwapChain.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = SwapChain.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void VuRenderer::EndRecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) {

    vkCmdEndRendering(commandBuffer);

    // This set of barriers prepares the color image for presentation, we don't need to care for the depth image
    VuSync::InsertImageMemoryBarrier2(
        commandBuffer,
        SwapChain.swapChainImages[imageIndex],
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        0,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});


    VK_CHECK(vkEndCommandBuffer(commandBuffer));
}

void VuRenderer::RenderMesh(::Mesh& mesh, glm::mat4 trs) {
    auto commandBuffer = commandBuffers[currentFrame];
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, DebugPipeline.Pipeline);

    PushConstants(VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(trs), &trs);

    std::array vertexBuffers = {mesh.vertexBuffer.VulkanBuffer, mesh.normalBuffer.VulkanBuffer, mesh.uvBuffer.VulkanBuffer};
    VkDeviceSize offsets[] = {0, 0, 0};
    vkCmdBindVertexBuffers(commandBuffer, 0, vertexBuffers.size(), vertexBuffers.data(), offsets);

    vkCmdBindIndexBuffer(commandBuffer, mesh.indexBuffer.VulkanBuffer, 0, VK_INDEX_TYPE_UINT32);

    //vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, DebugPipeline.PipelineLayout,
    //                        0, 1, &descriptorSets[currentFrame], 0, nullptr);

    // Descriptor buffer bindings
    // Set 0 = uniform buffer
    VkDescriptorBufferBindingInfoEXT bindingInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_BUFFER_BINDING_INFO_EXT,
        .address = uniformDescriptor.bufferDeviceAddress,
        .usage = VK_BUFFER_USAGE_RESOURCE_DESCRIPTOR_BUFFER_BIT_EXT
    };

    vkCmdBindDescriptorBuffersEXT(commandBuffer, 1, &bindingInfo);

    uint32_t bufferIndexUbo = 0;
    VkDeviceSize bufferOffset = uniformDescriptor.layoutSize * currentFrame;

    // Global Matrices (set 0)
    bufferOffset = 0;
    vkCmdSetDescriptorBufferOffsetsEXT(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                       DebugPipeline.PipelineLayout, 0, 1, &bufferIndexUbo, &bufferOffset);


    vkCmdDrawIndexed(commandBuffer, mesh.indexBuffer.Lenght, 1, 0, 0, 0);
}

void VuRenderer::BeginImgui() {
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
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

    VkSwapchainKHR swapChains[] = {SwapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &currentFrameImageIndex;

    auto result = vkQueuePresentKHR(Vu::presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VuRenderer::UpdateUniformBuffer(FrameUBO ubo) {
    uniformBuffers[currentFrame].SetData(&ubo, sizeof(ubo));
    //memcpy(uniformBuffersMapped[currentFrame], &ubo, sizeof(ubo));
    //TODO
}

void VuRenderer::PushConstants(VkShaderStageFlags stage, uint32_t offset, uint32_t size, const void* pValues) {
    vkCmdPushConstants(commandBuffers[currentFrame], DebugPipeline.PipelineLayout,
                       stage, offset, size, pValues);
}

void VuRenderer::RecreateSwapChain() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(Vu::window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(Vu::window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(Vu::Device);
    SwapChain.CleanupSwapchain();
    CreateSwapChain();
}
