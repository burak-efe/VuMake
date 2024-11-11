#include "VuRenderer.h"

#include <filesystem>

#include "VkBootstrap.h"
#include "vk_mem_alloc.h"

#include "VuPipelineLayout.h"

namespace Vu {

    void VuRenderer::Init() {
        InitWindow();

        InitVulkanDevice();
        CreateVulkanMemoryAllocator();

        materialDataPool.init();
        disposeStack.push([&] { materialDataPool.dispose(); });

        CreateSwapChain();
        CreateCommandPool();


        CreateUniformBuffers();

        CreateDescriptorSetLayout();
        CreateDescriptorPool();
        CreateDescriptorSets();

        std::array descSetLayouts{ctx::globalDescriptorSetLayout};
        Vu::Initializers::CreatePipelineLayout(descSetLayouts, sizeof(VuPushConstant), ctx::globalPipelineLayout);
        disposeStack.push([&] { vkDestroyPipelineLayout(ctx::device, ctx::globalPipelineLayout, nullptr); });

        CreateCommandBuffers();

        CreateSyncObjects();

        SetupImGui();

        globalSetManager.init(config::STORAGE_COUNT,config::IMAGE_COUNT,config::SAMPLER_COUNT);
        disposeStack.push([&] { globalSetManager.dispose(); });

        ctx::materialDataPool.init();
        disposeStack.push([&] { ctx::materialDataPool.dispose(); });

        //debug resources
        debugTexture.alloc(std::filesystem::path("assets/textures/error.png"), VK_FORMAT_R8G8B8A8_UNORM);
        disposeStack.push([&] { debugTexture.Dispose(); });

        uint32 dbgTex = globalSetManager.registerTexture(debugTexture);
        assert(dbgTex == 0);
        //VuTexture::allTextures.push_back(debugTexture);


        debugSampler.createImageSampler();
        disposeStack.push([&] { debugSampler.Dispose(); });

        uint32 dbgSmp = globalSetManager.registerSampler(debugSampler);
        assert(dbgSmp == 0);

    }


