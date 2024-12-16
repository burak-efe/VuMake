#include "Common.slang"


struct VSInput
{
    [[vk::location(0)]] float3 VertexPosition : POSITION0;
    [[vk::location(1)]] float3 VertexNormal : NORMAL0;
    [[vk::location(2)]] float4 VertexTangent : TANGENT0;
    [[vk::location(3)]] float2 VertexUV : TEXCOORD0;
};


[shader("vertex")]
VSOutput vertexMain(VSInput input)
{
    //FrameConst fc = *((Ptr<FrameConst>) (globalUniformBuffers[0]));
    var fc = frameConst;

    //var testBuffer = (float3*)(globalStorageBuffers[0]);
    //printf("ponter is  = %llu", globalStorageBuffers[0]);
    //float3 testf3 = testBuffer[0];

    VSOutput o = (VSOutput)0;
    o.Pos = mul(fc.proj, mul(fc.view, mul(pc.model, float4(input.VertexPosition, 1))));

    o.PosWS = mul( float4(input.VertexPosition, 1.0),pc.model ).xyz;

    o.Normal    =   normalize(mul((float3x3)pc.model, input.VertexNormal.xyz));
    o.Tangent   =   normalize(mul((float3x3)pc.model, input.VertexTangent.xyz));
    o.Bitangent =   normalize(cross(o.Normal, o.Tangent));

    o.UV = input.VertexUV;

    return o;
}
