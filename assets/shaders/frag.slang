#include "Common.slang"


static const float3 lightColor = float3(1.0, 1.0, 1.0);

static const float3 ambient = float3(0.01f, 0.01f, 0.01f);
static const float roughness = 0.5f;
static const float specularIntensity = 0.5f;
[shader("fragment")]
float4 fragmentMain(VSOutput i) : SV_Target
{
    var fc = frameConst;

    float3 lightPos = fc.cameraPos;

    PBRMaterialData* data = getMaterialData(pc.materialDataIndex);

    float2 uv = i.UV;

    float4 colorSample  = globalSampledImages[data.texture0].Sample(globalSamplers[0], uv);
    float3 normalSample = globalSampledImages[data.texture1].Sample(globalSamplers[0], uv).xyz;
    float3 normalTS = normalSample * 2 - 1;

    float3 nrm = normalize(normalTS.r * i.Tangent.xyz + normalTS.g * i.Bitangent + normalTS.b * i.Normal);

    // Calculate TBN matrix
    float3x3 TBN = float3x3(i.Tangent, i.Bitangent, i.Normal);

    float3 normalWS = mul(normalTS,TBN);

    // Calculate lighting
    float3 viewDir = normalize(fc.cameraPos - i.PosWS);

    float3 lightDir = normalize(lightPos - i.PosWS);

    float3 halfVector = normalize(viewDir + lightDir);

    // Adjust shininess based on roughness (roughness of 0 -> sharp highlights, roughness of 1 -> soft highlights)
    float shininess = pow(1.0 - roughness, 4.0) * 128.0;

    // Blinn-Phong lighting model
    float3 diffuse = max(dot(lightDir, normalWS), 0.0) * lightColor * colorSample.rgb;
    float3 specular = pow(max(dot(normalWS, halfVector), 0.0), shininess) * lightColor * specularIntensity;

    // Final color
    float3 color = (ambient + diffuse + specular);

    return float4(color.xyz,1);

}