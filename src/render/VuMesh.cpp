#include "VuMesh.h"
#include "Common.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include "VuUtils.h"
#include <filesystem>

namespace Vu {
    VuMesh::VuMesh(const std::filesystem::path& gltfPath) {

        //vertexBuffer = 0;
        fastgltf::Parser parser;

        auto data = fastgltf::GltfDataBuffer::FromPath(gltfPath);

        if (data.error() != fastgltf::Error::None) {
            std::cout << "gltf file cannot be loaded!" << "\n";
        }

        auto asset = parser.loadGltf(
            data.get(),gltfPath.parent_path(), fastgltf::Options::LoadExternalBuffers);
        if (auto error = asset.error(); error != fastgltf::Error::None) {
            std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
        }


        auto mesh = asset.get().meshes.at(0);
        auto prims = mesh.primitives;

        auto primitive = prims.at(0);

        //Indices
        if (primitive.indicesAccessor.has_value()) {
            auto& accessor = asset->accessors[primitive.indicesAccessor.value()];
            indices.resize(accessor.count);


            std::size_t idx = 0;
            fastgltf::iterateAccessor<std::uint32_t>(asset.get(), accessor, [&](std::uint32_t index) {
                indices[idx++] = index;
            });
        }
        //indexBuffer = VuBuffer();
        indexBuffer.Alloc({
            .lenght = indices.size(),
            .strideInBytes = sizeof(indices[0]),
            .vkUsageFlags = VK_BUFFER_USAGE_INDEX_BUFFER_BIT
        });
        VkCheck(indexBuffer.SetData(indices.data(), indices.size() * sizeof(indices[0])));

        //Position
        auto* positionIt = primitive.findAttribute("POSITION");
        auto& positionAccessor = asset->accessors[positionIt->accessorIndex];
        auto bufferIndex = positionAccessor.bufferViewIndex.value();
        auto& bufferView = asset->bufferViews.at(bufferIndex);
        auto posBuffer = asset->buffers.at(bufferView.bufferIndex);

        vertices.resize(positionAccessor.count);
        fastgltf::iterateAccessorWithIndex<glm::vec3>(
            asset.get(), positionAccessor,
            [&](glm::vec3 pos, std::size_t idx) { vertices[idx] = pos; }
        );

        //vertexBuffer = VuBuffer();
        vertexBuffer.Alloc({
            .lenght = static_cast<uint32>(vertices.size()),
            .strideInBytes = sizeof(vertices[0]),
            .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        });
        VkCheck(vertexBuffer.SetData(vertices.data(), vertices.size() * sizeof(vertices[0])));

        //normal
        auto* normalIt = primitive.findAttribute("NORMAL");
        auto& normalAccessor = asset->accessors[normalIt->accessorIndex];
        auto normalbufferIndex = normalAccessor.bufferViewIndex.value();
        auto& normalbufferView = asset->bufferViews.at(normalbufferIndex);
        auto normalDataBuffer = asset->buffers.at(normalbufferView.bufferIndex);

        normals.resize(positionAccessor.count);
        fastgltf::iterateAccessorWithIndex<glm::vec3>(
            asset.get(), normalAccessor,
            [&](glm::vec3 normal, std::size_t idx) { normals[idx] = normal; }
        );

        normalBuffer.Alloc({
            .lenght = static_cast<uint32>(normals.size()),
            .strideInBytes = sizeof(normals[0]),
            .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        });
        VkCheck(normalBuffer.SetData(normals.data(), normals.size() * sizeof(normals[0])));

        //tangent
        auto* tangentIt = primitive.findAttribute("TANGENT");
        auto& tangentAccessor = asset->accessors[tangentIt->accessorIndex];
        auto tangentbufferIndex = tangentAccessor.bufferViewIndex.value();
        auto& tangentbufferView = asset->bufferViews.at(tangentbufferIndex);
        auto tangentDataBuffer = asset->buffers.at(tangentbufferView.bufferIndex);

        tangents.resize(positionAccessor.count);
        fastgltf::iterateAccessorWithIndex<glm::vec4>(
            asset.get(), tangentAccessor,
            [&](glm::vec4 tangent, std::size_t idx) { tangents[idx] = tangent; }
        );

        tangentBuffer.Alloc({
            .lenght = static_cast<uint32>(tangents.size()),
            .strideInBytes = sizeof(tangents[0]),
            .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        });
        VkCheck(tangentBuffer.SetData(tangents.data(), tangents.size() * sizeof(tangents[0])));


        //uv
        auto* uvIter = primitive.findAttribute("TEXCOORD_0");
        auto& uvAccessor = asset->accessors[uvIter->accessorIndex];
        auto uvbufferIndex = uvAccessor.bufferViewIndex.value();
        auto& uvbufferView = asset->bufferViews.at(uvbufferIndex);
        auto uvDataBuffer = asset->buffers.at(uvbufferView.bufferIndex);

        uvs.resize(positionAccessor.count);
        fastgltf::iterateAccessorWithIndex<glm::vec2>(
            asset.get(), uvAccessor,
            [&](glm::vec2 uv, std::size_t idx) { uvs[idx] = uv; }
        );

        uvBuffer.Alloc({
            .lenght = static_cast<uint32>(uvs.size()),
            .strideInBytes = sizeof(uvs[0]),
            .vkUsageFlags = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
        });
        VkCheck(uvBuffer.SetData(uvs.data(), uvs.size() * sizeof(uvs[0])));

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
        vertexBuffer.Dispose();
        indexBuffer.Dispose();
        normalBuffer.Dispose();
        tangentBuffer.Dispose();
        uvBuffer.Dispose();
    }
}
