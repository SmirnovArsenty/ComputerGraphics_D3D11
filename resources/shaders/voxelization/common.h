struct BvhNode
{
    float3 min_p;
    uint right_child;
    float3 max_p;
    uint left_child;
};

struct Vertex
{
    float3 position;
    float uv_x;
    float3 normal;
    float uv_y;
};

struct ModelData {
    float4 max_p;
    float4 min_p;
};
cbuffer ModelData_cbuffer : register(b0)
{
    ModelData model_data;
}