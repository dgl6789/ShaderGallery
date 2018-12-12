#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

#include "Camera.h"
#include "SimpleShader.h"

class Camera;

struct Particle
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
	DirectX::XMFLOAT3 velocity;
	float size;
	float age;
};

struct ParticleVertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT2 uv;
	DirectX::XMFLOAT4 color;
	float size;
};

class Emitter
{
public:
	Emitter(
		DirectX::XMFLOAT3 position,
		DirectX::XMFLOAT3 startVelocity,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 endColor,
		float startSize,
		float endSize,
		unsigned int maxParticleCount,
		float emissionRate,
		float lifetime,
		ID3D11Device* device,
		SimpleVertexShader* vs,
		SimplePixelShader* ps,
		ID3D11ShaderResourceView* texture
	);
	~Emitter();

	void Update(float dt);

	void SpawnParticle();
	void UpdateParticle(float dt, int index);

	void CopyParticlesToGPU(ID3D11DeviceContext* context);
	void CopyParticle(int index);
	void Draw(ID3D11DeviceContext* context, Camera* camera);

private:
	DirectX::XMFLOAT3 position;
	Particle* particleList;
	float timeSinceEmit;
	unsigned int livingParticleCount;

	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	float startSize;
	float endSize;

	unsigned int maxParticleCount;
	unsigned int firstAliveIndex;
	unsigned int firstDeadIndex;
	float emissionRate;
	float secondsPerParticle;
	float lifetime;

	// Rendering vars
	ParticleVertex* localParticleVertices;
	ID3D11Buffer* vertexBuffer;
	ID3D11Buffer* indexBuffer;

	ID3D11ShaderResourceView* texture;
	SimpleVertexShader* vs;
	SimplePixelShader* ps;
};

