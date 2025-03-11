#include "VuRenderer.h"

#include <filesystem>

#include <tracy/Tracy.hpp>
#include "vk_mem_alloc.h"
#include "SDL3/SDL_vulkan.h"

#include "VuResourceManager.h"
#include "VuShader.h"


namespace Vu {

    void VuRenderer::init() {
        ZoneScoped;
        initSDL();
        initWindow();

        initVulkanInstance();
        initSurface();
        initVulkanDevice();
        initSwapchain();

        ctx::vuDevice->initBindless(config::BINDLESS_CONFIG_INFO, config::MAX_FRAMES_IN_FLIGHT);

        VuResourceManager::init(config::BINDLESS_CONFIG_INFO);

        disposeStack.push([&] { VuResourceManager::uninit(); });

        VuMaterialDataPool::init();
        disposeStack.push([&] { VuMaterialDataPool::uninit(); });

        VuShader::initCompilerSystem();
        disposeStack.push([&] { VuShader::uninitCompilerSystem(); });

        initUniformBuffers();
        initCommandBuffers();
        initSyncObjects();
        initImGui();


        //debug resources
        debugTexture.createHandle()->init({std::filesystem::path("assets/textures/error.png"), VK_FORMAT_R8G8B8A8_UNORM});
        VuResourceManager::writeSampledImageToGlobalPool(debugTexture.index, debugTexture.get()->imageView);
        disposeStack.push([&] { debugTexture.destroyHandle(); });

        debugSampler.createHandle()->init({});
        disposeStack.push([&] { debugSampler.destroyHandle(); });
    }


    void VuRenderer::initWindow() {
        ZoneScoped;
        ctx::window = SDL_CreateWindow("VuRenderer", WIDTH, HEIGHT,SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        disposeStack.push([] { SDL_DestroyWindow(ctx::window); });
    }

    void VuRenderer::initSDL() {
        ZoneScoped;
        assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
        disposeStack.push([] { SDL_Quit(); });
    }

    void VuRenderer::initVulkanInstance() {
        ZoneScoped;
        //volk
        {
            ZoneScopedN("Volk");
            VkCheck(volkInitialize());
            disposeStack.push([] { volkFinalize(); });
        }
        // Vulkan Instance
        {
            uint32 count = 0;
            const char* const * instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count);
            std::vector<const char *> extensions(instance_extensions, instance_extensions + count);
            extensions.append_range(config::INSTANCE_EXTENSIONS);
            ctx::vuDevice->initInstance(config::ENABLE_VALIDATION_LAYERS_LAYERS, config::VALIDATION_LAYERS, extensions);
        }
        //volkLoadInstance
        {
            ZoneScopedN("VolkLoadInstance");
            volkLoadInstance(ctx::vuDevice->instance);
        }
    }

    void VuRenderer::initVulkanDevice() {
        ZoneScoped;

        VkPhysicalDevice8BitStorageFeatures m_8BitStorageFeatures {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_8BIT_STORAGE_FEATURES,
            .pNext = nullptr,
            .storageBuffer8BitAccess = VK_TRUE,
            .uniformAndStorageBuffer8BitAccess = VK_TRUE,
            .storagePushConstant8 = VK_TRUE
        };

        VkPhysicalDeviceScalarBlockLayoutFeatures scalarBlockFeatures{};
        scalarBlockFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SCALAR_BLOCK_LAYOUT_FEATURES;
        scalarBlockFeatures.pNext = &m_8BitStorageFeatures;
        scalarBlockFeatures.scalarBlockLayout = VK_TRUE;

        VkPhysicalDeviceBufferDeviceAddressFeaturesKHR bufferDeviceAddressFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES_KHR,
            .pNext = &scalarBlockFeatures,
            .bufferDeviceAddress = VK_TRUE,
        };

        VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexingFeatures = {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES_EXT,
            .pNext = &bufferDeviceAddressFeatures,
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

        VkPhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderInt64 = VK_TRUE;

        VkPhysicalDeviceFeatures2 deviceFeatures2{
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
            .pNext = &descriptorIndexingFeatures,
            .features = deviceFeatures,
        };


        ctx::vuDevice->initDevice({
            config::ENABLE_VALIDATION_LAYERS_LAYERS,
            deviceFeatures2,
            surface,
            config::DEVICE_EXTENSIONS
        });
    }

