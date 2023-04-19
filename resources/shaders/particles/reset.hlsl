#include "Globals.h"

RWStructuredBuffer<Particle> particle_pool : register( u0 );

[numthreads(256,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
    particle_pool[ id.x ] = (Particle)0;
}