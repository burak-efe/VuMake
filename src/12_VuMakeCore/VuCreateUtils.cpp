#include "VuCreateUtils.h"


#include <cstdint>                  // for uint32_t
#include <optional>                 // for optional
#include <set>                      // for set, _Rb_tree_const_iterator
#include <stdexcept>                // for runtime_error
#include <string>                   // for basic_string, string
#include <vector>                   // for vector

#include "VuCommon.h"
#include "10_Core/Common.h"       // for vk::Check
#include "../10_Core/VuCtx.h"
#include "12_VuMakeCore/VuTypes.h"  // for QueueFamilyIndices
#include "VuUtils.h"                // for fillDebugMessengerCreateInfo,

std::expected<vk::raii::Instance, vk::Result>
Vu::CreateUtils::createInstance(vk::raii::Context&     raiiContext,
                                bool                   enableValidationLayers,
                                std::span<const char*> validationLayers,
                                std::span<const char*> extensions)
{
    if (enableValidationLayers && !Utils::checkValidationLayerSupport(validationLayers))
    {
        throw std::runtime_error("validation layers requested, but not available!");
    }

    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName   = "VuMake";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName        = "No Engine";
    appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion         = VK_API_VERSION_1_2;

    vk::InstanceCreateInfo instanceCreateInfo{};
    instanceCreateInfo.pApplicationInfo        = &appInfo;
    instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
    instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

    vk::DebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (enableValidationLayers)
    {
        instanceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
        instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
        Utils::fillDebugMessengerCreateInfo(debugCreateInfo);

        instanceCreateInfo.pNext = &debugCreateInfo;
    }

    return raiiContext.createInstance(instanceCreateInfo);
}

std::expected<vk::PhysicalDevice, vk::Result>
Vu::CreateUtils::createPhysicalDevice(const vk::Instance&    instance,
                                      const vk::SurfaceKHR&  surface,
                                      std::span<const char*> enabledExtensions)
{

    uint32_t deviceCount = 0;

    RET_ON_FAIL(instance.enumeratePhysicalDevices(&deviceCount, nullptr));

    if (deviceCount == 0)
    {
        return std::unexpected{vk::Result::eErrorInitializationFailed};
    }

    std::vector<vk::PhysicalDevice> devices(deviceCount);
    RET_ON_FAIL(instance.enumeratePhysicalDevices( &deviceCount, devices.data()));

    for (const auto& device : devices)
    {
        if (Utils::isDeviceSupportExtensions(device, surface, enabledExtensions))
        {
            return device;
        }
    }

    return std::unexpected{vk::Result::eErrorInitializationFailed};
}

void Vu::CreateUtils::createDevice(const vk::PhysicalDeviceFeatures2& features,
                                   const QueueFamilyIndices&          indices,
                                   const vk::PhysicalDevice&          physicalDevice,
                                   std::span<const char*>             enabledExtensions,
                                   vk::Device&                        outDevice,
                                   vk::Queue&                         outGraphicsQueue,
                                   vk::Queue&                         outPresentQueue)
{
    std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos{};
    std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies)
    {
        vk::DeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType            = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount       = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    vk::DeviceCreateInfo createInfo{};
    createInfo.sType                   = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext                   = &features;
    createInfo.queueCreateInfoCount    = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos       = queueCreateInfos.data();
    createInfo.pEnabledFeatures        = nullptr;
    createInfo.enabledExtensionCount   = static_cast<uint32_t>(enabledExtensions.size());
    createInfo.ppEnabledExtensionNames = enabledExtensions.data();
    //create
    {
        vk::Check(vkCreateDevice(physicalDevice, &createInfo, nullptr, &outDevice));
    }

    //queue
    {
        vkGetDeviceQueue(outDevice, indices.graphicsFamily.value(), 0, &outGraphicsQueue);
        vkGetDeviceQueue(outDevice, indices.presentFamily.value(), 0, &outPresentQueue);
    }
}

void Vu::CreateUtils::createPipelineLayout(const vk::Device                         device,
                                           const std::span<vk::DescriptorSetLayout> descriptorSetLayouts,
                                           const u32                                pushConstantSizeAsByte,
                                           vk::PipelineLayout&                      outPipelineLayout)
{
    //push constants
    vk::PushConstantRange pcRange{
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0,
            .size = pushConstantSizeAsByte,
    };

    vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount         = descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts            = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges    = &pcRange;

    vk::Check(vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &outPipelineLayout));
}


void Vu::CreateUtils::createDebugMessenger(const vk::Instance& instance, vk::DebugUtilsMessengerEXT& outDebugMessenger)
{
    vk::DebugUtilsMessengerCreateInfoEXT createInfo{};
    Utils::fillDebugMessengerCreateInfo(createInfo);
    vk::Check(Vu::ctx::vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &outDebugMessenger));
}
