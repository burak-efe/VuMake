#include "common.slang"

[shader("vertex")]
VSOutput vertexMain(uint32_t id :SV_VertexID )
{
    VSOutput o = (VSOutput)0;
    var fc = frameConst;

    float3 pos  = pc.mesh.getPositionPtr()[id];
    float3 norm = pc.mesh.getNormalPtr()[id];
    float4 tan  = pc.mesh.getTangentPtr()[id];
    float2 uv   = pc.mesh.getUV_Ptr()[id];

    float4 posWS =  mul(pc.model,float4(pos, 1.0));

    o.Pos = mul(fc.proj, mul(fc.view,posWS));

    o.PosWS = posWS.xyz;
    //o.PosWS = mul(float4(pos, 1.0), pc.model).xyz;

    o.Normal = normalize(mul((float3x3)pc.model, norm.xyz));

    o.Tangent = normalize(mul((float3x3)pc.model, tan.xyz));

    o.Bitangent = normalize(cross(o.Normal, o.Tangent));

    o.UV = uv;

    return o;
}
