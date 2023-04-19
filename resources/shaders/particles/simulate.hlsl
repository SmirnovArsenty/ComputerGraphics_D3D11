#include "Globals.h"

// Particle buffer in two parts
RWStructuredBuffer<Particle> particle_pool : register( u0 );

// The dead list, so any particles that are retired this frame can be added to this list
AppendStructuredBuffer<uint> dead_list : register( u1 );

// The alive list which gets built using this shader
RWStructuredBuffer<float2> g_IndexBuffer			: register( u3 );

// Viewspace particle positions are calculated here and stored
RWStructuredBuffer<float4>				g_ViewSpacePositions	: register( u4 );

// The maximum radius in XY is calculated here and stored
RWStructuredBuffer<float>				g_MaxRadiusBuffer		: register( u5 );

// The draw args for the DrawInstancedIndirect call needs to be filled in before the rasterization path is called, so do it here
RWBuffer<uint>							g_DrawArgs				: register( u6 );

// The opaque scene's depth buffer read as a texture
Texture2D								g_DepthBuffer			: register( t0 );


// Calculate the view space position given a point in screen space and a texel offset
float3 calcViewSpacePositionFromDepth( float2 normalizedScreenPosition, int2 texelOffset )
{
	float2 uv;

	// Add the texel offset to the normalized screen position
	normalizedScreenPosition.x += (float)texelOffset.x / (float)g_ScreenWidth;
	normalizedScreenPosition.y += (float)texelOffset.y / (float)g_ScreenHeight;

	// Scale, bias and convert to texel range
	uv.x = (0.5 + normalizedScreenPosition.x * 0.5) * (float)g_ScreenWidth; 
	uv.y = (1-(0.5 + normalizedScreenPosition.y * 0.5)) * (float)g_ScreenHeight; 

	// Fetch the depth value at this point
	float depth = g_DepthBuffer.Load( uint3( uv.x, uv.y, 0 ) ).x;
	
	// Generate a point in screen space with this depth
	float4 viewSpacePosOfDepthBuffer;
	viewSpacePosOfDepthBuffer.xy = normalizedScreenPosition.xy;
	viewSpacePosOfDepthBuffer.z = depth;
	viewSpacePosOfDepthBuffer.w = 1;

	// Transform into view space using the inverse projection matrix
	viewSpacePosOfDepthBuffer = mul( viewSpacePosOfDepthBuffer, g_mProjectionInv );
	viewSpacePosOfDepthBuffer.xyz /= viewSpacePosOfDepthBuffer.w;

	return viewSpacePosOfDepthBuffer.xyz;
}


