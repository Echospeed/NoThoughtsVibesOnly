#include "AEEngine.h"
#include "Util.h"
#include "SpriteRenderer.h"

AEGfxVertexList* list{ nullptr };

void InitSpriteRenderer(SpriteRenderer& spriteRenderer, const char* texturePath, f32 width, f32 height, MeshType mesh)
{
    spriteRenderer.texture = (texturePath != NULL) ? AEGfxTextureLoad(texturePath) : nullptr;
    spriteRenderer.width = width;
    spriteRenderer.height = height;
    spriteRenderer.meshType = mesh;
}

void DrawSpriteRenderer(const SpriteRenderer& spriteRenderer, Transform& transform)
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
    AEGfxSetColorToAdd(spriteRenderer.colour.r, spriteRenderer.colour.g, spriteRenderer.colour.b, 1.0f);
    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
    AEGfxSetTransparency(1.0f);
	TransformMovement(transform);
    if (spriteRenderer.meshType == MESH_SQUARE)
    {
        list = Meshes::pSquareCOriMesh;
    }
    else if (spriteRenderer.meshType == MESH_TRIANGLE)
    {
        list = Meshes::pTriangleMesh;
    }
    else if (spriteRenderer.meshType == MESH_CIRCLE)
    {
        list = Meshes::pCircleMesh;
    }
    AEGfxMeshDraw(list, AE_GFX_MDM_TRIANGLES);
}

void FreeSpriteRenderer(SpriteRenderer& spriteRenderer)
{
    if(spriteRenderer.texture)
    {
        // Assuming there's a function to free textures
        AEGfxTextureUnload(spriteRenderer.texture);
        spriteRenderer.texture = nullptr;
	}
    list = nullptr;
}