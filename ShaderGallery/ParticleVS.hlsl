
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: UV;
	float4 color		: COLOR;
	float size			: SIZE;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float2 uv           : UV;
	float4 color		: COLOR;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix viewProj = mul(view, projection);
	output.position = mul(float4(input.position, 1.0f), viewProj);

	float2 offset = input.uv * 2 - 1;
	offset *= input.size;
	offset.y *= -1;
	output.position.xy += offset;

	output.uv = input.uv;
	output.color = input.color;

	return output;
}