// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Pixels		: register(t0);
Texture2D Original		: register(t1);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 originalColor = Original.Sample(Sampler, input.uv);
	float4 pixelColor = Pixels.Sample(Sampler, input.uv);

	/*pixelColor.r -= .3f;
	pixelColor.g -= .3f;
	pixelColor.b -= .3f;

	pixelColor.r *= 1;
	pixelColor.g *= 1;
	pixelColor.b *= 1;

	if (pixelColor.r < 0) pixelColor.r = 0;
	if (pixelColor.g < 0) pixelColor.g = 0;
	if (pixelColor.b < 0) pixelColor.b = 0;*/

	return pixelColor + originalColor;
}