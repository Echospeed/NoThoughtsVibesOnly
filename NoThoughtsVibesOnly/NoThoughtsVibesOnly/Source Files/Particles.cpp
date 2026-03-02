// ============================================================================
// Particles.cpp - Particle System Implementation
// ============================================================================
// Object-pooled particle system with configurable velocity, lifetime, and size.
// Uses a free-list pattern: freeParticles holds available indices;
// activeParticles holds indices of currently live particles.
//
// USAGE:
// ----------------------------------------------------------------------------
//   ParticleSystem ps;
//
//   // In Load():
//   ps.Load();
//
//   // In Init():
//   ps.Init(
//       50,         // maxParticles
//       -100, 100,  // minVelX, maxVelX
//       -100, 100,  // minVelY, maxVelY
//       1.5f,       // maxLifetime (seconds)
//       8.0f        // size (world units)
//   );
//
//   // In Update():
//   ps.Update(deltaTime);
//   // Emit at a position:
//   ps.Emit(someWorldPosition);
//
//   // In Draw() (call Render per active particle):
//   for (u8 idx : activeParticles) ps.Render(&particles[idx]);
//   // NOTE: activeParticles is private - expose via getter if needed.
//
//   // In Unload():
//   ps.Free();
// ============================================================================

#include "pch.hpp"
#include "Particles.hpp"

// ============================================================================
// Constructor / Destructor
// ============================================================================
ParticleSystem::ParticleSystem()
    : particleMesh(nullptr)
    , texture(nullptr)
{
    printf("[Particles] System created.\n");
}

ParticleSystem::~ParticleSystem()
{
    Free();
    printf("[Particles] System destroyed.\n");
}

// ============================================================================
// Load
// ============================================================================
// Builds the circle mesh used to draw each particle.
// A 20-segment fan gives a smooth circle at particle sizes.
// ============================================================================
void ParticleSystem::Load()
{
    const u8  segments = 20;
    const f32 step = (PI * 2.0f) / segments;

    AEGfxMeshStart();

    for (u8 i = 0; i < segments; ++i)
    {
        const f32 t1 = i * step;
        const f32 t2 = (i + 1) * step;

        AEGfxTriAdd(
            0.0f, 0.0f, 0xFFFFFFFF, 0.5f, 0.5f,
            cosf(t1) * 0.5f, sinf(t1) * 0.5f, 0xFFFFFFFF, (cosf(t1) + 1.0f) * 0.5f, (sinf(t1) + 1.0f) * 0.5f,
            cosf(t2) * 0.5f, sinf(t2) * 0.5f, 0xFFFFFFFF, (cosf(t2) + 1.0f) * 0.5f, (sinf(t2) + 1.0f) * 0.5f
        );
    }

    particleMesh = AEGfxMeshEnd();
}

// ============================================================================
// Init
// ============================================================================
// Sets particle system parameters and pre-allocates the particle pool.
// All indices start in freeParticles (available for emission).
// ============================================================================
void ParticleSystem::Init(u8 _maxParticles,
    f32 _minVelX, f32 _maxVelX,
    f32 _minVelY, f32 _maxVelY,
    f32 _maxLifetime, f32 _size)
{
    maxParticles = _maxParticles;
    maxLifetime = _maxLifetime;
    minVelX = _minVelX;
    maxVelX = _maxVelX;
    minVelY = _minVelY;
    maxVelY = _maxVelY;
    size = _size;
    isActive = false;

    particles.resize(maxParticles);
    activeParticles.resize(0);
    freeParticles.resize(maxParticles);

    // Optionally load a texture; fall back to colour rendering if missing
    texture = AEGfxTextureLoad("particle.png");
    if (!texture)
        printf("[Particles] No texture found - using colour mode.\n");

    // Initialise all particles to their default state and mark as free
    for (u8 i = 0; i < maxParticles; ++i)
    {
        particles[i].velocity = { minVelX, minVelY };
        particles[i].lifetime = maxLifetime;
        particles[i].size = size;
        particles[i].isActive = false;
        freeParticles[i] = i;
    }
}

// ============================================================================
// Render
// ============================================================================
// Draws a single particle at its current position and size.
// ============================================================================
void ParticleSystem::Render(Particles& particle)
{
    AEMtx33 rot, sc, tr, transform;

    AEMtx33Rot(&rot, 0.0f);
    AEMtx33Scale(&sc, particle.size, particle.size);
    AEMtx33Trans(&tr, particle.position.x, particle.position.y);
    AEMtx33Concat(&transform, &sc, &rot);
    AEMtx33Concat(&transform, &tr, &transform);

    if (texture)
    {
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(texture, 0, 0);
    }
    else
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    }

    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(particleMesh, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// Emit
// ============================================================================
// Activates one particle from the free list at the given world position.
// No-ops if all particles are in use.
// ============================================================================
void ParticleSystem::Emit(const AEVec2& position)
{
    if (freeParticles.empty())
    {
        printf("[Particles] Pool exhausted - cannot emit.\n");
        return;
    }

    const u8 index = freeParticles.back();
    freeParticles.pop_back();

    Particles& p = particles[index];
    p.position = position;
    p.lifetime = maxLifetime;
    p.isActive = true;

    activeParticles.push_back(index);
}

// ============================================================================
// Update
// ============================================================================
// Moves all active particles and removes those whose lifetime has expired.
// Uses reverse iteration so removal from activeParticles is safe.
// ============================================================================
void ParticleSystem::Update(f32 deltaTime)
{
    for (s8 i = static_cast<s8>(activeParticles.size()) - 1; i >= 0; --i)
    {
        const u8 index = activeParticles[i];
        Particles& p = particles[index];

        // Move particle
        p.position.x += p.velocity.x * deltaTime;
        p.position.y += p.velocity.y * deltaTime;

        // Count down lifetime
        p.lifetime -= deltaTime;

        if (p.lifetime <= 0.0f)
        {
            // Return index to free list via swap-and-pop (O(1) removal)
            p.isActive = false;
            activeParticles[i] = activeParticles.back();
            activeParticles.pop_back();
            freeParticles.push_back(index);
        }
    }
}

// ============================================================================
// Free
// ============================================================================
// Unloads GPU resources and clears all particle data.
// ============================================================================
void ParticleSystem::Free()
{
    if (texture) { AEGfxTextureUnload(texture);      texture = nullptr; }
    if (particleMesh) { AEGfxMeshFree(particleMesh);      particleMesh = nullptr; }

    activeParticles.clear();
    freeParticles.clear();
    particles.clear();
}