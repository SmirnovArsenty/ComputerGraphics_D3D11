struct VS_IN
{
	// float4 pos : POSITION0;
	// float4 col : COLOR0;
	uint id : SV_VertexID;
};

struct PS_IN
{
	float4 pos : SV_POSITION;
 	float4 col : COLOR;
};

PS_IN VSMain( VS_IN input )
{
	PS_IN output = (PS_IN)0;

	if (input.id == 0) {
		output.pos = float4(0.5f, 0.5f, 0.5f, 1.0f);
		output.col = float4(1.0f, 0.0f, 0.0f, 1.0f);
	}
	if (input.id == 1) {
		output.pos = float4(-0.5f, -0.5f, 0.5f, 1.0f);
		output.col = float4(0.0f, 0.0f, 1.0f, 1.0f);
	}
	if (input.id == 2) {
		output.pos = float4(0.5f, -0.5f, 0.5f, 1.0f);
		output.col = float4(0.0f, 1.0f, 0.0f, 1.0f);
	}
	if (input.id == 3) {
		output.pos = float4(-0.5f, 0.5f, 0.5f, 1.0f);
		output.col = float4(1.0f, 1.0f, 1.0f, 1.0f);
	}

	// output.pos = input.pos;
	// output.col = input.col;
	
	return output;
}

float4 PSMain( PS_IN input ) : SV_Target
{
	float4 col = input.col;
#ifdef TEST
	if (input.pos.x > 400) col = TCOLOR;
#endif
	return col;
}