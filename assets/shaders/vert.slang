#include "Common.slang"


struct VSInput
{
    [[vk::location(0)]] float3 VertexPosition : POSITION0;
    [[vk::location(1)]] float3 VertexNormal : NORMAL0;
    [[vk::location(2)]] float4 VertexTangent : TANGENT0;
    [[vk::location(3)]] float2 VertexUV : TEXCOORD0;
};


[shader("vertex")]
VSOutput main(VSInput input)
{
    VSOutput o = (VSOutput)0;
    o.Pos = mul(ubo.proj, mul(ubo.view, mul(pc.model, float4(input.VertexPosition, 1))));

    o.PosWS = mul( float4(input.VertexPosition, 1.0),pc.model ).xyz;

    o.Normal    =   normalize(mul((float3x3)pc.model, input.VertexNormal.xyz));
    o.Tangent   =   normalize(mul((float3x3)pc.model, input.VertexTangent.xyz));
    o.Bitangent =   normalize(cross(o.Normal, o.Tangent));

    o.UV = input.VertexUV;

    return o;
}
