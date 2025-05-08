#include "VuRenderer.h"

#include <cassert>                        // for assert
#include <cstddef>                        // for size_t
#include <format>                         // for format
#include <functional>                     // for function
#include <iostream>                       // for basic_ostream, operator<<
#include <optional>                       // for optional
#include <span>                           // for span
#include <string>                         // for char_traits, basic_string
#include <vector>                         // for vector


#include "SDL3/SDL_init.h"                // for SDL_Init, SDL_Quit, SDL_INI...
#include "SDL3/SDL_video.h"               // for SDL_CreateWindow, SDL_Destr...
#include "SDL3/SDL_vulkan.h"              // for SDL_Vulkan_CreateSurface
#include "imgui.h"                        // for CreateContext, DestroyContext
#include "imgui_impl_sdl3.h"              // for ImGui_ImplSDL3_InitForVulkan
#include "imgui_impl_vulkan.h"            // for ImGui_ImplVulkan_InitInfo
#include "vk_mem_alloc.h"                 // for VmaAllocationCreateFlagBits

#include "08_LangUtils/ScopeTimer.h"      // for ScopeTimer
#include "08_LangUtils/TypeDefs.h"        // for u32
#include "10_Core/Common.h"             // for vk::Check
#include "../10_Core/VuConfig.h"
#include "../10_Core/VuCtx.h"
#include "12_VuMakeCore/VuCommon.h"
#include "12_VuMakeCore/VuBuffer.h"       // for VuBuffer
#include "12_VuMakeCore/VuCreateUtils.h"  // for createDebugMessenger, creat...
#include "12_VuMakeCore/VuRenderPass.h"   // for VuRenderPass
#include "12_VuMakeCore/VuTypes.h"        // for VuDisposeStack, GPU_FrameConst
#include "12_VuMakeCore/VuUtils.h"        // for giveDebugName
#include "14_VuMake/VuSwapChain.h"        // for VuSwapChain
#include "VuDevice.h"                     // for VuDevice

namespace Vu
{
VuRenderer::VuRenderer()
{
    ScopeTimer timer;
    bool       isValidationEnabled = config::ENABLE_VALIDATION_LAYERS_LAYERS;
    //init window
    {
        assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);
        disposeStack.push([] { SDL_Quit(); });

        ctx::window = SDL_CreateWindow("VuRenderer", config::START_WIDTH, config::START_HEIGHT,
                                       SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);
        disposeStack.push([] { SDL_DestroyWindow(ctx::window); });
    }
    //init instance
    {
        u32 count = 0;

        const char* const *      instance_extensions = SDL_Vulkan_GetInstanceExtensions(&count);
        std::vector<const char*> instanceExtensions(instance_extensions, instance_extensions + count);

        for (auto extension : config::INSTANCE_EXTENSIONS)
        {
            instanceExtensions.push_back(extension);
        }


        // CreateUtils::createInstance(isValidationEnabled, config::VALIDATION_LAYERS, instanceExtensions);
        // disposeStack.push([&] { vkDestroyInstance(instance, nullptr); });


        //Vu::ctx::loadExtensionFunctions(instance);


        // if (isValidationEnabled)
        // {
        //     CreateUtils::createDebugMessenger(instance, debugMessenger);
        //     disposeStack.push([&]
        //     {
        //         //CreateUtils::destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        //         ctx::vkDestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        //     });
        // }
    }
    //init surface
    {

        bool res = SDL_Vulkan_CreateSurface(ctx::window, instance, nullptr, reinterpret_cast<VkSurfaceKHR*>(&surface));
        assert(res == true);
        disposeStack.push([&] { SDL_Vulkan_DestroySurface(vuDevice.instance, surface, nullptr); });
    }

    //CreateUtils::createPhysicalDevice(instance, surface, config::DEVICE_EXTENSIONS, physicalDevice);

