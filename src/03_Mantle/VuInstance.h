#pragma once

namespace Vu {

struct VuInstance {
  vk::raii::Context                raiiContext    = {};
  vk::raii::Instance               instance       = {nullptr};
  vk::raii::DebugUtilsMessengerEXT debugMessenger = {nullptr};

  VuInstance();

private:
public:
  static std::expected<VuInstance, vk::Result>
  make(bool                   enableValidationLayers,
       std::span<const char*> validationLayers,
       std::span<const char*> extensions) noexcept;

private:
  // can throw vk::Result
  VuInstance(bool enableValidationLayers, std::span<const char*> validationLayers, std::span<const char*> extensions);

  static bool
  checkValidationLayerSupport(const vk::raii::Context& raiiContext, const std::span<const char*> validationLayers);

  static vk::Bool32
  debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      messageSeverity,
                vk::DebugUtilsMessageTypeFlagsEXT             messageType,
                const vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData,
                void*                                         pUserData);
};

} // namespace Vu
