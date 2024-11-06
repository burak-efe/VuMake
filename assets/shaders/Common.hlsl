struct UBO
{
    float4x4 view;
    float4x4 proj;
    float3 cameraPos;
    float pad0;
    float3 cameraDir;
    float pad1;
    float time;
    float3 pad2;
};

struct PushConsts
{
    float4x4 model;
    uint materialDataIndex;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UBO> ubo;

[[vk::binding(2, 0)]]
SamplerState globalSamplers[];

[[vk::binding(3, 0)]]
Texture2D globalTextures[];

[[vk::push_constant]]
PushConsts pc;

struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]]float3 Normal : NORMAL0;
    [[vk::location(1)]]float3 Tangent : TANGENT0;
    [[vk::location(2)]]float3 Bitangent : TEXCOORD2;
    [[vk::location(3)]]float2 UV : TEXCOORD0;
    [[vk::location(4)]]float3 PosWS : TEXCOORD1;

};
