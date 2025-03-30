#include "common.slang"

// Remap function
float Remap(float value, float inMin, float inMax, float outMin, float outMax)
{
    // Clamp the value to be within the input range
    value = max(inMin, min(value, inMax));

    // Remap to output range
    return outMin + (value - inMin) / (inMax - inMin) * (outMax - outMin);
}

float3 Remap(float3 value, float3 inMin, float3 inMax, float3 outMin, float3 outMax)
{
    // Clamp each component of the value to be within the input range
    value = max(inMin, min(value, inMax));

    // Remap to output range for each component
    return outMin + (value - inMin) / (inMax - inMin) * (outMax - outMin);
}

float3 WorldPosFromDepth(float2 uv, float depth, float4x4 inverseProj, float4x4 inverseView)
{
    // Calculate NDC from depth and screen position
    float2 ndcPos = (uv * 2.0f) - 1.0f; // Convert to NDC [-1, 1]
    float3 ndc = float3(uv, depth);
    //ndc.y *= -1;

    // Inverse the projection matrix to get from NDC to view space
    float4 viewSpacePosition = mul( inverseProj, float4(ndc, 1));
    //viewSpacePosition.xyz /= viewSpacePosition.w; // Perspective divide

    // Inverse the view matrix to get from view space to world space
    float4 worldSpacePosition = mul( inverseView, viewSpacePosition);

    return worldSpacePosition.xyz; // The world space position
}

struct GBufferPassVSOutput
{
    float4 Pos : SV_Position;
    float2 UV : TEXCOORD;
};

static const float3 lightColor = float3(1.0, 1.0, 1.0);
static const float3 ambient = float3(0.01f, 0.01f, 0.01f);
static const float roughness = 0.5f;
static const float specularIntensity = 0.5f;

[shader("fragment")]
float4 fragmentMain(GBufferPassVSOutput i)
    : SV_Target
{
    var fc = frameConst;
    float3 lightPos = fc.cameraPos;

    PBRMaterialData *data = getMaterialData(pc.materialDataIndex);

    float2 uv = i.UV;

    float4 colorSample = globalSampledImages[data.texture0].Sample(globalSamplers[0], uv);
    float3 normalSample = globalSampledImages[data.texture1].Sample(globalSamplers[0], uv).xyz;
    float3 armSample = globalSampledImages[data.texture2].Sample(globalSamplers[0], uv).xyz;
    float depthSample = globalSampledImages[data.texture3].Sample(globalSamplers[0], uv).x;

    float3 normalWS = normalSample;
    float ao = armSample.x;
    float roughness = armSample.y;
    float metallic = armSample.z;


    //float3 worldPos = WorldPosFromDepth(uv,depthSample,fc.inverseProj,fc.inverseView);
    float3 worldPos = armSample;
    // Calculate lighting
    float3 viewDir = normalize(fc.cameraPos - worldPos);
    float3 lightDir = normalize(lightPos - worldPos);
    float3 halfVector = normalize(viewDir + lightDir);

    // Adjust shininess based on roughness (roughness of 0 -> sharp highlights, roughness of 1 -> soft highlights)
    float shininess = pow(1.0 - roughness, 4.0) * 128.0;

    // Blinn-Phong lighting model
    float3 diffuse = max(dot(lightDir, normalWS), 0.0) * lightColor * colorSample.rgb;
    float3 specular = pow(max(dot(normalWS, halfVector), 0.0), shininess) * lightColor * specularIntensity;

    // Final color
    float3 color = (ambient + diffuse);
    return float4(color.xyz, 1);

    //debug frag normal ws
    //return float4(normalWS.xyz * 0.5 + 0.5, 1);

}
