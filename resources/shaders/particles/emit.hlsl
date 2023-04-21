#include "globals.h"

// The particle buffers to fill with new particles
RWStructuredBuffer<Particle> particle_pool : register(u0);

// The dead list interpretted as a consume buffer. So every time we consume an index from this list, it automatically decrements the atomic counter (ie the number of dead particles)
ConsumeStructuredBuffer<uint> dead_list : register(u1);

cbuffer EmitterConstantBuffer : register(b1)
{
    float3 origin;
    uint random;

    float3 velocity;
    float particle_life_span;

    float start_size;
    float end_size;
    float mass;
    uint max_particles_this_frame;

    float4 start_color;
    float4 end_color;
};

// Emit particles, one per thread, in blocks of 1024 at a time
[numthreads(1024,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
    // Check to make sure we don't emit more particles than we specified
    if (id.x < dead_particles && id.x < max_particles_this_frame)
    {
        // Initialize the particle data to zero to avoid any unexpected results
        Particle p = (Particle)0;

        // Generate some random numbers from reading the random texture
        // float velocityMagnitude = length(velocity.xyz);

        p.position = origin.xyz; // + ( randomValues0.xyz * g_PositionVariance.xyz );
        p.distance_to_camera_sqr = -1;
        p.mass = mass;
        p.mass_delta = 0.f;
        p.velocity = velocity.xyz +
            float3(sin(global_time * id.x), sin(global_time * 0.5 * id.x), sin(global_time * 2.0 * id.x)) +
            abs(0.3 * float3(sin(global_time * 4 + id.x), sin(global_time * 5 + id.x), sin(global_time * 4.8 + id.x)));
        p.acceleration = float3(0, 9.81, 0) + 0.5 * normalize(-p.velocity);//float3(sin(global_time * id.x * 0.35 + 34), sin(global_time * id.x * 0.67 + 23), sin(global_time * id.x * 0.57 + 83)).xxx;
        p.age = particle_life_span + sin(id.x * global_time * 0.86);
        p.life_span = particle_life_span;
        p.start_color = start_color;
        p.end_color = end_color;
        p.start_size = start_size;
        p.end_size = end_size;

        // The index into the global particle list obtained from the dead list.
        // Calling consume will decrement the counter in this buffer.
        uint index = dead_list.Consume();

        // Write the new particle state into the global particle buffer
        particle_pool[ index ] = p;
    }
}
