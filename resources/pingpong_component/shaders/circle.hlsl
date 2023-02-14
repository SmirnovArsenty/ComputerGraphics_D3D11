struct VS_IN
{
    uint index : SV_VertexID;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

cbuffer CircleInfo : register(b0)
{
    float2 position;
    float radius;
    uint triangle_count;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    // setup out pos by index
    if (input.index == 0)
    {
        output.pos = float4(position.xy, .5f, 1.f);
    }
    else
    {
        static const float PI = 3.14159265f;
        float angle = 2 * PI / triangle_count;
        float x_pos = position.x + radius * cos(angle * input.index);
        float y_pos = position.y + radius * sin(angle * input.index);
        output.pos = float4(x_pos, y_pos, .5f, 1.f);
    }

    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    return (1.f).xxxx;
}
