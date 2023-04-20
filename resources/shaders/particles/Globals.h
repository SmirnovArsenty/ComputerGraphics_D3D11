// Per-frame constant buffer
cbuffer PerFrameConstBuffer : register(b0)
{
    float4x4 view;
    float4x4 view_inv;
    float4x4 proj;
    float4x4 proj_inv;
    float4x4 view_proj;
    float4x4 view_proj_inv;

    float3 camera_position;
    float screen_width;

    float3 camera_direction;
    float screen_height;

    float global_time;
    float frame_delta;
    float2 PerFrameConstBuffer_dummy;
};

// The number of dead particles in the system
cbuffer DeadList : register(b2)
{
    uint  dead_particles;
    uint3 dead_list_count;
};

// The number of alive particles this frame
cbuffer SortList : register(b3)
{
    uint  active_particles;
    uint3 active_list_count;
};

struct Particle
{
    float3 position;
    float distance_to_camera_sqr;

    float3 velocity;
    float mass;
    float3 acceleration;
    float mass_delta;

    float4 start_color;
    float4 end_color;
    float4 color;

    float age;
    float life_span;
    float start_size;
    float end_size;
};

// Function to calculate the streak radius in X and Y given the particles radius and velocity
float2 calcEllipsoidRadius(float radius, float2 viewSpaceVelocity)
{
    float minRadius = radius * max(1.0, 0.1 * length(viewSpaceVelocity));
    return float2(radius, minRadius);
}

// this creates the standard Hessian-normal-form plane equation from three points, 
// except it is simplified for the case where the first point is the origin
float3 CreatePlaneEquation(float3 b, float3 c)
{
    return normalize(cross(b, c));
}

// point-plane distance, simplified for the case where
// the plane passes through the origin
float GetSignedDistanceFromPlane(float3 p, float3 eqn)
{
    // dot( eqn.xyz, p.xyz ) + eqn.w, , except we know eqn.w is zero 
    // (see CreatePlaneEquation above)
    return dot(eqn, p);
}

// convert a point from post-projection space into view space
float3 ConvertProjToView(float4 p)
{
    p = mul(p, proj_inv);
    p /= p.w;
    
    return p.xyz;
}
