// ============================================================================
// Particles.cpp - Particle Engine Implementation
// ============================================================================
//
// ============================================================================
// HOW TO USE THIS SYSTEM - QUICK GUIDE
// ============================================================================
//
// --- STEP 1: GLOBAL SETUP (do this once in GamePage.cpp) ---
//
//   Game_Load()   -> Meshes::CreateCircleMesh();   // already called - no extra step needed
//   Game_Unload() -> Meshes::FreeMeshes();          // already called - no extra step needed
//
// --- STEP 2: DECLARE A PARTICLE SYSTEM (in your class .hpp) ---
//
//   ParticleSystem myPS;
//
// --- STEP 3: USE A PRESET OR MAKE YOUR OWN ---
//
//   // Option A: Use a preset (easiest)
//   myPS = ParticleSystem::MakeSmoke();
//   myPS = ParticleSystem::MakeExplosion();
//
//   // Option B: Custom effect via Init()
//   myPS.Init(
//       30,               // maxParticles  : max alive at once
//      -100.0f, 100.0f,  // velX range    : horizontal spread (neg=left, pos=right)
//      -100.0f, 100.0f,  // velY range    : vertical spread   (neg=down, pos=up)
//       0.5f,   1.0f,    // lifetime      : seconds alive (randomised in this range)
//       10.0f,  0.0f,    // size          : radius at birth -> radius at death
//       1.0f, 0.5f, 0.0f,// startRGB      : colour at birth  (r, g, b)  0.0-1.0
//       0.8f, 0.0f, 0.0f,// endRGB        : colour at death  (r, g, b)  0.0-1.0
//       80.0f,           // gravity       : 0=float, 80=slight fall, 200=heavy fall
//       2.0f             // drag          : 0=no slowdown, 1.5=medium, 3=stops fast
//   );
//
// --- STEP 4: SPAWN PARTICLES ---
//
//   myPS.Emit(position);             // spawn 1 particle (good for streams/trails)
//   myPS.EmitBurst(position, 20);    // spawn 20 at once (good for explosions)
//   // NOTE: burst count cannot exceed maxParticles - extra are silently dropped
//
// --- STEP 5: TICK AND DRAW EVERY FRAME ---
//
//   myPS.Update(deltaTime);   // in your Update()
//   myPS.Render();            // in your Draw() / Render()
//
// --- STEP 6: ADDING A NEW PRESET ---
//
//   In Particles.hpp, add inside the class:
//       static ParticleSystem MakeMyEffect();
//
//   In Particles.cpp, copy MakeSmoke() or MakeExplosion() and tune the values.
//
// ============================================================================
// PARAMETER CHEAT SHEET
// ============================================================================
//
//   maxParticles : how many can be alive at the same time
//                  more = bigger bursts possible, more memory used
//
//   velX range   : [-300, 300] = wide horizontal spray
//                  [-10,   10] = barely any horizontal movement
//
//   velY range   : [30,  80]   = floats upward  (smoke)
//                  [-300, 300] = sprays in all directions (explosion)
//                  [-200,  0]  = falls downward  (rain/debris)
//
//   lifetime     : [0.1, 0.3]  = very short flash
//                  [0.4, 0.8]  = quick puff
//                  [1.0, 2.0]  = long lasting trail
//
//   size         : [20, 0]     = starts big, shrinks to nothing (explosion)
//                  [5,  5]     = stays same size throughout
//                  [2, 10]     = grows over lifetime (shockwave)
//
//   startRGB     : (1, 0.5, 0) = orange    (1, 1, 1) = white
//   endRGB       : (0.8, 0, 0) = dark red  (0, 0, 0) = black (fades to dark)
//
//   gravity      :  0   = weightless (smoke, magic)
//                   80  = slight arc (explosions)
//                   200 = heavy fall (debris, blood)
//
//   drag         :  0   = keeps moving forever
//                   1.5 = slows to a stop quickly   (smoke)
//                   3.0 = almost instant stop       (sparks)
//
// ============================================================================
// HOW THIS SYSTEM WORKS (big picture):
//
//   Each ParticleSystem owns a fixed-size pool of Particle structs allocated
//   once in Init(). Two index lists track which slots are in use:
//
//     freeParticles   - indices of slots available to spawn into
//     activeParticles - indices of currently alive particles
//
//   On Emit():  grab an index from freeParticles, initialise that slot,
//               push the index onto activeParticles.
//
//   On Update(): iterate activeParticles backwards (so swap-and-pop removal
//               is safe). When a particle dies, swap its index to the back
//               of activeParticles, pop it off, and return the index to
//               freeParticles. This keeps removal O(1).
//
//   On Render(): iterate activeParticles and draw each live particle using
//               Meshes::pCircleMesh (shared with the rest of the game).
//               Colour and alpha are applied via AEGfxSetColorToAdd so no
//               texture is needed.
//
// SHARED MESH:
//   Particles reuse Meshes::pCircleMesh from Util.hpp - no separate mesh needed.
//   Just make sure Meshes::CreateCircleMesh() is called in Game_Load() as usual.
// ============================================================================

