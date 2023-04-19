#include "Globals.h"

// The particle buffers to fill with new particles
RWStructuredBuffer<Particle> particle_pool : register( u0 );

// The dead list interpretted as a consume buffer. So every time we consume an index from this list, it automatically decrements the atomic counter (ie the number of dead particles)
ConsumeStructuredBuffer<uint> dead_list : register( u1 );

cbuffer EmitterConstantBuffer : register( b1 )
{
	float4	g_vEmitterPosition;
	float4	g_vEmitterVelocity;
	float4	g_PositionVariance;
	
	int		g_MaxParticlesThisFrame;
	float	g_ParticleLifeSpan;
	float	g_StartSize;
	float	g_EndSize;
	
	float	g_VelocityVariance;
	float	g_Mass;
	uint	g_EmitterIndex;
	uint	g_EmitterStreaks;

	uint	g_TextureIndex;
	uint	g_pads[ 3 ];
};


// Emit particles, one per thread, in blocks of 1024 at a time
[numthreads(1024,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
	// Check to make sure we don't emit more particles than we specified
	if ( id.x < g_NumDeadParticles && id.x < g_MaxParticlesThisFrame )
	{
		// Initialize the particle data to zero to avoid any unexpected results
		Particle p = (Particle)0;
		
		// Generate some random numbers from reading the random texture
		float velocityMagnitude = length( g_vEmitterVelocity.xyz );

		p.m_Position = g_vEmitterPosition.xyz + ( randomValues0.xyz * g_PositionVariance.xyz );

		p.m_EmitterProperties = WriteEmitterProperties( g_EmitterIndex, g_TextureIndex, g_EmitterStreaks ? true : false );
		p.m_Rotation = 0;
		p.m_IsSleeping = 0;
		p.m_CollisionCount = 0;

		p.m_Mass = g_Mass;
		p.m_Velocity = g_vEmitterVelocity.xyz + ( randomValues1.xyz * velocityMagnitude * g_VelocityVariance );
		p.m_Lifespan = g_ParticleLifeSpan;
		p.m_Age = pb.m_Lifespan;
		p.m_StartSize = g_StartSize;
		p.m_EndSize = g_EndSize;

		// The index into the global particle list obtained from the dead list. 
		// Calling consume will decrement the counter in this buffer.
		uint index = dead_list.Consume();

		// Write the new particle state into the global particle buffer
		particle_pool[ index ] = p;
	}
}