#pragma once
#include "GameObject.hpp"
#include "Particles.hpp"
#include "Abilities.hpp"

// Forward declaration
class PowerUpSystem;

class Player : public GameObject
{
public:
    void Start();
    void Update(f32 deltaTime);
    void Shoot(AEVec2 dir);
    
    f32 health = 100.0f;    // starting health
    f32 maxHealth = 100.0f;
    
    // NEW: Power-up system reference
    PowerUpSystem* powerUpSystem{nullptr};
    
    // NEW: Get current stats for damage calculations
    f32 GetBulletDamage() const;
    f32 GetAoeDamage() const;
    f32 GetAoeRadius() const;
    f32 GetSpeed() const;

    ParticleSystem smokePS;

    InvulnerabilityAbility invulnAbility;

private:
    f32 shootCooldown = 0.0f;
	f32 rotationSpeed = 3.0f; // radians per second
 
};
