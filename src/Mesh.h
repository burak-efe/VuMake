#pragma once

#include "Common.h"
#include "VuBuffer.h"
#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <filesystem>
#include <array>


class Mesh {
public:
    uint32 VertexCount;
    std::vector<uint32> indices;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    VuBuffer vertexBuffer;
    VuBuffer normalBuffer;
    VuBuffer indexBuffer;

    Mesh(std::filesystem::path gltfPath, VmaAllocator allocator);

    void Dispose();

    static std::array<VkVertexInputBindingDescription, 2> getBindingDescription() {
        std::array<VkVertexInputBindingDescription, 2> bindingDescriptions{};
        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(glm::vec3);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;


        bindingDescriptions[1].binding = 1;
        bindingDescriptions[1].stride = sizeof(glm::vec3);
        bindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescriptions;
    }

    static std::array<VkVertexInputAttributeDescription, 2> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 2> attributeDescriptions{};
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = 0;

        attributeDescriptions[1].binding = 1;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = 0;
        return attributeDescriptions;
    }
};
