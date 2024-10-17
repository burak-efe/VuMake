#include <filesystem>

#include "VuRenderer.h"


void VuRenderer::Init() {
    InitWindow();
    InitVulkan();
}

void VuRenderer::InitWindow() {
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
    Vu::Window = SDL_CreateWindow("VuRenderer", WIDTH, HEIGHT,SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
}

void VuRenderer::CreateVulkanMemoryAllocator() {

    VmaVulkanFunctions vma_vulkan_func{
        .vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties,
        .vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties,
        .vkAllocateMemory = vkAllocateMemory,
        .vkFreeMemory = vkFreeMemory,
        .vkMapMemory = vkMapMemory,
        .vkUnmapMemory = vkUnmapMemory,
        .vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges,
        .vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges,
        .vkBindBufferMemory = vkBindBufferMemory,
        .vkBindImageMemory = vkBindImageMemory,
        .vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements,
        .vkGetImageMemoryRequirements = vkGetImageMemoryRequirements,
        .vkCreateBuffer = vkCreateBuffer,
        .vkDestroyBuffer = vkDestroyBuffer,
        .vkCreateImage = vkCreateImage,
        .vkDestroyImage = vkDestroyImage,
        .vkCmdCopyBuffer = vkCmdCopyBuffer,
    };

    VmaAllocatorCreateInfo createInfo{
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = Vu::PhysicalDevice,
        .device = Vu::Device,
        .pVulkanFunctions = &vma_vulkan_func,
        .instance = Vu::Instance,
    };

    VK_CHECK(vmaCreateAllocator(&createInfo, &Vu::VmaAllocator));
}

void VuRenderer::CreateSurface() {

    SDL_Vulkan_CreateSurface(Vu::Window, Vu::Instance, nullptr, &Surface);
}

void VuRenderer::InitVulkan() {
    //volk
    VK_CHECK(volkInitialize());

    // Vulkan Instance
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("VuMake")
            .request_validation_layers(enableValidationLayers)
            .use_default_debug_messenger()
            .require_api_version(1, 1)
            .build();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
    }
    vkb::Instance vkb_inst = inst_ret.value();
    Vu::Instance = vkb_inst.instance;
    debugMessenger = vkb_inst.debug_messenger;

    volkLoadInstance(vkb_inst);

    //Surface
    CreateSurface();

    //PhysicalDevice
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    auto phys_ret = selector.set_surface(Surface)
            .set_minimum_version(1, 1)
            .require_dedicated_transfer_queue()
            .add_required_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
            .select();
    if (!phys_ret) {
        std::cerr << "Failed to select Vulkan Physical Device. Error: " << phys_ret.error().message() << "\n";
    }

    Vu::PhysicalDevice = phys_ret.value().physical_device;

    VkPhysicalDeviceFeatures deviceFeatures{};
    deviceFeatures.samplerAnisotropy = VK_TRUE;


    VkPhysicalDeviceFeatures2 deviceFeatures2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .features = deviceFeatures,
    };

    VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
        .bufferDeviceAddress = VK_TRUE,
    };

    // Device
    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto dev_ret = device_builder
            .add_pNext(&deviceFeatures2)
            .add_pNext(&bufferDeviceAddressFeatures)
            .build();

    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
    }
    vkb::Device vkb_device = dev_ret.value();
    Vu::Device = vkb_device.device;
    auto graphics_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
    if (!graphics_queue_ret) {
        std::cerr << "Failed to get graphics queue. Error: " << graphics_queue_ret.error().message() << "\n";
    }
    Vu::graphicsQueue = graphics_queue_ret.value();
    auto presentQueueRet = vkb_device.get_queue(vkb::QueueType::present);
    if (!presentQueueRet) {
        std::cerr << "Failed to get present queue. Error: " << presentQueueRet.error().message() << "\n";
    }
    Vu::presentQueue = presentQueueRet.value();

    CreateVulkanMemoryAllocator();
    CreateSwapChain();
    CreateCommandPool();
    DebugTexture.Alloc(std::filesystem::path("shaders/uvChecker.png"));
    CreateUniformBuffers();
    CreateDescriptorSetLayout();
    CreateGraphicsPipeline();
    CreateDescriptorPool();
    CreateDescriptorSets();
    CreateCommandBuffers();
    CreateSyncObjects();
    SetupImGui();
}

void VuRenderer::CreateSwapChain() {
    SwapChain = Vu::VuSwapChain{};
    SwapChain.InitSwapChain(Surface);
}

void VuRenderer::CreateGraphicsPipeline() {

    std::vector descriptorLayouts{FrameConstantsDescriptorSetLayout, ImageDescriptorSetLayout};
    DebugPipeline.CreateGraphicsPipeline(descriptorLayouts, SwapChain.RenderPass.RenderPass);
}

void VuRenderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = Vu::FindQueueFamilies(Vu::PhysicalDevice, Surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VK_CHECK(vkCreateCommandPool(Vu::Device, &poolInfo, nullptr, &Vu::commandPool));
}

