// ============================================================================
// SplashPage.cpp - DigiPen Logo Splash Screen
// ============================================================================
// Displays the DigiPen Singapore logo for up to 6 seconds.
// The logo smoothly grows from 1000x300 to 1525x445 using linear interpolation.
// Press SPACE to skip.
//
// USAGE:
// ----------------------------------------------------------------------------
//   Entered automatically as STATE_SPLASH (the first state in StateManagerInit).
//   Transitions to STATE_MENU after timer expires or SPACE is pressed.
// ============================================================================

#include "pch.hpp"
#include "SplashPage.hpp"
#include "AEEngine.h"
#include "Util.hpp"
#include "StarBackground.hpp"
#include "AudioManager.hpp"

// ============================================================================
// File-scope state
// ============================================================================
static AEGfxTexture* ss_DigiPen_Logo = nullptr;
static f32           timer = 0.0f;

// Linear interpolation helper
namespace { f32 Lerp(f32 a, f32 b, f32 t) { return a + t * (b - a); } }

// ============================================================================
// SplashPage_Load
// ============================================================================
void SplashPage_Load()
{
    const char* path = "Assets/DigiPen_Singapore_WEB_RED.png";
    ss_DigiPen_Logo = AEGfxTextureLoad(path);
    printf("[Splash] Loaded texture '%s' -> %p\n", path, (void*)ss_DigiPen_Logo);

    Meshes::CreateCircleMesh(); // Required by StarBackground
    StarBackground::Init();

    Meshes::CreateSquareCenterOriginMesh();
}

// ============================================================================
// SplashPage_Init
// ============================================================================
void SplashPage_Init()
{
    //AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
    timer = 0.0f;
//    AudioManager::PlaySFX("MenuMusic");
}

// ============================================================================
// SplashPage_Update
// ============================================================================
// Advances the timer. Transitions to menu after 6 seconds or on SPACE.
// ============================================================================
void SplashPage_Update()
{
    StarBackground::Update((f32)AEFrameRateControllerGetFrameTime());
    StarBackground::DrawBackground();
    StarBackground::Draw();

    timer += (f32)AEFrameRateControllerGetFrameTime();

    if (timer > 6.0f || AEInputCheckTriggered(AEVK_SPACE))
        StateManagerChangeState(STATE_MENU);
}

// ============================================================================
// SplashPage_Draw
// ============================================================================
// Draws the DigiPen logo with a grow animation driven by the timer.
// ============================================================================
void SplashPage_Draw()
{
    if (!ss_DigiPen_Logo) return;

    AEGfxSetCamPosition(0.0f, 0.0f);

    // Compute current logo dimensions via lerp (grows from small to large over 6s)
    const f32 t = timer / 6.0f;
    const f32 w = Lerp(1000.0f, 1525.0f, t);
    const f32 h = Lerp(300.0f, 445.0f, t);

    AEMtx33 transform, scale, translate;
    AEMtx33Scale(&scale, w, h);
    AEMtx33Trans(&translate, 0.0f, 0.0f);
    AEMtx33Concat(&transform, &translate, &scale);

    // Texture mode for the logo
    AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
    AEGfxTextureSet(ss_DigiPen_Logo, 0, 0);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// SplashPage_Free
// ============================================================================
void SplashPage_Free()
{
    Meshes::FreeMeshes();
}

// ============================================================================
// SplashPage_Unload
// ============================================================================
void SplashPage_Unload()
{
    if (ss_DigiPen_Logo)
    {
        AEGfxTextureUnload(ss_DigiPen_Logo);
        ss_DigiPen_Logo = nullptr;
    }
}