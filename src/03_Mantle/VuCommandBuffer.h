#pragma once
#include <memory>

#include "VuDevice.h"

namespace Vu {

struct VuCommandBuffer {
  std::shared_ptr<VuDevice> m_vuDevice {};
  VkCommandBuffer           m_commandBuffer {};
};

} // namespace Vu
