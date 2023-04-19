// Per-frame constant buffer



// The number of dead particles in the system
cbuffer DeadListCount : register( b2 )
{
    uint    g_NumDeadParticles;
    uint3    DeadListCount_pad;
};


// The number of alive particles this frame
cbuffer ActiveListCount : register( b3 )
{
    uint    g_NumActiveParticles;
    uint3    ActiveListCount_pad;
};

// Particle structures
// ===================

struct Particle
{
    float3 position;
    float3 prev_position;
    float3 velocity;
    float3 acceleration;

    float size;
    float sizeDelta;
    float weigth;
    float weight_delta;

    float energy;
    float3 color;

    float3 color_delta;
    float dummy;
};

// Function to calculate the streak radius in X and Y given the particles radius and velocity
float2 calcEllipsoidRadius( float radius, float2 viewSpaceVelocity )
{
    float minRadius = radius * max( 1.0, 0.1*length( viewSpaceVelocity ) );
    return float2( radius, minRadius );
}


// this creates the standard Hessian-normal-form plane equation from three points, 
// except it is simplified for the case where the first point is the origin
float3 CreatePlaneEquation( float3 b, float3 c )
{
    return normalize(cross( b, c ));
}


// point-plane distance, simplified for the case where 
// the plane passes through the origin
float GetSignedDistanceFromPlane( float3 p, float3 eqn )
{
    // dot( eqn.xyz, p.xyz ) + eqn.w, , except we know eqn.w is zero 
    // (see CreatePlaneEquation above)
    return dot( eqn, p );
}


// convert a point from post-projection space into view space
float3 ConvertProjToView( float4 p )
{
    p = mul( p, g_mProjectionInv );
    p /= p.w;
    
    return p.xyz;
}
