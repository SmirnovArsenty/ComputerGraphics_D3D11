struct VS_IN
{
    float4 pos : POSITION_UV_X0;
    float4 normal : NORMAL_UV_Y0;
    float4 col : COLOR0;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
    float3 real_pos : POSITION;
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
    float4x4 model_transform;
    float4x4 view_proj;
    float3 camera_pos;
    float res_0;
    float3 camera_dir;
    float res_1;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    float4 model_pos = mul(float4(input.pos.xyz, 1.f), model_transform);
    float3 model_normal = normalize(mul(input.normal.xyz, (float3x3)model_transform));

    output.pos = mul(model_pos, view_proj);
    output.real_pos = model_pos.xyz / model_pos.w;
    output.col = input.col;
    output.normal = model_normal;
    output.uv = float2(input.pos.w, input.normal.w);

    return output;
}

PS_OUT PSMain( PS_IN input ) : SV_Target
{
    PS_OUT res = (PS_OUT)0;

    float3 model_pos = input.real_pos;
    float3 model_normal = input.normal;

    // res.color.xyz = camera_dir;
    // res.color.w = 1.f;
    // return res;

    // if (dot(camera_dir, model_normal) > 0)
    // {
    //     res.color = float4(1.f, 0.f, 1.f, 1.f);
    //     return res;
    // }

    float3 light_pos = float3(0.f, 1000.f, 0.f);
    float3 to_light_dir = normalize(light_pos - model_pos.xyz);
    float to_light_dist = distance(light_pos, model_pos.xyz);

    float4 col = input.col * 0.1f; // ambient

    // diffuse
    float light_dot = dot(input.normal, to_light_dir);
    col.xyz += saturate(col.xyz / (to_light_dist * to_light_dist / 1e6 + 1));
    
    // specular
    float3 v = normalize(model_pos.xyz - camera_pos);
    if (dot(v, model_normal) > 0)
    {
        discard;
        res.color = float4(1.f, 0.f, 1.f, 1.f);
        return res;
    }

    float3 h = normalize(-v + to_light_dir);
    float nh = dot(model_normal, h);
    col.xyz += nh / (to_light_dist * to_light_dist / 1e4 + 1);

    res.color = col;

    return res;
}
