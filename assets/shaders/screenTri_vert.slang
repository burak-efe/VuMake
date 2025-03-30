#include "common.slang"

struct GBufferPassVSOutput
{
    float4 Pos : SV_Position;
    float2 UV : TEXCOORD;
};

[shader("vertex")]
GBufferPassVSOutput vertexMain(uint32_t vertexIndex: SV_VertexID)
{
    GBufferPassVSOutput output;

//      Top Right
//           ▲
//          /|
//         / |
//        /  |
//       /___|
//
//Left Bottom     Right Bottom

    const float2 positions[3] = float2[3](
        float2(-3.0, -1.0), //left bottom
        float2(1.0, -1.0), //right bottom
        float2(1.0, 3.0)   //top right
    );
    
//     const float2 uvs[3] = float2[3](
//         float2(-1.0, 0.0), //left bottom
//         float2(1.0, 0.0), //right bottom
//         float2(1.0, 2.0)   //top right
//     );

        const float2 uvs2[3] = float2[3](
            float2(-1.0, 2.0), //left bottom
            float2(1.0, 2.0), //right bottom
            float2(1.0, 0.0)   //top right
        );


    output.Pos = float4(positions[vertexIndex], 0.0, 1.0);
    output.UV = uvs2[vertexIndex];
    return output;
}