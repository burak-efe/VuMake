#pragma once

#include <filesystem>
#include <iostream>

#include "Common.h"
#include "VuMesh.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "VuGlobalSetManager.h"
#include "VuUtils.h"

namespace Vu {
    struct VuAssetLoader {
        static void LoadGltf(std::filesystem::path path, VuMesh& dstMesh, PBRMaterialData& dstMaterialData) {

            fastgltf::Parser parser;

            auto data = fastgltf::GltfDataBuffer::FromPath(path);

            if (data.error() != fastgltf::Error::None) {
                std::cout << "gltf file cannot be loaded!" << "\n";
            }

            auto asset = parser.loadGltf(
                data.get(), path.parent_path(), fastgltf::Options::LoadExternalBuffers);
            if (auto error = asset.error(); error != fastgltf::Error::None) {
                std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
            }

            auto mesh = asset.get().meshes.at(0);
            auto prims = mesh.primitives;
            auto primitive = prims.at(0);

            auto parentPath = path.parent_path();

            fastgltf::Optional<size_t> matIndex = primitive.materialIndex;
            fastgltf::Material& material = asset->materials.at(matIndex.value());
            fastgltf::TextureInfo& colorTexInfo = material.pbrData.baseColorTexture.value();
            fastgltf::Image& colorImage = asset->images[colorTexInfo.textureIndex];
            fastgltf::sources::URI colorPath = std::get<fastgltf::sources::URI>(colorImage.data);
            auto colorTexPath = parentPath / colorPath.uri.string();
            VuTexture colorTexture;
            colorTexture.init(colorTexPath,VK_FORMAT_R8G8B8A8_SRGB);
            dstMaterialData.texture0 = VuGlobalSetManager::registerTexture(colorTexture);


            fastgltf::TextureInfo& normalTexInfo = material.normalTexture.value();
            fastgltf::Image& normalImage = asset->images[normalTexInfo.textureIndex];
            fastgltf::sources::URI normalPath = std::get<fastgltf::sources::URI>(normalImage.data);
            auto normalAbsolutePath = parentPath / normalPath.uri.string();
            VuTexture normalTexture;
            normalTexture.init(normalAbsolutePath,VK_FORMAT_R8G8B8A8_UNORM);
            dstMaterialData.texture1 = VuGlobalSetManager::registerTexture(normalTexture);

            //Indices
            if (!primitive.indicesAccessor.has_value()) {
                std::cout << "Primitive index accessor has not been set!" << "\n";
                return;
            }
            auto& indexAccesor = asset->accessors[primitive.indicesAccessor.value()];
            uint32 indexCount = indexAccesor.count;
            dstMesh.indexBuffer.Alloc({
                .lenght = indexCount,
                .strideInBytes = sizeof(uint32),
                .vkUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            });
            dstMesh.indexBuffer.Map();
            auto indexSpanByte = dstMesh.indexBuffer.getSpan(0, indexCount * sizeof(uint32));
            std::span<uint32> indexSpan = std::span(reinterpret_cast<uint32 *>(indexSpanByte.data()), indexCount);
            fastgltf::iterateAccessorWithIndex<uint32>(
                asset.get(), indexAccesor,
                [&](uint32 index, std::size_t idx) { indexSpan[idx] = index; }
            );
            dstMesh.indexBuffer.Unmap();


            //Position
            fastgltf::Attribute* positionIt = primitive.findAttribute("POSITION");
            fastgltf::Accessor& positionAccessor = asset->accessors[positionIt->accessorIndex];

            dstMesh.vertexCount = positionAccessor.count;
            dstMesh.vertexBuffer.Alloc({
                .lenght = dstMesh.vertexCount * dstMesh.totalAttributesSizePerVertex(),
                .strideInBytes = 1,
                .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
            });
            dstMesh.vertexBuffer.Map();

            std::span<uint8> vertexSpanByte = dstMesh.vertexBuffer.getSpan(0, sizeof(float3) * dstMesh.vertexCount);
            std::span<float3> vertexSpan = std::span(reinterpret_cast<float3 *>(vertexSpanByte.data()), dstMesh.vertexCount);

            std::span<uint8> normalSpanByte = dstMesh.vertexBuffer.getSpan(dstMesh.getNormalOffsetAsByte(),
                                                                           sizeof(float3) * dstMesh.vertexCount);
            std::span<float3> normalSpan = std::span(reinterpret_cast<float3 *>(normalSpanByte.data()), dstMesh.vertexCount);

            std::span<uint8> tangentSpanByte = dstMesh.vertexBuffer.getSpan(dstMesh.getTangentOffsetAsByte(),
                                                                            sizeof(float4) * dstMesh.vertexCount);
            std::span<float4> tangentSpan = std::span(reinterpret_cast<float4 *>(tangentSpanByte.data()), dstMesh.vertexCount);

            std::span<uint8> uvSpanByte = dstMesh.vertexBuffer.getSpan(dstMesh.getUV_OffsetAsByte(), sizeof(float2) * dstMesh.vertexCount);
            std::span<float2> uvSpan = std::span(reinterpret_cast<float2 *>(uvSpanByte.data()), dstMesh.vertexCount);

            fastgltf::iterateAccessorWithIndex<glm::vec3>(
                asset.get(), positionAccessor,
                [&](glm::vec3 pos, std::size_t idx) { vertexSpan[idx] = pos; }
            );

            //normal
            auto* normalIt = primitive.findAttribute("NORMAL");
            auto& normalAccessor = asset->accessors[normalIt->accessorIndex];

            fastgltf::iterateAccessorWithIndex<glm::vec3>(
                asset.get(), normalAccessor,
                [&](glm::vec3 normal, std::size_t idx) { normalSpan[idx] = normal; }
            );

            //uv
            auto* uvIter = primitive.findAttribute("TEXCOORD_0");
            auto& uvAccessor = asset->accessors[uvIter->accessorIndex];

            fastgltf::iterateAccessorWithIndex<glm::vec2>(
                asset.get(), uvAccessor,
                [&](glm::vec2 uv, std::size_t idx) { uvSpan[idx] = uv; }
            );


            //tangent
            auto* tangentIt = primitive.findAttribute("TANGENT");
            fastgltf::Accessor& tangentAccessor = asset->accessors[tangentIt->accessorIndex];
            auto tangentbufferIndex = tangentAccessor.bufferViewIndex.value();
            if (tangentbufferIndex == 0 && tangentAccessor.byteOffset == 0) {
                std::cout << "Gltf file has no tangents" << std::endl;

                dstMesh.calculateTangents(indexSpan, vertexSpan, normalSpan, uvSpan, tangentSpan);

            } else {
                fastgltf::iterateAccessorWithIndex<float4>(
                    asset.get(), tangentAccessor,
                    [&](float4 tangent, std::size_t idx) { tangentSpan[idx] = tangent; }
                );
            }


            dstMesh.vertexBuffer.Unmap();
        }
    };
}
