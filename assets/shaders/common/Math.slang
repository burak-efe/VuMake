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