#include "pch.hpp"
#include "Particles.hpp"
#include "Util.hpp"
#include <cstdlib>
#include <cmath>
#include <random>


// ============================================================================
// RandRange
// ============================================================================
// Returns a uniformly distributed random float between lo and hi (inclusive).
// Used to randomise per-particle velocity and lifetime on spawn.
// ============================================================================
f32 ParticleSystem::RandRange(f32 lo, f32 hi) const
{
    if (lo >= hi) return lo;
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<f32> dist(lo, hi);
    return dist(rng);
}
// ============================================================================
// Init
// ============================================================================
// Configures this system and pre-allocates all particle slots.
// Call before any Emit(). Calling Init() again fully resets the system.
//
// Parameters:
//   maxParticles      - hard cap on simultaneously alive particles
//   minVelX/maxVelX   - random horizontal velocity range at spawn
//   minVelY/maxVelY   - random vertical velocity range at spawn
//   minLifetime/max   - random lifespan in seconds
//   startSize/endSize - particle radius at birth -> death (lerped over lifetime)
//   startRGB/endRGB   - colour at birth -> death (lerped over lifetime)
//   gravity           - downward acceleration (units/sec^2). 0 = weightless
//   drag              - velocity damping per second. 0 = no drag, 2 = heavy drag
// ============================================================================
void ParticleSystem::Init(int  _maxParticles,
    f32  _minVelX, f32 _maxVelX,
    f32  _minVelY, f32 _maxVelY,
    f32  _minLifetime, f32 _maxLifetime,
    f32  _startSize, f32 _endSize,
    f32  _startR, f32 _startG, f32 _startB,
    f32  _endR, f32 _endG, f32 _endB,
    f32  _gravity,
    f32  _drag)
{
    Free(); // Wipe any existing particles before reinitialising

    // Store spawn parameters - used every time Emit() is called
    minVelX = _minVelX;   maxVelX = _maxVelX;
    minVelY = _minVelY;   maxVelY = _maxVelY;
    minLifetime = _minLifetime; maxLifetime = _maxLifetime;
    startSize = _startSize; endSize = _endSize;
    startR = _startR; startG = _startG; startB = _startB;
    endR = _endR;   endG = _endG;   endB = _endB;
    gravity = _gravity;
    drag = _drag;

    // Pre-allocate all particle slots upfront - no heap allocation during gameplay
    particles.resize(_maxParticles);

    // Reserve so activeParticles never triggers a realloc mid-frame
    activeParticles.reserve(_maxParticles);

    // Fill freeParticles with all indices [0, maxParticles-1].
    // Emit() pops from the back; dead particles push their index back here.
    freeParticles.resize(_maxParticles);
    for (int i = 0; i < _maxParticles; ++i)
        freeParticles[i] = i;
}

// ============================================================================
// Emit
// ============================================================================
// Spawns one particle at the given world position.
// If the pool is full (all slots active), silently drops the request.
// ============================================================================
void ParticleSystem::Emit(const AEVec2& position)
{
    if (freeParticles.empty()) return; // Pool exhausted - silently skip

    // Claim a free slot from the back of the free list (O(1))
    const int idx = freeParticles.back();
    freeParticles.pop_back();

    Particle& p = particles[idx];

    // Velocity and lifetime are randomised within the ranges set in Init()
    p.position = position;
    p.velocity = { RandRange(minVelX, maxVelX), RandRange(minVelY, maxVelY) };
    p.maxLifetime = RandRange(minLifetime, maxLifetime); // Total lifespan
    p.lifetime = p.maxLifetime;                          // Countdown timer starts full
    p.startSize = startSize;
    p.endSize = endSize;
    p.size = startSize;  // Visual size starts at birth value
    p.startR = startR; p.startG = startG; p.startB = startB;
    p.endR = endR;   p.endG = endG;   p.endB = endB;
    p.alpha = 1.0f;    // Fully opaque at birth
    p.isActive = true;

    // Register this index as active so Update() and Render() process it
    activeParticles.push_back(idx);
}

