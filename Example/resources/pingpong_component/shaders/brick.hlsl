struct VS_IN
{
    uint index : SV_VertexID;
};

struct PS_IN
{
    float4 pos : SV_POSITION;
};

cbuffer BrickInfo : register(b0)
{
    float2 position;
    float width;
    float height;
};

PS_IN VSMain( VS_IN input )
{
    PS_IN output = (PS_IN)0;

    float half_width = width / 2;
    float half_height = height / 2;
    if (input.index == 0) {
        output.pos = float4(position.x + half_height, position.y + half_width, .5f, 1.f);
    }
    if (input.index == 1) {
        output.pos = float4(position.x - half_height, position.y - half_width, .5f, 1.f);
    }
    if (input.index == 2) {
        output.pos = float4(position.x + half_height, position.y - half_width, .5f, 1.f);
    }
    if (input.index == 3) {
        output.pos = float4(position.x - half_height, position.y + half_width, .5f, 1.f);
    }

    return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
    return (1.f).xxxx;
}
