struct OutputVS
{
	float4 position : SV_Position;
	float3 color : COLOR;
};

// Pixel shader main function
float4 PS(OutputVS inp) : SV_Target
{
	return float4(inp.color, 1);
};