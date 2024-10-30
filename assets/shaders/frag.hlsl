#include "Common.hlsl"


struct FSOutput
{
    float4 Color0 : SV_TARGET0;
};


static const float3 lightPos = float3(0.0, 2.0, 3.0);
static const float3 lightColor = float3(1.0, 1.0, 1.0);

static const float ambient = 0.03f;
static const float roughness = 0.0f;

FSOutput main(VSOutput input)
{
     FSOutput output = (FSOutput) 0;

    float3 viewPos = float3(-ubo.view[0][3], -ubo.view[1][3], -ubo.view[2][3]);

    float4 texColor = globalTextures[0].Sample(globalSamplers[0],input.UV);


    // float3 normal = normalize(input.Normal);
    // float3 lightDir = normalize(lightPos - input.FragPosWS);
    // float3 viewDir = normalize(viewPos - input.FragPosWS);
    // float diff = max(dot(normal, lightDir), 0.0f);
    // float3 diffuse = diff * lightColor;

    // float3 halfwayDir = normalize(lightDir + viewDir);


    // float shininess = 1.0 / (roughness * roughness + 0.001f); // Avoid division by 0 for roughness = 0
    // float spec = pow(max(dot(normal, halfwayDir), 0.0f), 64);
    // float3 specular = spec * lightColor; // Specular reflection color

    // float3 finalColor = (ambient + diffuse + specular) * texColor.rgb;

    output.Color0 = float4(texColor.xyz, 1);

    return output;

}