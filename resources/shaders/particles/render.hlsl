#include "globals.h"

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

// A buffer containing the pre-computed view space positions of the particles
// StructuredBuffer<float4> view_space_positions : register(t1);

// The sorted index list of particles
StructuredBuffer<float2> sorted_index_buffer : register(t2);

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
    
    const float2 offsets[ 4 ] =
    {
        float2( -1,  1 ),
        float2(  1,  1 ),
        float2( -1, -1 ),
        float2(  1, -1 ),
    };

    //uint index = (uint)sorted_index_buffer[ active_particles - particleIndex - 1 ].y;
    uint index = (uint)sorted_index_buffer[ particleIndex ].y;
    Particle p = particle_pool[ index ];

    // float4 ViewSpaceCentreAndRadius = view_space_positions[index];
    // float3 VelocityXYEmitterNdotL = float3( p.m_VelocityXY.x, p.m_VelocityXY.y, p.m_EmitterNdotL );

    float2 offset = offsets[cornerIndex];
    float2 uv = (offset + 1) * float2(0.25, 0.5);

    // float radius = ViewSpaceCentreAndRadius.w;
    // float3 cameraFacingPos;

    {
        //float s, c;
        //sincos( p.m_Rotation, s, c );
        //float2x2 rotation = { float2( c, -s ), float2( s, c ) };
        
        //offset = mul( offset, rotation );

        // cameraFacingPos = ViewSpaceCentreAndRadius.xyz;
        // cameraFacingPos.xy += radius * offset;
    }

    // Output.Position = mul(float4(cameraFacingPos, 1), proj);
    Output.ViewSpaceCentreAndRadius.xyz = mul(view, float4(p.position, 1)).xyz; // ViewSpaceCentreAndRadius;
    Output.ViewSpaceCentreAndRadius.w = ((p.age / p.life_span) * (p.start_size - p.end_size) + p.start_size);
    Output.ViewPos = Output.ViewSpaceCentreAndRadius.xyz + float3(uv * Output.ViewSpaceCentreAndRadius.w, 0.f);
    Output.Position = mul(proj, float4(Output.ViewPos, 1.f));

    Output.TexCoord = uv;
    Output.Color = p.color;
    //Output.VelocityXYEmitterNdotL = (0).xxx; // VelocityXYEmitterNdotL;
    // Output.ViewPos = cameraFacingPos;

    return Output;
}

// The texture atlas for the particles
// Texture2D particle_texture : register(t0);

// The opaque scene depth buffer read as a texture
Texture2D<float> depth_texture : register(t1);


// Ratserization path's pixel shader
float4 PSMain(PS_INPUT In) : SV_TARGET
{
    // return float4(1.f, 1.f, 1.f, 1.f);

    // Retrieve the particle data
    float3 particleViewSpacePos = In.ViewSpaceCentreAndRadius.xyz;
    float  particleRadius = In.ViewSpaceCentreAndRadius.w;

    // Get the depth at this point in screen space
    // float depth = g_DepthTexture.Load( uint3( In.Position.x, In.Position.y, 0 ) ).x;
    float depth = 0.5f;

    // Get viewspace position by generating a point in screen space at the depth of the depth buffer
    float4 viewSpacePos;
    viewSpacePos.x = In.Position.x / screen_width;
    viewSpacePos.y = 1 - (In.Position.y / screen_height);
    viewSpacePos.xy = (2 * viewSpacePos.xy) - 1;
    viewSpacePos.z = depth;
    viewSpacePos.w = 1;

    // ...then transform it into view space using the inverse projection matrix and a divide by W
    viewSpacePos = mul(viewSpacePos, proj);
    viewSpacePos.xyz /= viewSpacePos.w;

    // remove this?
    // if (particleViewSpacePos.z > viewSpacePos.z)
    // {
    //     clip( -1 );
    // }
    //~
    
    // Calculate the depth fade factor
    // float depthFade = saturate((viewSpacePos.z - particleViewSpacePos.z) / particleRadius);

    float4 albedo = (1).xxxx;
    // albedo.a = depthFade;

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

    // Scale and bias
    uv = (1 + uv) * 0.5;

    float pi = 3.1415926535897932384626433832795;

    n.x = -cos( pi * uv.x );
    n.y = -cos( pi * uv.y );
    n.z = sin( pi * length( uv ) );
    n = normalize( n );

    float3 sun_direction = normalize(float3(1, -1, 1));
    float ndotl = saturate(dot(-sun_direction, n));

    // Fetch the emitter's lighting term
    // float emitterNdotL = In.VelocityXYEmitterNdotL.z;

    // Mix the particle lighting term with the emitter lighting
    // ndotl = lerp( ndotl, emitterNdotL, 0.5 );

    // Ambient lighting plus directional lighting
    float3 lighting = (ndotl * 0.3f).xxx; //g_AmbientColor + ndotl * g_SunColor;

    // Multiply lighting term in
    color.rgb *= lighting;

    return color;
}
