#include "VuSurface.h"

#include <cassert>

#include "SDL3/SDL_vulkan.h"
#include "VuInstance.h"

namespace Vu {

VuSurface::VuSurface(std::shared_ptr<VuInstance> vuInstance, SDL_Window* window) :
    m_vuInstance {std::move(vuInstance)} {

  bool surfaceResult = SDL_Vulkan_CreateSurface(window, m_vuInstance->m_instance, NO_ALLOC_CALLBACK, &m_surface);
  assert(surfaceResult == true);
}
VuSurface::VuSurface() = default;

VuSurface::VuSurface(VuSurface&& other) noexcept :
    m_vuInstance(std::move(other.m_vuInstance)),
    m_surface(other.m_surface) {
  other.m_surface = VK_NULL_HANDLE;
}
VuSurface&
VuSurface::operator=(VuSurface&& other) noexcept {
  if (this != &other) {
    cleanup();
    m_vuInstance    = std::move(other.m_vuInstance);
    m_surface       = other.m_surface;
    other.m_surface = VK_NULL_HANDLE;
  }
  return *this;
}
VuSurface::~VuSurface() { cleanup(); }
void
VuSurface::cleanup() {
  if (m_surface != VK_NULL_HANDLE && m_vuInstance) {
    vkDestroySurfaceKHR(m_vuInstance->m_instance, m_surface, nullptr);
    m_surface = VK_NULL_HANDLE;
  }
  m_vuInstance.reset();
}
} // namespace Vu