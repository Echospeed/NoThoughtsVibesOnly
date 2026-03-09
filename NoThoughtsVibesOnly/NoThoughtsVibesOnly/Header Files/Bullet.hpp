#pragma once
#include "GameObject.hpp"
#include "NPC.hpp"
#include "GameObjectType.hpp"

// ============================================================================
// class Bullet : public GameObject
// ============================================================================
// Represents a single projectile in the bullet pool.
// Each bullet tracks its owner, direction, speed, and remaining lifetime.
// ============================================================================
class Bullet : public GameObject
{
public:
    // ------------------------------------------------------------------------
    // Start - Called once after construction (via obj->Start() in Game_Init).
    // Sets initial transform, mesh type, and hides the bullet until fired.
    // ------------------------------------------------------------------------
    void Start();

    // ------------------------------------------------------------------------
    // Update - Called every frame (only executes logic if isActive == true).
    // Moves the bullet, counts down lifetime, checks world bounds,
    // and handles collision with NPCs (player bullets) or Player (enemy bullets).
    // ------------------------------------------------------------------------
    void Update(f32 deltaTime);

    // ------------------------------------------------------------------------
    // Activate - Convenience method to fire a bullet from a shooter.
    // Sets position, direction, owner, resets lifetime, and makes it visible.
    //
    // shooter   : The GameObject that fired this bullet (player or NPC)
    // direction : Normalised AEVec2 pointing toward the target
    // newOwner  : BulletOwner::PLAYER or BulletOwner::ENEMY
    // ------------------------------------------------------------------------
    void Activate(GameObject* shooter, AEVec2 direction, BulletOwner newOwner);

    // -----------------------------------------------------------------------
    // Public Data
    // -----------------------------------------------------------------------
    GameObject* startPos{ nullptr };       // The GameObject that owns/fired this bullet
    AEVec2      dir{ 0.0f, 0.0f };        // Normalised movement direction
    f32         speed{ 1500.0f };          // Units per second
    f32         lifeTime{ 0.0f };          // Remaining time before auto-despawn
    f32         maxLifeTime{ 4.0f };       // Maximum travel time in seconds
    BulletOwner owner{ BulletOwner::PLAYER }; // Determines who this bullet can damage
    bool        spent{ false };            // True once fired; cleared only on reload

private:
    // ------------------------------------------------------------------------
    // HideBullet - Deactivates the bullet and moves it offscreen.
    // Called on: lifetime expiry, out-of-bounds, and successful hit.
    // ------------------------------------------------------------------------
    void HideBullet();
};

// ============================================================================
// Bullet.hpp - Projectile Object (Player & Enemy)
// ============================================================================
// Bullets are pooled GameObjects. Instead of creating/destroying them at
// runtime, a fixed pool is allocated in Game_Init() and bullets are
// reactivated/hidden as needed via isActive and HideBullet().
//
// POOL SETUP (in Game_Init):
// ----------------------------------------------------------------------------
//   // Player bullet pool
//   for (int i = 0; i < 500; ++i)
//   {
//       Bullet* b    = new Bullet();
//       b->startPos  = pPlayer;
//       b->owner     = BulletOwner::PLAYER;
//       b->isActive  = false;
//       b->spriteRenderer.colour = { 1.0f, 1.0f, 0.0f, 0.0f };
//   }
//
//   // Enemy bullet pool (startPos assigned per-NPC in WaveSystem)
//   for (int i = 0; i < 100; ++i)
//   {
//       Bullet* b   = new Bullet();
//       b->owner    = BulletOwner::ENEMY;
//       b->isActive = false;
//       b->spriteRenderer.colour = { 1.0f, 0.0f, 0.0f, 0.0f };
//   }
//
// FIRING (Player - in Player::Shoot):
// ----------------------------------------------------------------------------
//   // Find an inactive player bullet and activate it
//   for (GameObject* obj : gamePageObj)
//   {
//       if (obj->ObjectType == SHOT && !obj->isActive)
//       {
//           Bullet* b = dynamic_cast<Bullet*>(obj);
//           if (b && b->owner == BulletOwner::PLAYER)
//           {
//               b->Activate(this, direction, BulletOwner::PLAYER);
//               break;
//           }
//       }
//   }
//
// FIRING (Enemy NPC - in NPC::RangerNPCs):
// ----------------------------------------------------------------------------
//   // Find an inactive enemy bullet assigned to this NPC
//   for (auto& obj : gamePageObj)
//   {
//       if (obj->ObjectType == SHOT)
//       {
//           Bullet* b = dynamic_cast<Bullet*>(obj);
//           if (b && b->owner == BulletOwner::ENEMY
//               && !b->isActive && b->startPos == this)
//           {
//               b->Activate(this, direction, BulletOwner::ENEMY);
//               break;
//           }
//       }
//   }
// ============================================================================

//#include "GameObject.hpp"
//#include "NPC.hpp"
//#include "GameObjectType.hpp"
//
//// ============================================================================
//// class Bullet : public GameObject
//// ============================================================================
//// Represents a single projectile in the bullet pool.
//// Each bullet tracks its owner, direction, speed, and remaining lifetime.
//// ============================================================================
//class Bullet : public GameObject
//{
//public:
//    // ------------------------------------------------------------------------
//    // Start - Called once after construction (via obj->Start() in Game_Init).
//    // Sets initial transform, mesh type, and hides the bullet until fired.
//    // ------------------------------------------------------------------------
//    void Start();
//
//    // ------------------------------------------------------------------------
//    // Update - Called every frame (only executes logic if isActive == true).
//    // Moves the bullet, counts down lifetime, checks world bounds,
//    // and handles collision with NPCs (player bullets) or Player (enemy bullets).
//    // ------------------------------------------------------------------------
//    void Update(f32 deltaTime);
//
//    // ------------------------------------------------------------------------
//    // Activate - Convenience method to fire a bullet from a shooter.
//    // Sets position, direction, owner, resets lifetime, and makes it visible.
//    //
//    // shooter   : The GameObject that fired this bullet (player or NPC)
//    // direction : Normalised AEVec2 pointing toward the target
//    // newOwner  : BulletOwner::PLAYER or BulletOwner::ENEMY
//    // ------------------------------------------------------------------------
//    void Activate(GameObject* shooter, AEVec2 direction, BulletOwner newOwner);
//
//    // -----------------------------------------------------------------------
//    // Public Data
//    // -----------------------------------------------------------------------
//    GameObject* startPos{ nullptr };       // The GameObject that owns/fired this bullet
//    AEVec2      dir{ 0.0f, 0.0f };        // Normalised movement direction
//    f32         speed{ 1500.0f };          // Units per second
//    f32         lifeTime{ 0.0f };          // Remaining time before auto-despawn
//    f32         maxLifeTime{ 4.0f };       // Maximum travel time in seconds
//    BulletOwner owner{ BulletOwner::PLAYER }; // Determines who this bullet can damage
//
//private:
//    // ------------------------------------------------------------------------
//    // HideBullet - Deactivates the bullet and moves it offscreen.
//    // Called on: lifetime expiry, out-of-bounds, and successful hit.
//    // ------------------------------------------------------------------------
//    void HideBullet();
//};