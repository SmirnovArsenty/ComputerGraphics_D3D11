#include "common.h"

RWStructuredBuffer<Vertex> vertices_pool : register(u0);
RWStructuredBuffer<uint> indices_pool : register(u1);

RWStructuredBuffer<BvhNode> bvh_model : register(u2);

[numthreads(256,1,1)]
void CSMain(uint3 id : SV_DispatchThreadID) {
    Vertex p1 = vertex_pool[indices_pool[id * 3]];
    Vertex p2 = vertex_pool[indices_pool[id * 3 + 1]];
    Vertex p3 = vertex_pool[indices_pool[id * 3 + 2]];


}
