#include "../../common/ShaderCommon.slang"

[shader("fragment")]
float4 fragmentMain(VSOutput i) : SV_Target
{
    float2 uv = i.UV;
    return float4(uv,0.0f,1.0f);
}