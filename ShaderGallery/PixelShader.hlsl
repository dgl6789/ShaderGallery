
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
	float4 position		: SV_POSITION;
	float3 normal		: NORMAL;
	float3 tangent		: TANGENT;
	float3 worldPos		: POSITION;
	float2 uv			: TEXCOORD;
	float4 posForShadow : POSITION1; // Shadow map position information
};

struct DirectionalLight {
	float4 AmbientColor;
	float4 DiffuseColor;
	float3 Direction;
};

cbuffer Light : register(b1) {
	DirectionalLight light;
};

cbuffer Camera : register(b2) {
	float3 cameraPosition;
}

cbuffer ReceiveShadows : register(b3) {
	int receiveShadows;
};

Texture2D diffuseTexture : register(t0);
Texture2D specularMap : register(t1);
Texture2D normalMap : register(t2);
Texture2D ShadowMap			: register(t3);
SamplerState basicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
	input.normal = normalize(input.normal);
	input.tangent = normalize(input.tangent);

	float4 surfaceColor = diffuseTexture.Sample(basicSampler, input.uv);


	// normal map calculations
	//if (normalMap != undefined) {
		float3 normalFromMap = normalMap.Sample(basicSampler, input.uv).rgb * 2 - 1;
		float3 N = input.normal;
		float3 T = normalize(input.tangent - N * dot(input.tangent, N)); // Ensure tangent is 90 degrees from normal
		float3 B = cross(T, N);
		float3x3 TBN = float3x3(T, B, N);
		input.normal = normalize(mul(normalFromMap, TBN));
	//}
	
	float3 lightDir = normalize(-light.Direction);
	float NdotL = dot(input.normal, lightDir);

	//specular calculation
	float3 reflection = reflect(-lightDir, input.normal);
	float3 dirToCamera = normalize(cameraPosition - input.worldPos);
	float specAmt = pow(saturate(dot(reflection, dirToCamera)), 64.0f);
	float4 specColor = specularMap.Sample(basicSampler, input.uv) * specAmt;

	NdotL = saturate(NdotL);

	// Shadow map calculations ---------------

	// posForShadow has the normalized device coords (-1 to 1) for rendering
	// into a render target, but we need to convert to the (0 - 1) space
	// for sampling FROM our shadow map
	float2 shadowUV = input.posForShadow.xy / input.posForShadow.w * 0.5f + 0.5f;
	shadowUV.y = 1.0f - shadowUV.y; // Flip the Y's

									// Calculate this pixel's depth from the LIGHT
	float depthFromLight = input.posForShadow.z / input.posForShadow.w;

	// Sample where this pixel WOULD have been in the shadow map,
	// and see how far the light saw along that ray
	float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight);

	float4 color;
	if (receiveShadows == 0) {
		color = surfaceColor * ((light.AmbientColor + shadowAmount * (light.DiffuseColor * NdotL) + specColor));
	}
	else {
		color = surfaceColor * ((light.AmbientColor + (light.DiffuseColor * NdotL) + specColor));
	}
	color.a = diffuseTexture.Sample(basicSampler, input.uv).a;
	return color;

}