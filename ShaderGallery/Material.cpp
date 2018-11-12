#include "Material.h"

Material::Material(SimpleVertexShader * pVertexShader, SimplePixelShader * pPixelShader)
{
	SetVertexShader(pVertexShader);
	SetPixelShader(pPixelShader);
	texture = 0;
	specularMap = 0;
	sampleState = 0;
	sampleDescription = 0;
}

Material::~Material()
{
	delete vertexShader;
	delete pixelShader;
	delete sampleDescription;
	texture->Release();
	specularMap->Release();
	normalMap->Release();
	sampleState->Release();
}

SimpleVertexShader * Material::GetVertexShader() { return vertexShader; }

SimplePixelShader * Material::GetPixelShader() { return pixelShader; }

ID3D11ShaderResourceView * Material::GetTexture() { return texture; }

ID3D11ShaderResourceView * Material::GetSpecularMap() {	return specularMap; }

ID3D11ShaderResourceView * Material::GetNormalMap() { return normalMap; }

ID3D11SamplerState * Material::GetSampleState() { return sampleState; }

void Material::SetVertexShader(SimpleVertexShader * pVertexShader) { vertexShader = pVertexShader; }

void Material::SetPixelShader(SimplePixelShader * pPixelShader) { pixelShader = pPixelShader; }

void Material::SetTexture(ID3D11Device * device, ID3D11DeviceContext * context, wchar_t * fileName)
{
	sampleDescription = new D3D11_SAMPLER_DESC();
	sampleDescription->AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDescription->AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDescription->AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDescription->BorderColor[0] = 0.0f;
	sampleDescription->BorderColor[1] = 0.0f;
	sampleDescription->BorderColor[2] = 0.0f;
	sampleDescription->BorderColor[3] = 0.0f;
	sampleDescription->ComparisonFunc = D3D11_COMPARISON_NEVER;
	sampleDescription->Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sampleDescription->MaxAnisotropy = 0;
	sampleDescription->MaxLOD = D3D11_FLOAT32_MAX;
	sampleDescription->MinLOD = 0;
	sampleDescription->MipLODBias = 0;

	device->CreateSamplerState(sampleDescription, &sampleState);

	CreateWICTextureFromFile(device, context, fileName, 0, &texture);
}

void Material::SetSpecularMap(ID3D11Device * device, ID3D11DeviceContext * context, wchar_t * fileName)
{
	CreateWICTextureFromFile(device, context, fileName, 0, &specularMap);
}

void Material::SetNormalMap(ID3D11Device * device, ID3D11DeviceContext * context, wchar_t * fileName)
{
	CreateWICTextureFromFile(device, context, fileName, 0, &normalMap);
}
