struct UBO
{
    float4x4 view;
    float4x4 proj;
};

struct PushConsts
{
    float4x4 model;
    uint materialDataIndex;
};

[[vk::binding(0, 0)]]
ConstantBuffer<UBO> ubo;

[[vk::binding(1, 1)]]
SamplerState globalSamplers[];

[[vk::binding(2, 1)]]
Texture2D globalTextures[];


[[vk::push_constant]]
PushConsts pc;

struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]]float3 FragPosWS : POSITION0;
    [[vk::location(1)]]float3 FragNormalWS : NORMAL0;
    [[vk::location(2)]]float2 FragUV : TEXCOORD0;
};

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
    //float4 texColor = texture(sampler2D(globalTextures[0], globalSamplers[0]), fragUV);
    float4 texColor = globalTextures[0].Sample(globalSamplers[0],input.FragUV);


    float3 viewPos = float3(-ubo.view[0][3], -ubo.view[1][3], -ubo.view[2][3]);

    float3 normal = normalize(input.FragNormalWS);
    float3 lightDir = normalize(lightPos - input.FragPosWS);
    float3 viewDir = normalize(viewPos - input.FragPosWS);
    float diff = max(dot(normal, lightDir), 0.0f);
    float3 diffuse = diff * lightColor;

    float3 halfwayDir = normalize(lightDir + viewDir);


    float shininess = 1.0 / (roughness * roughness + 0.001f); // Avoid division by 0 for roughness = 0
    float spec = pow(max(dot(normal, halfwayDir), 0.0f), 64);
    float3 specular = spec * lightColor; // Specular reflection color

    float3 finalColor = (ambient + diffuse + specular) * texColor.rgb;

    output.Color0 = float4(finalColor, 1);

    return output;

}