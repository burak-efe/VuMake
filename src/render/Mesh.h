#pragma once

#include "Common.h"

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

    uint32 vertexCount;

    std::vector<uint32> indices;
    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<float2> uvs;


    Mesh(const std::filesystem::path& gltfPath);

    void Dispose();

    static std::array<VkVertexInputBindingDescription, 3> getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
