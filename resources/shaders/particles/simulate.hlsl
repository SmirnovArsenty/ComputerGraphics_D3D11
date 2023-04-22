#include "global.h"

// Particle buffer in two parts
RWStructuredBuffer<Particle> particle_pool : register(u0);

// The dead list, so any particles that are retired this frame can be added to this list
// AppendStructuredBuffer<uint> dead_list : register(u1);
AppendStructuredBuffer<uint> dead_list : register(u1);

// The alive list which gets built using this shader
RWStructuredBuffer<float2> sort_list : register(u2);

// The draw args for the DrawInstancedIndirect call needs to be filled in before the rasterization path is called, so do it here
RWBuffer<uint> draw_args : register(u3);

// The opaque scene's depth buffer read as a texture
Texture2D depth_buffer : register(t0);

// G-Buffer from opaque pass which stores normals data
Texture2D<float3> normal_buffer : register(t1);

// Simulate 256 particles per thread group, one thread per particle
[numthreads(256,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
    // Initialize the draw args using the first thread in the Dispatch call
    if (id.x == 0)
    {
        draw_args[ 0 ] = 0; // Number of primitives reset to zero
        draw_args[ 1 ] = 1; // Number of instances is always 1
        draw_args[ 2 ] = 0;
        draw_args[ 3 ] = 0;
        draw_args[ 4 ] = 0;
    }

    // Wait after draw args are written so no other threads can write to them before they are initialized
    GroupMemoryBarrierWithGroupSync();

    const float3 g = float3(0.0, -9.81, 0.0);

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
        p.velocity += p.mass * (g + p.acceleration) * frame_delta;

        // Calculate the new position of the particle
        vNewPosition += p.velocity * frame_delta;

        // Calculate the normalized age
        float fScaledLife = 1.0 - saturate(p.age / p.life_span);

        // Calculate the size of the particle based on age
        float radius = lerp(p.start_size, p.end_size, fScaledLife);

        // By default, we are not going to kill the particle
        bool killParticle = false;

        uint is_reflected = false;
        // calc reflect
        float4 view_proj_pos = mul(view_proj, float4(vNewPosition, 1.f));
        view_proj_pos = view_proj_pos / view_proj_pos.w;
        float2 depth_uv = float2((view_proj_pos.x * 0.5 + 0.5), (-view_proj_pos.y * 0.5 + 0.5));
        if (depth_uv.x < 1 && depth_uv.x > 0 && depth_uv.y > 0 && depth_uv.y < 1) {
            float depth = depth_buffer.Load(uint3(depth_uv.x * screen_width, depth_uv.y * screen_height, 0)).x;
            float3 normal = normal_buffer.Load(uint3(depth_uv.x * screen_width, depth_uv.y * screen_height, 0));
            if (abs(view_proj_pos.z - depth) < 0.0001) {
                is_reflected = true;
                p.velocity = reflect(p.velocity, normal);
                p.velocity = p.velocity * 0.7; // + float3(sin(global_time * id.x), sin(global_time * id.x + 8), sin(global_time * id.x + 4));
            }
        } else {
            // out of screen
            // killParticle = true;
        }

        if (is_reflected)
        { // emit new particles
            // uint dead_index = dead_list.DecrementCounter();
            // uint dead_list_index = dead_list[dead_index];
            // Particle p_new = particle_pool[dead_list_index];
            // p_new = p;
            // p_new.age = p_new.life_span;
            // p_new.velocity += length(p_new.velocity)
            //     * float3(sin(global_time * (dead_list_index * 1.23 + 3)), sin(global_time * (dead_list_index * 1.56 + 7)), sin(global_time * (dead_list_index * 1.16 + 9)));
            // uint sort_index = sort_list.IncrementCounter();
            // float3 vec = p_new.position - camera_position.xyz;
            // p_new.distance_to_camera_sqr = dot(vec, vec);
            // sort_list[sort_index] = float2(p_new.distance_to_camera_sqr, (float)dead_list_index);
        }
        else
        {
            p.position = vNewPosition;
            float3 vec = vNewPosition - camera_position.xyz;
            p.distance_to_camera_sqr = dot(vec, vec);
        }

        // Lerp the color based on the age
        float4 color0 = p.start_color;
        float4 color1 = p.end_color;

        p.color = lerp(color0, color1, saturate(5 * fScaledLife));
        p.color.a = lerp(1, 0, saturate(fScaledLife - 0.8) / 0.2);

        // Dead particles are added to the dead list for recycling
        if (p.age <= 0.0f || killParticle)
        {
            p.age = -1;
            // uint index = dead_list.IncrementCounter();
            // dead_list[index] = id.x;
            dead_list.Append(id.x);
        }
        else
        {
            uint index = sort_list.IncrementCounter();
            sort_list[index] = float2(p.distance_to_camera_sqr, (float)id.x);
            // sort_list.Append(float2(p.distance_to_camera_sqr, (float)id.x));

            uint dstIdx = 0;
            // VS only path uses 6 indices per particle billboard
            InterlockedAdd(draw_args[0], 6, dstIdx);
        }
    }

    // Write the particle data back to the global particle buffer
    particle_pool[id.x] = p;
}
