#pragma once
#include <cstring>
#include <expected>
#include <iostream>
#include <vulkan/vulkan_raii.hpp>

namespace Vu
{


struct VuInstance
{
    vk::raii::Context                raiiContext    = {};
    vk::raii::Instance               instance       = {nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger = {nullptr};

private:

public:
    static std::expected<VuInstance, vk::Result>
    make(bool                   enableValidationLayers,
         std::span<const char*> validationLayers,
         std::span<const char*> extensions) noexcept
    {
        try
        {
            VuInstance instance{enableValidationLayers, validationLayers, extensions};

            return instance;
        }
        catch (vk::Result result)
        {
            return std::unexpected{result};
        }
    }

private:
    //can throw vk::Result
    VuInstance(bool                   enableValidationLayers,
               std::span<const char*> validationLayers,
               std::span<const char*> extensions)
    {
        if (enableValidationLayers && !checkValidationLayerSupport(raiiContext, validationLayers))
        {
            throw vk::Result::eErrorLayerNotPresent;
        }

        vk::ApplicationInfo appInfo{};
        appInfo.pApplicationName   = "VuMake";
        appInfo.applicationVersion = vk::makeApiVersion(0, 1, 0, 0);
        appInfo.pEngineName        = "No Engine";
        appInfo.engineVersion      = vk::makeApiVersion(0, 1, 0, 0);
        appInfo.apiVersion         = VK_API_VERSION_1_2;

        vk::InstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.pApplicationInfo        = &appInfo;
        instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

        vk::DebugUtilsMessengerCreateInfoEXT dbgMessengerCreateInfo{};

        dbgMessengerCreateInfo.messageSeverity = vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo
                                                 | vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning
                                                 | vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        dbgMessengerCreateInfo.messageType = vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral
                                             | vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation
                                             | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance;

        dbgMessengerCreateInfo.pfnUserCallback = debugCallback;

        if (enableValidationLayers)
        {
            instanceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
            instanceCreateInfo.pNext               = &dbgMessengerCreateInfo;
        }

        std::expected<vk::raii::Instance, vk::Result> res = raiiContext.createInstance(instanceCreateInfo);
        if (!res)
        {
            throw res.error();
        }

        instance = std::move(res.value());

        auto debugMessengerOrNull = instance.createDebugUtilsMessengerEXT(dbgMessengerCreateInfo);
        if (!debugMessengerOrNull)
        {
            throw res.error();
        }
        debugMessenger = std::move(debugMessengerOrNull.value());
    }


    static bool
    checkValidationLayerSupport(const vk::raii::Context& raiiContext, const std::span<const char*> validationLayers)
    {
        const std::vector<vk::LayerProperties> availableLayers = raiiContext.enumerateInstanceLayerProperties();

        for (const auto layerName : validationLayers)
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers)
            {
                if (std::strcmp(layerName, layerProperties.layerName) == 0)
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


    static vk::Bool32
    debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                  vk::DebugUtilsMessageTypeFlagsEXT             messageType,
                  const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                  void*                                         pUserData)
    {
        std::cout
                << "###############################################################################################################\n"
                << "[VALIDATION]: " << pCallbackData->pMessage << "\n"
                << "###############################################################################################################\n"
                << std::endl;
        return vk::False;
    }
};

} // Vu
