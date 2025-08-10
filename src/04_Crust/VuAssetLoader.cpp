#include "VuAssetLoader.h"

#include "02_OuterCore/CollectionUtils.h"
#include "02_OuterCore/math/VuFloat2.h"
#include "02_OuterCore/math/VuFloat3.h"
#include "02_OuterCore/math/VuFloat4.h"
#include "03_Mantle/VuImage.h"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "VuMesh.h"
#include "VuRenderer.h"

namespace Vu {

std::expected<VuImage, VkResult>
VuAssetLoader::loadMapFromGLTF(VuRenderer& vuRenderer, const std::filesystem::path& gltfPath, MapType type) {

  fastgltf::Parser parser;

  auto data = fastgltf::GltfDataBuffer::FromPath(gltfPath);

  if (data.error() != fastgltf::Error::None) { return std::unexpected {VK_ERROR_UNKNOWN}; }

  auto asset = parser.loadGltf(data.get(), gltfPath.parent_path(), fastgltf::Options::LoadExternalBuffers);

  if (auto error = asset.error(); error != fastgltf::Error::None) {
    std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
  }

  auto& mesh      = asset.get().meshes.at(0);
  auto& prims     = mesh.primitives;
  auto& primitive = prims.at(0);

  path parentPath     = gltfPath.parent_path();
  auto matIndexOrNull = primitive.materialIndex;

  if (matIndexOrNull.has_value() == false) { return std::unexpected {VK_ERROR_UNKNOWN}; }

  fastgltf::Material& material = asset->materials.at(matIndexOrNull.value());

  if (type == MapType::baseColor) {

    fastgltf::Optional<fastgltf::TextureInfo>& textureInfoOrNull = material.pbrData.baseColorTexture;
    if (textureInfoOrNull.has_value()) {
      fastgltf::Image&       colorImage   = asset->images[textureInfoOrNull.value().textureIndex];
      fastgltf::sources::URI colorPath    = std::get<fastgltf::sources::URI>(colorImage.data);
      auto                   colorTexPath = parentPath / colorPath.uri.string();

      VuImage img = vuRenderer.createImageFromAsset(colorTexPath, VK_FORMAT_R8G8B8A8_SRGB);
      return img;
    }
    return std::unexpected {VK_ERROR_UNKNOWN};
  } else if (type == MapType::normal) {

    auto& textureInfoOrNull = material.normalTexture;
    if (textureInfoOrNull.has_value()) {
      fastgltf::Image&       image    = asset->images[textureInfoOrNull.value().textureIndex];
      fastgltf::sources::URI path     = std::get<fastgltf::sources::URI>(image.data);
      auto                   realPath = parentPath / path.uri.string();

      VuImage img = vuRenderer.createImageFromAsset(realPath, VK_FORMAT_R8G8B8A8_UNORM);
      return img;
    }
    return std::unexpected {VK_ERROR_UNKNOWN};

  } else if (type == MapType::ao_rough_metal) {
    fastgltf::Optional<fastgltf::TextureInfo>& textureInfoOrNull = material.pbrData.metallicRoughnessTexture;
    if (textureInfoOrNull.has_value()) {
      fastgltf::Image&       image    = asset->images[textureInfoOrNull.value().textureIndex];
      fastgltf::sources::URI path     = std::get<fastgltf::sources::URI>(image.data);
      auto                   realPath = parentPath / path.uri.string();

      VuImage img = vuRenderer.createImageFromAsset(realPath, VK_FORMAT_R8G8B8A8_UNORM);
      return img;
    }
    return std::unexpected {VK_ERROR_UNKNOWN};
  } else {
    return std::unexpected {VK_ERROR_UNKNOWN};
  }
}
void
VuAssetLoader::loadGLTF(VuRenderer& vuRenderer, const std::filesystem::path& gltfPath, VuMesh& dstMesh) {
  fastgltf::Parser parser;

  auto data = fastgltf::GltfDataBuffer::FromPath(gltfPath);
  if (data.error() != fastgltf::Error::None) { std::cout << "gltf file cannot be loaded!" << "\n"; }

  auto asset = parser.loadGltf(data.get(), gltfPath.parent_path(), fastgltf::Options::LoadExternalBuffers);
  if (auto error = asset.error(); error != fastgltf::Error::None) {
    std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
  }

  auto mesh      = asset.get().meshes.at(0);
  auto prims     = mesh.primitives;
  auto primitive = prims.at(0);

  // auto parentPath = gltfPath.parent_path();
  // fastgltf::Optional<size_t> matIndexOrNull = primitive.materialIndex;
  //
  // if (matIndexOrNull.has_value()) {
  //   fastgltf::Material& material            = asset->materials.at(matIndexOrNull.value());
  //   auto&               baseColorInfoOrNull = material.pbrData.baseColorTexture;
  //
  //   if (baseColorInfoOrNull.has_value()) {
  //     fastgltf::Image&       colorImage   = asset->images[baseColorInfoOrNull.value().textureIndex];
  //     fastgltf::sources::URI colorPath    = std::get<fastgltf::sources::URI>(colorImage.data);
  //     auto                   colorTexPath = parentPath / colorPath.uri.string();
  //
  //     auto vuRenderer.createImageFromAsset(colorTexPath,VkFormat::eR8G8B8A8Srgb);
  //
  //   }
  // }

  // Handle<VuTexture> colorTexture;
  // colorTexture.createHandle().init({colorTexPath, VK_FORMAT_R8G8B8A8_SRGB});
  // dstMaterialData.texture0 = colorTexture.index;

  // fastgltf::TextureInfo& normalTexInfo = material.normalTexture.value();
  // fastgltf::Image& normalImage = asset->images[normalTexInfo.textureIndex];
  // fastgltf::sources::URI normalPath = std::get<fastgltf::sources::URI>(normalImage.data);
  // auto normalAbsolutePath = parentPath / normalPath.uri.string();
  // Handle<VuTexture> normalTexture;
  // normalTexture.createHandle().init({normalAbsolutePath, VK_FORMAT_R8G8B8A8_UNORM});
  // dstMaterialData.texture1 = normalTexture.index;
  // dstMesh.vertexBuffer = vuDevice.createBuffer();

  // Indices
  if (!primitive.indicesAccessor.has_value()) {
    std::cout << "Primitive index accessor has not been set!" << "\n";
    return;
  }
  fastgltf::Accessor& indexAccessor = asset->accessors[primitive.indicesAccessor.value()];
  u32                 indexCount    = static_cast<u32>(indexAccessor.count);

  auto indexBufferOrErr = VuBuffer::make(vuRenderer.m_vuDevice,
                                         {.name         = "IndexBuffer",
                                          .sizeInBytes  = indexCount * sizeof(uint32_t),
                                          .vkUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT});
  THROW_if_unexpected(indexBufferOrErr);
  dstMesh.m_indexBuffer = std::make_shared<VuBuffer>(std::move(indexBufferOrErr.value()));
  // vuRenderer.registerToBindless( *dstMesh.indexBuffer);

  auto& indexBuffer = dstMesh.m_indexBuffer;

  indexBuffer->map();
  std::span<byte> indexSpanByte = indexBuffer->getMappedSpan(0, indexCount * sizeof(u32));
  std::span<u32>  indexSpan     = std::span(reinterpret_cast<u32*>(indexSpanByte.data()), indexCount);
  fastgltf::iterateAccessorWithIndex<u32>(
      asset.get(), indexAccessor, [&](u32 index, std::size_t idx) { indexSpan[idx] = index; });
  // indexBuffer->unmap();

  // Position
  fastgltf::Attribute* positionIt       = primitive.findAttribute("POSITION");
  fastgltf::Accessor&  positionAccessor = asset->accessors[positionIt->accessorIndex];

  dstMesh.m_vertexCount = positionAccessor.count;
  // vuDevice.createBuffer(
  //     {.name          = "VertexBuffer",
  //      .length        = dstMesh.vertexCount * dstMesh.totalAttributesSizePerVertex(),
  //      .strideInBytes = 1u,
  //      .vkUsageFlags  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT});
  // dstMesh.vertexBuffer = ;

  auto vertexBufferOrErr = VuBuffer::make(
      vuRenderer.m_vuDevice,
      {
          .name         = "VertexBuffer",
          .sizeInBytes  = dstMesh.m_vertexCount * VuMesh::totalAttributesSizePerVertex(),
          .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
      });
  THROW_if_unexpected(vertexBufferOrErr);
  dstMesh.m_vertexBuffer = std::make_shared<VuBuffer>(std::move(vertexBufferOrErr.value()));
  vuRenderer.registerToBindless(*dstMesh.m_vertexBuffer);

  VuBuffer* vertexBuffer = dstMesh.m_vertexBuffer.get();
  vertexBuffer->map();

  std::span<byte> vertexSpanByte =
      vertexBuffer->getMappedSpan(0, sizeof(fastgltf::math::f32vec3) * dstMesh.m_vertexCount);
  std::span<fastgltf::math::f32vec3> vertexSpan =
      std::span(reinterpret_cast<fastgltf::math::f32vec3*>(vertexSpanByte.data()), dstMesh.m_vertexCount);

  std::span<byte>                    normalSpanByte = vertexBuffer->getMappedSpan(dstMesh.getNormalOffsetAsByte(),
                                                               sizeof(fastgltf::math::f32vec3) * dstMesh.m_vertexCount);
  std::span<fastgltf::math::f32vec3> normalSpan =
      std::span(reinterpret_cast<fastgltf::math::f32vec3*>(normalSpanByte.data()), dstMesh.m_vertexCount);

  std::span<byte> tangentSpanByte = vertexBuffer->getMappedSpan(dstMesh.getTangentOffsetAsByte(),
                                                                sizeof(fastgltf::math::f32vec4) * dstMesh.m_vertexCount);

  std::span<fastgltf::math::f32vec4> tangentSpan =
      std::span(reinterpret_cast<fastgltf::math::f32vec4*>(tangentSpanByte.data()), dstMesh.m_vertexCount);

  std::span<byte> uvSpanByte =
      vertexBuffer->getMappedSpan(dstMesh.getUV_OffsetAsByte(), sizeof(fastgltf::math::f32vec2) * dstMesh.m_vertexCount);

  std::span<fastgltf::math::f32vec2> uvSpan =
      std::span(reinterpret_cast<fastgltf::math::f32vec2*>(uvSpanByte.data()), dstMesh.m_vertexCount);

  // pos
  {
    fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
        asset.get(), positionAccessor, [&](const fastgltf::math::f32vec3& pos, const std::size_t idx) {
          vertexSpan[idx] = pos;
        });
  }

