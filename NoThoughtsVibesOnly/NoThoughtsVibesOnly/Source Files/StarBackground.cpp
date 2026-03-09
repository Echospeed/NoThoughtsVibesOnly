// ============================================================================
// StarBackground.cpp - Animated star field for menu/UI screens
// ============================================================================
// WHY THE BACKGROUND COVERED BUTTONS:
// ----------------------------------------------------------------------------
// The 3000x3000 dark background quad was drawn inside Draw(), which is called
// AFTER Update(). Buttons self-register to mainPageObj and draw themselves
// during Update(), so the background quad was overwriting them every frame.
//
// FIX: Split into two functions:
//   DrawBackground() - just the dark quad. Call FIRST in _Draw() or even at
//                      the END of the previous frame (before anything draws).
//   Draw()           - just the stars, no background quad. Call after
//                      DrawBackground() but still before buttons.
//
// CORRECT CALL ORDER each frame:
//   _Update(): StarBackground::Update(dt);
//   _Draw():
//     StarBackground::DrawBackground();  // 1. dark blue fill
//     StarBackground::Draw();            // 2. stars on top
//     myText.Draw();                     // 3. text on top of stars
//     // buttons already drew in Update, but that's fine - they drew AFTER
//     // the background was cleared, so they appear on top naturally
// ============================================================================

#include "pch.hpp"
#include "StarBackground.hpp"

namespace StarBackground
{
    struct Star
    {
        float x, y;
        float size;
        float brightness;
        float twinklePhase;
    };

    static const int STAR_COUNT = 600;
    static Star      s_Stars[STAR_COUNT];
    static float     s_Time = 0.0f;

    void Init()
    {
        s_Time = 0.0f;
        srand(42);

        const float fieldW = 2400.0f;
        const float fieldH = 1350.0f;

        for (int i = 0; i < STAR_COUNT; ++i)
        {
            s_Stars[i].x = ((float)rand() / RAND_MAX) * fieldW - fieldW / 2.0f;
            s_Stars[i].y = ((float)rand() / RAND_MAX) * fieldH - fieldH / 2.0f;
            s_Stars[i].brightness = 0.3f + ((float)rand() / RAND_MAX) * 0.7f;
            s_Stars[i].twinklePhase = ((float)rand() / RAND_MAX) * 6.28f;

            if (i < 400)
                s_Stars[i].size = 0.8f + ((float)rand() / RAND_MAX) * 1.2f;
            else
                s_Stars[i].size = 1.5f + ((float)rand() / RAND_MAX) * 2.0f;
        }
    }

    void Update(float dt)
    {
        s_Time += dt;
    }

    // ========================================================================
    // DrawBackground
    // ========================================================================
    // Draws ONLY the dark blue background quad. Call this FIRST in _Draw()
    // before anything else so it sits behind all other elements.
    // ========================================================================
    void DrawBackground()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetColorToAdd(0.03f, 0.03f, 0.07f, 1.0f); // Deep space blue

        Transform tf;
        tf.SetPosition(0.0f, 0.0f);
        tf.SetScale(3000.0f, 3000.0f);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    }

    // ========================================================================
    // Draw
    // ========================================================================
    // Draws ONLY the stars (no background quad). Call after DrawBackground().
    // ========================================================================
    void Draw()
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.0f);

        Transform tf;
        for (int i = 0; i < STAR_COUNT; ++i)
        {
            const Star& s = s_Stars[i];

            const float twinkle = 1.0f + 0.15f * sinf(s_Time * 1.5f + s.twinklePhase);
            const float bright = s.brightness * twinkle;
            const float warmth = (i % 3 == 0) ? 0.85f : 1.0f;

            AEGfxSetColorToAdd(bright * warmth, bright * warmth, bright, 1.0f);

            tf.SetPosition(s.x, s.y);
            tf.SetUniformScale(s.size);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);
        }

        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    }

} // namespace StarBackground