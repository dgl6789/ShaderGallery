#include "Material.h"



Material::Material(SimpleVertexShader* pVertexShader, SimplePixelShader* pPixelShader)
{
	SetVertexShader(pVertexShader);
	SetPixelShader(pPixelShader);
}


Material::~Material()
{
	delete vertexShader;
	delete pixelShader;
}

SimpleVertexShader * Material::GetVertexShader() { return vertexShader; }

SimplePixelShader * Material::GetPixelShader() { return pixelShader; }

void Material::SetVertexShader(SimpleVertexShader * pVertexShader) { vertexShader = pVertexShader; }

void Material::SetPixelShader(SimplePixelShader * pPixelShader) { pixelShader = pPixelShader; }
