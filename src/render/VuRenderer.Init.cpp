#include <filesystem>

#include "VuPipelineLayout.h"
#include "VuRenderer.h"

void VuRenderer::Init() {
    InitWindow();

    InitVulkanDevice();
    CreateVulkanMemoryAllocator();

    materialDataPool.Init();
    disposeStack.push([&] { materialDataPool.Dispose(); });

    CreateSwapChain();
    CreateCommandPool();

    debugTexture.Alloc(std::filesystem::path("shaders/uvChecker.png"));
    disposeStack.push([&] { debugTexture.Dispose(); });
    VuTexture::allTextures.push_back(debugTexture);

    debugSampler.createImageSampler();
    disposeStack.push([&] { debugSampler.Dispose(); });


    CreateUniformBuffers();

    CreateDescriptorSetLayout();
    CreateDescriptorPool();
    CreateDescriptorSets();

    std::array descSetLayouts{Vu::FrameConstantsDescriptorSetLayout, Vu::globalDescriptorSetLayout};
    Vu::Init::CreatePipelineLayout(descSetLayouts, sizeof(VuPushConstant), Vu::globalPipelineLayout);
    disposeStack.push([&] { vkDestroyPipelineLayout(Vu::Device, Vu::globalPipelineLayout, nullptr); });

    CreateCommandBuffers();

    CreateSyncObjects();

    SetupImGui();

}

void VuRenderer::InitWindow() {
    assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
    disposeStack.push([] { SDL_Quit(); });

    Vu::Window = SDL_CreateWindow("VuRenderer", WIDTH, HEIGHT,SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
    disposeStack.push([] { SDL_DestroyWindow(Vu::Window); });
}

void VuRenderer::InitVulkanDevice() {
    //volk
    VK_CHECK(volkInitialize());
    disposeStack.push([] { volkFinalize(); });

    // Vulkan Instance
    vkb::InstanceBuilder builder;
    auto inst_ret = builder
            .set_app_name("VuMake")
            .request_validation_layers(enableValidationLayers)
            .use_default_debug_messenger()
            .require_api_version(1, 1)
            .build();
    if (!inst_ret) {
        std::cerr << "Failed to create Vulkan instance. Error: " << inst_ret.error().message() << "\n";
    }
    vkb::Instance vkb_inst = inst_ret.value();
    Vu::Instance = vkb_inst.instance;
    disposeStack.push([&] { vkDestroyInstance(Vu::Instance, nullptr); });
    debugMessenger = vkb_inst.debug_messenger;

    auto f = [&] {
        if (enableValidationLayers) {
            Vu::DestroyDebugUtilsMessengerEXT(Vu::Instance, debugMessenger, nullptr);
        }
    };
    disposeStack.push(f);

    volkLoadInstance(vkb_inst);

    //Surface
    CreateSurface();


    //PhysicalDevice
    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::Result<vkb::PhysicalDevice> phys_ret = selector
            .set_surface(surface)
            .set_minimum_version(1, 1)
            .require_dedicated_transfer_queue()
            .add_required_extension(VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME)
            .add_required_extension(VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME)
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

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
        .shaderInputAttachmentArrayDynamicIndexing = VK_TRUE,
        .shaderUniformTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayDynamicIndexing = VK_TRUE,
        .shaderUniformBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageImageArrayNonUniformIndexing = VK_TRUE,
        .shaderInputAttachmentArrayNonUniformIndexing = VK_TRUE,
        .shaderUniformTexelBufferArrayNonUniformIndexing = VK_TRUE,
        .shaderStorageTexelBufferArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingUniformBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingSampledImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageImageUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUniformTexelBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingStorageTexelBufferUpdateAfterBind = VK_TRUE,
        .descriptorBindingUpdateUnusedWhilePending = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };

    // Device
    vkb::DeviceBuilder device_builder{phys_ret.value()};
    auto dev_ret = device_builder
            .add_pNext(&deviceFeatures2)
            .add_pNext(&bufferDeviceAddressFeatures)
            .add_pNext(&descriptorIndexingFeatures)
            .build();

    if (!dev_ret) {
        std::cerr << "Failed to create Vulkan device. Error: " << dev_ret.error().message() << "\n";
    }
    vkb::Device vkb_device = dev_ret.value();
    Vu::Device = vkb_device.device;
    disposeStack.push([] { vkDestroyDevice(Vu::Device, nullptr); });

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
    disposeStack.push([] { vmaDestroyAllocator(Vu::VmaAllocator); });
}

