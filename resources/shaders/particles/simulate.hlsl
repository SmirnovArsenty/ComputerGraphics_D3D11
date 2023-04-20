#include "globals.h"

// Particle buffer in two parts
RWStructuredBuffer<Particle> particle_pool : register(u0);

// The dead list, so any particles that are retired this frame can be added to this list
AppendStructuredBuffer<uint> dead_list : register(u1);

RWStructuredBuffer<float2> sort_list : register(u2);

// The alive list which gets built using this shader
// RWStructuredBuffer<float2> index_buffer : register(u3);

// Viewspace particle positions are calculated here and stored
// RWStructuredBuffer<float4> view_space_positions : register(u3);

// The draw args for the DrawInstancedIndirect call needs to be filled in before the rasterization path is called, so do it here
RWBuffer<uint> draw_args : register(u4);

// The opaque scene's depth buffer read as a texture
// Texture2D depth_buffer : register(t0);


// Calculate the view space position given a point in screen space and a texel offset
// float3 calcViewSpacePositionFromDepth( float2 normalizedScreenPosition, int2 texelOffset )
// {
//     float2 uv;
// 
//     // Add the texel offset to the normalized screen position
//     normalizedScreenPosition.x += (float)texelOffset.x / screen_width;
//     normalizedScreenPosition.y += (float)texelOffset.y / screen_height;
// 
//     // Scale, bias and convert to texel range
//     uv.x = (0.5 + normalizedScreenPosition.x * 0.5) * screen_width;
//     uv.y = (1 - (0.5 + normalizedScreenPosition.y * 0.5)) * screen_height;
// 
//     // Fetch the depth value at this point
//     // float depth = depth_buffer.Load(uint3(uv.x, uv.y, 0)).x;
//     float depth = 0.5f;
//     
//     // Generate a point in screen space with this depth
//     float4 viewSpacePosOfDepthBuffer;
//     viewSpacePosOfDepthBuffer.xy = normalizedScreenPosition.xy;
//     viewSpacePosOfDepthBuffer.z = depth;
//     viewSpacePosOfDepthBuffer.w = 1;
// 
//     // Transform into view space using the inverse projection matrix
//     viewSpacePosOfDepthBuffer = mul(viewSpacePosOfDepthBuffer, proj_inv);
//     viewSpacePosOfDepthBuffer.xyz /= viewSpacePosOfDepthBuffer.w;
// 
//     return viewSpacePosOfDepthBuffer.xyz;
// }


// Simulate 256 particles per thread group, one thread per particle
[numthreads(256,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
    // Initialize the draw args using the first thread in the Dispatch call
    if ( id.x == 0 )
    {
        draw_args[ 0 ] = 0; // Number of primitives reset to zero
        draw_args[ 1 ] = 1; // Number of instances is always 1
        draw_args[ 2 ] = 0;
        draw_args[ 3 ] = 0;
        draw_args[ 4 ] = 0;
    }

    // Wait after draw args are written so no other threads can write to them before they are initialized
    GroupMemoryBarrierWithGroupSync();

    const float3 vGravity = float3( 0.0, -9.81, 0.0 );

    // Fetch the particle from the global buffer
    Particle p = particle_pool[id.x];

    // If the partile is alive
    if (p.age > 0.0f)
    {
        // Age the particle by counting down from Lifespan to zero
        p.age -= frame_delta;
        p.mass += p.mass_delta;

        float3 vNewPosition = p.position;

        // Apply force due to gravity
        // if ( p.m_IsSleeping == 0 )
        // {
            p.velocity += p.mass * (vGravity + p.acceleration) * frame_delta;

            // Apply a little bit of a wind force
            float3 windDir = float3( 1, 1, 0 );
            float windStrength = 0.1;

            p.velocity += normalize(windDir) * windStrength * frame_delta;

            // Calculate the new position of the particle
            vNewPosition += p.velocity * frame_delta;
        // }
    
        // Calculate the normalized age
        float fScaledLife = 1.0 - saturate( p.age / p.life_span );
        
        // Calculate the size of the particle based on age
        float radius = lerp(p.start_size, p.end_size, fScaledLife);
        
        // By default, we are not going to kill the particle
        bool killParticle = false;

        // If the position is below the floor, let's kill it now rather than wait for it to retire
        if ( vNewPosition.y < -10 )
        {
            // killParticle = true;
        }

        // Write the new position
        p.position = vNewPosition;

        // Calculate the the distance to the eye for sorting in the rasterization path
        float3 vec = vNewPosition - camera_position.xyz;
        p.distance_to_camera_sqr = dot(vec, vec);

        // The opacity is a function of the age
        // float alpha = lerp( 1, 0, saturate(fScaledLife - 0.8) / 0.2 );
        // p.m_TintAndAlpha.a = p.m_Age <= 0 ? 0 : alpha;

        // Lerp the color based on the age
        float4 color0 = p.start_color;
        float4 color1 = p.end_color;

        p.color = lerp(color0, color1, saturate(5 * fScaledLife));

        //if (g_ShowSleepingParticles && p.m_IsSleeping == 1)
        //{
        //    p.m_TintAndAlpha.rgb = float3(1, 0, 1);
        //}

        // The emitter-based lighting models the emitter as a vertical cylinder
        // float2 emitterNormal = normalize(vNewPosition.xz - g_EmitterLightingCenter[emitterIndex].xz);

        // Generate the lighting term for the emitter
        // float emitterNdotL = saturate( dot( g_SunDirection.xz, emitterNormal) + 0.5 );

        // Transform the velocity into view space
        // float2 vsVelocity = mul(float4(p.velocity.xyz, 0), view).xy;

        // p.m_VelocityXY = vsVelocity;
        // p.m_EmitterNdotL = emitterNdotL;

        // Pack the view spaced position and radius into a float4 buffer
        // float4 viewSpacePositionAndRadius;

        // viewSpacePositionAndRadius.xyz = mul(view, float4(vNewPosition, 1)).xyz;
        // viewSpacePositionAndRadius.w = radius;

        // view_space_positions[id.x] = viewSpacePositionAndRadius;

        // Dead particles are added to the dead list for recycling
        if (p.age <= 0.0f || killParticle)
        {
            p.age = -1;
            dead_list.Append(id.x);
        }
        else
        {
            uint index = sort_list.IncrementCounter();
            sort_list[index] = float2(p.distance_to_camera_sqr, (float)id.x);
            
            uint dstIdx = 0;
            // VS only path uses 6 indices per particle billboard
            InterlockedAdd(draw_args[0], 6, dstIdx);
        }
    }

    // Write the particle data back to the global particle buffer
    particle_pool[id.x] = p;
}
