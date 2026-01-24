#include "AEEngine.h"
#include "Util.h"
#include "SpriteRenderer.h"

void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height)
{
    spriteRenderer.texture = (texturePath != NULL) ? AEGfxTextureLoad(texturePath) : nullptr;
    spriteRenderer.width = width;
    spriteRenderer.height = height;
}

void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform, f32 r = 0, f32 g = 0, f32 b = 0)
{
    if (!spriteRenderer.texture)
    {
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    }
    else
    {
        
        AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
        AEGfxTextureSet(spriteRenderer.texture, 0, 0);
    }
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 1.0f);
    AEGfxSetColorToAdd(r, g, b, 1.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
	TransformMovement(transform);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
}

void FreeSpriteRenderer(SpriteRenderer& spriteRenderer)
{
    if(spriteRenderer.texture)
    {
        // Assuming there's a function to free textures
        AEGfxTextureUnload(spriteRenderer.texture);
        spriteRenderer.texture = nullptr;
	}
}