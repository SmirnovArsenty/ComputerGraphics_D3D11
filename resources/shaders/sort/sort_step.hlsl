RWStructuredBuffer<float2> data : register(u0);

cbuffer NumElements : register(b0)
{
    int4 num_elements;
};

cbuffer SortConstants : register(b1)
{
    int4 job_params;
};

[numthreads(256, 1, 1)]
void CSMain(uint3 Gid  : SV_GroupID,
            uint3 GTid : SV_GroupThreadID)
{
    int4 tgp;

    tgp.x = Gid.x * 256;
    tgp.y = 0;
    tgp.z = num_elements.x;
    tgp.w = min(512, max(0, num_elements.x - Gid.x * 512));

    uint localID = tgp.x + GTid.x; // calculate threadID within this sortable-array

    uint index_low = localID & (job_params.x - 1);
    uint index_high = 2 * (localID - index_low);

    uint index     = tgp.y + index_high + index_low;
    uint nSwapElem = tgp.y + index_high + job_params.y + job_params.z * index_low;

    if (nSwapElem < uint(tgp.y + tgp.z))
    {
        float2 a = data[index];
        float2 b = data[nSwapElem];

        if (a.x > b.x)
        { 
            data[index] = b;
            data[nSwapElem] = a;
        }
    }
}