// ============================================================================
// EmitBurst
// ============================================================================
// Convenience wrapper: spawns 'count' particles in one call.
// Used for one-shot effects like enemy death explosions.
// Each particle still gets its own randomised velocity and lifetime.
// ============================================================================
void ParticleSystem::EmitBurst(const AEVec2& position, int count)
{
    for (int i = 0; i < count; ++i)
        Emit(position);
}

// ============================================================================
// Update
// ============================================================================
// Steps all active particles forward by dt seconds.
// Iterates BACKWARDS so swap-and-pop removal never skips an element.
// ============================================================================
void ParticleSystem::Update(f32 dt)
{
    // Backwards: when we remove element [i] via swap-and-pop, the new [i]
    // is the old last element - already processed since we go high->low.
    for (int i = static_cast<s8>(activeParticles.size()) - 1; i >= 0; --i)
    {
        const int idx = activeParticles[i];
        Particle& p = particles[idx];

        // --- Drag ---
        // Multiplies velocity by (1 - drag*dt) each frame -> exponential slowdown.
        // drag=0: no effect. drag=1.5: heavy braking (smoke). drag=2: very fast stop.
        if (drag > 0.0f)
        {
            p.velocity.x *= (1.0f - drag * dt);
            p.velocity.y *= (1.0f - drag * dt);
        }

        // --- Gravity ---
        // Subtracts from Y velocity each frame, pulling particles downward.
        // gravity=0: weightless float (smoke). gravity=80: slight fall (explosions).
        p.velocity.y -= gravity * dt;

        // --- Position integration (Euler method) ---
        p.position.x += p.velocity.x * dt;
        p.position.y += p.velocity.y * dt;

        // --- Age the particle ---
        p.lifetime -= dt;

        // ratio: 0.0 at the moment of birth, 1.0 at the moment of death.
        // All lerps below use this single value.
        const f32 ratio = 1.0f - (p.lifetime / p.maxLifetime);

        // --- Size lerp: grows or shrinks over lifetime ---
        p.size = p.startSize + (p.endSize - p.startSize) * ratio;

        // --- Alpha fade-out: fully opaque at birth, invisible at death ---
        p.alpha = 1.0f - ratio;

        // --- Death check ---
        if (p.lifetime <= 0.0f)
        {
            p.isActive = false;

            // Swap-and-pop: replace this entry with the last one, shrink the list.
            // O(1) removal without shifting the rest of the array.
            activeParticles[i] = activeParticles.back();
            activeParticles.pop_back();

            // Return the slot so it can be reused by a future Emit()
            freeParticles.push_back(idx);
        }
    }
}

// ============================================================================
// Render
// ============================================================================
// Draws every active particle as a colour-tinted circle.
// Colour and alpha are driven by per-particle lerp values via ColorToAdd.
// No texture is needed - the mesh vertex colour is zeroed out by ColorToMultiply.
// Reuses Meshes::pCircleMesh to avoid allocating a separate mesh.
// ============================================================================
void ParticleSystem::Render()
{
    // Early out: skip if the circle mesh isn't ready or there's nothing alive
    if (!Meshes::pCircleMesh || activeParticles.empty()) return;

    // COLOR mode: vertex colours are irrelevant; we set colour manually each draw
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);

    // BLEND mode: enables alpha transparency so particles fade smoothly
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);

    // Global transparency at 1.0 - per-particle alpha is handled by ColorToAdd below
    AEGfxSetTransparency(1.0f);

    for (int idx : activeParticles)
    {
        const Particle& p = particles[idx];
        if (!p.isActive) continue; // Defensive - should always be active in this list

        // How far through its life: 0=just born, 1=about to die
        const f32 ratio = 1.0f - (p.lifetime / p.maxLifetime);

        // Interpolate colour from birth colour toward death colour
        const f32 r = p.startR + (p.endR - p.startR) * ratio;
        const f32 g = p.startG + (p.endG - p.startG) * ratio;
        const f32 b = p.startB + (p.endB - p.startB) * ratio;

        // Multiply=all zeros: zeroes out the mesh's own vertex colour
        // Add=our colour:     adds our lerped RGB+alpha on top -> final colour is exactly ours
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(r, g, b, p.alpha);

        // Build the world transform: scale to particle size, then move to world position.
        // No rotation needed - circles look the same from every angle.
        AEMtx33 sc, tr, transform;
        AEMtx33Scale(&sc, p.size, p.size);              // Scale matrix (uniform - circle stays round)
        AEMtx33Trans(&tr, p.position.x, p.position.y);  // Translation matrix
        AEMtx33Concat(&transform, &tr, &sc);             // T * S: scale first, then translate
        AEGfxSetTransform(transform.m);

        AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
    }
}

