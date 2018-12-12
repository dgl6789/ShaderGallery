
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : UV;
	float4 color		: COLOR;
};

// Textures and such
Texture2D particle		: register(t0);
SamplerState trilinear	: register(s0);

// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	return particle.Sample(trilinear, input.uv) * input.color * input.color.a;
}