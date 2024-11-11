#pragma once

#include "Common.h"
#include "VuBuffer.h"

namespace std::filesystem {
    class path;
}

namespace Vu {

    struct VuMesh {
        uint32 vertexCount;

        VuBuffer indexBuffer;
        VuBuffer vertexBuffer;

        //std::vector<uint32> indices;

        VkDeviceSize totalAttributesSizePerVertex() {
            //pos, norm, tan , uv
            return sizeof(float3) + sizeof(float3) + sizeof(float4) + sizeof(float2);
        }

        VkDeviceSize getNormalOffsetAsByte() const {
            return sizeof(float3) * vertexCount;
        }

        VkDeviceSize getTangentOffsetAsByte() const {
            return (sizeof(float3) + sizeof(float3)) * vertexCount;
        }

        VkDeviceSize getUV_OffsetAsByte() const {
            return (sizeof(float3) + sizeof(float3) + sizeof(float4)) * vertexCount;
        }


        void init();

        void Dispose();

        static std::array<VkVertexInputBindingDescription, 4> getBindingDescription();

        static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions();

        static void calculateTangents(
            const std::span<uint32> indices,
            const std::span<float3> positions,
            const std::span<float3> normals,
            const std::span<float2> uvs,
            std::span<float4> tangents) {

            uint32 vertexCount = positions.size();
            uint32 triangleCount = indices.size() / 3;

            std::vector<float3> tan1(vertexCount);
            std::vector<float3> tan2(vertexCount);

            for (long a = 0; a < triangleCount; a++) {
                long i1 = indices[a * 3 + 0];
                long i2 = indices[a * 3 + 1];
                long i3 = indices[a * 3 + 2];

                const float3& v1 = positions[i1];
                const float3& v2 = positions[i2];
                const float3& v3 = positions[i3];

                const float2& w1 = uvs[i1];
                const float2& w2 = uvs[i2];
                const float2& w3 = uvs[i3];

                float x1 = v2.x - v1.x;
                float x2 = v3.x - v1.x;
                float y1 = v2.y - v1.y;
                float y2 = v3.y - v1.y;
                float z1 = v2.z - v1.z;
                float z2 = v3.z - v1.z;

                float s1 = w2.x - w1.x;
                float s2 = w3.x - w1.x;
                float t1 = w2.y - w1.y;
                float t2 = w3.y - w1.y;

                float r = 1.0F / (s1 * t2 - s2 * t1);
                float3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r,
                            (t2 * z1 - t1 * z2) * r);
                float3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r,
                            (s1 * z2 - s2 * z1) * r);

                tan1[i1] += sdir;
                tan1[i2] += sdir;
                tan1[i3] += sdir;

                tan2[i1] += tdir;
                tan2[i2] += tdir;
                tan2[i3] += tdir;
            }

            for (long a = 0; a < vertexCount; a++) {
                float3 n = normals[a];
                float3 t = tan1[a];
                // Gram-Schmidt orthogonalize
                float3 vec = glm::normalize(t - n * glm::dot(n, t));
                float sign = glm::dot(glm::cross(n, t), tan2[a]) < 0.0F ? -1.0F : 1.0F;
                tangents[a] = float4(vec, sign);
            }
        }
    };
}
