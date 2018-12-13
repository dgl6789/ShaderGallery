
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 direction	: DIRECTION;
};

// Texture-related variables
TextureCube SkyTex		: register(t0);
SamplerState SkySampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return SkyTex.Sample(SkySampler, input.direction);
}