cbuffer SceneData : register(b0)
{
    float4x4 view_proj;
    float4x4 inv_view_proj;
    float3 camera_pos;
    float time;
    float3 camera_dir;
    uint SceneData_dummy;
};

cbuffer LightData : register(b1)
{
    float4x4 transform;
};

struct PS_IN
{
    float4 position : SV_POSITION;
};

PS_IN VSMain(float4 position : POSITION)
{
    PS_IN res = (PS_IN)0;
    res.position = position;
    res.position = mul(transform, res.position);
    res.position = mul(view_proj, res.position);
    return res;
}

float4 PSMain(PS_IN input) : SV_Target
{
    return input.position;
}

