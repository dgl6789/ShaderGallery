#pragma once
#include "Game.h"

class Material
{
public:
	Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader, ID3D11ShaderResourceView * pSRV, ID3D11SamplerState* pSampleState);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();
	ID3D11ShaderResourceView* GetSRV();
	ID3D11SamplerState* GetSampleState();
	
	void SetVertexShader(SimpleVertexShader* pVertexShader);
	void SetPixelShader(SimplePixelShader* pPixelShader);
	void SetSRV(ID3D11ShaderResourceView* pSRV);
	void SetSampleState(ID3D11SamplerState* pSampleState);

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
	ID3D11ShaderResourceView* srv;
	ID3D11SamplerState* sampleState;
};

