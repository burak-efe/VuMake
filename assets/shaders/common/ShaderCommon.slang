#pragma once
#include "InteroptStructs.h"
#include "Math.slang"
#include "GlobalBindings.slang"

struct Span<T> {
    Ptr<T> _pointer;
    uint64_t _length;

    __init(uint64_t ptr, uint64_t lenght)
    {
        _pointer = (Ptr<T>)ptr;
        _length = lenght;
    }

    __subscript(uint64_t index) -> T
    {
        get { return _pointer[index]; }
        set { _pointer[index] = newValue; }
    }

    //Create a new subspan
    Span<T> Slice(uint64_t _pointerIndex, uint64_t _length){
        Span<T> sliced;
        sliced._pointer = &this._pointer[_pointerIndex];
        sliced._length = _length;
        return sliced;
    }
}

[Flags]
enum MeshFlags : uint32_t {
    HasNormal,
    HasTangent,
    HasUV_0,
    HasUV_1,
    HasUV_2,
}


__generic<typename T>
T* getMaterialDataAs(uint32_t materialDataIndex)
{
    var materialDataBuffer = (T *)globalStorageBuffers[1];
    return &materialDataBuffer[materialDataIndex];
}


struct VSOutput
{
    float4 Pos : SV_POSITION;
    [[vk::location(0)]] float3 Normal : NORMAL0;
    [[vk::location(1)]] float3 Tangent : TANGENT0;
    [[vk::location(2)]] float3 Bitangent : BINORMAL0;
    [[vk::location(3)]] float2 UV : TEXCOORD0;
    [[vk::location(4)]] float3 PosWS : TEXCOORD1;
};

const static float PI = 3.14159274;


float3 calculatePointLight(PointLight light, float3 worldPos, float3 normal)
{
    float3 toLight = light.position - worldPos;
    float distance = length(toLight);
    if (distance > light.range) return 0;

    float3 lightDir = toLight / distance;
    float NdotL = max(dot(normal, lightDir), 0.0);

    // Optional: soft falloff
    float falloff = saturate(1.0 - distance / light.range);
    float attenuation = (light.intensity / (distance * distance)) * falloff;

    return attenuation * light.color * NdotL;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float DistributionGGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
    return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness)
{
    return GeometrySchlickGGX(NdotV, roughness) * GeometrySchlickGGX(NdotL, roughness);
}