    //init device
    {
        vk::PhysicalDeviceSynchronization2FeaturesKHR sync2Features{
                .synchronization2 = VK_TRUE
        };

        vk::PhysicalDeviceVulkan11Features vk11_features{
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

        vk::PhysicalDeviceVulkan12Features vk12_features{
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

        vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.samplerAnisotropy = VK_TRUE;
        deviceFeatures.shaderInt64       = VK_TRUE;

        vk::PhysicalDeviceFeatures2 deviceFeatures2{
                .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
                .pNext = &vk12_features,
                .features = deviceFeatures,
        };

        vuDevice = VuDevice{
                {
                        .instance = instance,
                        .physicalDevice = physicalDevice,
                        .enableValidationLayers = config::ENABLE_VALIDATION_LAYERS_LAYERS,
                        .physicalDeviceFeatures2 = deviceFeatures2,
                        .surface = surface,
                        .deviceExtensions = config::DEVICE_EXTENSIONS,

                        .uboBinding = config::BINDLESS_UNIFORM_BUFFER_BINDING,
                        .samplerBinding = config::BINDLESS_SAMPLER_BINDING,
                        .sampledImageBinding = config::BINDLESS_SAMPLED_IMAGE_BINDING,
                        .storageImageBinding = config::BINDLESS_STORAGE_IMAGE_BINDING,
                        .storageBufferBinding = config::BINDLESS_STORAGE_BUFFER_BINDING,

                        .uboCount = config::BINDLESS_UNIFORM_BUFFER_COUNT,
                        .samplerCount = config::BINDLESS_SAMPLER_COUNT,
                        .sampledImageCount = config::BINDLESS_SAMPLED_IMAGE_COUNT,
                        .storageImageCount = config::BINDLESS_STORAGE_IMAGE_COUNT,
                        .storageBufferCount = config::BINDLESS_STORAGE_BUFFER_COUNT,
                }
        };
        disposeStack.push([&] { vuDevice.uninit(); });
    }


    //init swapchain
    {
        swapChain = {&vuDevice, surface};
        disposeStack.push([&] { swapChain.uninit(); });
    }

    //init uniform buffers
    {
        uniformBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);
        for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
        {
            vk::DeviceSize bufferSize = sizeof(GPU_FrameConst);

            uniformBuffers[i] = VuBuffer{};
            uniformBuffers[i].init(vuDevice.device, vuDevice.vma, {
                                           .name = "UniformBuffer",
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
            vuDevice.writeUBO_ToGlobalPool(uniformBuffers[i], 0, i);
        }

        disposeStack.push([this]
        {
            for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
            {
                auto v = uniformBuffers;
                v[i].uninit();
            }
        });
    }
    //init command buffers
    {
        commandBuffers.resize(config::MAX_FRAMES_IN_FLIGHT);

        vk::CommandBufferAllocateInfo allocInfo{};
        allocInfo.commandPool        = vuDevice.commandPool;
        allocInfo.level              = vk::CommandBufferLevel::ePrimary;
        allocInfo.commandBufferCount = static_cast<u32>(commandBuffers.size());

        VkCheck(vkAllocateCommandBuffers(vuDevice.device, &allocInfo, commandBuffers.data()));
        for (u32 i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++)
        {
            std::string name = std::format("Command Buffer {}", i);
            Utils::giveDebugName(vuDevice.device, VK_OBJECT_TYPE_COMMAND_BUFFER, commandBuffers[i], name.c_str());
        }
    }
    //init sync objects
    {
        vk::SemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        vk::FenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        uint32_t swapChainImageCount = swapChain.swapChainImages.size();
        imageAvailableSemaphores.resize(swapChainImageCount);
        renderFinishedSemaphores.resize(swapChainImageCount);
        inFlightFences.resize(swapChainImageCount);
        for (size_t i = 0; i < swapChainImageCount; i++)
        {
            vk::Check(vkCreateSemaphore(vuDevice.device, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]));
            vk::Check(vkCreateSemaphore(vuDevice.device, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]));
            vk::Check(vkCreateFence(vuDevice.device, &fenceInfo, nullptr, &inFlightFences[i]));
        }

        disposeStack.push([ this, swapChainImageCount ]
        {
            for (size_t i = 0; i < swapChainImageCount; i++)
            {
                vkDestroySemaphore(vuDevice.device, imageAvailableSemaphores[i], nullptr);
                vkDestroySemaphore(vuDevice.device, renderFinishedSemaphores[i], nullptr);
                vkDestroyFence(vuDevice.device, inFlightFences[i], nullptr);
            }
        });
    }

    //initImGui();
}

void VuRenderer::initImGui()
{
    vk::DescriptorPoolSize pool_sizes[] =
    {
            {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000},
    };

    vk::DescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes    = pool_sizes;
    poolInfo.maxSets       = 1000;
    poolInfo.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

    vk::Check(vkCreateDescriptorPool(vuDevice.device, &poolInfo, nullptr, &vuDevice.uiDescriptorPool));
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
    init_info.Instance = vuDevice.instance;
    init_info.PhysicalDevice = vuDevice.physicalDevice;
    init_info.Device = vuDevice.device;
    init_info.QueueFamily = VuSwapChain::findQueueFamilies(vuDevice.physicalDevice, surface).graphicsFamily.
            value();
    init_info.Queue               = vuDevice.graphicsQueue;
    init_info.DescriptorPool      = vuDevice.uiDescriptorPool;
    init_info.MinImageCount       = 2;
    init_info.ImageCount          = 2;
    init_info.UseDynamicRendering = false;
    init_info.RenderPass          = swapChain.gBufferPass.renderPass;

    ImGui_ImplVulkan_Init(&init_info);
    disposeStack.push([] { ImGui_ImplVulkan_Shutdown(); });

    ImGui_ImplVulkan_CreateFontsTexture();
    disposeStack.push([] { ImGui_ImplVulkan_DestroyFontsTexture(); });
}

void VuRenderer::bindGlobalBindlessSet(const vk::CommandBuffer& commandBuffer)
{
    vkCmdBindDescriptorSets(commandBuffer,
                            VK_PIPELINE_BIND_POINT_GRAPHICS,
                            vuDevice.globalPipelineLayout,
                            0,
                            1,
                            &vuDevice.globalDescriptorSets[currentFrame],
                            0,
                            nullptr);
}

void VuRenderer::uninit()
{
    std::cout << "VuRenderer::uninit" << std::endl;
    vkDeviceWaitIdle(vuDevice.device);
    disposeStack.disposeAll();
}
}
