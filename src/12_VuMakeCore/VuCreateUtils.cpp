#include "VuCreateUtils.h"

#include <set>

#include "VuUtils.h"

void Vu::CreateUtils::createInstance(bool                   enableValidationLayers,
                                     std::span<const char*> validationLayers,
                                     std::span<const char*> extensions,
                                     VkInstance&            outInstance)
{
    ZoneScoped;

    if (enableValidationLayers && !Utils::checkValidationLayerSupport(validationLayers))
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
        Utils::fillDebugMessengerCreateInfo(debugCreateInfo);

        instanceCreateInfo.pNext = &debugCreateInfo;
    }
    //create
    {
        ZoneScopedN("create call");
        VkCheck(vkCreateInstance(&instanceCreateInfo, nullptr, &outInstance));
    }
}

void Vu::CreateUtils::createPhysicalDevice(const VkInstance&      instance,
                                           const VkSurfaceKHR&    surface,
                                           std::span<const char*> enabledExtensions,
                                           VkPhysicalDevice&      outPhysicalDevice)
{
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
        if (Utils::isDeviceSupportExtensions(device, surface, enabledExtensions))
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

void Vu::CreateUtils::createDevice(const VkPhysicalDeviceFeatures2& features,
                                   const QueueFamilyIndices&        indices,
                                   const VkPhysicalDevice&          physicalDevice,
                                   std::span<const char*>           enabledExtensions,
                                   VkDevice&                        outDevice,
                                   VkQueue&                         outGraphicsQueue,
                                   VkQueue&                         outPresentQueue)
{
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

void Vu::CreateUtils::createPipelineLayout(const VkDevice                         device,
                                           const std::span<VkDescriptorSetLayout> descriptorSetLayouts,
                                           const u32                              pushConstantSizeAsByte,
                                           VkPipelineLayout&                      outPipelineLayout)
{
    //push constants
    VkPushConstantRange pcRange{
        .stageFlags = VK_SHADER_STAGE_ALL,
        .offset = 0,
        .size = pushConstantSizeAsByte,
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pcRange;

    VkCheck(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout));
}

VkResult Vu::CreateUtils::createDebugUtilsMessengerEXT(VkInstance                                instance,
                                                       const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                                       const VkAllocationCallbacks*              pAllocator,
                                                       VkDebugUtilsMessengerEXT*                 pDebugMessenger)
{
    auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void Vu::CreateUtils::destroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                                    VkDebugUtilsMessengerEXT     debugMessenger,
                                                    const VkAllocationCallbacks* pAllocator)
{
    auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

void Vu::CreateUtils::createDebugMessenger(const VkInstance& instance, VkDebugUtilsMessengerEXT& outDebugMessenger)
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    Utils::fillDebugMessengerCreateInfo(createInfo);
    VkCheck(createDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &outDebugMessenger));
}