// ============================================================================
// Free
// ============================================================================
// Clears all particle data. Safe to call multiple times.
// Does NOT touch Meshes::pCircleMesh - that is owned by Util/Meshes.
// ============================================================================
void ParticleSystem::Free()
{
    particles.clear();
    activeParticles.clear();
    freeParticles.clear();
}

// ============================================================================
// MakeSmoke - preset for the player's gun smoke puff
// ============================================================================
// Behaviour : gentle upward drift, decelerates quickly, shrinks and fades out
// Colours   : ash green -> dark ash green
// Lifetime  : 0.8 - 1.2 seconds  |  Max 30 simultaneous particles
//
// Tuning tips:
//   More spread    : widen velX range (e.g. -50 to 50)
//   Rises faster   : increase velY min/max (e.g. 60-120)
//   Lasts longer   : increase lifetime range and reduce drag (e.g. 0.8)
// ============================================================================
ParticleSystem ParticleSystem::MakeSmoke()
{
    ParticleSystem ps;
    ps.Init(
        30,               // maxParticles  - cap on simultaneous smoke puffs
        -25.0f, 25.0f,   // velX          - slight horizontal spread
        30.0f, 80.0f,   // velY          - drifts upward
        0.8f, 1.2f,    // lifetime      - short-lived puffs
        24.0f, 4.0f,    // size          - visibly shrinks as it fades
        0.1f, 0.6f, 0.6f, // start colour: mid ash green
        0.1f, 0.2f, 0.2f, // end colour:   dark ash green
        0.0f,   // gravity=0: smoke floats upward, not pulled down
        1.5f    // drag=1.5:  velocity halves fast, puffs hang rather than fly
    );
    return ps;
}

// ============================================================================
// MakeExplosion - preset for enemy death burst
// ============================================================================
// Behaviour : fast wide outward spray, falls slightly, shrinks to nothing
// Colours   : bright orange -> dark red
// Lifetime  : 0.3 - 0.7 seconds  |  Max 40 particles (all fired via EmitBurst)
//
// Tuning tips:
//   Bigger bang    : widen velX/velY range (e.g. +-500) and increase startSize (e.g. 30)
//   Longer burn    : increase lifetime range (e.g. 0.5-1.2)
//   More floaty    : reduce gravity (e.g. 20) and drag (e.g. 0.5)
//   Different NPC  : call EmitBurst(pos, 20) for normal enemies, EmitBurst(pos, 40) for boss
// ============================================================================
ParticleSystem ParticleSystem::MakeExplosion()
{
    ParticleSystem ps;
    ps.Init(
        40,               // maxParticles  - 40 slots; boss uses all 40, others use 20
        -300.0f, 300.0f,  // velX          - wide horizontal spray
        -300.0f, 300.0f,  // velY          - equal vertical spray (omnidirectional burst)
        0.3f, 0.7f,    // lifetime      - fast burst, gone quickly
        20.0f, 0.0f,    // size          - shrinks completely to nothing at death
        1.0f, 0.5f, 0.0f, // start colour: bright orange
        0.8f, 0.0f, 0.0f, // end colour:   dark red
        80.0f,  // gravity=80: particles arc downward slightly (adds physical weight)
        2.0f    // drag=2:     velocity drops fast so particles don't travel too far
    );
    return ps;
}