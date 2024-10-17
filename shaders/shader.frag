#version 450
layout (set = 1, binding = 0) uniform sampler2D samplerColor;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;

layout (location = 0) out vec4 outColor;

void main() {

    vec3 lightPos = vec3(0.0, 2.0, 0.0);
    //vec3 lightDir = vec3(0.0, -1.0, 0.0);
    vec3 lightColor = vec3(1.0, 1.0, 1.0);

    // Normalize the normal vector
    vec3 normal = normalize(inNormal);

    // Direction to the light source
    vec3 lightDir = normalize(lightPos - inNormal);

    // Calculate the diffuse term using the dot product
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 diffuse = diff * lightColor + 0.1f;

    // Sample the color from the texture
    vec4 texColor = texture(samplerColor, inUV);

    // Multiply the texture color by the diffuse lighting
    vec3 finalColor = texColor.rgb * diffuse;


    outColor = vec4(finalColor, 1);


    //outColor = texture(samplerColor, inUV * 4.0, 0.0);
}