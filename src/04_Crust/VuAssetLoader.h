#pragma once

#include "02_OuterCore/VuCommon.h"

#include <expected>

namespace Vu {
struct VuImage;
struct VuMesh;
struct VuRenderer;
struct GPU_PBR_MaterialData;

enum class MapType { baseColor, normal, ao_rough_metal };

struct VuAssetLoader {

  static std::expected<VuImage, VkResult>
  loadMapFromGLTF(VuRenderer& vuRenderer, const std::filesystem::path& gltfPath, MapType type);

  static void
  loadGLTF(VuRenderer& vuRenderer, const std::filesystem::path& gltfPath, VuMesh& dstMesh);
};
} // namespace Vu
