#pragma once

#include <array>
#include <vector>
#include <vulkan/vulkan.h>
#include <vk_mem_alloc.h>
#include "Common.h"
#include "VuBuffer.h"

namespace std::filesystem {
    class path;
}

//#include <filesystem>

class Mesh {
public:
    uint32 VertexCount;
    std::vector<uint32> indices;
    std::vector<float3> vertices;
    std::vector<float3> normals;
    std::vector<float2> uvs;

    VuBuffer vertexBuffer;
    VuBuffer normalBuffer;
    VuBuffer uvBuffer;
    VuBuffer indexBuffer;

    Mesh(std::filesystem::path gltfPath, VmaAllocator& allocator);

    static std::array<VkVertexInputBindingDescription, 3> getBindingDescription();

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

    void Dispose();


};
