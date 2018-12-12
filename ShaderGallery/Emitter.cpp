#include "Emitter.h"

Emitter::Emitter(
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
)
{
	this->position = position;

	// Initial values for particle
	this->startVelocity = startVelocity;
	this->startColor = startColor;
	this->endColor = endColor;
	this->startSize = startSize;
	this->endSize = endSize;

	this->maxParticleCount = maxParticleCount;

	this->emissionRate = emissionRate;
	secondsPerParticle = 1.0f / emissionRate;

	this->lifetime = lifetime;

	particleList = new Particle[maxParticleCount];

	localParticleVertices = new ParticleVertex[4 * maxParticleCount];
	for (int i = 0; i < maxParticleCount * 4; i += 4)
	{
		localParticleVertices[i + 0].uv = XMFLOAT2(0, 0);
		localParticleVertices[i + 1].uv = XMFLOAT2(1, 0);
		localParticleVertices[i + 2].uv = XMFLOAT2(1, 1);
		localParticleVertices[i + 3].uv = XMFLOAT2(0, 1);
	}

	this->firstAliveIndex = 0;
	this->firstDeadIndex = 0;

	timeSinceEmit = 0;
	livingParticleCount = 0;

	this->texture = texture;
	this->vs = vs;
	this->ps = ps;

	// Define the buffers
	D3D11_BUFFER_DESC vbDesc = {};
	vbDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	vbDesc.Usage = D3D11_USAGE_DYNAMIC;
	vbDesc.ByteWidth = sizeof(ParticleVertex) * 4 * maxParticleCount;
	device->CreateBuffer(&vbDesc, 0, &vertexBuffer);

	// Index buffer data
	unsigned int* indices = new unsigned int[maxParticleCount * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticleCount * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Regular index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticleCount * 6;
	device->CreateBuffer(&ibDesc, &indexData, &indexBuffer);

	delete[] indices;
}


Emitter::~Emitter()
{
	delete[] particleList;
	delete[] localParticleVertices;
	vertexBuffer->Release();
	indexBuffer->Release();
}

void Emitter::Update(float dt)
{
	// Are the living particles continuous?
	if (firstAliveIndex <= firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
		{
			UpdateParticle(dt, i);
		}
	}

	// They hop the border between 0 and max; handle accordingly
	else
	{
		for (int i = firstAliveIndex; i < maxParticleCount; i++)
		{
			UpdateParticle(dt, i);
		}

		for (int i = 0; i < firstDeadIndex; i++)
		{
			UpdateParticle(dt, i);
		}
	}

	timeSinceEmit += dt;

	// Do we need to create a new particle?
	if (timeSinceEmit >= secondsPerParticle)
	{
		SpawnParticle();
	}
}

void Emitter::UpdateParticle(float dt, int index)
{
	// Failsafe: Make sure the particle's even alive first
	if (particleList[index].age >= lifetime)
	{
		return;
	}

	particleList[index].age += dt;

	// If the particle has reached its end, update the list accordingly!
	if (particleList[index].age >= lifetime)
	{
		firstAliveIndex++;
		firstAliveIndex %= maxParticleCount;

		livingParticleCount--;

		// No need to continue updating since the particle just died
		return;
	}

	// Update position based on velocity;
	particleList[index].position.x += particleList[index].velocity.x;
	particleList[index].position.y += particleList[index].velocity.y;
	particleList[index].position.z += particleList[index].velocity.z;

	// LERP!
	float agePercent = particleList[index].age / lifetime;

	// Lerp size
	particleList[index].size = (startSize * (1.0f - agePercent)) + (endSize * agePercent);

	// Lerp color
	XMStoreFloat4(
		&particleList[index].color,
		XMVectorLerp(
			XMLoadFloat4(&startColor),
			XMLoadFloat4(&endColor),
			agePercent));
}

void Emitter::SpawnParticle()
{
	// If too many particles already exist, we can't spawn one
	if (livingParticleCount == maxParticleCount)
	{
		return;
	}

	// Reset new particle's data
	particleList[firstDeadIndex].position = position;
	particleList[firstDeadIndex].color = startColor;
	particleList[firstDeadIndex].size = startSize;
	particleList[firstDeadIndex].age = 0.0f;

	// Randomize the particle's velocity, because FIRE

	int angles = 91;					// Should always be odd and > 0; the number of angles the fire can fly at
	float calmness = 10000.0f;		// The higher this number, the calmer the fire

	particleList[firstDeadIndex].velocity = startVelocity;
	particleList[firstDeadIndex].velocity.x = ((rand() % angles) - (angles / 2)) / (calmness * angles);
	particleList[firstDeadIndex].velocity.z = ((rand() % angles) - (angles / 2)) / (calmness * angles);

	firstDeadIndex++;
	firstDeadIndex %= maxParticleCount;

	livingParticleCount++;

	// We spawned a new particle, it has now been 0 seconds since last spawn
	timeSinceEmit = 0.0f;
}

void Emitter::CopyParticlesToGPU(ID3D11DeviceContext* context)
{
	// Update local buffer (living particles only as a speed up)

	// Are the living particles continuous?
	if (firstAliveIndex < firstDeadIndex)
	{
		for (int i = firstAliveIndex; i < firstDeadIndex; i++)
		{
			CopyParticle(i);
		}
	}

	// They hop the border between 0 and max; handle accordingly
	else
	{
		for (int i = firstAliveIndex; i < maxParticleCount; i++)
		{
			CopyParticle(i);
		}

		for (int i = 0; i < firstDeadIndex; i++)
		{
			CopyParticle(i);
		}
	}

	// All particles copied locally - send whole buffer to GPU
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	context->Map(vertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);

	memcpy(mapped.pData, localParticleVertices, sizeof(ParticleVertex) * 4 * maxParticleCount);

	context->Unmap(vertexBuffer, 0);
}

void Emitter::CopyParticle(int index)
{
	// Convert index to ensure we get the correct vertex
	int i = index * 4;

	localParticleVertices[i + 0].position = particleList[index].position;
	localParticleVertices[i + 1].position = particleList[index].position;
	localParticleVertices[i + 2].position = particleList[index].position;
	localParticleVertices[i + 3].position = particleList[index].position;

	localParticleVertices[i + 0].size = particleList[index].size;
	localParticleVertices[i + 1].size = particleList[index].size;
	localParticleVertices[i + 2].size = particleList[index].size;
	localParticleVertices[i + 3].size = particleList[index].size;

	localParticleVertices[i + 0].color = particleList[index].color;
	localParticleVertices[i + 1].color = particleList[index].color;
	localParticleVertices[i + 2].color = particleList[index].color;
	localParticleVertices[i + 3].color = particleList[index].color;
}

void Emitter::Draw(ID3D11DeviceContext* context, Camera* camera)
{
	// Copy to dynamic buffer
	CopyParticlesToGPU(context);

	// Set up buffers
	UINT stride = sizeof(ParticleVertex);
	UINT offset = 0;
	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->SetShader();
	vs->CopyAllBufferData();

	ps->SetShaderResourceView("particle", texture);
	ps->SetShader();
	ps->CopyAllBufferData();

	// Draw the correct parts of the buffer
	if (firstAliveIndex < firstDeadIndex)
	{
		context->DrawIndexed(livingParticleCount * 6, firstAliveIndex * 6, 0);
	}
	else
	{
		// Draw first half (0 -> dead)
		context->DrawIndexed(firstDeadIndex * 6, 0, 0);

		// Draw second half (alive -> max)
		context->DrawIndexed((maxParticleCount - firstAliveIndex) * 6, firstAliveIndex * 6, 0);
	}

}