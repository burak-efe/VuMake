#include "VuRenderer.h"

#include <filesystem>
#include "SDL3/SDL_vulkan.h"
#include "11_Config/VuCtx.h"
#include "VuDevice.h"
#include "VuResourceManager.h"
#include "VuShader.h"


namespace Vu
{
    void VuRenderer::init()
    {
        ZoneScoped;

        //init window
        {
            assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
            disposeStack.push([] { SDL_Quit(); });

            ctx::window = SDL_CreateWindow("VuRenderer", WIDTH, HEIGHT,SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
            disposeStack.push([] { SDL_DestroyWindow(ctx::window); });
        }
        //init instance
        {
            VkCheck(volkInitialize());
            disposeStack.push([] { volkFinalize(); });

            uint32                   count               = 0;
            const char* const *      instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count);
            std::vector<const char*> extensions(instance_extensions, instance_extensions + count);

            for (auto extension : config::INSTANCE_EXTENSIONS)
            {
                extensions.push_back(extension);
            }
            vuDevice.initInstance(config::ENABLE_VALIDATION_LAYERS_LAYERS, config::VALIDATION_LAYERS, extensions);

            volkLoadInstance(vuDevice.instance);
        }
        //init surface
        {
            SDL_Vulkan_CreateSurface(ctx::window, vuDevice.instance, nullptr, &surface);
            disposeStack.push([&] { SDL_Vulkan_DestroySurface(vuDevice.instance, surface, nullptr); });
        }
        //init device
        {
            VkPhysicalDeviceSynchronization2FeaturesKHR sync2Features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SYNCHRONIZATION_2_FEATURES_KHR,
                .pNext = nullptr,
                .synchronization2 = VK_TRUE
            };

            VkPhysicalDeviceVulkan11Features vk11_features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
                .pNext = &sync2Features,
                .storageBuffer16BitAccess = VK_FALSE,
                .uniformAndStorageBuffer16BitAccess = VK_FALSE,
                .storagePushConstant16 = VK_FALSE,
                .storageInputOutput16 = VK_FALSE,
                .multiview = VK_FALSE,
                .multiviewGeometryShader = VK_FALSE,
                .multiviewTessellationShader = VK_FALSE,
                .variablePointersStorageBuffer = VK_FALSE,
                .variablePointers = VK_FALSE,
                .protectedMemory = VK_FALSE,
                .samplerYcbcrConversion = VK_FALSE,
                .shaderDrawParameters = VK_FALSE
            };

            VkPhysicalDeviceVulkan12Features vk12_features{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
                .pNext = &vk11_features,
                .samplerMirrorClampToEdge = VK_FALSE,
                .drawIndirectCount = VK_FALSE,
                .storageBuffer8BitAccess = VK_FALSE,
                .uniformAndStorageBuffer8BitAccess = VK_FALSE,
                .storagePushConstant8 = VK_FALSE,
                .shaderBufferInt64Atomics = VK_FALSE,
                .shaderSharedInt64Atomics = VK_FALSE,
                .shaderFloat16 = VK_FALSE,
                .shaderInt8 = VK_FALSE,
                .descriptorIndexing = VK_TRUE,
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
                .descriptorBindingVariableDescriptorCount = VK_FALSE,
                .runtimeDescriptorArray = VK_TRUE,
                .samplerFilterMinmax = VK_FALSE,
                .scalarBlockLayout = VK_TRUE,
                .imagelessFramebuffer = VK_FALSE,
                .uniformBufferStandardLayout = VK_FALSE,
                .shaderSubgroupExtendedTypes = VK_FALSE,
                .separateDepthStencilLayouts = VK_FALSE,
                .hostQueryReset = VK_FALSE,
                .timelineSemaphore = VK_TRUE,
                .bufferDeviceAddress = VK_TRUE,
                .bufferDeviceAddressCaptureReplay = VK_FALSE,
                .bufferDeviceAddressMultiDevice = VK_FALSE,
                .vulkanMemoryModel = VK_FALSE,
                .vulkanMemoryModelDeviceScope = VK_FALSE,
                .vulkanMemoryModelAvailabilityVisibilityChains = VK_FALSE,
                .shaderOutputViewportIndex = VK_FALSE,
                .shaderOutputLayer = VK_FALSE,
                .subgroupBroadcastDynamicId = VK_FALSE,
            };

            VkPhysicalDeviceFeatures deviceFeatures{};
            deviceFeatures.samplerAnisotropy = VK_TRUE;
            deviceFeatures.shaderInt64       = VK_TRUE;

            VkPhysicalDeviceFeatures2 deviceFeatures2{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vk12_features,
                .features = deviceFeatures,
            };

