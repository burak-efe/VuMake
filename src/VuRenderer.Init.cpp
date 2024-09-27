#include "VkBootstrap.h"
#include "VuRenderer.h"

#include "GLFW/glfw3.h"

void VuRenderer::Init() {
    InitWindow();
    InitVulkan();
}

void VuRenderer::InitWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
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

void VuRenderer::InitVulkan() {
    // Vulkan Instance
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("VuMake")
            .request_validation_layers(enableValidationLayers)
            .use_default_debug_messenger()
            .require_api_version(1, 3)
            .build();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
    }
    vkb::Instance vkb_inst = inst_ret.value();
    VuContext::Instance = vkb_inst.instance;
    debugMessenger = vkb_inst.debug_messenger;

    //Surface
    CreateSurface();

    //PhysicalDevice
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto phys_ret = selector.set_surface(surface)
            .set_minimum_version(1, 3) // require a vulkan 1.3 capable device
            .require_dedicated_transfer_queue()
            .select();
    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
    }

    VuContext::PhysicalDevice = phys_ret.value().physical_device;
    // Enable Dynamic Rendering
    VkPhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES_KHR,
        .dynamicRendering = VK_TRUE,
    };
    //Enbale Sync2
    VkPhysicalDeviceSynchronization2Features sync2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES,
        .synchronization2 = VK_TRUE,
    };

    // //Enable Descriptor Indexing
    // VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures{
    //     .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
    //     .
    // };

    // Device
    vkb::DeviceBuilder device_builder{phys_ret.value()};
    // automatically propagate needed data from instance & physical device
    auto dev_ret = device_builder.add_pNext(&dynamic_rendering_feature).add_pNext(&sync2).build();
    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
    }
    vkb::Device vkb_device = dev_ret.value();

    // Get the VkDevice handle used in the rest of a vulkan application
    VuContext::Device = vkb_device.device;

    // Get the graphics queue with a helper function
    auto graphics_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret) {
        std::cerr << "Failed to get graphics queue. Error: " << graphics_queue_ret.error().message() << "\n";
    }
    graphicsQueue = graphics_queue_ret.value();

    // Get the present queue with a helper function
    auto presentQueueRet = vkb_device.get_queue(vkb::QueueType::present);
    if (!presentQueueRet) {
        std::cerr << "Failed to get present queue. Error: " << presentQueueRet.error().message() << "\n";
    }
    presentQueue = presentQueueRet.value();

    //VuContext::Instance = VuInit::CreateVulkanInstance(enableValidationLayers, k_ValidationLayers);
    //SetupDebugMessenger();
    //VuDeviceUtils::PickPhysicalDevice(surface);
    //VuDeviceUtils::CreateLogicalDevice(surface, graphicsQueue, presentQueue);
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

void VuRenderer::CreateSwapChain() {
    SwapChain = Vu::VuSwapChain{};
    SwapChain.CreateSwapChain(window, surface);
}

void VuRenderer::CreateGraphicsPipeline() {
    DebugPipeline = VuGraphicsPipeline{};
    DebugPipeline.CreateGraphicsPipeline(descriptorSetLayout, DepthStencil);
}

void VuRenderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = Vu::FindQueueFamilies(VuContext::PhysicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VK_CHECK(vkCreateCommandPool(VuContext::Device, &poolInfo, nullptr, &commandPool));
}

void VuRenderer::CreateCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
    VK_CHECK(vkAllocateCommandBuffers(VuContext::Device, &allocInfo, commandBuffers.data()));
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
        VkDeviceSize bufferSize = sizeof(FrameUBO);
        VuBuffer::createBuffer(
            bufferSize,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            uniformBuffers[i],
            uniformBuffersMemory[i]);

        vkMapMemory(VuContext::Device, uniformBuffersMemory[i], 0, bufferSize, 0, &uniformBuffersMapped[i]);
    }
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
        bufferInfo.range = sizeof(FrameUBO);

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

void VuRenderer::Dispose() {
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
        Vu::DestroyDebugUtilsMessengerEXT(VuContext::Instance, debugMessenger, nullptr);
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
    ImGuiIO& io = ImGui::GetIO();
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

void VuRenderer::MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    VuContext::MouseX = xpos;
    VuContext::MouseY = ypos;
}
