#include "../../common/ShaderCommon.slang"

struct GBufferPassVSOutput
{
    float4 Pos : SV_Position;
    float2 UV : TEXCOORD;
};


[shader("fragment")]
float4 fragmentMain(GBufferPassVSOutput i)
    : SV_Target
{
    var fc = frameConstant;
    var pc = pushConstant;

    GPU::MatData_PbrDeferred *data = getMaterialDataAs<GPU::MatData_PbrDeferred>(pc.materialDataHandle.index);

    float2 uv = i.UV;

    float4 colorSample    = globalSampledImages[data.colorTexture].Sample(globalSamplers[0], uv);
    float3 normalSample   = globalSampledImages[data.normalTexture].Sample(globalSamplers[0], uv).xyz;
    float3 armSample      = globalSampledImages[data.aoRoughMetalTexture].Sample(globalSamplers[0], uv).xyz;
    float3 posWS          = globalSampledImages[data.worldSpacePosTexture].Sample(globalSamplers[0], uv).xyz;
    float depthSample     = globalSampledImages[data.depthTexture].Sample(globalSamplers[0], uv).x;

    float3 normalWS = normalSample;
    float ao = 0;//armSample.x;
    float roughness = armSample.y;
    float metallic = armSample.z;

    float3 viewDir = normalize(fc.camera.position.xyz - posWS);
    float shininess = pow(1.0 - roughness, 4.0) * 128.0;

    float3 totalDiffuse = 0;
    float3 totalSpecular = 0;
    float3 baseColor = colorSample.rgb;
    float3 N = normalize(normalWS);
    float3 V = normalize(viewDir);

    float3 F0 = lerp(float3(0.04), baseColor, metallic); // Dielectric vs metallic F0

    for (int i = 0; i < 2; ++i)
    {
        float3 L = normalize(fc.pointLights[i].position - posWS);
        float3 H = normalize(V + L);

        float dist = length(fc.pointLights[i].position - posWS);
        if (dist > fc.pointLights[i].range) continue;

        float attenuation = fc.pointLights[i].intensity / (dist * dist);
        float falloff = saturate(1.0 - dist / fc.pointLights[i].range);
        attenuation *= falloff;

        float NdotL = saturate(dot(N, L));
        float NdotV = saturate(dot(N, V));
        float NdotH = saturate(dot(N, H));
        float HdotV = saturate(dot(H, V));

        // Cook-Torrance terms
        float D = DistributionGGX(NdotH, roughness);
        float G = GeometrySmith(NdotV, NdotL, roughness);
        float3 F = fresnelSchlick(HdotV, F0);

        float3 numerator = D * G * F;
        float denominator = 4.0 * NdotV * NdotL + 0.001;
        float3 specular = numerator / denominator;

        float3 kS = F;
        float3 kD = 1.0 - kS;
        kD *= 1.0 - metallic;

        float3 radiance = fc.pointLights[i].color * attenuation;

        totalDiffuse += kD * baseColor / PI * radiance * NdotL;
        totalSpecular += specular * radiance * NdotL;
    }

    float3 ambient =  baseColor * ao;
    float3 finalColor = ambient + totalDiffuse + totalSpecular;
    finalColor = finalColor * fc.camera.exposureScale;
    return float4(finalColor, 1.0);
}
