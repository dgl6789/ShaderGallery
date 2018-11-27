
cbuffer Data : register(b0)
{
	float pixelWidth;
	float pixelHeight;
	int blurAmount;
	int xDir;
}


// Defines the input to this pixel shader
// - Should match the output of our corresponding vertex shader
struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : TEXCOORD0;
};

// Textures and such
Texture2D Pixels		: register(t0);
SamplerState Sampler	: register(s0);


// Entry point for this pixel shader
float4 main(VertexToPixel input) : SV_TARGET
{
	float4 totalColor = float4(0,0,0,0);
	uint sampleCount = 0;

	if (xDir == 1) {
		for (int x = -blurAmount; x <= blurAmount; x++)
		{
			float2 uv = input.uv + float2(x * pixelWidth, 0);
			float4 startColor = Pixels.Sample(Sampler, uv);

			startColor.r -= .2;
			startColor.g -= .2;
			startColor.b -= .2;

			totalColor += startColor;

			sampleCount++;
		}
	}
	else {
		for (int y = -blurAmount; y <= blurAmount; y++)
		{
			float2 uv = input.uv + float2(0, y * pixelHeight);
			totalColor += Pixels.Sample(Sampler, uv);

			sampleCount++;
		}
	}


	return totalColor / sampleCount;
}