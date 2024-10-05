#pragma once

#include "Common.h"
#include <array>
#include <vector>
#include "VuBuffer.h"

namespace std::filesystem {
    class path;
}

class Mesh {
public:
    VuBuffer indexBuffer;
    VuBuffer vertexBuffer;
    VuBuffer normalBuffer;
    VuBuffer uvBuffer;

    uint32 VertexCount;

    std::vector<uint32> indices;
    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<float2> uvs;


    Mesh(const std::filesystem::path& gltfPath, VmaAllocator& allocator);

    static std::array<VkVertexInputBindingDescription, 3> getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    void Dispose();


};
