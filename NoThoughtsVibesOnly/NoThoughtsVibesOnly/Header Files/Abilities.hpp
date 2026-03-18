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
                currentCooldown = 0.0f;
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

    bool IsActive()             const { return currentDuration > 0.0f; }
    bool IsOnCooldown()         const { return currentCooldown > 0.0f; }
    f32  GetCooldownRemaining() const { return currentCooldown; }

protected:
    f32 maxDuration;
    f32 maxCooldown;
    f32 currentDuration;
    f32 currentCooldown;
};

// ============================================================================
// InvulnerabilityAbility
// ============================================================================
// Duration and cooldown are read from gameplay_config.json
// ("invulnDuration" and "invulnCooldown") via GameConfig::Gameplay().
// Fallback defaults (3s / 30s) match the original hardcoded values and
// are used only if the JSON file is missing.
// ============================================================================
class InvulnerabilityAbility : public Ability
{
public:
    InvulnerabilityAbility()
        : Ability(
            GameConfig::Gameplay().invulnDuration,   // was hardcoded 3.0f
            GameConfig::Gameplay().invulnCooldown    // was hardcoded 30.0f
        )
    {
    }
};