    void VuRenderer::InitWindow() {
        assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
        disposeStack.push([] { SDL_Quit(); });

        ctx::window = SDL_CreateWindow("VuRenderer", WIDTH, HEIGHT,SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        disposeStack.push([] { SDL_DestroyWindow(ctx::window); });
    }

    void VuRenderer::InitVulkanDevice() {
        //volk
        VkCheck(volkInitialize());
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
        ctx::instance = vkb_inst.instance;
        disposeStack.push([&] { vkDestroyInstance(ctx::instance, nullptr); });
        debugMessenger = vkb_inst.debug_messenger;

        auto f = [&] {
            if (enableValidationLayers) {
                DestroyDebugUtilsMessengerEXT(ctx::instance, debugMessenger, nullptr);
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

        ctx::physicalDevice = phys_ret.value().physical_device;

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderInt64 = VK_TRUE;


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
        ctx::device = vkb_device.device;
        disposeStack.push([] { vkDestroyDevice(ctx::device, nullptr); });

        auto graphics_queue_ret = vkb_device.get_queue(vkb::QueueType::graphics);
        if (!graphics_queue_ret) {
            std::cerr << "Failed to get graphics queue. Error: " << graphics_queue_ret.error().message() << "\n";
        }
        ctx::graphicsQueue = graphics_queue_ret.value();
        auto presentQueueRet = vkb_device.get_queue(vkb::QueueType::present);
        if (!presentQueueRet) {
            std::cerr << "Failed to get present queue. Error: " << presentQueueRet.error().message() << "\n";
        }
        ctx::presentQueue = presentQueueRet.value();
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
            .physicalDevice = ctx::physicalDevice,
            .device = ctx::device,
            .pVulkanFunctions = &vma_vulkan_func,
            .instance = ctx::instance,
        };

        VkCheck(vmaCreateAllocator(&createInfo, &ctx::vma));
        disposeStack.push([] { vmaDestroyAllocator(ctx::vma); });
    }

    void VuRenderer::CreateSurface() {

        SDL_Vulkan_CreateSurface(ctx::window, ctx::instance, nullptr, &surface);
        disposeStack.push([&] { SDL_Vulkan_DestroySurface(ctx::instance, surface, nullptr); });
    }

    void VuRenderer::CreateSwapChain() {
        swapChain = VuSwapChain{};
        swapChain.InitSwapChain(surface);
        disposeStack.push([&] { swapChain.Dispose(); });
    }

    void VuRenderer::CreateCommandPool() {
        QueueFamilyIndices queueFamilyIndices = QueueFamilyIndices::FindQueueFamilies(ctx::physicalDevice, surface);

        VkCommandPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();

        VkCheck(vkCreateCommandPool(ctx::device, &poolInfo, nullptr, &ctx::commandPool));
        disposeStack.push([] { vkDestroyCommandPool(ctx::device, ctx::commandPool, nullptr); });
    }

    void VuRenderer::CreateCommandBuffers() {
        commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = ctx::commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
        VkCheck(vkAllocateCommandBuffers(ctx::device, &allocInfo, commandBuffers.data()));
    }

    void VuRenderer::CreateSyncObjects() {
        imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkCheck(vkCreateSemaphore(ctx::device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            VkCheck(vkCreateSemaphore(ctx::device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
            VkCheck(vkCreateFence(ctx::device, &fenceInfo, nullptr, &inFlightFences[i]));

        }

        disposeStack.push([vr = *this] {
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {

                vkDestroySemaphore(ctx::device, vr.imageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(ctx::device, vr.renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(ctx::device, vr.inFlightFences[i], nullptr);
            }
        });


    }

    void VuRenderer::CreateUniformBuffers() {

        uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDeviceSize bufferSize = sizeof(VuFrameConst);

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
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {

                auto v = vr.uniformBuffers;
                v[i].Dispose();
            }
        });
    }

    void VuRenderer::CreateDescriptorSetLayout() {
        //Global Set
        VkDescriptorSetLayoutBinding globalUniform{
            .binding = config::UBO_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = config::UNIFORM_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };

        VkDescriptorSetLayoutBinding globalStorage{
            .binding = config::STORAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = config::STORAGE_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };

        VkDescriptorSetLayoutBinding globalSampler{
            .binding = config::SAMPLER_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
            .descriptorCount = config::SAMPLER_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };

        VkDescriptorSetLayoutBinding globalImage{
            .binding = config::IMAGE_BINDING,
            .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
            .descriptorCount = config::IMAGE_COUNT,
            .stageFlags = VK_SHADER_STAGE_ALL,
        };


        std::array descriptorSetLayoutBindings{globalUniform, globalStorage, globalSampler, globalImage};


        VkDescriptorSetLayoutCreateInfo globalSetLayout{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT_EXT,
            .bindingCount = descriptorSetLayoutBindings.size(),
            .pBindings = descriptorSetLayoutBindings.data(),
        };

        const VkDescriptorBindingFlagsEXT flag =
                VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
                | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
                | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

        // const VkDescriptorBindingFlagsEXT lastFlag =
        //         VK_DESCRIPTOR_BINDING_VARIABLE_DESCRIPTOR_COUNT_BIT_EXT
        //         | VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT_EXT
        //         | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT_EXT
        //         | VK_DESCRIPTOR_BINDING_UPDATE_UNUSED_WHILE_PENDING_BIT_EXT;

        std::array descriptorSetLayoutFlags{flag, flag, flag, flag};

        VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
        binding_flags.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
        binding_flags.bindingCount = descriptorSetLayoutFlags.size();
        binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();

        globalSetLayout.pNext = &binding_flags;

        VkCheck(vkCreateDescriptorSetLayout(ctx::device, &globalSetLayout, nullptr, &ctx::globalDescriptorSetLayout));
        disposeStack.push([&] { vkDestroyDescriptorSetLayout(ctx::device, ctx::globalDescriptorSetLayout, nullptr); });
    }

    void VuRenderer::CreateDescriptorPool() {
        std::array<VkDescriptorPoolSize, 4> poolSizes{};
        poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        poolSizes[0].descriptorCount = config::UNIFORM_COUNT;

        poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        poolSizes[1].descriptorCount = config::STORAGE_COUNT;

        poolSizes[2].type = VK_DESCRIPTOR_TYPE_SAMPLER;
        poolSizes[2].descriptorCount = config::SAMPLER_COUNT;

        poolSizes[3].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        poolSizes[3].descriptorCount = config::IMAGE_COUNT;

        VkDescriptorPoolCreateInfo poolInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
            .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
            .maxSets = 2,
            .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
            .pPoolSizes = poolSizes.data(),
        };

        VkCheck(vkCreateDescriptorPool(ctx::device, &poolInfo, nullptr, &ctx::descriptorPool));
        disposeStack.push([&] { vkDestroyDescriptorPool(ctx::device, ctx::descriptorPool, nullptr); });
    }

    void VuRenderer::CreateDescriptorSets() {

        //global sets
        std::vector globalLayouts(config::MAX_FRAMES_IN_FLIGHT, ctx::globalDescriptorSetLayout);

        VkDescriptorSetAllocateInfo globalSetsAllocInfo{
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = ctx::descriptorPool,
            .descriptorSetCount = static_cast<uint32_t>(config::MAX_FRAMES_IN_FLIGHT),
            .pSetLayouts = globalLayouts.data(),
        };

        ctx::globalDescriptorSets.resize(config::MAX_FRAMES_IN_FLIGHT);
        VkCheck(vkAllocateDescriptorSets(ctx::device, &globalSetsAllocInfo, ctx::globalDescriptorSets.data()));


        //uniforms
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDescriptorBufferInfo bufferInfo{};
            bufferInfo.buffer = uniformBuffers[i].buffer;
            bufferInfo.offset = 0;
            bufferInfo.range = sizeof(VuFrameConst);

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = ctx::globalDescriptorSets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = &bufferInfo;

            vkUpdateDescriptorSets(ctx::device, 1, &descriptorWrite, 0, nullptr);
        }
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

        VkCheck(vkCreateDescriptorPool(ctx::device, &poolInfo, nullptr, &ctx::uiDescriptorPool));
        disposeStack.push([&] { vkDestroyDescriptorPool(ctx::device, ctx::uiDescriptorPool, nullptr); });

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

        ImGui_ImplSDL3_InitForVulkan(ctx::window);
        disposeStack.push([] { ImGui_ImplSDL3_Shutdown(); });

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = ctx::instance;
        init_info.PhysicalDevice = ctx::physicalDevice;
        init_info.Device = ctx::device;
        init_info.QueueFamily = VuSwapChain::FindQueueFamilies(ctx::physicalDevice, surface).graphicsFamily.value();
        init_info.Queue = ctx::graphicsQueue;
        init_info.DescriptorPool = ctx::uiDescriptorPool;
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
        vkDeviceWaitIdle(ctx::device);
        while (!disposeStack.empty()) {
            std::function<void()> disposeFunc = disposeStack.top();
            disposeFunc();
            disposeStack.pop();
        }
    }


}
