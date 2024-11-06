#include "Common.hlsl"


struct VSInput
{
    [[vk::location(0)]] float3 VertexPosition : POSITION0;
    [[vk::location(1)]] float3 VertexNormal : NORMAL0;
    [[vk::location(2)]] float4 VertexTangent : TANGENT0;
    [[vk::location(3)]] float2 VertexUV : TEXCOORD0;
};



VSOutput main(VSInput input)
{
    VSOutput output = (VSOutput)0;
    output.Pos = mul(ubo.proj, mul(ubo.view, mul(pc.model, float4(input.VertexPosition, 1))));

    output.PosWS = mul( float4(input.VertexPosition, 1.0),pc.model ).xyz;

    output.Normal = normalize(mul(float4(input.VertexNormal, 0.0), pc.model).xyz);
    output.Tangent = normalize(mul(float4(input.VertexTangent.xyz, 0.0), pc.model).xyz);

    output.Bitangent = normalize(input.VertexTangent.w * cross(output.Normal, output.Tangent));

    output.UV = input.VertexUV;

    return output;
}
