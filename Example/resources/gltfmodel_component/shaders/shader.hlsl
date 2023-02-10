struct VS_IN
{
    float3 pos : POSITION0;
    float3 normal : NORMAL0;
    float2 uv : TEXCOORD0;
    float3 col : COLOR0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float4 col : COLOR;
};

cbuffer UniformData : register(b0)
{
    matrix VP;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    // if (input.id == 0) {
    //     output.pos = float4(0.5f, 0.5f, 0.5f, 1.0f);
    //     output.col = float4(1.0f, 0.0f, 0.0f, 1.0f);
    // }
    // if (input.id == 1) {
    //     output.pos = float4(-0.5f, -0.5f, 0.5f, 1.0f);
    //     output.col = float4(0.0f, 0.0f, 1.0f, 1.0f);
    // }
    // if (input.id == 2) {
    //     output.pos = float4(0.5f, -0.5f, 0.5f, 1.0f);
    //     output.col = float4(0.0f, 1.0f, 0.0f, 1.0f);
    // }
    // if (input.id == 3) {
    //     output.pos = float4(-0.5f, 0.5f, 0.5f, 1.0f);
    //     output.col = float4(1.0f, 1.0f, 1.0f, 1.0f);
    // }

    output.pos = mul(float4(input.pos, 1.f), VP);
    output.col = float4(input.normal, 1.f);

    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    float4 col = input.col;
// #ifdef TEST
//     if (input.pos.x > 400) col = TCOLOR;
// #endif
    return col;
}
