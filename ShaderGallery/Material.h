#pragma once
#include "Game.h"

class Material
{
public:
	Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader);
	~Material();

	SimpleVertexShader* GetVertexShader();
	SimplePixelShader* GetPixelShader();
	
	void SetVertexShader(SimpleVertexShader* pVertexShader);
	void SetPixelShader(SimplePixelShader* pPixelShader);

private:
	SimpleVertexShader* vertexShader;
	SimplePixelShader* pixelShader;
};

