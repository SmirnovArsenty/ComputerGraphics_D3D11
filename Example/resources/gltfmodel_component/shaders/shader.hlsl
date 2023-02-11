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
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
};

struct PS_OUT
{
    float4 color : SV_Target;
    float depth : SV_Depth;
};

cbuffer UniformData : register(b0)
{
    float4x4 mvp;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    output.pos = mul(float4(input.pos, 0.f), mvp);
    output.col = float4(input.normal, 1.f);
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}

PS_OUT PSMain( PS_IN input ) : SV_Target
{
    float3 light_pos = float3(0.f, 10000.f, 0.f);
    float light_dot = dot(input.normal, normalize(light_pos - input.pos.xyz));
    float4 col = float4(float3(light_dot, light_dot, light_dot), 1.f);
    
    float depth = input.pos.w;

    PS_OUT res = (PS_OUT)0;
    res.color = col;
    res.depth = depth;

    return res;
}
