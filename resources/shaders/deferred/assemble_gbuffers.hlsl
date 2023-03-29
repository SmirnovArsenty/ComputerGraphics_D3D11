float4 VSMain( unsigned int id : SV_VertexID ) : SV_POSITION
{
    return float4(4 * ((id & 2) >> 1) - 1.0, 4 * (id & 1) - 1.0, 0, 1);
}

Texture2D<float4> position_tex  : register(t0);
Texture2D<float4> normal_tex    : register(t1);
Texture2D<float4> diffuse_tex   : register(t2);
Texture2D<float4> specular_tex  : register(t3);
Texture2D<float4> ambient_tex   : register(t4);
Texture2D<float> depth_tex      : register(t5);

SamplerState tex_sampler                : register(s0);
SamplerComparisonState depth_sampler    : register(s1);

float4 PSMain(float4 position : SV_POSITION) : SV_Target0
{
    float4 result;
    result = normal_tex.Load(int3(position.xy, 0));
    return result;
}
