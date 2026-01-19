// ---------------------------------------------------------------------------
// includes

#include <crtdbg.h> // To check for memory leaks
#include "AEEngine.h"
#include "Util.h"

AEGfxVertexList* pSquareMesh{ 0 };   // Standard Square Mesh for UI

Square button{
	0.0f,
	0.0f,
	1200.0f,
	50.0f,
	0.0f,
	1.0f, 0.0f, 0.0f, 1.0f

};

// Standard Square Mesh for UI
void static CreateSquareMesh()
{
	AEGfxMeshStart();

	AEGfxTriAdd(
		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
	);
	AEGfxTriAdd(
		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
	);

	pSquareMesh = AEGfxMeshEnd();
}



// ---------------------------------------------------------------------------
// main

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	int gGameRunning = 1;

	// Initialization of your own variables go here

	// Using custom window procedure
	AESysInit(hInstance, nCmdShow, 1600, 900, 1, 60, false, NULL);

	// Changing the window title
	AESysSetWindowTitle("No Thoughts Vibes Only");

	CreateSquareMesh();

	// reset the system modules
	AESysReset();

	// Variables for Matrix 
	AEMtx33 rot{ 0 }, scale{ 0 }, trans{ 0 }, transform{ 0 };

	printf("Start project\n");

	// Game Loop
	while (gGameRunning)
	{
		// Informing the system about the loop's start
		AESysFrameStart();

		// Basic way to trigger exiting the application
		// when ESCAPE is hit or when the window is closed
		if (AEInputCheckTriggered(AEVK_ESCAPE) || 0 == AESysDoesWindowExist())
			gGameRunning = 0;

		// Your own update logic goes here


		// Your own rendering logic goes here
		AEGfxSetBackgroundColor(0.5f, 0.5f, 0.5f);
		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
		AEGfxSetBlendMode(AE_GFX_BM_BLEND);

		CreateSquare(pSquareMesh, &transform, &scale, &rot, &trans,
			button.xpos, button.ypos, button.scaleX, button.scaleY, button.rot,
			button.r, button.g, button.b, button.a);

		// Informing the system about the loop's end
		AESysFrameEnd();

	}


	// free the system
	AESysExit();
}