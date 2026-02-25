// ============================================================================
// GameObject.cpp - Base Entity Implementation
// ============================================================================
// All game entities (Player, NPC, Bullet, Button) inherit from GameObject.
// On construction, the object registers itself into the appropriate object
// list (gamePageObj for gameplay objects, mainPageObj for menu objects).
//
// LIFECYCLE:
// ----------------------------------------------------------------------------
//   new GameObject()   -> ctor registers in list
//   obj->Start()       -> override to set initial state (called manually)
//   obj->Update(dt)    -> override for per-frame logic; base calls Draw()
//   obj->Draw()        -> calls DrawSpriteRenderer with current transform
//   delete obj         -> dtor calls Free() (unloads texture if any)
// ============================================================================

#include "pch.hpp"
#include "GameObject.hpp"
#include "GamePage.hpp"

// ============================================================================
// Constructor
// ============================================================================
// Registers this object in the correct global list based on GameState.
// STATE_PLAYING -> gamePageObj  (game entities)
// STATE_MENU    -> mainPageObj  (menu entities)
// ============================================================================
GameObject::GameObject(AEVec2 pos, AEVec2 scale, f32 rot,
    AEGfxTexture* texture, f32 width, f32 height,
    GameState state)
    : transform({ pos, scale, rot })
    , spriteRenderer({ texture, width, height })
    , ObjectType(ObjectType::NONE)
{
    switch (state)
    {
    case STATE_MENU:    mainPageObj.push_back(this);  break;
    case STATE_PLAYING: gamePageObj.push_back(this);  break;
    default:                                           break;
    }
}

// ============================================================================
// Destructor
// ============================================================================
GameObject::~GameObject()
{
    Free();
}

// ============================================================================
// Init
// ============================================================================
// Manual re-initialisation of transform and sprite (not called by default).
// ============================================================================
void GameObject::Init(AEVec2 pos, AEVec2 scale, f32 rot,
    AEGfxTexture* texture, f32 width, f32 height)
{
    transform.position = pos;
    transform.scale = scale;
    transform.rotation = rot;

    spriteRenderer.texture = texture;
    spriteRenderer.width = width;
    spriteRenderer.height = height;
}

// ============================================================================
// Start - Override in derived classes to set initial state.
// ============================================================================
void GameObject::Start() {}

// ============================================================================
// Update - Base implementation just calls Draw().
// Override in derived classes for custom per-frame logic.
// ============================================================================
void GameObject::Update(f32 deltaTime)
{
    static_cast<void>(deltaTime); // Suppress C4100 unused parameter warning
    Draw();
}

// ============================================================================
// Draw - Renders the sprite using the current transform.
// ============================================================================
void GameObject::Draw()
{
    DrawSpriteRenderer(spriteRenderer, transform);
}

// ============================================================================
// Free (private) - Unloads the sprite renderer's texture.
// ============================================================================
void GameObject::Free()
{
    FreeSpriteRenderer(spriteRenderer);
}