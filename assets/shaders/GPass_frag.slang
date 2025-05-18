#include "common.slang"


struct GPassFragOutput
{
    float4 color : SV_Target0;
    float4 normal : SV_Target1;
    float4 arm : SV_Target2;
};

[shader("fragment")]
GPassFragOutput fragmentMain(VSOutput input)
{
    PBRMaterialData* data = getMaterialData(pc.materialDataIndex);

    GPassFragOutput out = {};

    float2 uv = input.UV;

    float4 colorSample  = globalSampledImages[data.texture0].Sample(globalSamplers[0], uv);
    float3 normalSample = globalSampledImages[data.texture1].Sample(globalSamplers[0], uv).xyz * 2 - 1;
    float3 armSample    = globalSampledImages[data.texture2].Sample(globalSamplers[0], uv).xyz;

    float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);

    float3 normalWS = mul(normalSample,TBN);
    //float3 normalWS = mul(TBN,normalSample);

    out.color = colorSample;
    out.normal = float4(normalWS, 1.0);
    out.arm = float4(input.PosWS,1);

    return out;
}