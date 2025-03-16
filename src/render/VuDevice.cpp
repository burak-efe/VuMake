#include "VuDevice.h"

#include <set>

#include "VuUtils.h"

void Vu::VuDevice::uninit()
{
    disposeStack.disposeAll();
}

void Vu::VuDevice::initInstance(VkBool32               enableValidationLayers, std::span<const char*> validationLayers,
                                std::span<const char*> instanceExtensions)
{
    createInstance(enableValidationLayers, validationLayers, instanceExtensions, instance);
    disposeStack.push([&]
    {
        vkDestroyInstance(instance, nullptr);
    });

    if (enableValidationLayers)
    {
        createDebugMessenger(instance, debugMessenger);
        disposeStack.push([&]
        {
            destroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        });
    }
}

void Vu::VuDevice::initDevice(const VuDeviceCreateInfo& info)
{
    createPhysicalDevice(instance, info.surface, info.deviceExtensions, physicalDevice);
    queueFamilyIndices = QueueFamilyIndices::findQueueFamilies(physicalDevice, info.surface);

    createDevice(info.physicalDeviceFeatures2,
                 queueFamilyIndices,
                 physicalDevice,
                 info.deviceExtensions,
                 device, graphicsQueue, presentQueue
                );
    disposeStack.push([&]
    {
        vkDestroyDevice(device, nullptr);
    });

    initVMA();
    disposeStack.push([&]
    {
        vmaDestroyAllocator(vma);
    });
    initCommandPool();
    disposeStack.push([&]
    {
        vkDestroyCommandPool(device, commandPool, nullptr);
    });
}

void Vu::VuDevice::initVMA()
{
    ZoneScoped;
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
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &vma_vulkan_func,
        .instance = instance,
    };

    VkCheck(vmaCreateAllocator(&createInfo, &vma));
}

void Vu::VuDevice::initCommandPool()
{
    ZoneScoped;
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType            = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags            = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
    VkCheck(vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool));
}

void Vu::VuDevice::initBindless(const VuBindlessConfigInfo& info, uint32 maxFramesInFligth)
{
    initDescriptorSetLayout(info);
    disposeStack.push([&]
    {
        vkDestroyDescriptorSetLayout(device, globalDescriptorSetLayout, nullptr);
    });
    initDescriptorPool(info);
    disposeStack.push([&]
    {
        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    });
    initGlobalDescriptorSet(maxFramesInFligth);
    std::array descSetLayouts{globalDescriptorSetLayout};

    createPipelineLayout(device, descSetLayouts, config::PUSH_CONST_SIZE, globalPipelineLayout);
    disposeStack.push([&]
    {
        vkDestroyPipelineLayout(device, globalPipelineLayout, nullptr);
    });
}

void Vu::VuDevice::initDescriptorSetLayout(const VuBindlessConfigInfo& info)
{
    ZoneScoped;
    VkDescriptorSetLayoutBinding ubo{
        .binding = info.uboBinding,
        .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        .descriptorCount = info.uboCount,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };
    VkDescriptorSetLayoutBinding sampler{
        .binding = info.samplerBinding,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER,
        .descriptorCount = info.samplerCount,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };
    VkDescriptorSetLayoutBinding sampledImage{
        .binding = info.sampledImageBinding,
        .descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        .descriptorCount = info.sampledImageCount,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };
    VkDescriptorSetLayoutBinding storageImage{
        .binding = info.storageImageBinding,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        .descriptorCount = info.storageImageCount,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };

    VkDescriptorSetLayoutBinding storageBuffer{
        .binding = info.storageBufferBinding,
        .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        .descriptorCount = 1,
        .stageFlags = VK_SHADER_STAGE_ALL,
    };
    std::array descriptorSetLayoutBindings{
        ubo,
        sampler,
        sampledImage,
        storageImage,
        storageBuffer,
    };

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

    std::array descriptorSetLayoutFlags{flag, flag, flag, flag, flag};

    VkDescriptorSetLayoutBindingFlagsCreateInfoEXT binding_flags{};
    binding_flags.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO_EXT;
    binding_flags.bindingCount  = descriptorSetLayoutFlags.size();
    binding_flags.pBindingFlags = descriptorSetLayoutFlags.data();
    globalSetLayout.pNext       = &binding_flags;

    VkCheck(vkCreateDescriptorSetLayout(device, &globalSetLayout, nullptr, &globalDescriptorSetLayout));
}

void Vu::VuDevice::initDescriptorPool(const VuBindlessConfigInfo& info)
{
    ZoneScoped;

    std::array<VkDescriptorPoolSize, 5> poolSizes{
        {
            {.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = info.uboCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_SAMPLER, .descriptorCount = info.samplerCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, .descriptorCount = info.sampledImageCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, .descriptorCount = info.storageImageCount * config::MAX_FRAMES_IN_FLIGHT},
            {.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, .descriptorCount = info.storageBufferCount * config::MAX_FRAMES_IN_FLIGHT},
        },
    };

    VkDescriptorPoolCreateInfo poolInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 2,
        .poolSizeCount = static_cast<uint32_t>(poolSizes.size()),
        .pPoolSizes = poolSizes.data(),
    };
    VkCheck(vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool));
}

