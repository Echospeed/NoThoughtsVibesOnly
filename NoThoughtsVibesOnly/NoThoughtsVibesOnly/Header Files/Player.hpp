#pragma once
#include "GameObject.hpp"
#include "Particles.hpp"
#include "Abilities.hpp"

#include "SpriteAnimator.hpp"

// Forward declaration
class PowerUpSystem;

class Player : public GameObject
{
public:
    void Start();
    void Update(f32 deltaTime);
    void Shoot(AEVec2 dir);
    void Free();

    f32 health = 100.0f;    // starting health
    f32 maxHealth = 100.0f;

    // Power-up system reference
    PowerUpSystem* powerUpSystem{ nullptr };

    // Get current stats for damage calculations
    f32 GetBulletDamage() const;
    f32 GetAoeDamage() const;
    f32 GetAoeRadius() const;
    f32 GetSpeed() const;

    bool IsReloading()       const { return isReloading; }
    f32  GetReloadProgress() const { return isReloading ? (1.0f - reloadTimer / reloadDuration) : 1.0f; }
    int  GetAmmoInMagazine() const { return ammoInMagazine; }

    // Manually starts a reload. No-ops if already reloading or magazine is full.
    void TriggerReload();

    bool suppressShootOneFrame{ false }; // Set to true after picking a power-up to eat the click

    ParticleSystem smokePS;

    InvulnerabilityAbility invulnAbility;

    AEGfxTexture* playerSpritesheet{ nullptr };

    Animation idleAnim;
    Animation runAnim;

    Animator playerAnimator;

private:
    f32 shootCooldown = 0.0f;
    f32 rotationSpeed = 3.0f; // radians per second

    // --- Reload system ---
    bool isReloading = false;
    f32  reloadTimer = 0.0f;
    f32  reloadDuration = 2.0f; // seconds to reload
    int ammoInMagazine = 10; // starts with 10 bullets available

};