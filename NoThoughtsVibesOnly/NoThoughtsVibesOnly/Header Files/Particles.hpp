#pragma once
#include "AEEngine.h"
#include <vector>

// ============================================================================
// Particles.hpp - Particle Engine
// ============================================================================
// Supports two effect presets out of the box:
//
//   ParticleSystem::MakeSmoke()     - grey upward drift, fades + shrinks
//   ParticleSystem::MakeExplosion() - orange burst, fast fade + shrink
//
// Or configure manually via Init() for custom effects.
//
// SHARED MESH:
//   Call ParticleSystem::LoadSharedMesh() once in Game_Load().
//   Call ParticleSystem::FreeSharedMesh() once in Game_Unload().
//   Individual systems do NOT own a mesh.
//
// USAGE:
//   ParticleSystem ps;
//   ps = ParticleSystem::MakeSmoke();
//   ps.Emit(position);                 // stream: 1 particle per call
//   ps.EmitBurst(position, 20);        // burst:  N particles at once
//   ps.Update(dt);
//   ps.Render();
//   ps.Free();
// ============================================================================

struct Particle
{
    AEVec2 position{};
    AEVec2 velocity{};
    f32    lifetime{};
    f32    maxLifetime{};
    f32    size{};
    f32    startSize{};
    f32    endSize{};
    f32    startR{}, startG{}, startB{};
    f32    endR{}, endG{}, endB{};
    f32    alpha{};
    bool   isActive{};
};

class ParticleSystem
{
public:
    ParticleSystem() = default;
    ~ParticleSystem() { Free(); }

    void Init(int  maxParticles,
        f32  minVelX, f32 maxVelX,
        f32  minVelY, f32 maxVelY,
        f32  minLifetime, f32 maxLifetime,
        f32  startSize, f32 endSize,
        f32  startR, f32 startG, f32 startB,
        f32  endR, f32 endG, f32 endB,
        f32  gravity = 0.0f,
        f32  drag = 0.0f);

    void Update(f32 dt);
    void Render();
    void Free();

    void Emit(const AEVec2& position);
    void EmitBurst(const AEVec2& position, int count);

    // Preset factories
    static ParticleSystem MakeSmoke();
    static ParticleSystem MakeExplosion();

    // Call once in Game_Load / Game_Unload
    static void LoadSharedMesh();
    static void FreeSharedMesh();

    bool HasActiveParticles() const { return !activeParticles.empty(); }

private:
    std::vector<Particle> particles;
    std::vector<int>      activeParticles;
    std::vector<int>      freeParticles;

    f32 minVelX{}, maxVelX{};
    f32 minVelY{}, maxVelY{};
    f32 minLifetime{}, maxLifetime{};
    f32 startSize{}, endSize{};
    f32 startR{}, startG{}, startB{};
    f32 endR{}, endG{}, endB{};
    f32 gravity{};
    f32 drag{};

    static AEGfxVertexList* s_Mesh;

    f32 RandRange(f32 lo, f32 hi) const;
};