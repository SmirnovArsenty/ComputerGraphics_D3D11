#include "common.h"
AppendStructuredBuffer<BvhNode> bvh_model : register(0);
[numthreads(256,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID ) {
    BvhNode node = (BvhNode)0;
    bvh_model.Append(node);
}
