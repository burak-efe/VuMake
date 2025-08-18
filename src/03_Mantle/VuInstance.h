#pragma once
#include "02_OuterCore/VuCommon.h"
namespace Vu {

struct VuInstance {
  VkInstance               m_instance {nullptr};
  VkDebugUtilsMessengerEXT m_debugMessenger {nullptr};

  //--------------------------------------------------------------------------------------------------------------------

  VuInstance();

  VuInstance(const VuInstance&) = delete;
  VuInstance&
  operator=(const VuInstance&) = delete;

  VuInstance(VuInstance&& other) noexcept;

  VuInstance&
  operator=(VuInstance&& other) noexcept;

  ~VuInstance();

  SETUP_EXPECTED_WRAPPER(VuInstance,
                         (bool                   enableValidationLayers,
                          std::span<const char*> validationLayers,
                          std::span<const char*> extensions),
                         (enableValidationLayers, validationLayers, extensions))

private:
  void
  cleanup();
  //--------------------------------------------------------------------------------------------------------------------

  VuInstance(bool enableValidationLayers, std::span<const char*> validationLayers, std::span<const char*> extensions);

  static bool
  checkValidationLayerSupport(const std::span<const char*>& validationLayers);

  static VkBool32
  debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                VkDebugUtilsMessageTypeFlagsEXT             messageType,
                const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*                                       pUserData);
};

} // namespace Vu
