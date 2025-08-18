#include "../../common/ShaderCommon.slang"


struct GPassFragOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 arm : SV_Target2;
    float4 posWS : SV_Target3;
};

[shader("fragment")]
GPassFragOutput fragmentMain(VSOutput input)
{    
    var pc = pushConstant;
    GPU::MatData_PbrDeferred *matData = getMaterialDataAs<GPU::MatData_PbrDeferred>(pc.materialDataHandle.index);
    
    GPassFragOutput outData = {};

    float2 uv = input.UV;

    float4 colorSample  = globalSampledImages[matData.colorTexture].Sample(globalSamplers[0], uv);
    float3 normalSample = globalSampledImages[matData.normalTexture].Sample(globalSamplers[0], uv).xyz * 2 - 1;
    float3 armSample    = globalSampledImages[matData.aoRoughMetalTexture].Sample(globalSamplers[0], uv).xyz;

    float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);

    float3 normalWS = mul(normalSample,TBN);

    outData.color = colorSample;
    outData.normal = float4(normalWS, 1);
    outData.arm = float4(armSample, 1);
    outData.posWS = float4(input.PosWS,1);

    return outData;
}