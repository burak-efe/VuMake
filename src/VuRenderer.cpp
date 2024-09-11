#include "VuRenderer.h"

#include <set>

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "GLFW/glfw3.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui.h"

#include "VuDeviceUtils.h"
#include "VuContext.h"
#include "VuShader.h"
#include "VuInit.h"
#include "VuUtils.h"
#include "VuTypes.h"


void VuRenderer::Init() {
    InitWindow();
    InitVulkan();
}

void VuRenderer::InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
}

void VuRenderer::InitVulkan() {
    VuContext::Instance = VuInit::CreateVulkanInstance(enableValidationLayers, k_ValidationLayers);
    SetupDebugMessenger();
    CreateSurface();
    VuDeviceUtils::PickPhysicalDevice(surface);
    VuDeviceUtils::CreateLogicalDevice(surface, graphicsQueue, presentQueue);
    CreateVulkanMemoryAllocator();
    CreateSwapChain();
    DepthStencil = VuDepthStencil{};
    DepthStencil.Create(SwapChain);
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateCommandPool();
    CreateUniformBuffers();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
    SetupImGui();
}

bool VuRenderer::ShouldWindowClose() {
    return glfwWindowShouldClose(window);
}

void VuRenderer::Tick() {
    UpdateFpsCounter();
    glfwPollEvents();
    DrawFrame();
}

void VuRenderer::CreateVulkanMemoryAllocator() {
    VmaAllocatorCreateInfo createInfo{};
    createInfo.device = VuContext::Device;
    createInfo.physicalDevice = VuContext::PhysicalDevice;
    createInfo.instance = VuContext::Instance;
    VK_CHECK(vmaCreateAllocator(&createInfo, &VuContext::VmaAllocator));
}

void VuRenderer::CreateSurface() {
    VK_CHECK(glfwCreateWindowSurface(VuContext::Instance, window, nullptr, &surface));
}

void VuRenderer::WaitIdle() {
    vkDeviceWaitIdle(VuContext::Device);
}

void VuRenderer::SetupDebugMessenger() {
    if constexpr (!enableValidationLayers) return;
    debugMessenger = VuInit::CreateDebugMessenger(VuContext::Instance);
}

void VuRenderer::RecreateSwapChain() {
    int width = 0;
    int height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(VuContext::Device);
    SwapChain.CleanupSwapchain();
    CreateSwapChain();

    ImGui_ImplVulkan_SetMinImageCount(SwapChain.imageCount);
    // ImGui_ImplVulkanH_CreateWindow(g_Instance, g_PhysicalDevice, g_Device, &g_MainWindowData,
    //         g_QueueFamily, g_Allocator, g_SwapChainResizeWidth, g_SwapChainResizeHeight, g_MinImageCount);
    // g_MainWindowData.FrameIndex = 0;
}

void VuRenderer::CreateSwapChain() {
    SwapChain = Vu::VuSwapChain{};
    SwapChain.CreateSwapChain(window, surface);
}

void VuRenderer::CreateGraphicsPipeline() {
    DebugPipeline = VuGraphicsPipeline{};
    DebugPipeline.CreateGraphicsPipeline(descriptorSetLayout, DepthStencil);
}

void VuRenderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = VuDeviceUtils::findQueueFamilies(VuContext::PhysicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VK_CHECK(vkCreateCommandPool(VuContext::Device, &poolInfo, nullptr, &commandPool))
}

void VuRenderer::CreateCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());

    VK_CHECK(vkAllocateCommandBuffers(VuContext::Device, &allocInfo, commandBuffers.data()))
}

void VuRenderer::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32 imageIndex) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    VK_CHECK(vkBeginCommandBuffer(commandBuffer, &beginInfo));

    // With dynamic rendering there are no subpass dependencies, so we need to take care of proper layout transitions by using barriers
    // This set of barriers prepares the color and depth images for output
    VuInit::InsertImageMemoryBarrier(
        commandBuffer,
        SwapChain.swapChainImages[imageIndex],
        0,
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1});

    VuInit::InsertImageMemoryBarrier(
        commandBuffer,
        DepthStencil.Image,
        0,
        VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED,
        VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
        VkImageSubresourceRange{VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT, 0, 1, 0, 1});


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
    viewport.y = 0.0f;
    viewport.width = (float) SwapChain.swapChainExtent.width;
    viewport.height = (float) SwapChain.swapChainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = SwapChain.swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, DebugPipeline.Pipeline);

    for (auto mesh: meshes) {
        VkBuffer vertexBuffers[] = {mesh->vertexBuffer.VulkanBuffer, mesh->normalBuffer.VulkanBuffer};
        VkDeviceSize offsets[] = {0, 0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 2, vertexBuffers, offsets);

        vkCmdBindIndexBuffer(commandBuffer, mesh->indexBuffer.VulkanBuffer, 0, VK_INDEX_TYPE_UINT32);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, DebugPipeline.PipelineLayout, 0, 1,
                                &descriptorSets[currentFrame], 0, nullptr);

        vkCmdDrawIndexed(commandBuffer, mesh->indexBuffer.Lenght, 1, 0, 0, 0);
    }

    /////
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    ////

    vkCmdEndRendering(commandBuffer);

    // This set of barriers prepares the color image for presentation, we don't need to care for the depth image
    VuInit::InsertImageMemoryBarrier(
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

void VuRenderer::BeginRecord() {
}

void VuRenderer::DrawFrame() {
    vkWaitForFences(VuContext::Device, 1, &inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

    uint32 imageIndex;
    VkResult result = vkAcquireNextImageKHR(VuContext::Device, SwapChain.swapChain, UINT64_MAX,
                                            imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        RecreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swapcahin image!");
    }
    UpdateUniformBuffer(currentFrame);

    vkResetFences(VuContext::Device, 1, &inFlightFences[currentFrame]);

    vkResetCommandBuffer(commandBuffers[currentFrame], /*VkCommandBufferResetFlagBits*/ 0);
    //Record
    RecordCommandBuffer(commandBuffers[currentFrame], imageIndex);

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

    VK_CHECK(vkQueueSubmit(graphicsQueue, 1, &submitInfo, inFlightFences[currentFrame]));

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {SwapChain.swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = &imageIndex;

    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        RecreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void VuRenderer::CreateSyncObjects() {
    imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateSemaphore(VuContext::Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(VuContext::Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(VuContext::Device, &fenceInfo, nullptr, &inFlightFences[i]));
    }
}

void VuRenderer::CreateUniformBuffers() {
    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
    uniformBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDeviceSize bufferSize = sizeof(UniformBufferObject);
        VuBuffer::createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformBuffers[i],
            uniformBuffersMemory[i]);

        vkMapMemory(VuContext::Device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
}

void VuRenderer::UpdateUniformBuffer(uint32 currentImage) {
    static auto startTime = std::chrono::high_resolution_clock::now();

    auto currentTime = std::chrono::high_resolution_clock::now();
    float time = std::chrono::duration<float, std::chrono::seconds::period>
            (currentTime - startTime).count();

    UniformBufferObject ubo{};

    ubo.model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f),
                            glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f),
                           glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));

    ubo.proj = glm::perspective(
        glm::radians(90.0f),
        static_cast<float>(SwapChain.swapChainExtent.width) / static_cast<float>(SwapChain.swapChainExtent.height),
        0.1f,
        10.0f);
    ubo.proj[1][1] *= -1;

    memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
}

