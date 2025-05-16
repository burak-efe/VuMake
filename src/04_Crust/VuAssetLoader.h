#pragma once

#include <03_Mantle/VuDevice.h>
#include <filesystem>
#include <iostream>

#include "02_OuterCore/CollectionUtils.h"
#include "02_OuterCore/Common.h"
#include "03_Mantle/VuBuffer.h"
#include "fastgltf/core.hpp"
#include "fastgltf/tools.hpp"
#include "VuMesh.h"

namespace Vu {
struct GPU_PBR_MaterialData;

struct VuAssetLoader {
  static void
  LoadGltf(VuDevice& vuDevice, const std::filesystem::path& gltfPath, VuMesh& dstMesh) {
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

    auto parentPath = gltfPath.parent_path();

    fastgltf::Optional<size_t> matIndex = primitive.materialIndex;

    // if (matIndex.has_value())
    // {
    //     fastgltf::Material&    material     = asset->materials.at(matIndex.value());
    //     fastgltf::TextureInfo& colorTexInfo = material.pbrData.baseColorTexture.value();
    //     fastgltf::Image&       colorImage   = asset->images[colorTexInfo.textureIndex];
    //     fastgltf::sources::URI colorPath    = std::get<fastgltf::sources::URI>(colorImage.data);
    //     auto                   colorTexPath = parentPath / colorPath.uri.string();
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

    auto indexBufferOrErr = VuBuffer::make(vuDevice, {.name         = "IndexBuffer",
                                                      .sizeInBytes  = indexCount * sizeof(uint32_t),
                                                      .vkUsageFlags = vk::BufferUsageFlagBits::eIndexBuffer});
    throw_if_unexpected(indexBufferOrErr);
    dstMesh.indexBuffer = std::make_shared<VuBuffer>(std::move(indexBufferOrErr.value()));

    auto* indexBuffer = dstMesh.indexBuffer.get();

    indexBuffer->map();
    std::span<byte> indexSpanByte = indexBuffer->getMappedSpan(0, indexCount * sizeof(u32));
    std::span<u32>  indexSpan     = std::span(reinterpret_cast<u32*>(indexSpanByte.data()), indexCount);
    fastgltf::iterateAccessorWithIndex<u32>(asset.get(), indexAccessor,
                                            [&](u32 index, std::size_t idx) { indexSpan[idx] = index; });
    indexBuffer->unmap();

    // Position
    fastgltf::Attribute* positionIt       = primitive.findAttribute("POSITION");
    fastgltf::Accessor&  positionAccessor = asset->accessors[positionIt->accessorIndex];

    dstMesh.vertexCount = positionAccessor.count;
    // vuDevice.createBuffer(
    //     {.name          = "VertexBuffer",
    //      .length        = dstMesh.vertexCount * dstMesh.totalAttributesSizePerVertex(),
    //      .strideInBytes = 1u,
    //      .vkUsageFlags  = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT});
    // dstMesh.vertexBuffer = ;

    auto vertexBufferOrErr =
        VuBuffer::make(vuDevice, {
                                     .name         = "VertexBuffer",
                                     .sizeInBytes  = dstMesh.vertexCount * VuMesh::totalAttributesSizePerVertex(),
                                     .vkUsageFlags = vk::BufferUsageFlagBits::eVertexBuffer |
                                                     vk::BufferUsageFlagBits::eShaderDeviceAddress,
                                 });
    throw_if_unexpected(vertexBufferOrErr);
    dstMesh.vertexBuffer = std::make_shared<VuBuffer>(std::move(vertexBufferOrErr.value()));

    VuBuffer* vertexBuffer = dstMesh.vertexBuffer.get();
    vertexBuffer->map();

    std::span<byte> vertexSpanByte =
        vertexBuffer->getMappedSpan(0, sizeof(fastgltf::math::f32vec3) * dstMesh.vertexCount);
    std::span<fastgltf::math::f32vec3> vertexSpan =
        std::span(reinterpret_cast<fastgltf::math::f32vec3*>(vertexSpanByte.data()), dstMesh.vertexCount);

    std::span<byte>                    normalSpanByte = vertexBuffer->getMappedSpan(dstMesh.getNormalOffsetAsByte(),
                                                                                    sizeof(fastgltf::math::f32vec3) * dstMesh.vertexCount);
    std::span<fastgltf::math::f32vec3> normalSpan =
        std::span(reinterpret_cast<fastgltf::math::f32vec3*>(normalSpanByte.data()), dstMesh.vertexCount);

    std::span<byte> tangentSpanByte = vertexBuffer->getMappedSpan(
        dstMesh.getTangentOffsetAsByte(), sizeof(fastgltf::math::f32vec4) * dstMesh.vertexCount);

    std::span<fastgltf::math::f32vec4> tangentSpan =
        std::span(reinterpret_cast<fastgltf::math::f32vec4*>(tangentSpanByte.data()), dstMesh.vertexCount);

    std::span<byte> uvSpanByte = vertexBuffer->getMappedSpan(dstMesh.getUV_OffsetAsByte(),
                                                             sizeof(fastgltf::math::f32vec2) * dstMesh.vertexCount);

    std::span<fastgltf::math::f32vec2> uvSpan =
        std::span(reinterpret_cast<fastgltf::math::f32vec2*>(uvSpanByte.data()), dstMesh.vertexCount);

    // pos
    {
      fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
          asset.get(), positionAccessor,
          [&](const fastgltf::math::f32vec3& pos, const std::size_t idx) { vertexSpan[idx] = pos; });
    }

    // normal
    {
      auto* normalIt       = primitive.findAttribute("NORMAL");
      auto& normalAccessor = asset->accessors[normalIt->accessorIndex];

      fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
          asset.get(), normalAccessor,
          [&](const fastgltf::math::f32vec3& normal, const std::size_t idx) { normalSpan[idx] = normal; });
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

        auto pos  = rpCastSpan<fastgltf::math::f32vec3, vec3>(vertexSpan);
        auto norm = rpCastSpan<fastgltf::math::f32vec3, vec3>(normalSpan);
        auto uv   = rpCastSpan<fastgltf::math::f32vec2, vec2>(uvSpan);
        auto tang = rpCastSpan<fastgltf::math::f32vec4, vec4>(tangentSpan);

        VuMesh::calculateTangents(indexSpan, pos, norm, uv, tang);
      } else {
        fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec4>(
            asset.get(), tangentAccessor,
            [&](const fastgltf::math::f32vec4& tangent, const std::size_t idx) { tangentSpan[idx] = tangent; });
      }
    }

    vertexBuffer->unmap();
  }
};
} // namespace Vu
