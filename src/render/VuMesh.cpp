#include "VuMesh.h"
#include "Common.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include "VuUtils.h"
#include <filesystem>

namespace Vu {
    void VuMesh::init() {
        //
        // //vertexBuffer = 0;
        // fastgltf::Parser parser;
        //
        // auto data = fastgltf::GltfDataBuffer::FromPath(gltfPath);
        //
        // if (data.error() != fastgltf::Error::None) {
        //     std::cout << "gltf file cannot be loaded!" << "\n";
        // }
        //
        // auto asset = parser.loadGltf(
        //     data.get(), gltfPath.parent_path(), fastgltf::Options::LoadExternalBuffers);
        // if (auto error = asset.error(); error != fastgltf::Error::None) {
        //     std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
        // }
        //
        // auto mesh = asset.get().meshes.at(0);
        // auto prims = mesh.primitives;
        // auto primitive = prims.at(0);
        //
        // fastgltf::Optional<size_t> matIndex = primitive.materialIndex;
        // fastgltf::Material& material = asset->materials.at(matIndex.value());
        // fastgltf::TextureInfo& colorTex = material.pbrData.baseColorTexture.value();
        // auto& image = asset->images[colorTex.textureIndex];
        //
        //
        // //Indices
        // if (!primitive.indicesAccessor.has_value()) {
        //     std::cout << "Primitive index accessor has not been set!" << "\n";
        //     return;
        // }
        // auto& indexAccesor = asset->accessors[primitive.indicesAccessor.value()];
        // uint32 indexCount = indexAccesor.count;
        // indexBuffer.Alloc({
        //     .lenght = indexCount,
        //     .strideInBytes = sizeof(uint32),
        //     .vkUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        // });
        // indexBuffer.Map();
        // auto indexSpanByte = indexBuffer.getSpan(0, indexCount * sizeof(uint32));
        // std::span<uint32> indexSpan = std::span(reinterpret_cast<uint32 *>(indexSpanByte.data()), indexCount);
        // fastgltf::iterateAccessorWithIndex<uint32>(
        //     asset.get(), indexAccesor,
        //     [&](uint32 index, std::size_t idx) { indexSpan[idx] = index; }
        // );
        // indexBuffer.Unmap();
        //
        //
        // //Position
        // fastgltf::Attribute* positionIt = primitive.findAttribute("POSITION");
        // fastgltf::Accessor& positionAccessor = asset->accessors[positionIt->accessorIndex];
        //
        // vertexCount = positionAccessor.count;
        // vertexBuffer.Alloc({
        //     .lenght = vertexCount * totalAttributesSizePerVertex(),
        //     .strideInBytes = 1,
        //     .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        // });
        // vertexBuffer.Map();
        //
        // std::span<uint8> vertexSpanByte = vertexBuffer.getSpan(0, sizeof(float3) * vertexCount);
        // std::span<float3> vertexSpan = std::span(reinterpret_cast<float3 *>(vertexSpanByte.data()), vertexCount);
        //
        // std::span<uint8> normalSpanByte = vertexBuffer.getSpan(getNormalOffsetAsByte(), sizeof(float3) * vertexCount);
        // std::span<float3> normalSpan = std::span(reinterpret_cast<float3 *>(normalSpanByte.data()), vertexCount);
        //
        // std::span<uint8> tangentSpanByte = vertexBuffer.getSpan(getTangentOffsetAsByte(), sizeof(float4) * vertexCount);
        // std::span<float4> tangentSpan = std::span(reinterpret_cast<float4 *>(tangentSpanByte.data()), vertexCount);
        //
        // std::span<uint8> uvSpanByte = vertexBuffer.getSpan(getUV_OffsetAsByte(), sizeof(float2) * vertexCount);
        // std::span<float2> uvSpan = std::span(reinterpret_cast<float2 *>(uvSpanByte.data()), vertexCount);
        //
        // fastgltf::iterateAccessorWithIndex<glm::vec3>(
        //     asset.get(), positionAccessor,
        //     [&](glm::vec3 pos, std::size_t idx) { vertexSpan[idx] = pos; }
        // );
        //
        // //normal
        // auto* normalIt = primitive.findAttribute("NORMAL");
        // auto& normalAccessor = asset->accessors[normalIt->accessorIndex];
        //
        // fastgltf::iterateAccessorWithIndex<glm::vec3>(
        //     asset.get(), normalAccessor,
        //     [&](glm::vec3 normal, std::size_t idx) { normalSpan[idx] = normal; }
        // );
        //
        // //uv
        // auto* uvIter = primitive.findAttribute("TEXCOORD_0");
        // auto& uvAccessor = asset->accessors[uvIter->accessorIndex];
        //
        // fastgltf::iterateAccessorWithIndex<glm::vec2>(
        //     asset.get(), uvAccessor,
        //     [&](glm::vec2 uv, std::size_t idx) { uvSpan[idx] = uv; }
        // );
        //
        //
        // //tangent
        // auto* tangentIt = primitive.findAttribute("TANGENT");
        // fastgltf::Accessor& tangentAccessor = asset->accessors[tangentIt->accessorIndex];
        // auto tangentbufferIndex = tangentAccessor.bufferViewIndex.value();
        // if (tangentbufferIndex == 0 && tangentAccessor.byteOffset == 0) {
        //     std::cout << "Gltf file has no tangents" << std::endl;
        //
        //     calculateTangents(indexSpan, vertexSpan, normalSpan, uvSpan, tangentSpan);
        //     // for (float4& tan: tangentSpan) {
        //     //     tan = float4(1, 0, 0, 1);
        //     // }
        //
        // } else {
        //     fastgltf::iterateAccessorWithIndex<float4>(
        //         asset.get(), tangentAccessor,
        //         [&](float4 tangent, std::size_t idx) { tangentSpan[idx] = tangent; }
        //     );
        // }
        //
        //
        // vertexBuffer.Unmap();
    }

    std::array<VkVertexInputBindingDescription, 4> VuMesh::getBindingDescription() {
        std::array<VkVertexInputBindingDescription, 4> bindingDescriptions{};
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(glm::vec3);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(glm::vec3);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[2].binding = 2;
        bindingDescriptions[2].stride = sizeof(glm::vec4);
        bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        bindingDescriptions[3].binding = 3;
        bindingDescriptions[3].stride = sizeof(glm::vec2);
        bindingDescriptions[3].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
    }

    std::array<VkVertexInputAttributeDescription, 4> VuMesh::getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 4> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = 0;

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = 0;

        attributeDescriptions[2].binding = 2;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32B32A32_SFLOAT;
        attributeDescriptions[2].offset = 0;

        attributeDescriptions[3].binding = 3;
        attributeDescriptions[3].location = 3;
        attributeDescriptions[3].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[3].offset = 0;

        return attributeDescriptions;
    }

    void VuMesh::Dispose() {
        vertexBuffer.uninit();
        indexBuffer.uninit();
        //normalBuffer.Dispose();
        //tangentBuffer.Dispose();
        //uvBuffer.Dispose();
    }
}
