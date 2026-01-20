#pragma once
#include "AEEngine.h"
#include <vector>

struct Particles
{
	AEVec2 position{};
	AEVec2 velocity{};
	f32 lifetime{};
	f32 size{};
	bool isActive{};
};

class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();
	void Load();
	void Init(u8 maxParticles, f32 minVelX, f32 maxVelX, f32 minVelY, f32 maxVelY, f32 maxLifetime, f32 size);
	void Update(f32 dt);
	void Render(Particles* particle);
	void Emit(const AEVec2& emitPosition);
	void Free();

private:
	std::vector<Particles> particles;
	std::vector<u8> activeParticles;
	std::vector<u8> freeParticles;

	f32 minVelX{}, maxVelX{};
	f32 minVelY{}, maxVelY{};
	f32 maxLifetime{};
	u8 maxParticles{};
	f32 size{};
	bool isActive{ false };

	AEGfxVertexList* particleMesh{ nullptr };
	AEGfxTexture* texture{ nullptr };
};