    void VuRenderer::initSurface() {
        ZoneScoped;
        SDL_Vulkan_CreateSurface(ctx::window, ctx::vuDevice->instance, nullptr, &surface);
        disposeStack.push([&] { SDL_Vulkan_DestroySurface(ctx::vuDevice->instance, surface, nullptr); });
    }

    void VuRenderer::initSwapchain() {
        ZoneScoped;
        swapChain = VuSwapChain{};
        swapChain.init(surface);
        disposeStack.push([&] { swapChain.uninit(); });
    }

    void VuRenderer::initCommandBuffers() {
        ZoneScoped;
        commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = ctx::vuDevice->commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
        VkCheck(vkAllocateCommandBuffers(ctx::vuDevice->device, &allocInfo, commandBuffers.data()));
    }

    void VuRenderer::initSyncObjects() {
        ZoneScoped;
        imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
        inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkCheck(vkCreateSemaphore(ctx::vuDevice->device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            VkCheck(vkCreateSemaphore(ctx::vuDevice->device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
            VkCheck(vkCreateFence(ctx::vuDevice->device, &fenceInfo, nullptr, &inFlightFences[i]));
        }

        disposeStack.push([vr = *this] {
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {

                vkDestroySemaphore(ctx::vuDevice->device, vr.imageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(ctx::vuDevice->device, vr.renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(ctx::vuDevice->device, vr.inFlightFences[i], nullptr);
            }
        });

    }

    void VuRenderer::initUniformBuffers() {
        ZoneScoped;

        uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VkDeviceSize bufferSize = sizeof(GPU_FrameConst);

            uniformBuffers[i] = VuBuffer();
            uniformBuffers[i].init({
                .lenght = 1,
                .strideInBytes = bufferSize,
                .vkUsageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
                .vmaCreateFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
            });
        }
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
            VuResourceManager::writeUBO_ToGlobalPool(0, i, uniformBuffers[i]);
        }

        VuResourceManager::registerUniformBuffer(0, uniformBuffers[0]);

        disposeStack.push([vr = *this] {
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
                auto v = vr.uniformBuffers;
                v[i].uninit();
            }
        });
    }


    void VuRenderer::initImGui() {
        ZoneScoped;
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

        VkCheck(vkCreateDescriptorPool(ctx::vuDevice->device, &poolInfo, nullptr, &ctx::vuDevice->uiDescriptorPool));
        disposeStack.push([&] { vkDestroyDescriptorPool(ctx::vuDevice->device, ctx::vuDevice->uiDescriptorPool, nullptr); });

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        disposeStack.push([] { ImGui::DestroyContext(); });
        ImGuiIO& io = ImGui::GetIO();
        //(void) io;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //io.ConfigFlags |= 1 << 7;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        ImGui_ImplSDL3_InitForVulkan(ctx::window);
        disposeStack.push([] { ImGui_ImplSDL3_Shutdown(); });

        ImGui_ImplVulkan_InitInfo init_info = {};
        init_info.Instance = ctx::vuDevice->instance;
        init_info.PhysicalDevice = ctx::vuDevice->physicalDevice;
        init_info.Device = ctx::vuDevice->device;
        init_info.QueueFamily = VuSwapChain::findQueueFamilies(ctx::vuDevice->physicalDevice, surface).graphicsFamily.value();
        init_info.Queue = ctx::vuDevice->graphicsQueue;
        init_info.DescriptorPool = ctx::vuDevice->uiDescriptorPool;
        init_info.MinImageCount = 2;
        init_info.ImageCount = 2;
        init_info.UseDynamicRendering = false;
        init_info.RenderPass = swapChain.renderPass.renderPass;

        ImGui_ImplVulkan_Init(&init_info);
        disposeStack.push([] { ImGui_ImplVulkan_Shutdown(); });

        ImGui_ImplVulkan_CreateFontsTexture();
        disposeStack.push([] { ImGui_ImplVulkan_DestroyFontsTexture(); });
    }

    void VuRenderer::reloadShaders() {
        //TODO:
    }

    void VuRenderer::bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer) {
        vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            ctx::vuDevice->globalPipelineLayout,
            0,
            1,
            &ctx::vuDevice->globalDescriptorSets[currentFrame],
            0,
            nullptr
        );
    }

    void VuRenderer::uninit() {
        vkDeviceWaitIdle(ctx::vuDevice->device);
        while (!disposeStack.empty()) {
            std::function<void()> disposeFunc = disposeStack.top();
            disposeFunc();
            disposeStack.pop();
        }
    }

}
