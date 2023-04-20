#if( SORT_SIZE>4096 )
    #error due to LDS size SORT_SIZE must be 4096 or smaller
#else
    #define ITEMS_PER_GROUP ( SORT_SIZE )
#endif

#define HALF_SIZE   (SORT_SIZE / 2)
#define ITERATIONS  (HALF_SIZE > 1024 ? HALF_SIZE / 1024 : 1)
#define NUM_THREADS (HALF_SIZE / ITERATIONS)
#define INVERSION   (16 * 2 + 8 * 3)

cbuffer NumElementsCB : register( b0 )
{
    int4 num_elements;
};

RWStructuredBuffer<float2> Data : register( u0 );

groupshared float2 local_buffer[SORT_SIZE];

[numthreads(NUM_THREADS, 1, 1)]
void CSMain(uint3 Gid  : SV_GroupID,
            uint3 DTid : SV_DispatchThreadID,
            uint3 GTid : SV_GroupThreadID,
            uint  GI   : SV_GroupIndex )
{
    int global_base_index = (Gid.x * SORT_SIZE) + GTid.x;
    int local_base_index = GI;

    int num_elements_in_thread_group = min(SORT_SIZE, num_elements.x - (Gid.x * SORT_SIZE));

    // Load shared data
    int i;
    [unroll]
    for (i = 0; i < 2 * ITERATIONS; ++i)
    {
        if (GI + i * NUM_THREADS < uint(num_elements_in_thread_group)) {
            local_buffer[local_base_index + i * NUM_THREADS] = Data[global_base_index + i * NUM_THREADS];
        }
    }
    GroupMemoryBarrierWithGroupSync();

    // Bitonic sort
    for (unsigned int nMergeSize = 2; nMergeSize <= SORT_SIZE; nMergeSize = nMergeSize * 2)
    {
        for (int nMergeSubSize = nMergeSize >> 1; nMergeSubSize > 0; nMergeSubSize = nMergeSubSize >> 1)
        {
            [unroll]
            for (i = 0; i < ITERATIONS; ++i)
            {
                int tmp_index = GI + NUM_THREADS * i;
                int index_low = tmp_index & (nMergeSubSize - 1);
                int index_high = 2 * (tmp_index - index_low);
                int index = index_high + index_low;

                unsigned int nSwapElem = uint(nMergeSubSize) == nMergeSize >> 1 ? uint(index_high + (2 * nMergeSubSize - 1) - index_low) : uint(index_high + nMergeSubSize + index_low);
                if (nSwapElem < uint(num_elements_in_thread_group))
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
        }
    }
    
    // Store shared data
    [unroll]
    for (i = 0; i < 2 * ITERATIONS; ++i)
    {
        if (GI + i * NUM_THREADS < uint(num_elements_in_thread_group)) {
            Data[global_base_index + i * NUM_THREADS] = local_buffer[local_base_index + i * NUM_THREADS];
        }
    }
}
