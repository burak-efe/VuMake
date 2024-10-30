#include "Common.hlsl"


struct VSInput
{
    [[vk::location(0)]] float3 VertexPosition : POSITION0;
    [[vk::location(1)]] float3 VertexNormal : NORMAL0;
    [[vk::location(2)]] float2 VertexUV : TEXCOORD0;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]]float3 FragPosWS : POSITION0;
    [[vk::location(1)]]float3 FragNormalWS : NORMAL0;
    [[vk::location(2)]]float2 FragUV : TEXCOORD0;
};


VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Pos = mul(ubo.proj, mul(ubo.view, mul(pc.model, float4(input.VertexPosition, 1))));

    float4 m = mul(pc.model , float4(input.VertexPosition, 1.0));
    output.FragPosWS = m.xyz;

    output.FragNormalWS.xyz = mul(pc.model, input.VertexNormal).xyz;
    output.FragUV = input.VertexUV;

    return output;
}
