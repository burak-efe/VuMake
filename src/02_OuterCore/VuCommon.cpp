#include "VuCommon.h"

namespace Vu {} // namespace Vu
void
THROW_if_fail(VkResult res) {
  if (res != VK_SUCCESS) { throw res; }
}