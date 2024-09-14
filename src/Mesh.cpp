#include "Mesh.h"
#include <iostream>
#include <fastgltf/core.hpp>
#include <fastgltf/tools.hpp>
#include <fastgltf/util.hpp>
#include <fastgltf/math.hpp>
#include <fastgltf/glm_element_traits.hpp>

#include "VuUtils.h"

Mesh::Mesh(std::filesystem::path gltfPath, VmaAllocator allocator) {
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
        auto &accessor = asset->accessors[primitive.indicesAccessor.value()];
        indices.resize(accessor.count);


        std::size_t idx = 0;
        fastgltf::iterateAccessor<std::uint32_t>(asset.get(), accessor, [&](std::uint32_t index) {
            indices[idx++] = index;
        });
    }
    VK_CHECK(indexBuffer.Init(
        allocator, static_cast<uint32>(indices.size()), sizeof(indices[0]),VK_BUFFER_USAGE_INDEX_BUFFER_BIT));
    VK_CHECK(indexBuffer.SetData(indices.data(), indices.size() * sizeof(indices[0])));

    // Vertex Position
    auto *positionIt = primitive.findAttribute("POSITION");
    auto &positionAccessor = asset->accessors[positionIt->second];
    auto bufferIndex = positionAccessor.bufferViewIndex.value();
    auto &bufferView = asset->bufferViews.at(bufferIndex);
    auto posBuffer = asset->buffers.at(bufferView.bufferIndex);

    vertices.resize(positionAccessor.count);
    fastgltf::iterateAccessorWithIndex<glm::vec3>(
        asset.get(), positionAccessor,
        [&](glm::vec3 pos, std::size_t idx) { vertices[idx] = pos; }
    );

    VK_CHECK(vertexBuffer.Init(
    allocator, static_cast<uint32>(vertices.size()), sizeof(vertices[0]),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));

    VK_CHECK(vertexBuffer.SetData(vertices.data(), vertices.size() * sizeof(vertices[0])));

    //normals
    auto *normalIt = primitive.findAttribute("NORMAL");
    auto &normalAccessor = asset->accessors[normalIt->second];
    auto normalbufferIndex = normalAccessor.bufferViewIndex.value();
    auto &normalbufferView = asset->bufferViews.at(normalbufferIndex);
    auto normalDataBuffer = asset->buffers.at(normalbufferView.bufferIndex);

    normals.resize(positionAccessor.count);
    fastgltf::iterateAccessorWithIndex<glm::vec3>(
        asset.get(), normalAccessor,
        [&](glm::vec3 normal, std::size_t idx) { normals[idx] = normal; }
    );

    VK_CHECK(normalBuffer.Init(
    allocator, static_cast<uint32>(normals.size()), sizeof(normals[0]),VK_BUFFER_USAGE_VERTEX_BUFFER_BIT));

    VK_CHECK(normalBuffer.SetData(normals.data(), normals.size() * sizeof(normals[0])));


}

void Mesh::Dispose() {
    vertexBuffer.Dispose();
    indexBuffer.Dispose();
    normalBuffer.Dispose();
}
