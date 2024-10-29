#version 450
#extension GL_ARB_separate_shader_objects: enable
#extension GL_EXT_nonuniform_qualifier : require

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

//layout (set = 1, binding = 0) uniform buffer<uint64> globalBuffers[];
layout (set = 1, binding = 1) uniform sampler globalSamplers[];
layout (set = 1, binding = 2) uniform texture2D globalTextures[];

layout (push_constant) uniform constants
{
    mat4 model;
    uint materialDataIndex;
} PushConstants;

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexUV;

layout (location = 0) out vec3 FragPosWS;
layout (location = 1) out vec3 FragNormalWS;
layout (location = 2) out vec2 FragUV;


void main() {
    gl_Position = ubo.proj * ubo.view * PushConstants.model * vec4(VertexPosition, 1.0);

    FragPosWS = vec3(PushConstants.model * vec4(VertexPosition, 1.0));

    FragNormalWS.xyz = mat3(PushConstants.model) * VertexNormal;
    FragUV = VertexUV;

}