float4 VSMain( unsigned int id : SV_VertexID ) : SV_POSITION
{
    return float4(4 * ((id & 2) >> 1) - 1.0, 4 * (id & 1) - 1.0, 0, 1);
}

Texture2D<float4> normal_tex    : register(t0);
Texture2D<float4> diffuse_tex   : register(t1);
Texture2D<float4> specular_tex  : register(t2);
Texture2D<float4> ambient_tex   : register(t3);
Texture2D<float> depth_tex      : register(t4);

SamplerState tex_sampler                : register(s0);
SamplerComparisonState depth_sampler    : register(s1);

cbuffer SceneData : register(b0)
{
    float4x4 view_proj;
    float3 camera_pos;
    float time;
    float3 camera_dir;
    uint SceneData_dummy;
};

float4 PSMain(float4 position : SV_POSITION) : SV_Target0
{
    float3 result;

    float depth = depth_tex.Load(int3(position.xy, 0));
    return float4((pow(depth, 100)).xxx, 1.f);
    float3 pos = mul(view_proj, float4(position.xy, depth, 1.f)).xyz;
    // float3 pos = position_tex.Load(int3(position.xy, 0)).xyz * (2.f).xxx - (1.f).xxx;
    float3 normal = normal_tex.Load(int3(position.xy, 0)).xyz * (2.f).xxx - (1.f).xxx;

    if (normal.x + normal.y + normal.z == -3) {
        return float4(.3f, .5f, .7f, 1.f);
    }
    float4 diffuse = diffuse_tex.Load(int3(position.xy, 0));
    float4 specular = specular_tex.Load(int3(position.xy, 0));
    float4 ambient = ambient_tex.Load(int3(position.xy, 0));

    float3 light_dir = float3(0, -1, 0);
    float3 light_color = float3(1, 1, 1);

    float3 reflected_light_dir = reflect(normalize(light_dir), normal);
    float cos_phi = dot(reflected_light_dir, normalize(camera_pos - pos));
    result = 
        dot(normal, -light_dir) * diffuse.xyz +
        light_color * specular.xyz * pow(max(cos_phi, 0), 5);

    result += ambient;

    result = pow(abs(result), 1/2.2f);
    return float4(result, 1.f);
}
