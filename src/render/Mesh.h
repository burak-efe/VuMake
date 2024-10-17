#pragma once

#include "Common.h"

#include "VuBuffer.h"

namespace std::filesystem {
    class path;
}

class Mesh {
public:
    VuBuffer IndexBuffer;
    VuBuffer VertexBuffer;
    VuBuffer NormalBuffer;
    VuBuffer UvBuffer;

    uint32 VertexCount;

    std::vector<uint32> Indices;
    std::vector<float3> Vertices;
    std::vector<float3> Normals;
    std::vector<float2> Uvs;


    Mesh(const std::filesystem::path& gltfPath);

    void Dispose();

    static std::array<VkVertexInputBindingDescription, 3> getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();
};
