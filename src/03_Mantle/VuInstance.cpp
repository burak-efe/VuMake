#include "VuInstance.h"

#include <cstring>
#include <iostream>
#include <vector>

namespace Vu {

VuInstance::VuInstance() = default;
VuInstance::VuInstance(VuInstance&& other) noexcept :
    m_instance(other.m_instance),
    m_debugMessenger(other.m_debugMessenger) {
  other.m_instance       = VK_NULL_HANDLE;
  other.m_debugMessenger = VK_NULL_HANDLE;
}
VuInstance&
VuInstance::operator=(VuInstance&& other) noexcept {
  if (this != &other) {
    cleanup();
    m_instance             = other.m_instance;
    m_debugMessenger       = other.m_debugMessenger;
    other.m_instance       = VK_NULL_HANDLE;
    other.m_debugMessenger = VK_NULL_HANDLE;
  }
  return *this;
}
VuInstance::~VuInstance() { cleanup(); }
void
VuInstance::cleanup() {
  if (m_instance != VK_NULL_HANDLE && m_debugMessenger != VK_NULL_HANDLE) {
    auto destroyDebugMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
        vkGetInstanceProcAddr(m_instance, "vkDestroyDebugUtilsMessengerEXT"));
    if (destroyDebugMessenger) { destroyDebugMessenger(m_instance, m_debugMessenger, nullptr); }
    m_debugMessenger = VK_NULL_HANDLE;
  }
  if (m_instance != VK_NULL_HANDLE) {
    vkDestroyInstance(m_instance, nullptr);
    m_instance = VK_NULL_HANDLE;
  }
}
VuInstance::VuInstance(bool                   enableValidationLayers,
                       std::span<const char*> validationLayers,
                       std::span<const char*> extensions) {
  if (enableValidationLayers && !checkValidationLayerSupport(validationLayers)) { throw VK_ERROR_LAYER_NOT_PRESENT; }
  VkApplicationInfo appInfo {.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO};
  appInfo.pApplicationName   = "VuMake";
  appInfo.applicationVersion = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.pEngineName        = "No Engine";
  appInfo.engineVersion      = VK_MAKE_API_VERSION(0, 1, 0, 0);
  appInfo.apiVersion         = VK_API_VERSION_1_2;

  VkInstanceCreateInfo instanceCreateInfo {.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO};

  instanceCreateInfo.pApplicationInfo        = &appInfo;
  instanceCreateInfo.enabledExtensionCount   = static_cast<uint32_t>(extensions.size());
  instanceCreateInfo.ppEnabledExtensionNames = extensions.data();

  VkDebugUtilsMessengerCreateInfoEXT dbgMessengerCreateInfo {};
  dbgMessengerCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
  std::vector<VkValidationFeatureEnableEXT> validation_feature_enables = {
      VK_VALIDATION_FEATURE_ENABLE_DEBUG_PRINTF_EXT};
  if (enableValidationLayers) {

    VkValidationFeaturesEXT validation_features {.sType = VK_STRUCTURE_TYPE_VALIDATION_FEATURES_EXT};
    validation_features.enabledValidationFeatureCount = 1;
    validation_features.pEnabledValidationFeatures    = validation_feature_enables.data();

    dbgMessengerCreateInfo.pNext = &validation_features;

    dbgMessengerCreateInfo.messageSeverity = //VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                             VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    dbgMessengerCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                         VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT ;//|
                                         //VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

    dbgMessengerCreateInfo.pfnUserCallback = debugCallback;

    instanceCreateInfo.enabledLayerCount   = static_cast<uint32_t>(validationLayers.size());
    instanceCreateInfo.ppEnabledLayerNames = validationLayers.data();
    instanceCreateInfo.pNext               = &dbgMessengerCreateInfo;
  }

  VkResult instanceRes = vkCreateInstance(&instanceCreateInfo, NO_ALLOC_CALLBACK, &m_instance);
  THROW_if_fail(instanceRes);

  if (enableValidationLayers) {

    PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT =
        reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
            vkGetInstanceProcAddr(m_instance, "vkCreateDebugUtilsMessengerEXT"));

    VkResult dbgUtilMsgRes =
        vkCreateDebugUtilsMessengerEXT(m_instance, &dbgMessengerCreateInfo, NO_ALLOC_CALLBACK, &m_debugMessenger);
    THROW_if_fail(dbgUtilMsgRes);
  }
}
bool
VuInstance::checkValidationLayerSupport(const std::span<const char*>& validationLayers) {

  // TODO remove allocations
  uint32_t layerCount = 0;
  vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
  std::vector<VkLayerProperties> layers(layerCount);
  vkEnumerateInstanceLayerProperties(&layerCount, layers.data());

  for (const auto layerName : validationLayers) {
    bool layerFound = false;

    for (const auto& layerProperties : layers) {
      if (std::strcmp(layerName, layerProperties.layerName) == 0) {
        layerFound = true;
        break;
      }
    }

    if (!layerFound) { return false; }
  }
  return true;
}
VkBool32
VuInstance::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                          VkDebugUtilsMessageTypeFlagsEXT             messageType,
                          const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                          void*                                       pUserData) {
  std::cout << "[VALIDATION]: " << pCallbackData->pMessage << std::endl;
  return VK_FALSE;
}
} // namespace Vu