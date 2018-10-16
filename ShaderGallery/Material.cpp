#include "Material.h"

Material::Material(SimpleVertexShader * pVertexShader, SimplePixelShader * pPixelShader, ID3D11ShaderResourceView * pSRV, ID3D11SamplerState * pSampleState)
{
	SetVertexShader(pVertexShader);
	SetPixelShader(pPixelShader);
	srv = pSRV;
	sampleState = pSampleState;
	//SetSRV(pSRV);
	//SetSampleState(pSampleState);
}

Material::~Material()
{
	delete vertexShader;
	delete pixelShader;
}

SimpleVertexShader * Material::GetVertexShader() { return vertexShader; }

SimplePixelShader * Material::GetPixelShader() { return pixelShader; }

ID3D11ShaderResourceView * Material::GetSRV() { return srv; }

ID3D11SamplerState * Material::GetSampleState() { return sampleState; }

void Material::SetVertexShader(SimpleVertexShader * pVertexShader) { vertexShader = pVertexShader; }

void Material::SetPixelShader(SimplePixelShader * pPixelShader) { pixelShader = pPixelShader; }

void Material::SetSRV(ID3D11ShaderResourceView * pSRV) { srv = pSRV; }

void Material::SetSampleState(ID3D11SamplerState * pSampleState) { sampleState = pSampleState; }
