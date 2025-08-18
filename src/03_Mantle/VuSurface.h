#pragma once
#include <memory>

#include "02_OuterCore/VuCommon.h"

struct SDL_Window;

namespace Vu {
struct VuInstance;

struct VuSurface {
  std::shared_ptr<VuInstance> m_vuInstance;
  VkSurfaceKHR                m_surface;

  VuSurface(std::shared_ptr<VuInstance> vuInstance, SDL_Window* window);
  VuSurface();
  VuSurface(const VuSurface&) = delete;
  VuSurface&
  operator=(const VuSurface&) = delete;

  VuSurface(VuSurface&& other) noexcept;

  VuSurface&
  operator=(VuSurface&& other) noexcept;

  ~VuSurface();

private:
  void
  cleanup();
};

} // namespace Vu