void VuRenderer::CreateCommandBuffers() {
    commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = Vu::commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
    VK_CHECK(vkAllocateCommandBuffers(Vu::Device, &allocInfo, commandBuffers.data()));
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
        VK_CHECK(vkCreateSemaphore(Vu::Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(Vu::Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(Vu::Device, &fenceInfo, nullptr, &inFlightFences[i]));
    }
}

void VuRenderer::CreateUniformBuffers() {

    uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDeviceSize bufferSize = sizeof(FrameUBO);

        uniformBuffers[i] = VuBuffer();
        uniformBuffers[i].Alloc(1, bufferSize,
                                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                VMA_MEMORY_USAGE_AUTO,
                                VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT);
    }
}

void VuRenderer::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding uboLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
        .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutCreateInfo frameLayout{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &uboLayoutBinding,
    };
    VK_CHECK(vkCreateDescriptorSetLayout(Vu::Device, &frameLayout, nullptr, &FrameConstantsDescriptorSetLayout));


    VkDescriptorSetLayoutBinding samplerLayoutBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
        .pImmutableSamplers = nullptr,
    };
    VkDescriptorSetLayoutCreateInfo imageLayout{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &samplerLayoutBinding,
    };
    VK_CHECK(vkCreateDescriptorSetLayout(Vu::Device, &imageLayout, nullptr, &ImageDescriptorSetLayout));
}

void VuRenderer::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 2> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT) * 4,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    VK_CHECK(vkCreateDescriptorPool(Vu::Device, &poolInfo, nullptr, &DescriptorPool));
}

void VuRenderer::CreateDescriptorSets() {

    //frame
    std::vector frameLayouts(MAX_FRAMES_IN_FLIGHT, FrameConstantsDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = DescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = frameLayouts.data(),
    };

    FrameConstantDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VK_CHECK(vkAllocateDescriptorSets(Vu::Device, &allocInfo, FrameConstantDescriptorSets.data()));


    //image
    std::vector imageLayouts(MAX_FRAMES_IN_FLIGHT, ImageDescriptorSetLayout);
    VkDescriptorSetAllocateInfo imageSetsAllocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = DescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = imageLayouts.data(),
    };

    ImageDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
    VK_CHECK(vkAllocateDescriptorSets(Vu::Device, &imageSetsAllocInfo, ImageDescriptorSets.data()));


    //frame
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].Buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(FrameUBO);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = FrameConstantDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(Vu::Device, 1, &descriptorWrite, 0, nullptr);
    }

    //image
    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.sampler = DebugTexture.TextureImageSampler;
        imageInfo.imageView = DebugTexture.TextureImageView;

        VkWriteDescriptorSet imageDescWrite{};
        imageDescWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        imageDescWrite.dstSet = ImageDescriptorSets[i];
        imageDescWrite.dstBinding = 0;
        imageDescWrite.dstArrayElement = 0;
        imageDescWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        imageDescWrite.descriptorCount = 1;
        imageDescWrite.pImageInfo = &imageInfo;

        vkUpdateDescriptorSets(Vu::Device, 1, &imageDescWrite, 0, nullptr);
    }
}

void VuRenderer::Dispose() {
    vkDeviceWaitIdle(Vu::Device);
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SwapChain.Dispose();
    DebugTexture.Dispose();
    vkDestroyDescriptorSetLayout(Vu::Device, FrameConstantsDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(Vu::Device, ImageDescriptorSetLayout, nullptr);
    vkDestroyDescriptorPool(Vu::Device, DescriptorPool, nullptr);
    vkDestroyDescriptorPool(Vu::Device, UI_DescriptorPool, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        vkDestroySemaphore(Vu::Device, renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(Vu::Device, imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(Vu::Device, inFlightFences[i], nullptr);
        uniformBuffers[i].Dispose();
    }

    DebugPipeline.Dispose();

    vkDestroyCommandPool(Vu::Device, Vu::commandPool, nullptr);
    vmaDestroyAllocator(Vu::VmaAllocator);
    vkDestroyDevice(Vu::Device, nullptr);
    if (enableValidationLayers) {
        Vu::DestroyDebugUtilsMessengerEXT(Vu::Instance, debugMessenger, nullptr);
    }
    vkDestroySurfaceKHR(Vu::Instance, Surface, nullptr);
    vkDestroyInstance(Vu::Instance, nullptr);

    SDL_DestroyWindow(Vu::Window);
    SDL_Quit();
}

void VuRenderer::SetupImGui() {

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

    VK_CHECK(vkCreateDescriptorPool(Vu::Device, &poolInfo, nullptr, &UI_DescriptorPool));

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForVulkan(Vu::Window);
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Vu::Instance;
    init_info.PhysicalDevice = Vu::PhysicalDevice;
    init_info.Device = Vu::Device;
    init_info.QueueFamily = Vu::VuSwapChain::FindQueueFamilies(Vu::PhysicalDevice, Surface).graphicsFamily.value();
    init_info.Queue = Vu::graphicsQueue;
    init_info.DescriptorPool = UI_DescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.UseDynamicRendering = false;
    init_info.RenderPass = SwapChain.RenderPass.RenderPass;

    ImGui_ImplVulkan_Init(&init_info);
    ImGui_ImplVulkan_CreateFontsTexture();
}