  // normal
  {
    auto* normalIt       = primitive.findAttribute("NORMAL");
    auto& normalAccessor = asset->accessors[normalIt->accessorIndex];

    fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
        asset.get(), normalAccessor, [&](const fastgltf::math::f32vec3& normal, const std::size_t idx) {
          normalSpan[idx] = normal;
        });
  }
  // uv
  {
    fastgltf::Attribute* uvIter     = primitive.findAttribute("TEXCOORD_0");
    fastgltf::Accessor&  uvAccessor = asset->accessors[uvIter->accessorIndex];

    fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec2>(
        asset.get(), uvAccessor, [&](const fastgltf::math::f32vec2& uv, const std::size_t idx) { uvSpan[idx] = uv; });
  }

  // tangent
  {
    fastgltf::Attribute* tangentIt          = primitive.findAttribute("TANGENT");
    fastgltf::Accessor&  tangentAccessor    = asset->accessors[tangentIt->accessorIndex];
    std::size_t          tangentBufferIndex = tangentAccessor.bufferViewIndex.value();
    if (tangentBufferIndex == 0 && tangentAccessor.byteOffset == 0) {
      std::cout << "Gltf file has no tangents" << std::endl;

      auto pos  = rpCastSpan<fastgltf::math::f32vec3, float3>(vertexSpan);
      auto norm = rpCastSpan<fastgltf::math::f32vec3, float3>(normalSpan);
      auto uv   = rpCastSpan<fastgltf::math::f32vec2, float2>(uvSpan);
      auto tang = rpCastSpan<fastgltf::math::f32vec4, float4>(tangentSpan);

      VuMesh::calculateTangents(indexSpan, pos, norm, uv, tang);
    } else {
      fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec4>(
          asset.get(), tangentAccessor, [&](const fastgltf::math::f32vec4& tangent, const std::size_t idx) {
            tangentSpan[idx] = tangent;
          });
    }
  }

  vertexBuffer->unmap();
}
} // namespace Vu