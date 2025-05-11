#include "VuRenderer.h"

#include <cstdint>                   // for UINT64_MAX
#include <iostream>                  // for char_traits, basic_ostream, oper...
#include <stdexcept>                 // for runtime_error

#include "imgui.h"                   // for GetDrawData, NewFrame, Render
#include "imgui_impl_sdl3.h"         // for ImGui_ImplSDL3_NewFrame, ImGui_I...
#include "SDL3/SDL_events.h"         // for SDL_WaitEvent, SDL_Event, SDL_Ev...
#include "SDL3/SDL_video.h"          // for SDL_GetWindowFlags, SDL_GetWindo...

#include "../02_OuterCore/VuConfig.h"
#include "../02_OuterCore/VuCtx.h"
#include "02_OuterCore/Common.h"    // for vk::Check
#include "12_VuMakeCore/VuBuffer.h" // for VuBuffer
#include "14_VuMake/VuMesh.h"       // for VuMesh
#include "14_VuMake/VuSwapChain.h"  // for VuSwapChain
#include "VuDevice.h"               // for VuDevice

namespace Vu
{
struct VuMaterial;
}

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
    vk::Result result = vkAcquireNextImageKHR(vuDevice.device,
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
    vkResetCommandBuffer(commandBuffers[currentFrame], 0);

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vk::Check(vkBeginCommandBuffer(commandBuffers[currentFrame], &beginInfo));
    swapChain.beginGBufferPass(commandBuffers[currentFrame], currentFrameImageIndex);

    vk::Viewport viewport{};
    viewport.x        = 0.0f;
    viewport.y        = (float)swapChain.swapChainExtent.height;
    viewport.width    = (float)swapChain.swapChainExtent.width;
    viewport.height   = -(float)swapChain.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffers[currentFrame], 0, 1, &viewport);

    vk::Rect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapChain.swapChainExtent;
    vkCmdSetScissor(commandBuffers[currentFrame], 0, 1, &scissor);
    bindGlobalBindlessSet(commandBuffers[currentFrame]);
}

void VuRenderer::beginLightningPass()
{
    vk::CommandBuffer cb = commandBuffers[currentFrame];
    swapChain.endRenderPass(cb);

    vk::PipelineStageFlags srcStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                    VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    vk::PipelineStageFlags dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT |
                                    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

    vk::MemoryBarrier memoryBarrier = {};
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
    vk::CommandBuffer cb = commandBuffers[currentFrame];
    swapChain.endRenderPass(cb);
    vk::Check(vkEndCommandBuffer(cb));


    vk::Semaphore          waitSemaphores[]   = {imageAvailableSemaphores[currentFrame]};
    vk::PipelineStageFlags waitStages[]       = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    vk::Semaphore          signalSemaphores[] = {renderFinishedSemaphores[currentFrameImageIndex]};

    vk::SubmitInfo submitInfo{};
    submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.pNext                = VK_NULL_HANDLE;
    submitInfo.waitSemaphoreCount   = 1u;
    submitInfo.pWaitSemaphores      = waitSemaphores;
    submitInfo.pWaitDstStageMask    = waitStages;
    submitInfo.commandBufferCount   = 1u;
    submitInfo.pCommandBuffers      = &commandBuffers[currentFrame];
    submitInfo.signalSemaphoreCount = 1u;
    submitInfo.pSignalSemaphores    = signalSemaphores;

    vk::Check(vkQueueSubmit(vuDevice.graphicsQueue, 1u, &submitInfo, inFlightFences[currentFrame]));

    vk::SwapchainKHR swapChains[] = {swapChain.swapChain};

    vk::PresentInfoKHR presentInfo{};
    presentInfo.sType              = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pNext              = nullptr;
    presentInfo.waitSemaphoreCount = 1u;
    presentInfo.pWaitSemaphores    = signalSemaphores;
    presentInfo.swapchainCount     = 1u;
    presentInfo.pSwapchains        = swapChains;
    presentInfo.pImageIndices      = &currentFrameImageIndex;
    presentInfo.pResults           = VK_NULL_HANDLE;


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
    vk::Check(uniformBuffers[currentFrame].setData(&ubo, sizeof(ubo)));
}


void VuRenderer::resetSwapChain()
{
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
