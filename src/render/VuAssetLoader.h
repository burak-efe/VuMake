#pragma once

#include <filesystem>
#include <iostream>

#include "Common.h"
#include "VuMesh.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/math.hpp>
//#include <fastgltf/glm_element_traits.hpp>

#include "VuResourceManager.h"

namespace Vu {

    template<typename T>
    struct VuAssetRef {
        const char* path;
        VkBool32    isLoaded;
        VuHandle<T> handle;
    };


    struct VuAssetLoader {

        //inline static std::vector<const char*>

        //TODO
        static void LoadGltf(std::filesystem::path path, VuMesh& dstMesh, GPU_PBR_MaterialData& dstMaterialData) {
            ZoneScoped;

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

            auto mesh      = asset.get().meshes.at(0);
            auto prims     = mesh.primitives;
            auto primitive = prims.at(0);

            auto parentPath = path.parent_path();

            fastgltf::Optional<size_t> matIndex     = primitive.materialIndex;
            fastgltf::Material&        material     = asset->materials.at(matIndex.value());
            fastgltf::TextureInfo&     colorTexInfo = material.pbrData.baseColorTexture.value();
            fastgltf::Image&           colorImage   = asset->images[colorTexInfo.textureIndex];
            fastgltf::sources::URI     colorPath    = std::get<fastgltf::sources::URI>(colorImage.data);
            auto                       colorTexPath = parentPath / colorPath.uri.string();

            // Handle<VuTexture> colorTexture; //TODO: leak
            // colorTexture.createHandle().init({colorTexPath, VK_FORMAT_R8G8B8A8_SRGB});
            // dstMaterialData.texture0 = colorTexture.index;


            // fastgltf::TextureInfo& normalTexInfo = material.normalTexture.value();
            // fastgltf::Image& normalImage = asset->images[normalTexInfo.textureIndex];
            // fastgltf::sources::URI normalPath = std::get<fastgltf::sources::URI>(normalImage.data);
            // auto normalAbsolutePath = parentPath / normalPath.uri.string();
            // Handle<VuTexture> normalTexture;
            // normalTexture.createHandle().init({normalAbsolutePath, VK_FORMAT_R8G8B8A8_UNORM});
            // dstMaterialData.texture1 = normalTexture.index;

            dstMesh.vertexBuffer.createHandle();
            dstMesh.indexBuffer.createHandle();


            //Indices

            if (!primitive.indicesAccessor.has_value()) {
                std::cout << "Primitive index accessor has not been set!" << "\n";
                return;
            }
            auto&  indexAccesor = asset->accessors[primitive.indicesAccessor.value()];
            uint32 indexCount   = indexAccesor.count;
            dstMesh.indexBuffer.get()->init({
                .lenght = indexCount,
                .strideInBytes = sizeof(uint32),
                .vkUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
            });
            dstMesh.indexBuffer.get()->map();
            auto              indexSpanByte = dstMesh.indexBuffer.get()->getMappedSpan(0, indexCount * sizeof(uint32));
            std::span<uint32> indexSpan     = std::span(reinterpret_cast<uint32 *>(indexSpanByte.data()), indexCount);
            fastgltf::iterateAccessorWithIndex<uint32>(
                asset.get(), indexAccesor,
                [&](uint32 index, std::size_t idx) { indexSpan[idx] = index; }
            );
            dstMesh.indexBuffer.get()->unmap();


            //Position
            fastgltf::Attribute* positionIt       = primitive.findAttribute("POSITION");
            fastgltf::Accessor&  positionAccessor = asset->accessors[positionIt->accessorIndex];

            dstMesh.vertexCount = positionAccessor.count;
            dstMesh.vertexBuffer.get()->init({
                .lenght = dstMesh.vertexCount * dstMesh.totalAttributesSizePerVertex(),
                .strideInBytes = 1,
                .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            });
            VuResourceManager::registerStorageBuffer(dstMesh.vertexBuffer.index, *dstMesh.vertexBuffer.get());
            dstMesh.vertexBuffer.get()->map();

            std::span<uint8> vertexSpanByte =
                    dstMesh.vertexBuffer.get()->getMappedSpan(0, sizeof(fastgltf::math::f32vec3) * dstMesh.vertexCount);
            std::span<fastgltf::math::f32vec3> vertexSpan =
                    std::span(reinterpret_cast<fastgltf::math::f32vec3 *>(vertexSpanByte.data()), dstMesh.vertexCount);


            std::span<uint8> normalSpanByte =
                    dstMesh.vertexBuffer.get()->getMappedSpan(dstMesh.getNormalOffsetAsByte(), sizeof(fastgltf::math::f32vec3) * dstMesh.vertexCount);
            std::span<fastgltf::math::f32vec3> normalSpan =
                    std::span(reinterpret_cast<fastgltf::math::f32vec3 *>(normalSpanByte.data()), dstMesh.vertexCount);


            std::span<uint8> tangentSpanByte =
                    dstMesh.vertexBuffer.get()->getMappedSpan(dstMesh.getTangentOffsetAsByte(), sizeof(fastgltf::math::f32vec4) * dstMesh.vertexCount);
            std::span<fastgltf::math::f32vec4> tangentSpan =
                    std::span(reinterpret_cast<fastgltf::math::f32vec4 *>(tangentSpanByte.data()), dstMesh.vertexCount);


            std::span<uint8> uvSpanByte =
                    dstMesh.vertexBuffer.get()->getMappedSpan(dstMesh.getUV_OffsetAsByte(), sizeof(fastgltf::math::f32vec2) * dstMesh.vertexCount);
            std::span<fastgltf::math::f32vec2> uvSpan =
                    std::span(reinterpret_cast<fastgltf::math::f32vec2 *>(uvSpanByte.data()), dstMesh.vertexCount);

            //pos
            {
                ZoneScopedN("Positions");
                fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
                    asset.get(), positionAccessor,
                    [&](fastgltf::math::f32vec3 pos, std::size_t idx) { vertexSpan[idx] = pos; }
                );
            }

