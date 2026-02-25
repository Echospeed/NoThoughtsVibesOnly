// ============================================================================
// TextRenderer.cpp - Text Rendering Helpers
// ============================================================================
// Wraps Alpha Engine's AEGfxPrint to provide convenient text layout.
//
// ALIGNMENT:
// ----------------------------------------------------------------------------
//   ALIGN_CENTER (default) : Text is centered horizontally and vertically
//                            around the given position.
//   ALIGN_RIGHT            : Text is right-aligned (partial implementation).
//
// USAGE:
// ----------------------------------------------------------------------------
//   // Load (in *_Load):
//   TextRenderer myText;
//   LoadTextRenderer(myText, fontHandle);
//
//   // Init (in *_Init):
//   InitTextRenderer(myText, "Hello World", {1.0f, 1.0f}, 1.0f, 1.0f, 1.0f);
//
//   // Draw (in *_Draw, camera at 0,0 for screen-space):
//   DrawTextRenderer(myText, {0.0f, 200.0f}, 1.5f);
//
//   // Free (in *_Free):
//   FreeTextRenderer(myText);
// ============================================================================

#include "pch.hpp"
#include "TextRenderer.hpp"
#include "StateManager.hpp"

// ============================================================================
// TextRenderer::Draw (member function)
// ============================================================================
// Draws the text at the stored position using the stored scale and colour.
// Resets render mode after printing to avoid affecting subsequent draw calls.
// ============================================================================
void TextRenderer::Draw()
{
    f32 width, height;
    AEGfxGetPrintSize(font, text.c_str(), 1.0f, &width, &height);

    // Convert world position to AEGfxPrint normalised coordinates [-1, 1]
    const f32 normX = position.x / (SCREEN_W / 2.0f);
    const f32 normY = position.y / (SCREEN_H / 2.0f);

    f32 drawX = 0.0f, drawY = 0.0f;

    if (alignment == ALIGN_CENTER)
    {
        drawX = normX - (width / 2.0f);
        drawY = normY - (height / 2.0f);
    }
    else if (alignment == ALIGN_RIGHT)
    {
        drawX = normX - (width / 2.0f);
        drawY = height / 2.0f;
    }

    AEGfxPrint(font, text.c_str(), drawX, drawY, 1.0f,
        colour.r, colour.g, colour.b, 1.0f);

    // Reset render state - AEGfxPrint leaves the GPU in an unknown state
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
}

// ============================================================================
// LoadTextRenderer
// ============================================================================
// Associates a font handle with the renderer. Must be called before Draw().
// ============================================================================
void LoadTextRenderer(TextRenderer& tr, s8 font)
{
    tr.font = font;
}

// ============================================================================
// InitTextRenderer
// ============================================================================
// Sets the text string and colour. Call in *_Init() or when text changes.
// ============================================================================
void InitTextRenderer(TextRenderer& tr, const char* text, AEVec2 scale,
    f32 r, f32 g, f32 b)
{
    tr.SetText(text);
    tr.colour = { r, g, b, 1.0f };
    tr.scale = scale;
}

// ============================================================================
// DrawTextRenderer
// ============================================================================
// Draws text at a given world/screen position with a scale multiplier.
// Uses ALIGN_CENTER by default (centered on the given position).
// ============================================================================
void DrawTextRenderer(const TextRenderer& tr, AEVec2 position, f32 scale)
{
    f32 width, height;
    AEGfxGetPrintSize(tr.font, tr.text.c_str(), scale, &width, &height);

    const f32 normX = position.x / (SCREEN_W / 2.0f);
    const f32 normY = position.y / (SCREEN_H / 2.0f);

    f32 drawX = 0.0f, drawY = 0.0f;

    if (tr.alignment == ALIGN_CENTER)
    {
        drawX = normX - (width / 2.0f);
        drawY = normY - (height / 2.0f);
    }
    else if (tr.alignment == ALIGN_RIGHT)
    {
        drawX = normX - (width / 2.0f);
        drawY = height / 2.0f;
    }

    AEGfxPrint(tr.font, tr.text.c_str(), drawX, drawY, scale,
        tr.colour.r, tr.colour.g, tr.colour.b, 1.0f);
}

// ============================================================================
// FreeTextRenderer
// ============================================================================
// Clears the stored text string. The font handle itself is destroyed separately
// via AEGfxDestroyFont() in the state's Unload() function.
// ============================================================================
void FreeTextRenderer(TextRenderer& tr)
{
    tr.text.clear();
}