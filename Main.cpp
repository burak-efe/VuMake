#include <cstdio>    // for puts
#include <cstdlib>   // for EXIT_SUCCESS
#include <exception> // for exception
#include <iostream>  // for char_traits, basic_ostream
#include <memory>    // for make_unique, unique_ptr

#include "01_InnerCore/VuLogger.h" // for LogLevel, Logger
// #include "12_VuMakeCore/VuInstance.h"
// #include "30_Scenes/Scene0.h"          // for Scene0

#include "02_OuterCore/VuConfig.h"
#include "02_OuterCore/VuCtx.h"
#include "03_Mantle/VuInstance.h"
#include "03_Mantle/VuPhysicalDevice.h"
#include "GetTimeSinceProcessStart.h"
#include "SDL3/SDL_vulkan.h"

int
main(int argc, char* argv[]) {

  // auto instance = Vu::VuInstance::make(true,{},{});

  // std::cout << "App Start Time: " << GetTimeSinceProcessStart() * 1000 << " millisecond" << std::endl;
  // Vu::Logger::SetLevel(Vu::LogLevel::Trace);
  // auto scene0 = std::make_unique<Vu::Scene0>();
  //
  // try
  // {
  //     scene0->Run();
  // }
  // catch (const std::exception& e)
  // {
  //     std::puts(e.what());
  // }

  SDL_Window* window = nullptr;
  assert(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == true);

  window = SDL_CreateWindow("VuRenderer", Vu::config::START_WIDTH, Vu::config::START_HEIGHT,
                            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

  u32                      count                          = 0;
  const char* const*       sdlRequestedInstanceExtensions = SDL_Vulkan_GetInstanceExtensions(&count);
  std::vector<const char*> instanceExtensions(sdlRequestedInstanceExtensions, sdlRequestedInstanceExtensions + count);

  for (auto extension : Vu::config::INSTANCE_EXTENSIONS) {
    instanceExtensions.push_back(extension);
  }

  VULKAN_HPP_DEFAULT_DISPATCHER.init();
  auto instance = Vu::VuInstance::make(true, Vu::config::VALIDATION_LAYERS, instanceExtensions);
  assert(instance);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(*instance.value().instance);
  VkSurfaceKHR         nonRaiiSurface = nullptr;
  bool                 res     = SDL_Vulkan_CreateSurface(window, *instance.value().instance, nullptr, &nonRaiiSurface);
  auto                 err     = SDL_GetError();
  vk::raii::SurfaceKHR surface = {instance.value().instance, nonRaiiSurface};
  assert(res == true);
  auto phyDev = Vu::VuPhysicalDevice::make(instance->instance, surface, Vu::config::DEVICE_EXTENSIONS);
  assert(phyDev);

  return EXIT_SUCCESS;
}
