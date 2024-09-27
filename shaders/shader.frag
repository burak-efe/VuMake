#version 450
//layout (binding = 1) uniform sampler2D samplerColor;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() {

    //vec4 color = texture(samplerColor, inUV, 0.0);
    //outColor = color;
    outColor = vec4(inNormal, 1.0);
}