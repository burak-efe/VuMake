struct UBO
{
    float4x4 view;
    float4x4 proj;
};

struct PushConsts
{
    float4x4 model;
    uint materialDataIndex;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UBO> ubo;

[[vk::binding(1, 1)]]
SamplerState globalSamplers[];

[[vk::binding(2, 1)]]
Texture2D globalTextures[];

[[vk::push_constant]]
PushConsts pc;