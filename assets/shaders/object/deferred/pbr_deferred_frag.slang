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
    MatData_PbrDeferred *matData = getMaterialDataAs<MatData_PbrDeferred>(pc.material_data_handle.index);
    
    GPassFragOutput out = {};

    float2 uv = input.UV;

    float4 colorSample  = globalSampledImages[matData.colorTexture].Sample(globalSamplers[0], uv);
    float3 normalSample = globalSampledImages[matData.normalTexture].Sample(globalSamplers[0], uv).xyz * 2 - 1;
    float3 armSample    = globalSampledImages[matData.aoRoughMetalTexture].Sample(globalSamplers[0], uv).xyz;

    float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);

    float3 normalWS = mul(normalSample,TBN);

    out.color = colorSample;
    out.normal = float4(normalWS, 1);
    out.arm = float4(armSample, 1);
    out.posWS = float4(input.PosWS,1);

    return out;
}