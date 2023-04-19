AppendStructuredBuffer<uint> dead_list : register( u0 );

[numthreads(256,1,1)]
void CSMain( uint3 id : SV_DispatchThreadID )
{
    dead_list.Append( id.x );
}
