#include "global.h"

struct VS_OUTPUT
{
    float4 ViewSpaceCentreAndRadius       : TEXCOORD0;
    float4 VelocityXYRotationEmitterNdotL : TEXCOORD1;
    uint   EmitterProperties              : TEXCOORD2;
    float4 Color                          : COLOR0;
};

struct PS_INPUT
{
    float4 ViewSpaceCentreAndRadius : TEXCOORD0;
    float2 TexCoord                 : TEXCOORD1;
    float3 ViewPos                  : TEXCOORD2;
    float3 VelocityXYEmitterNdotL   : TEXCOORD3;
    float3 Extrusion                : TEXCOORD4;
    float4 Color                    : COLOR0;
    float4 Position                 : SV_POSITION;
};


// The particle buffer data. Note this is only one half of the particle data - the data that is relevant to rendering as opposed to simulation
StructuredBuffer<Particle> particle_pool : register(t0);

// The sorted index list of particles
StructuredBuffer<float2> sorted_index_buffer : register(t1);

// The geometry shader path for rendering particles.
// Vertex shader only path
PS_INPUT VSMain( uint VertexId : SV_VertexID )
{
    PS_INPUT Output = (PS_INPUT)0;

    // Particle index
    uint particleIndex = VertexId / 4;

    // Per-particle corner index
    uint cornerIndex = VertexId % 4;

    float xOffset = 0;
    
    const float2 offsets[4] =
    {
        float2(-1, 1),
        float2(1, 1),
        float2(-1, -1),
        float2(1, -1),
    };

    uint index = (uint)sorted_index_buffer[active_particles - particleIndex - 1].y;
    Particle p = particle_pool[index];

    float2 offset = offsets[cornerIndex];
    float2 uv = (offset + 1) * float2(0.5, 0.5);

    Output.ViewSpaceCentreAndRadius.xyz = mul(view, float4(p.position, 1)).xyz;
    Output.ViewSpaceCentreAndRadius.w = (p.age / p.life_span) * ((p.end_size - p.start_size) + p.end_size);
    Output.ViewPos = Output.ViewSpaceCentreAndRadius.xyz + float3((uv * 2 - 1) * Output.ViewSpaceCentreAndRadius.w, 0.f);
    Output.Position = mul(proj, float4(Output.ViewPos, 1.f));

    Output.TexCoord = uv;
    Output.Color = p.color;

    return Output;
}

// The texture atlas for the particles
// Texture2D particle_texture : register(t0);

// The opaque scene depth buffer read as a texture
// Texture2D<float> depth_texture : register(t1);

// Ratserization path's pixel shader
float4 PSMain(PS_INPUT In) : SV_TARGET
{
    // Retrieve the particle data
    float3 particleViewSpacePos = In.ViewSpaceCentreAndRadius.xyz;
    float particleRadius = In.ViewSpaceCentreAndRadius.w;

    // Get the depth at this point in screen space
    // float depth = g_DepthTexture.Load( uint3( In.Position.x, In.Position.y, 0 ) ).x;
    // float depth = 0.5f;

    // Get viewspace position by generating a point in screen space at the depth of the depth buffer
    float4 viewSpacePos;
    viewSpacePos.x = In.Position.x / screen_width;
    viewSpacePos.y = 1 - (In.Position.y / screen_height);
    viewSpacePos.xy = (2 * viewSpacePos.xy) - 1;
    viewSpacePos.z = In.Position.z;
    viewSpacePos.w = 1;

    // ...then transform it into view space using the inverse projection matrix and a divide by W
    viewSpacePos = mul(viewSpacePos, proj);
    viewSpacePos.xyz /= viewSpacePos.w;

    // Calculate the depth fade factor
    float depthFade = saturate((viewSpacePos.z - particleViewSpacePos.z) / particleRadius);

    float4 albedo = (1).xxxx;
    albedo.a = depthFade;

    // Read the texture atlas
    // albedo *= g_ParticleTexture.SampleLevel( g_samClampLinear, In.TexCoord, 0 ); // 2d

    // Multiply in the particle color
    float4 color = albedo * In.Color;

    // Calculate the UV based the screen space position
    float3 n = 0;
    float2 uv;
    {
        uv = (In.ViewPos.xy - In.ViewSpaceCentreAndRadius.xy ) / particleRadius;
    }

    // circle particles
    if (dot(uv, uv) > 1)
    {
        color = (0).xxxx;
        discard;
    }

    // Scale and bias
    uv = (1 + uv) * 0.5;

    float pi = 3.1415926535897932384626433832795;

    n.x = -cos( pi * uv.x );
    n.y = -cos( pi * uv.y );
    n.z = sin( pi * length( uv ) );
    n = normalize(n);

    float3 sun_direction = normalize(float3(1, -1, 1));
    float ndotl = saturate(dot(sun_direction, n));

    // Ambient lighting plus directional lighting
    float3 lighting = (ndotl).xxx; //g_AmbientColor + ndotl * g_SunColor;
    // Multiply lighting term in
    color.rgb *= lighting;

    //return color;
    return float4(color.rgb, 1.f);
}