void VuRenderer::UpdateFpsCounter() {
    double lastTime = glfwGetTime() * 1000 * 1000;
    auto elapsed_seconds = lastTime - PrevTime;
    auto s = std::to_string(elapsed_seconds);
    glfwSetWindowTitle(window, s.c_str());
    PrevTime = lastTime;
}

void VuRenderer::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    VK_CHECK(vkCreateDescriptorSetLayout(VuContext::Device, &layoutInfo, nullptr, &descriptorSetLayout));
}

void VuRenderer::CreateDescriptorPool() {
    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VK_CHECK(vkCreateDescriptorPool(VuContext::Device, &poolInfo, nullptr, &descriptorPool));
}

void VuRenderer::CreateDescriptorSets() {
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    allocInfo.pSetLayouts = layouts.data();

    descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VK_CHECK(vkAllocateDescriptorSets(VuContext::Device, &allocInfo, descriptorSets.data()));

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(UniformBufferObject);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = descriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(VuContext::Device, 1, &descriptorWrite, 0, nullptr);
    }
}

void VuRenderer::Cleanup() {
    vkDeviceWaitIdle(VuContext::Device);

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    SwapChain.CleanupSwapchain();
    vkDestroyDescriptorSetLayout(VuContext::Device, descriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(VuContext::Device, descriptorPool, nullptr);
    vkDestroyDescriptorPool(VuContext::Device, uiDescriptorPool, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(VuContext::Device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(VuContext::Device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(VuContext::Device, inFlightFences[i], nullptr);
    }

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroyBuffer(VuContext::Device, uniformBuffers[i], nullptr);
        vkFreeMemory(VuContext::Device, uniformBuffersMemory[i], nullptr);
    }

    DepthStencil.Dispose(); //renderPass.Dispose();
    DebugPipeline.Dispose();

    vkDestroyCommandPool(VuContext::Device, commandPool, nullptr);
    vmaDestroyAllocator(VuContext::VmaAllocator);
    vkDestroyDevice(VuContext::Device, nullptr);
    if (enableValidationLayers) {
        VuInit::DestroyDebugUtilsMessengerEXT(VuContext::Instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(VuContext::Instance, surface, nullptr);
    vkDestroyInstance(VuContext::Instance, nullptr);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void VuRenderer::SetupImGui() {
    // VkDescriptorPoolSize poolSize{};
    // poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    // poolSize.descriptorCount = static_cast<uint32_t>(1000);

    VkDescriptorPoolSize pool_sizes[] =
    {
        {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    };

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = pool_sizes;
    poolInfo.maxSets = 1000;
    poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    VK_CHECK(vkCreateDescriptorPool(VuContext::Device, &poolInfo, nullptr, &uiDescriptorPool));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(window, true);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = VuContext::Instance;
    init_info.PhysicalDevice = VuContext::PhysicalDevice;
    init_info.Device = VuContext::Device;
    init_info.QueueFamily = Vu::VuSwapChain::FindQueueFamilies(VuContext::PhysicalDevice, surface).graphicsFamily.
            value();
    init_info.Queue = graphicsQueue;
    init_info.DescriptorPool = uiDescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.UseDynamicRendering = true;
    //init_info.CheckVkResultFn = check_vk_result;

    VkFormat colorRenderingFormats[1] = {
        VK_FORMAT_B8G8R8A8_SRGB,
    };

    VkPipelineRenderingCreateInfo rfInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .pNext = nullptr,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = colorRenderingFormats,
        .depthAttachmentFormat = DepthStencil.DepthFormat,
        .stencilAttachmentFormat = DepthStencil.DepthFormat
    };

    init_info.PipelineRenderingCreateInfo = rfInfo;
    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}