void VuRenderer::CreateSurface() {

    SDL_Vulkan_CreateSurface(Vu::Window, Vu::Instance, nullptr, &surface);
    disposeStack.push([&] { SDL_Vulkan_DestroySurface(Vu::Instance, surface, nullptr); });
}

void VuRenderer::CreateSwapChain() {
    swapChain = Vu::VuSwapChain{};
    swapChain.InitSwapChain(surface);
    disposeStack.push([&] { swapChain.Dispose(); });
}

void VuRenderer::CreateCommandPool() {
    QueueFamilyIndices queueFamilyIndices = Vu::FindQueueFamilies(Vu::PhysicalDevice, surface);

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

    VK_CHECK(vkCreateCommandPool(Vu::Device, &poolInfo, nullptr, &Vu::commandPool));
    disposeStack.push([] { vkDestroyCommandPool(Vu::Device, Vu::commandPool, nullptr); });
}

void VuRenderer::CreateCommandBuffers() {
    commandBuffers.resize(Vu::MAX_FRAMES_IN_FLIGHT);
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = Vu::commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
    VK_CHECK(vkAllocateCommandBuffers(Vu::Device, &allocInfo, commandBuffers.data()));
}

void VuRenderer::CreateSyncObjects() {
    imageAvailableSemaphores.resize(Vu::MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores.resize(Vu::MAX_FRAMES_IN_FLIGHT);
    inFlightFences.resize(Vu::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {
        VK_CHECK(vkCreateSemaphore(Vu::Device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
        VK_CHECK(vkCreateSemaphore(Vu::Device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
        VK_CHECK(vkCreateFence(Vu::Device, &fenceInfo, nullptr, &inFlightFences[i]));

    }

    disposeStack.push([vr = *this] {
        for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {

            vkDestroySemaphore(Vu::Device, vr.imageAvailableSemaphores[i], nullptr);
            vkDestroySemaphore(Vu::Device, vr.renderFinishedSemaphores[i], nullptr);
            vkDestroyFence(Vu::Device, vr.inFlightFences[i], nullptr);
        }
    });


}

void VuRenderer::CreateUniformBuffers() {

    uniformBuffers.resize(Vu::MAX_FRAMES_IN_FLIGHT);
    for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDeviceSize bufferSize = sizeof(FrameUBO);

        uniformBuffers[i] = VuBuffer();
        uniformBuffers[i].Alloc({
            .lenght = 1,
            .strideInBytes = bufferSize,
            .vkUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
            .vmaCreateFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
        });
    }

    disposeStack.push([vr = *this] {
        for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {

            auto v = vr.uniformBuffers;
            v[i].Dispose();
        }
    });
}

void VuRenderer::CreateDescriptorSetLayout() {
    VkDescriptorSetLayoutBinding frameConstBinding{
        .binding = 0,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };
    VkDescriptorSetLayoutCreateInfo frameConstLayout{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = 1,
        .pBindings = &frameConstBinding,
    };
    VK_CHECK(vkCreateDescriptorSetLayout(Vu::Device, &frameConstLayout, nullptr, &Vu::FrameConstantsDescriptorSetLayout));

    disposeStack.push([&] { vkDestroyDescriptorSetLayout(Vu::Device, Vu::FrameConstantsDescriptorSetLayout, nullptr); });

    //Global Set
    VkDescriptorSetLayoutBinding globalStorage{
        .binding = STORAGE_BINDING,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = STORAGE_COUNT,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };

    VkDescriptorSetLayoutBinding globalSampler{
        .binding = SAMPLER_BINDING,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = SAMPLER_COUNT,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };

    VkDescriptorSetLayoutBinding globalImage{
        .binding = IMAGE_BINDING,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = IMAGE_COUNT,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };


    std::array descriptorSetLayoutBindings{globalStorage, globalSampler, globalImage};


    VkDescriptorSetLayoutCreateInfo globalSetLayout{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
        .bindingCount = 3,
        .pBindings = descriptorSetLayoutBindings.data(),
    };

    const VkDescriptorBindingFlagsEXT flag =
            VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
            | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
            | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

    const VkDescriptorBindingFlagsEXT lastFlag =
            VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
            | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
            | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
            | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

    std::array<VkDescriptorSetLayoutCreateFlags, 3> descriptorSetLayoutFlags{flag, flag, flag};

    // In unextended Vulkan, there is no way to pass down flags to a binding, so we're going to do so via a pNext.
    // Each pBinding has a corresponding pBindingFlags.
    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
    binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    binding_flags.bindingCount = descriptorSetLayoutFlags.size();
    binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();

    globalSetLayout.pNext = &binding_flags;


    VK_CHECK(vkCreateDescriptorSetLayout(Vu::Device, &globalSetLayout, nullptr, &Vu::globalDescriptorSetLayout));
    disposeStack.push([&] { vkDestroyDescriptorSetLayout(Vu::Device, Vu::globalDescriptorSetLayout, nullptr); });
}

void VuRenderer::CreateDescriptorPool() {
    std::array<VkDescriptorPoolSize, 4> poolSizes{};
    poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSizes[0].descriptorCount = 1;

    poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    poolSizes[1].descriptorCount = STORAGE_COUNT;

    poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
    poolSizes[2].descriptorCount = SAMPLER_COUNT;

    poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
    poolSizes[3].descriptorCount = IMAGE_COUNT;

    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 2048,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };

    VK_CHECK(vkCreateDescriptorPool(Vu::Device, &poolInfo, nullptr, &Vu::DescriptorPool));
    disposeStack.push([&] { vkDestroyDescriptorPool(Vu::Device, Vu::DescriptorPool, nullptr); });
}

void VuRenderer::CreateDescriptorSets() {

    //frameConstant
    std::vector frameLayouts(Vu::MAX_FRAMES_IN_FLIGHT, Vu::FrameConstantsDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = Vu::DescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(Vu::MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = frameLayouts.data(),
    };

    Vu::frameConstantDescriptorSets.resize(Vu::MAX_FRAMES_IN_FLIGHT);
    VK_CHECK(vkAllocateDescriptorSets(Vu::Device, &allocInfo, Vu::frameConstantDescriptorSets.data()));


    //frame
    for (size_t i = 0; i < Vu::MAX_FRAMES_IN_FLIGHT; i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(FrameUBO);

        VkWriteDescriptorSet descriptorWrite{};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = Vu::frameConstantDescriptorSets[i];
        descriptorWrite.dstBinding = 0;
        descriptorWrite.dstArrayElement = 0;
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;

        vkUpdateDescriptorSets(Vu::Device, 1, &descriptorWrite, 0, nullptr);
    }


    //global sets
    std::vector globalLayouts(Vu::MAX_FRAMES_IN_FLIGHT, Vu::globalDescriptorSetLayout);

    VkDescriptorSetAllocateInfo globalSetsAllocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = Vu::DescriptorPool,
        .descriptorSetCount = static_cast<uint32_t>(Vu::MAX_FRAMES_IN_FLIGHT),
        .pSetLayouts = globalLayouts.data(),
    };

    Vu::globalDescriptorSets.resize(Vu::MAX_FRAMES_IN_FLIGHT);
    VK_CHECK(vkAllocateDescriptorSets(Vu::Device, &globalSetsAllocInfo, Vu::globalDescriptorSets.data()));
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

    VK_CHECK(vkCreateDescriptorPool(Vu::Device, &poolInfo, nullptr, &Vu::UI_DescriptorPool));
    disposeStack.push([&] { vkDestroyDescriptorPool(Vu::Device, Vu::UI_DescriptorPool, nullptr); });

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    disposeStack.push([] { ImGui::DestroyContext(); });
    ImGuiIO& io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    ImGui_ImplSDL3_InitForVulkan(Vu::Window);
    disposeStack.push([] { ImGui_ImplSDL3_Shutdown(); });

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = Vu::Instance;
    init_info.PhysicalDevice = Vu::PhysicalDevice;
    init_info.Device = Vu::Device;
    init_info.QueueFamily = Vu::VuSwapChain::FindQueueFamilies(Vu::PhysicalDevice, surface).graphicsFamily.value();
    init_info.Queue = Vu::graphicsQueue;
    init_info.DescriptorPool = Vu::UI_DescriptorPool;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.UseDynamicRendering = false;
    init_info.RenderPass = swapChain.renderPass.renderPass;

    ImGui_ImplVulkan_Init(&init_info);
    disposeStack.push([] { ImGui_ImplVulkan_Shutdown(); });

    ImGui_ImplVulkan_CreateFontsTexture();
    disposeStack.push([] { ImGui_ImplVulkan_DestroyFontsTexture(); });

}

void VuRenderer::Dispose() {
    vkDeviceWaitIdle(Vu::Device);
    while (!disposeStack.empty()) {
        std::function<void()> disposeFunc = disposeStack.top();
        disposeFunc();
        disposeStack.pop();
    }
}
