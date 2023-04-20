RWBuffer<uint> args : register( u0 );

cbuffer NumElements : register( b0 )
{
    int4 num_elements;
};

[numthreads(1, 1, 1)]
void CSMain(uint3 id : SV_DispatchThreadID)
{
    args[0] = ((num_elements.x - 1) >> 9) + 1;
    args[1] = 1;
    args[2] = 1;
    args[3] = 0;
}
