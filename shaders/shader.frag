#version 450

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout (push_constant) uniform constants
{
    mat4 model;
} PushConstants;


layout (set = 1, binding = 0) uniform sampler2D samplerColor;

layout (location = 0) in vec3 fragPosWS;
layout (location = 1) in vec3 fragNormalWS;
layout (location = 2) in vec2 fragUV;

layout (location = 0) out vec4 outColor;

const vec3 lightPos = vec3(0.0, 2.0, 3.0);
const vec3 lightColor = vec3(1.0, 1.0, 1.0);

const float ambient = 0.03f;
const float roughness = 0.0f;

void main() {
    //vec3 viewPos = vec3(-ubo.view[2][0], -ubo.view[2][1], -ubo.view[2][2]);
    vec3 viewPos = vec3(-ubo.view[0][3], -ubo.view[1][3], -ubo.view[2][3]);
    //vec3 viewPos = vec3(-ubo.view[3][0], -ubo.view[3][1], -ubo.view[3][2]);



    vec3 normal = normalize(fragNormalWS);
    vec3 lightDir = normalize(lightPos - fragPosWS);
    vec3 viewDir = normalize(viewPos - fragPosWS);
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;

    vec3 halfwayDir = normalize(lightDir + viewDir);
    vec4 texColor = texture(samplerColor, fragUV);




    float shininess = 1.0 / (roughness * roughness + 0.001); // Avoid division by 0 for roughness = 0
    float spec = pow(max(dot(normal, halfwayDir), 0.0), 64);
    vec3 specular = spec * lightColor; // Specular reflection color



    vec3 finalColor = (
    ambient +
    diffuse +
    specular
    )
    * texColor.rgb;
    ;

    outColor = vec4(finalColor, 1);
}