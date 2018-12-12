
cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;
};

struct VertexShaderInput
{
	float3 position		: POSITION;
	float2 uv			: UV;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
};

struct VertexToPixel
{
	float4 position		: SV_POSITION;
	float3 direction	: DIRECTION;
};

VertexToPixel main(VertexShaderInput input)
{
	VertexToPixel output;

	matrix modifiedView = view;
	modifiedView._41 = 0;
	modifiedView._42 = 0;
	modifiedView._43 = 0;

	matrix viewProjection = mul(modifiedView, projection);
	output.position = mul(float4(input.position, 1.0f), viewProjection);

	output.position.z = output.position.w;

	output.direction = input.position;

	return output;
}