            //normal
            {
                ZoneScopedN("Normals");
                auto* normalIt       = primitive.findAttribute("NORMAL");
                auto& normalAccessor = asset->accessors[normalIt->accessorIndex];

                fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec3>(
                    asset.get(), normalAccessor,
                    [&](fastgltf::math::f32vec3 normal, std::size_t idx) { normalSpan[idx] = normal; }
                );

            }
            //uv
            {
                ZoneScopedN("UVs");
                auto* uvIter     = primitive.findAttribute("TEXCOORD_0");
                auto& uvAccessor = asset->accessors[uvIter->accessorIndex];

                fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec2>(
                    asset.get(), uvAccessor,
                    [&](fastgltf::math::f32vec2 uv, std::size_t idx) { uvSpan[idx] = uv; }
                );
            }

            //tangent
            {
                ZoneScopedN("Tangents");
                auto*               tangentIt          = primitive.findAttribute("TANGENT");
                fastgltf::Accessor& tangentAccessor    = asset->accessors[tangentIt->accessorIndex];
                auto                tangentbufferIndex = tangentAccessor.bufferViewIndex.value();
                if (tangentbufferIndex == 0 && tangentAccessor.byteOffset == 0) {
                    std::cout << "Gltf file has no tangents" << std::endl;

                    auto pos = rpCastSpan<fastgltf::math::f32vec3, float3>(vertexSpan);
                    auto norm = rpCastSpan<fastgltf::math::f32vec3, float3>(normalSpan);
                    auto uv = rpCastSpan<fastgltf::math::f32vec2, float2>(uvSpan);
                    auto tang = rpCastSpan<fastgltf::math::f32vec4, float4>(tangentSpan);

                    dstMesh.calculateTangents(indexSpan, pos, norm, uv, tang);

                } else {
                    fastgltf::iterateAccessorWithIndex<fastgltf::math::f32vec4>(
                        asset.get(), tangentAccessor,
                        [&](fastgltf::math::f32vec4 tangent, std::size_t idx) { tangentSpan[idx] = tangent; }
                    );
                }
            }


            dstMesh.vertexBuffer.get()->unmap();
        }
    };
}
