#include "SplashPage.h"
#include "AEEngine.h"
#include "Util.h"

AEGfxTexture* ss_DigiPen_Logo;

// Simple timer or input check to transition to the menu
static float timer = 0.0f;

namespace
{
	float Lerp(float a, float b, float t)
	{
	return a + t * (b - a);
	}
}


void SplashPage_Load()
{
	const char* path = "Assets/DigiPen_Singapore_WEB_RED.png";
	ss_DigiPen_Logo = AEGfxTextureLoad(path);

	//std::printf("[Splash] AEGfxTextureLoad('%s') => %p\n", path, (void*)ss_DigiPen_Logo);

	//ss_DigiPen_Logo = AEGfxTextureLoad("Assets/DigiPen_Singapore_WEB_RED.png");
	//ss_DigiPen_Logo = nullptr;
}

void SplashPage_Init()
{
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f); // Black background
	Meshes::CreateSquareCenterOriginMesh();
}

void SplashPage_Update()
{
	timer += (float)AEFrameRateControllerGetFrameTime();
	if (timer > 6.0f || AEInputCheckTriggered(AEVK_SPACE)) // 6 seconds or space key
	{
		StateManagerChangeState(STATE_MENU);
	}
}

void SplashPage_Draw()
{
	if (!ss_DigiPen_Logo) return;
	AEGfxSetCamPosition(0.0f, 0.0f);

	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	// Example: Draw a simple rectangle as a placeholder for a logo
	AEMtx33 transform, scale, translate;
	AEMtx33Scale(&scale, Lerp(1000, 1525.0f, timer/6.0f), Lerp(300.0f, 445.0f, timer/6.0f));
	AEMtx33Trans(&translate, 0.0f, 0.0f);
	AEMtx33Concat(&transform, &translate, &scale);
	AEGfxSetTransform(transform.m);

	// Draw splash screen content here
	AEGfxSetRenderMode(AE_GFX_RM_TEXTURE);
	AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
	AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
	AEGfxSetTransparency(1.0f);
	AEGfxTextureSet(ss_DigiPen_Logo, 0, 0);
	AEGfxSetTransform(transform.m);
	AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

}

void SplashPage_Free()
{
	// Free resources specific to the splash page here
	Meshes::FreeMeshes();
}

void SplashPage_Unload()
{
	// Unload any remaining resources if necessary
	AEGfxTextureUnload(ss_DigiPen_Logo);
}