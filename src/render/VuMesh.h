#pragma once

#include "Common.h"
#include "VuBuffer.h"

namespace std::filesystem {
    class path;
}

namespace Vu {
    struct VuMesh {
        VuBuffer indexBuffer;
        VuBuffer vertexBuffer;
        VuBuffer normalBuffer;
        VuBuffer tangentBuffer;
        VuBuffer uvBuffer;

        uint32 vertexCount;

        std::vector<uint32> indices;
        std::vector<float3> vertices;
        std::vector<float3> normals;
        std::vector<float4> tangents;
        std::vector<float2> uvs;


        VuMesh(const std::filesystem::path& gltfPath);

        void Dispose();

        static std::array<VkVertexInputBindingDescription, 4> getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();
    };
}
