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
};

cbuffer UniformData : register(b0)
{
    float4x4 model;
    float4x4 view_proj;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    float4 model_pos = mul(float4(input.pos, 1.f), model);

    float3 light_pos = float3(0.f, 100.f, 0.f);
    float light_dot = dot(input.normal, normalize(light_pos - model_pos.xyz));
    float4 col = float4(float3(light_dot, light_dot, light_dot), 1.f);

    // float4x4 MVP = mul(mul(model, view), proj);
    output.pos = mul(model_pos, view_proj);
    output.col = col;
    output.normal = input.normal;
    output.uv = input.uv;

    return output;
}

PS_OUT PSMain( PS_IN input ) : SV_Target
{
    PS_OUT res = (PS_OUT)0;
    res.color = input.col;

    return res;
}