            vuDevice.initDevice({
                                    config::ENABLE_VALIDATION_LAYERS_LAYERS,
                                    deviceFeatures2,
                                    surface,
                                    config::DEVICE_EXTENSIONS
                                });
        }
        //init swapchain
        {
            swapChain = VuSwapChain{};
            swapChain.init({vuDevice.device, vuDevice.physicalDevice, surface});
            disposeStack.push([&] { swapChain.uninit(); });
        }

        vuDevice.initBindless(config::BINDLESS_CONFIG_INFO, config::MAX_FRAMES_IN_FLIGHT);
        VuResourceManager::init(config::BINDLESS_CONFIG_INFO);
        disposeStack.push([&] { VuResourceManager::uninit(); });


        //init uniform buffers
        {
            uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkDeviceSize bufferSize = sizeof(GPU_FrameConst);

                uniformBuffers[i] = VuBuffer();
                uniformBuffers[i].init({
                                           .length = 1,
                                           .strideInBytes = bufferSize,
                                           .vkUsageFlags =
                                           VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT |
                                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
                                           .vmaMemoryUsage = VMA_MEMORY_USAGE_AUTO,
                                           .vmaCreateFlags = VMA_ALLOCATION_CREATE_MAPPED_BIT |
                                                             VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT
                                       });
            }
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                VuResourceManager::writeUBO_ToGlobalPool(0, i, uniformBuffers[i]);
            }

            //VuResourceManager::registerUniformBuffer(0, uniformBuffers[0]);

            disposeStack.push([vr = *this]
            {
                for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
                {
                    auto v = vr.uniformBuffers;
                    v[i].uninit();
                }
            });
        }
        //init command buffers
        {
            commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
            VkCommandBufferAllocateInfo allocInfo{};
            allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            allocInfo.commandPool        = vuDevice.commandPool;
            allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            allocInfo.commandBufferCount = static_cast<uint32>(commandBuffers.size());
            VkCheck(vkAllocateCommandBuffers(vuDevice.device, &allocInfo, commandBuffers.data()));
            for (uint32 i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                std::string name = std::format("Command Buffer {}", i);
                giveDebugName(vuDevice.device, VK_OBJECT_TYPE_COMMAND_BUFFER, commandBuffers[i], name.c_str());
            }
        }
        //init sync objects
        {
            imageAvailableSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
            renderFinishedSemaphores.resize(config::MAX_FRAMES_IN_FLIGHT);
            inFlightFences.resize(config::MAX_FRAMES_IN_FLIGHT);

            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                VkCheck(vkCreateSemaphore(vuDevice.device, &semaphoreInfo, nullptr,
                                          &imageAvailableSemaphores[i]));
                VkCheck(vkCreateSemaphore(vuDevice.device, &semaphoreInfo, nullptr,
                                          &renderFinishedSemaphores[i]));
                VkCheck(vkCreateFence(vuDevice.device, &fenceInfo, nullptr, &inFlightFences[i]));
            }

            disposeStack.push([vr = *this, this]
            {
                for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
                {
                    vkDestroySemaphore(vuDevice.device, vr.imageAvailableSemaphores[i], nullptr);
                    vkDestroySemaphore(vuDevice.device, vr.renderFinishedSemaphores[i], nullptr);
                    vkDestroyFence(vuDevice.device, vr.inFlightFences[i], nullptr);
                }
            });
        }

        vuDevice.initDefaultResources();
        initImGui();
        //init default resources
        {
            defaultImageHandle    = vuDevice.imagePool.createHandle();
            auto* defaultImagePtr = vuDevice.imagePool.get(defaultImageHandle);
            defaultImagePtr->initFromAsset(vuDevice,
                                           std::filesystem::path("assets/textures/error.png"),
                                           VK_FORMAT_R8G8B8A8_UNORM);

            VuResourceManager::writeSampledImageToGlobalPool(defaultImageHandle.index, defaultImagePtr->imageView);
            //disposeStack.push([&] { debugTexture.destroyHandle(); });

            defaultSamplerHandle    = vuDevice.samplerPool.createHandle();
            auto* defaultSamplerPtr = vuDevice.samplerPool.get(defaultSamplerHandle);
            defaultSamplerPtr->init({.device = vuDevice.device, .physicalDevice = vuDevice.physicalDevice});

            VuResourceManager::writeSamplerToGlobalPool(defaultSamplerHandle.index, defaultSamplerPtr->vkSampler);
            //disposeStack.push([&] { debugSampler.destroyHandle(); });
        }
    }

    void VuRenderer::initImGui()
    {
        ZoneScoped;
        VkDescriptorPoolSize pool_sizes[] =
        {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
        };

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes    = pool_sizes;
        poolInfo.maxSets       = 1000;
        poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        VkCheck(vkCreateDescriptorPool(vuDevice.device, &poolInfo, nullptr, &vuDevice.uiDescriptorPool));
        disposeStack.push([&]
        {
            vkDestroyDescriptorPool(vuDevice.device, vuDevice.uiDescriptorPool, nullptr);
        });

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
        init_info.Instance                  = vuDevice.instance;
        init_info.PhysicalDevice            = vuDevice.physicalDevice;
        init_info.Device                    = vuDevice.device;
        init_info.QueueFamily               = VuSwapChain::findQueueFamilies(vuDevice.physicalDevice, surface).graphicsFamily.
            value();
        init_info.Queue               = vuDevice.graphicsQueue;
        init_info.DescriptorPool      = vuDevice.uiDescriptorPool;
        init_info.MinImageCount       = 2;
        init_info.ImageCount          = 2;
        init_info.UseDynamicRendering = false;
        init_info.RenderPass          = swapChain.renderPass.renderPass;

        ImGui_ImplVulkan_Init(&init_info);
        disposeStack.push([] { ImGui_ImplVulkan_Shutdown(); });

        ImGui_ImplVulkan_CreateFontsTexture();
        disposeStack.push([] { ImGui_ImplVulkan_DestroyFontsTexture(); });
    }

    void VuRenderer::reloadShaders()
    {
        //TODO:
    }

    void VuRenderer::bindGlobalBindlessSet(const VkCommandBuffer& commandBuffer)
    {
        vkCmdBindDescriptorSets(
                                commandBuffer,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                vuDevice.globalPipelineLayout,
                                0,
                                1,
                                &vuDevice.globalDescriptorSets[currentFrame],
                                0,
                                nullptr
                               );
    }

    void VuRenderer::uninit()
    {
        vkDeviceWaitIdle(vuDevice.device);
        while (!disposeStack.empty())
        {
            std::function<void()> disposeFunc = disposeStack.top();
            disposeFunc();
            disposeStack.pop();
        }
    }
}
