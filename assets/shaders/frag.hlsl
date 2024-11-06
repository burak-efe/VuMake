#include "Common.hlsl"


struct FSOutput
{
    float4 Color0 : SV_TARGET0;
};


//static const float3 lightPos = float3(0.0, 0.0, -1.0);
static const float3 lightColor = float3(1.0, 1.0, 1.0);

static const float3 ambient = float3(0.01f, 0.01f, 0.01f);
static const float roughness = 0.5f;
static const float specularIntensity = 0.5f;

FSOutput main(VSOutput input)
{

    float3 lightPos = ubo.cameraPos;
     FSOutput output = (FSOutput) 0;

     float2 uv = input.UV;
    float4 colorSample  = globalTextures[1].Sample(globalSamplers[0], uv);
    float3 normalSample = globalTextures[2].Sample(globalSamplers[0], uv).rgb;
    //normalSample.g = 1.0f - normalSample.g;
    float3 normal = normalize(normalSample * 2.0 - 1.0);

    // Calculate TBN matrix
    float3x3 TBN = float3x3(input.Tangent, input.Bitangent, input.Normal);

    // Transform the normal to world space
    normal = normalize(mul(normal, TBN));

    // Calculate lighting
    float3 viewDir = normalize(ubo.cameraPos - input.PosWS);
    viewDir =  mul (float4(viewDir,0), TBN);

    float3 lightDir = normalize(lightPos - input.PosWS);
    lightDir = mul (float4(lightDir,0), TBN);


    float3 halfVector = normalize(viewDir + lightDir);

    // Adjust shininess based on roughness (roughness of 0 -> sharp highlights, roughness of 1 -> soft highlights)
    float shininess = pow(1.0 - roughness, 4.0) * 128.0;

    // Blinn-Phong lighting model
    float3 diffuse = max(dot(lightDir, normal), 0.0) * lightColor * colorSample.rgb;
    float3 specular = pow(max(dot(normal, halfVector), 0.0), shininess) * lightColor * specularIntensity;

    // Final color
    float3 color = (ambient + diffuse + specular);

    //output.Color0 = float4(viewDir, 1.0); // Debug: Output view direction
    //output.Color0 = float4(lightDir, 1.0); // Debug: Output light direction
    //output.Color0 = float4(normal, 1);
    output.Color0 = float4(normal, 1);





    return output;
}