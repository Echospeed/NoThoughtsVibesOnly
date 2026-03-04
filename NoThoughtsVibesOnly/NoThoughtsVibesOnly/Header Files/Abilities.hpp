#pragma once
#include "pch.hpp"

// ============================================================================
// Base Ability Class
// ============================================================================
class Ability
{
public:
    Ability(f32 duration, f32 cooldown)
        : maxDuration(duration), maxCooldown(cooldown), currentDuration(0.0f), currentCooldown(0.0f)
    {
    }

    virtual ~Ability() = default;

    virtual void Update(f32 dt)
    {
        if (currentDuration > 0.0f)
        {
            currentDuration -= dt;
            if (currentDuration <= 0.0f)
            {
                currentDuration = 0.0f;
                currentCooldown = maxCooldown;
            }
        }
        else if (currentCooldown > 0.0f)
        {
            currentCooldown -= dt;
            if (currentCooldown <= 0.0f)
            {
                currentCooldown = 0.0f;
            }
        }
    }

    virtual bool TryActivate()
    {
        if (!IsActive() && !IsOnCooldown())
        {
            currentDuration = maxDuration;
            return true;
        }
        return false;
    }

    bool IsActive() const { return currentDuration > 0.0f; }
    bool IsOnCooldown() const { return currentCooldown > 0.0f; }
    f32  GetCooldownRemaining() const { return currentCooldown; }

protected:
    f32 maxDuration;
    f32 maxCooldown;
    f32 currentDuration;
    f32 currentCooldown;
};

// ============================================================================
// Specific Invulnerability Ability
// ============================================================================
class InvulnerabilityAbility : public Ability
{
public:
    InvulnerabilityAbility() : Ability(3.0f, 30.0f)
    {
    }
};