#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (set = 0, binding = 0) uniform UniformBufferObject {
    mat4 view;
    mat4 proj;
} ubo;

layout (push_constant) uniform constants
{
    mat4 model;
} PushConstants;

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;

layout (location = 0) out vec3 fragNormal;
layout (location = 1) out vec2 fragUV;


void main() {
    gl_Position = ubo.proj * ubo.view * PushConstants.model * vec4(inPosition, 1.0);

    fragNormal.xyz = mat3(PushConstants.model) * inNormal;

    fragUV = inUV;

}