// Simulate 256 particles per thread group, one thread per particle
[numthreads(256,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
	// Initialize the draw args using the first thread in the Dispatch call
	if ( id.x == 0 )
	{
		g_DrawArgs[ 0 ] = 0;	// Number of primitives reset to zero
		g_DrawArgs[ 1 ] = 1;	// Number of instances is always 1
		g_DrawArgs[ 2 ] = 0;
		g_DrawArgs[ 3 ] = 0;
		g_DrawArgs[ 4 ] = 0;
	}

	// Wait after draw args are written so no other threads can write to them before they are initialized
	GroupMemoryBarrierWithGroupSync();

	const float3 vGravity = float3( 0.0, -9.81, 0.0 );

	// Fetch the particle from the global buffer
	Particle p = particle_pool[id.x];
	
	// If the partile is alive
	if ( p.m_Age > 0.0f )
	{
		// Extract the individual emitter properties from the particle
		uint emitterIndex = GetEmitterIndex( p.m_EmitterProperties );
		bool streaks = IsStreakEmitter( p.m_EmitterProperties );

		// Age the particle by counting down from Lifespan to zero
		p.m_Age -= g_fFrameTime;

		// Update the rotation
		p.m_Rotation += 0.24 * g_fFrameTime;

		float3 vNewPosition = p.m_Position;

		// Apply force due to gravity
		if ( p.m_IsSleeping == 0 )
		{
			p.m_Velocity += p.m_Mass * vGravity * g_fFrameTime;

			// Apply a little bit of a wind force
			float3 windDir = float3( 1, 1, 0 );
			float windStrength = 0.1;

			p.m_Velocity += normalize( windDir ) * windStrength * g_fFrameTime;
			
			// Calculate the new position of the particle
			vNewPosition += p.m_Velocity * g_fFrameTime;
		}
	
		// Calculate the normalized age
		float fScaledLife = 1.0 - saturate( p.m_Age / p.m_Lifespan );
		
		// Calculate the size of the particle based on age
		float radius = lerp( p.m_StartSize, p.m_EndSize, fScaledLife );
		
		// By default, we are not going to kill the particle
		bool killParticle = false;

		if ( g_CollideParticles )
		{
			// Transform new position into view space
			float3 viewSpaceParticlePosition =  mul( float4( vNewPosition, 1 ), g_mView ).xyz;

			// Also obtain screen space position
			float4 screenSpaceParticlePosition =  mul( float4( vNewPosition, 1 ), g_mViewProjection );
			screenSpaceParticlePosition.xyz /= screenSpaceParticlePosition.w;

			// Only do depth buffer collisions if the particle is onscreen, otherwise assume no collisions
			if ( p.m_IsSleeping == 0 && screenSpaceParticlePosition.x > -1 && screenSpaceParticlePosition.x < 1 && screenSpaceParticlePosition.y > -1 && screenSpaceParticlePosition.y < 1 )
			{
				// Get the view space position of the depth buffer
				float3 viewSpacePosOfDepthBuffer = calcViewSpacePositionFromDepth( screenSpaceParticlePosition.xy, int2( 0, 0 ) );

				// If the particle view space position is behind the depth buffer, but not by more than the collision thickness, then a collision has occurred
				if ( ( viewSpaceParticlePosition.z > viewSpacePosOfDepthBuffer.z ) && ( viewSpaceParticlePosition.z < viewSpacePosOfDepthBuffer.z + g_CollisionThickness ) )
				{
					// Generate the surface normal. Ideally, we would use the normals from the G-buffer as this would be more reliable than deriving them
					float3 surfaceNormal;

					// Take three points on the depth buffer
					float3 p0 = viewSpacePosOfDepthBuffer;
					float3 p1 = calcViewSpacePositionFromDepth( screenSpaceParticlePosition.xy, int2( 1, 0 ) );
					float3 p2 = calcViewSpacePositionFromDepth( screenSpaceParticlePosition.xy, int2( 0, 1 ) );

					// Generate the view space normal from the two vectors
					float3 viewSpaceNormal = normalize( cross( p2 - p0, p1 - p0 ) );

					// Transform into world space using the inverse view matrix
					surfaceNormal = normalize( mul( -viewSpaceNormal, g_mViewInv ).xyz );

					// The velocity is reflected in the collision plane
					float3 newVelocity = reflect( p.m_Velocity, surfaceNormal );

					// Update the velocity and apply some restitution
					p.m_Velocity = 0.3*newVelocity;

					// Update the new collided position
					vNewPosition = p.m_Position + (p.m_Velocity * g_fFrameTime);

					p.m_CollisionCount++;
				}
			}
		}
	
		// Put particle to sleep if the velocity is small
		if ( g_EnableSleepState && p.m_CollisionCount > 10 && length( p.m_Velocity ) < 0.01 )
		{
			p.m_IsSleeping = 1;
		}

		// If the position is below the floor, let's kill it now rather than wait for it to retire
		if ( vNewPosition.y < -10 )
		{
			killParticle = true;
		}

		// Write the new position
		p.m_Position = vNewPosition;

		// Calculate the the distance to the eye for sorting in the rasterization path
		float3 vec = vNewPosition - g_EyePosition.xyz;
		p.m_DistanceToEye = length( vec );

		// The opacity is a function of the age
		float alpha = lerp( 1, 0, saturate(fScaledLife - 0.8) / 0.2 );
		p.m_TintAndAlpha.a = p.m_Age <= 0 ? 0 : alpha;

		// Lerp the color based on the age
		float4 color0 = g_StartColor[ emitterIndex ];
		float4 color1 = g_EndColor[ emitterIndex ];
	
		p.m_TintAndAlpha.rgb = lerp( color0, color1, saturate(5*fScaledLife) ).rgb;

		if ( g_ShowSleepingParticles && p.m_IsSleeping == 1 )
		{
			p.m_TintAndAlpha.rgb = float3( 1, 0, 1 );
		}
		
		// The emitter-based lighting models the emitter as a vertical cylinder
		float2 emitterNormal = normalize( vNewPosition.xz - g_EmitterLightingCenter[ emitterIndex ].xz );

		// Generate the lighting term for the emitter
		float emitterNdotL = saturate( dot( g_SunDirection.xz, emitterNormal ) + 0.5 );

		// Transform the velocity into view space
		float2 vsVelocity = mul( float4( p.m_Velocity.xyz, 0 ), g_mView ).xy;
		
		p.m_VelocityXY = vsVelocity;
		p.m_EmitterNdotL = emitterNdotL;

		// Pack the view spaced position and radius into a float4 buffer
		float4 viewSpacePositionAndRadius;

		viewSpacePositionAndRadius.xyz = mul( float4( vNewPosition, 1 ), g_mView ).xyz;
		viewSpacePositionAndRadius.w = radius;

		g_ViewSpacePositions[ id.x ] = viewSpacePositionAndRadius;

		// For streaked particles (the sparks), calculate the the max radius in XY and store in a buffer
		if ( streaks )
		{
			float2 r2 = calcEllipsoidRadius( radius, p.m_VelocityXY );
			g_MaxRadiusBuffer[ id.x ] = max( r2.x, r2.y );
		}
		else
		{
			// Not a streaked particle so will have rotation. When rotating, the particle has a max radius of the centre to the corner = sqrt( r^2 + r^2 )
			g_MaxRadiusBuffer[ id.x ] = 1.41 * radius;
		}

		// Dead particles are added to the dead list for recycling
		if ( p.m_Age <= 0.0f || killParticle )
		{
			p.m_Age = -1;
			g_DeadListToAddTo.Append( id.x );
		}
		else
		{
			// Alive particles are added to the alive list
			uint index = g_IndexBuffer.IncrementCounter();
			g_IndexBuffer[ index ] = float2( p.m_DistanceToEye, (float)id.x );
			
			uint dstIdx = 0;
#if defined (USE_GEOMETRY_SHADER)
			// GS path uses one vertex per particle
			InterlockedAdd( g_DrawArgs[ 0 ], 1, dstIdx );
#else
			// VS only path uses 6 indices per particle billboard
			InterlockedAdd( g_DrawArgs[ 0 ], 6, dstIdx );
#endif
		}
	}

	// Write the particle data back to the global particle buffer
	particle_pool[id.x] = p;
}
