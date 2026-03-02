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


TextRenderer::TextRenderer()
    : font(0)
    , scale({ 1.0f, 1.0f })
    , position({ 0.0f, 0.0f })
    , colour({ 1.0f, 1.0f, 1.0f, 1.0f })
    , text("")
    , alignment(ALIGN_CENTER)
{
    // Default constructor - no special initialization needed
}

TextRenderer::TextRenderer(u8 font, AEVec2 scale, AEVec2 position, Colour colour, std::string, TextAlignment alignment)
    : font(font)
    , scale(scale)
    , position(position)
    , colour(colour)
    , text(text)
	, alignment(alignment)
{
    // Constructor - no special initialization needed
}

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

TextRenderer::~TextRenderer()
{
    text.clear();
}

void TextRenderer::SetScale(AEVec2 newScale)
{
    scale = newScale;
}

// Implementation for the SetPosition declared in your header
AEVec2 TextRenderer::SetPosition(AEVec2 newPosition)
{
    position = newPosition;
    return position;
}

void TextRenderer::SetAlignment(TextAlignment newAlignment) 
{ 
    alignment = newAlignment; 
}

void TextRenderer::SetColour(Colour newColour) 
{ 
    colour = newColour; 
}