void Vu::VuDevice::initGlobalDescriptorSet(uint32 maxFramesInFlight)
{
    ZoneScoped;
    std::vector globalLayouts(maxFramesInFlight, globalDescriptorSetLayout);

    VkDescriptorSetAllocateInfo globalSetsAllocInfo{
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = descriptorPool,
        .descriptorSetCount = maxFramesInFlight,
        .pSetLayouts = globalLayouts.data(),
    };

    globalDescriptorSets.resize(maxFramesInFlight);
    VkCheck(vkAllocateDescriptorSets(device, &globalSetsAllocInfo, globalDescriptorSets.data()));
}

VkCommandBuffer Vu::VuDevice::BeginSingleTimeCommands()
{
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool        = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    VkCheck(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    return commandBuffer;
}

void Vu::VuDevice::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

VkBool32 Vu::VuDevice::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                     VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                     void* pUserData)
{
    std::cout << "###############################################################################################################\n"
        << "[VALIDATION]: " << pCallbackData->pMessage << "\n"
        << "###############################################################################################################\n"
        << std::endl;
    return VK_FALSE;
}

void Vu::VuDevice::fillDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo                 = {};
    createInfo.sType           = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
        VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                             VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debugCallback;
}

VkResult Vu::VuDevice::createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                    const VkAllocationCallbacks*              pAllocator,
                                                    VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
                                                                                           instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Vu::VuDevice::destroyDebugUtilsMessengerEXT(VkInstance                   instance, VkDebugUtilsMessengerEXT debugMessenger,
                                                 const VkAllocationCallbacks* pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
                                                                                            instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void Vu::VuDevice::createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    fillDebugMessengerCreateInfo(createInfo);
    VkCheck(createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &outDebugMessenger));
}

bool Vu::VuDevice::checkDeviceExtensionSupport(VkPhysicalDevice device, std::span<const char*> requestedExtensions)
{
    uint32_t extensionCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requested(requestedExtensions.begin(), requestedExtensions.end());

    for (const VkExtensionProperties extension : availableExtensions)
    {
        requested.erase(extension.extensionName);
    }

    if (requested.empty())
    {
        return true;
    }

    for (std::string ext : requested)
    {
        std::cout << "extension not supoorted: " << ext << std::endl;
    }
    return false;
}

bool Vu::VuDevice::isDeviceSuitable(VkPhysicalDevice       device, VkSurfaceKHR surface,
                                    std::span<const char*> enabledExtensions)
{
    QueueFamilyIndices indices             = QueueFamilyIndices::findQueueFamilies(device, surface);
    bool               extensionsSupported = checkDeviceExtensionSupport(device, enabledExtensions);
    bool               swapChainAdequate   = false;
    if (extensionsSupported)
    {
        SwapChainSupportDetails swapChainSupport = SwapChainSupportDetails::querySwapChainSupport(device, surface);
        swapChainAdequate                        = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

    return indices.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Vu::VuDevice::checkValidationLayerSupport(std::span<const char*> validationLayers)
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const auto layerName : validationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
        }

        if (!layerFound)
        {
            return false;
        }
    }

    return true;
}

void Vu::VuDevice::createInstance(bool                   enableValidationLayers, std::span<const char*> validationLayers,
                                  std::span<const char*> extensions, VkInstance&                        outInstance)
{
    ZoneScoped;

    if (enableValidationLayers && !checkValidationLayerSupport(validationLayers))
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName   = "VuMake";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    VkInstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.sType            = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instanceCreateInfo.pApplicationInfo = &appInfo;

    instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        fillDebugMessengerCreateInfo(debugCreateInfo);

        instanceCreateInfo.pNext = &debugCreateInfo;
    }
    //create
    {
        ZoneScopedN("create");
        VkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &outInstance));
    }
}

void Vu::VuDevice::createPhysicalDevice(const VkInstance&      instance, const VkSurfaceKHR&        surface,
                                        std::span<const char*> enabledExtensions, VkPhysicalDevice& outPhysicalDevice)
{
    ZoneScoped;
    outPhysicalDevice    = VK_NULL_HANDLE;
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0)
    {
        throw std::runtime_error("failed to find GPUs with Vulkan support!");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    for (const auto& device : devices)
    {
        if (isDeviceSuitable(device, surface, enabledExtensions))
        {
            outPhysicalDevice = device;
            break;
        }
    }

    if (outPhysicalDevice == VK_NULL_HANDLE)
    {
        throw std::runtime_error(std::string("failed to find a suitable GPU!"));
    }
}

void Vu::VuDevice::createDevice(const VkPhysicalDeviceFeatures2& features, const QueueFamilyIndices& indices,
                                const VkPhysicalDevice& physicalDevice, std::span<const char*> enabledExtensions, VkDevice& outDevice,
                                VkQueue& outGraphicsQueue, VkQueue& outPresentQueue)
{
    ZoneScoped;
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t>                   uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkDeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = &features;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = nullptr;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    //create
    {
        ZoneScopedN("create");
        VkCheck(vkCreateDevice(physicalDevice, &createInfo, nullptr, &outDevice));
    }

    //queue
    {
        ZoneScopedN("Queue");
        vkGetDeviceQueue(outDevice, indices.graphicsFamily.value(), 0, &outGraphicsQueue);
        vkGetDeviceQueue(outDevice, indices.presentFamily.value(), 0, &outPresentQueue);
    }
}
