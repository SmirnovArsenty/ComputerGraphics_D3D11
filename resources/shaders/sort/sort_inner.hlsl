#if (SORT_SIZE > 2048)
    #error
#endif

#define NUM_THREADS (SORT_SIZE / 2)
#define INVERSION (16 * 2 + 8 * 3)

cbuffer NumElements : register(b0)
{
    int4 num_elements;
};

RWStructuredBuffer<float2> data : register(u0);

groupshared float2 local_buffer[SORT_SIZE];

[numthreads(NUM_THREADS, 1, 1)]
void CSMain(uint3 Gid   : SV_GroupID,
            uint3 DTid  : SV_DispatchThreadID,
            uint3 GTid  : SV_GroupThreadID,
            uint  GI    : SV_GroupIndex)
{
    int4 tgp;

    tgp.x = Gid.x * 256;
    tgp.y = 0;
    tgp.z = num_elements.x;
    tgp.w = min(512, max(0, num_elements.x - Gid.x * 512));

    int global_base_index = tgp.y + tgp.x * 2 + GTid.x;
    int local_base_index = GI;
    int i;

    // Load shared data
    [unroll]
    for (i = 0; i < 2; ++i)
    {
        if (GI + i * NUM_THREADS < uint(tgp.w)) {
            local_buffer[local_base_index + i * NUM_THREADS] = data[global_base_index + i * NUM_THREADS];
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // sort threadgroup shared memory
    for (int nMergeSubSize = SORT_SIZE >> 1; nMergeSubSize > 0; nMergeSubSize = nMergeSubSize >> 1)
    {
        int tmp_index = GI;
        int index_low = tmp_index & (nMergeSubSize - 1);
        int index_high = 2 * (tmp_index - index_low);
        int index = index_high + index_low;

        uint nSwapElem = index_high + nMergeSubSize + index_low;

        if (nSwapElem < uint(tgp.w))
        {
            float2 a = local_buffer[index];
            float2 b = local_buffer[nSwapElem];

            if (a.x > b.x)
            { 
                local_buffer[index] = b;
                local_buffer[nSwapElem] = a;
            }
        }
        GroupMemoryBarrierWithGroupSync();
    }

    // Store shared data
    [unroll]
    for (i = 0; i < 2; ++i)
    {
        if (GI + i * NUM_THREADS < uint(tgp.w)) {
            data[global_base_index + i * NUM_THREADS] = local_buffer[local_base_index + i * NUM_THREADS];
        }
    }
}
