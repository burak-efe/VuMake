#include "Mesh.h"
#include "Common.h"
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/glm_element_traits.hpp>
#include "VuUtils.h"
#include <filesystem>

Mesh::Mesh(const std::filesystem::path& gltfPath, VmaAllocator& allocator) {

    //vertexBuffer = 0;
    fastgltf::Parser parser;

    auto data = fastgltf::GltfDataBuffer::FromPath(gltfPath);

    if (data.error() != fastgltf::Error::None) {
        std::cout << "gltf file cannot be loaded!" << "\n";
    }

    auto asset = parser.loadGltf(data.get(), gltfPath.parent_path(), fastgltf::Options::None);
    if (auto error = asset.error(); error != fastgltf::Error::None) {
        std::cout << "Some error occurred while reading the buffer, parsing the JSON, or validating the data." << "\n";
    }


    auto mesh = asset.get().meshes.at(0);
    auto prims = mesh.primitives;

    auto primitive = prims.at(0);

    //Indices
    if (primitive.indicesAccessor.has_value()) {
        auto& accessor = asset->accessors[primitive.indicesAccessor.value()];
        Indices.resize(accessor.count);


        std::size_t idx = 0;
        fastgltf::iterateAccessor<std::uint32_t>(asset.get(), accessor, [&](std::uint32_t index) {
            Indices[idx++] = index;
        });
    }
    IndexBuffer = VuBuffer(allocator, static_cast<uint32>(Indices.size()), sizeof(Indices[0]), VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
    VK_CHECK(IndexBuffer.SetData(Indices.data(), Indices.size() * sizeof(Indices[0])));

    //Position
    auto* positionIt = primitive.findAttribute("POSITION");
    auto& positionAccessor = asset->accessors[positionIt->accessorIndex];
    auto bufferIndex = positionAccessor.bufferViewIndex.value();
    auto& bufferView = asset->bufferViews.at(bufferIndex);
    auto posBuffer = asset->buffers.at(bufferView.bufferIndex);

    Vertices.resize(positionAccessor.count);
    fastgltf::iterateAccessorWithIndex<glm::vec3>(
        asset.get(), positionAccessor,
        [&](glm::vec3 pos, std::size_t idx) { Vertices[idx] = pos; }
    );

    VertexBuffer = VuBuffer(allocator, static_cast<uint32>(Vertices.size()),
                            sizeof(Vertices[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VK_CHECK(VertexBuffer.SetData(Vertices.data(), Vertices.size() * sizeof(Vertices[0])));

    //normal
    auto* normalIt = primitive.findAttribute("NORMAL");

    auto& normalAccessor = asset->accessors[normalIt->accessorIndex];
    auto normalbufferIndex = normalAccessor.bufferViewIndex.value();
    auto& normalbufferView = asset->bufferViews.at(normalbufferIndex);
    auto normalDataBuffer = asset->buffers.at(normalbufferView.bufferIndex);

    Normals.resize(positionAccessor.count);
    fastgltf::iterateAccessorWithIndex<glm::vec3>(
        asset.get(), normalAccessor,
        [&](glm::vec3 normal, std::size_t idx) { Normals[idx] = normal; }
    );

    NormalBuffer = VuBuffer(allocator, static_cast<uint32>(Normals.size()),
                            sizeof(Normals[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VK_CHECK(NormalBuffer.SetData(Normals.data(), Normals.size() * sizeof(Normals[0])));


    //normal
    auto* uvIter = primitive.findAttribute("TEXCOORD_0");
    auto& uvAccessor = asset->accessors[uvIter->accessorIndex];
    auto uvbufferIndex = uvAccessor.bufferViewIndex.value();
    auto& uvbufferView = asset->bufferViews.at(uvbufferIndex);
    auto uvDataBuffer = asset->buffers.at(uvbufferView.bufferIndex);

    Uvs.resize(positionAccessor.count);
    fastgltf::iterateAccessorWithIndex<glm::vec2>(
        asset.get(), uvAccessor,
        [&](glm::vec2 uv, std::size_t idx) { Uvs[idx] = uv; }
    );

    UvBuffer = VuBuffer(allocator, static_cast<uint32>(Uvs.size()), sizeof(Uvs[0]), VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
    VK_CHECK(UvBuffer.SetData(Uvs.data(), Uvs.size() * sizeof(Uvs[0])));

}

std::array<VkVertexInputBindingDescription, 3> Mesh::getBindingDescription() {
    std::array<VkVertexInputBindingDescription, 3> bindingDescriptions{};
    bindingDescriptions[0].binding = 0;
    bindingDescriptions[0].stride = sizeof(glm::vec3);
    bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


    bindingDescriptions[1].binding = 1;
    bindingDescriptions[1].stride = sizeof(glm::vec3);
    bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    bindingDescriptions[2].binding = 2;
    bindingDescriptions[2].stride = sizeof(glm::vec2);
    bindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescriptions;
}

std::array<VkVertexInputAttributeDescription, 3> Mesh::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
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
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = 0;

    return attributeDescriptions;
}

void Mesh::Dispose() {
    VertexBuffer.Dispose();
    IndexBuffer.Dispose();
    NormalBuffer.Dispose();
    UvBuffer.Dispose();
}
