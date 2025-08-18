
#include "InteroptStructs.h"

// struct Mesh {
//     Mesh_RawData raw_data;

//     Ptr<float3> getPositionPtr()
//     {
//         return (Ptr<float3>)globalStorageBuffers[raw_data.vertex_buffer_handle];
//     }

//     Ptr<float3> getNormalPtr()
//     {
//         uint64_t prev = (uint64_t)getPositionPtr();
//         uint64_t p = prev + sizeof(float3) * raw_data.vertex_count;
//         return (Ptr<float3>)p;
//     }

//     Ptr<float4> getTangentPtr()
//     {
//         uint64_t prev = (uint64_t)getNormalPtr();
//         uint64_t p = prev + sizeof(float3) * raw_data.vertex_count;
//         return (Ptr<float4>)p;
//     }

//     Ptr<float2> getUV_Ptr()
//     {
//         uint64_t prev = (uint64_t)getTangentPtr();
//         uint64_t p = prev + sizeof(float4) * raw_data.vertex_count;
//         return (Ptr<float2>)p;
//     }

// }

// struct PushConsts {
//     PushConsts_RawData data;
//     //property Mesh mesh { get { return (Mesh)mesh_rd;}};
// }

// struct FrameConst
// {
//     FrameConst_RawData data;
// };


[[vk::push_constant]]
GPU::PushConstant pushConstant;

[[vk::binding(0, 0)]]
ConstantBuffer<GPU::FrameConstant> frameConstant;

[[vk::binding(1, 0)]]
SamplerState globalSamplers[];

[[vk::binding(2, 0)]]
Texture2D globalSampledImages[];

[[vk::binding(3, 0)]]
RWTexture2D globalStorageImages[];

// all storage buffers pointers, reside here
// at index 0 there is empty buffer,
// at index 1 there is material data buffer
[[vk::binding(4, 0)]]
StructuredBuffer<uint64_t> globalStorageBuffers;