#pragma once
#include "Game.h"
#include "WICTextureLoader.h"

class Material
{
public:
	Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();
	ID3D11ShaderResourceView* GetSRV();
	ID3D11SamplerState* GetSampleState();
	
	void SetVertexShader(SimpleVertexShader* pVertexShader);
	void SetPixelShader(SimplePixelShader* pPixelShader);
	void SetTexture(ID3D11Device* device, ID3D11DeviceContext* context, wchar_t* fileName);

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* sampleState;
	D3D11_SAMPLER_DESC * sampleDescription;
};

