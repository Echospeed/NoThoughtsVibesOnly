#include "Particles.h"

ParticleSystem::ParticleSystem()
{
    particleMesh = nullptr;
    texture = nullptr;
    printf("Particle System Created\n");
}

ParticleSystem::~ParticleSystem()
{
    Free();
    printf("Particle System Destroyed\n");
}

void ParticleSystem::Load()
{
    /* --- Load Particle Mesh --- */
    const u8 segments = 20;
    const f32 steps = (PI * 2.0f) / segments;

    for (u16 i = 0; i < segments; i++)
    {
        f32 theta1 = i * steps;
        f32 theta2 = (i + 1) * steps;

        AEGfxTriAdd(
            0.0f, 0.0f, 0xFFFFFFFF, 0.5f, 0.5f,
            cosf(theta1) * 0.5f, sinf(theta1) * 0.5f, 0xFFFFFFFF, (cosf(theta1) + 1.0f) * 0.5f, (sinf(theta1) + 1.0f) * 0.5f,
            cosf(theta2) * 0.5f, sinf(theta2) * 0.5f, 0xFFFFFFFF, (cosf(theta2) + 1.0f) * 0.5f, (sinf(theta2) + 1.0f) * 0.5f
        );
    }

    particleMesh = AEGfxMeshEnd();
}

void ParticleSystem::Init(u8 _maxParticles, f32 _minVelX, f32 _maxVelX, f32 _minVelY, f32 _maxVelY, f32 _maxLifetime, f32 _size)
{
    /* --- Initialize Particle System Parameters --- */
    this->maxParticles = _maxParticles;
    this->maxLifetime = _maxLifetime;
    this->minVelX = _minVelX;
    this->maxVelX = _maxVelX;
    this->minVelY = _minVelY;
    this->maxVelY = _maxVelY;
    this->size = _size;
    this->isActive = false;

    particles.resize(maxParticles);
    activeParticles.resize(maxParticles);
    freeParticles.resize(maxParticles);

    /* --- Initialize Particle Texture --- */
    texture = AEGfxTextureLoad("particle.png");
    if (texture == nullptr)
    {
        printf("Failed to load particle texture, switching to Graphics\n");
    }
    else
    {
        printf("Particle texture loaded successfully\n");
    }

    for (u8 i = 0; i < maxParticles; i++)
    {
        particles[i].velocity = {this->minVelX, this->minVelY};
        particles[i].lifetime = this->maxLifetime;
        particles[i].size = this->size;
        particles[i].isActive = this->isActive;
		freeParticles[i] = i;
    }
}

void ParticleSystem::Render(Particles* particle)
{
    AEMtx33 rotation{}, scale{}, translation{}, transform{};

    AEMtx33Rot(&rotation, 0.0f);
    AEMtx33Scale(&scale, particle->size, particle->size);
    AEMtx33Trans(&translation, particle->position.x, particle->position.y);
    AEMtx33Concat(&transform, &scale, &rotation);
    AEMtx33Concat(&transform, &translation, &transform);

    if (texture == nullptr)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    }
    else
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(texture, 0, 0);
    }

    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetTransform(transform.m);

    AEGfxMeshDraw(particleMesh, AE_GFX_MDM_TRIANGLES);
}

void ParticleSystem::Emit(const AEVec2& position) 
{ 
    if(freeParticles.empty()) 
    { 
		printf("Particles.cpp - Emit: There's no more free particles to emit!\n");
        return; 
	}

	u8 index = freeParticles.back();
    freeParticles.pop_back();

    Particles& p = particles[index];
	p.position = position;
	p.lifetime = this->maxLifetime;
	p.isActive = true;

	activeParticles.push_back(index);
}

void ParticleSystem::Update(f32 deltaTime)
{
	s8 activeP = static_cast<s8>(activeParticles.size());
    for(s8 i = activeP - 1; i >= 0; i--) 
    {
		u8 index = activeParticles[i];
		Particles& p = particles[index];

        // Update position 
        p.position.x += p.velocity.x * deltaTime;
        p.position.y += p.velocity.y * deltaTime;

        // Decrease lifetime 
        p.lifetime -= deltaTime;

        // Deactivate if lifetime is over 
        if (p.lifetime <= 0.0f)
        {
            p.isActive = false;

			// Remove from active list and add to free list
			activeParticles[i] = activeParticles.back();
			activeParticles.pop_back();

			freeParticles.push_back(index);
        }
    }
}

void ParticleSystem::Free()
{
    if (texture)
    {
        AEGfxTextureUnload(texture);
        texture = nullptr;
    }

    if (particleMesh)
    {
        AEGfxMeshFree(particleMesh);
        particleMesh = nullptr;
    }

	activeParticles.clear();
	freeParticles.clear();
    particles.clear();
}