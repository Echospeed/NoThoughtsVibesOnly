#include "MainMenu.h"
#include <vector>

// Internal structs and globals
struct MenuButton {
    float x, y, width, height, r, g, b;
    GameState actionState;
    bool isHovered;
    const char* text;
};

static AEGfxVertexList* pSquareMesh = nullptr;
static s8 fontId = 0;
static std::vector<MenuButton> buttons;

// Helper: Point in Rect
bool IsPointInRect(float pX, float pY, float rectX, float rectY, float rectW, float rectH) {
    float halfW = rectW / 2.0f;
    float halfH = rectH / 2.0f;
    return (pX >= rectX - halfW && pX <= rectX + halfW && pY >= rectY - halfH && pY <= rectY + halfH);
}

void MainMenu_Load() {
    // Create Mesh
    AEGfxMeshStart();
    AEGfxTriAdd(-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f, 0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    AEGfxTriAdd(0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f, 0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f, -0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f);
    pSquareMesh = AEGfxMeshEnd();

    fontId = AEGfxCreateFont("../Assets/buggy-font.ttf", 30);

    buttons.push_back({ 0.0f, 0.0f, 300.0f, 80.0f, 0.0f, 0.7f, 0.0f, STATE_GAME, false, "START" });
    buttons.push_back({ 0.0f, -100.0f, 300.0f, 80.0f, 0.8f, 0.0f, 0.0f, STATE_QUIT, false, "QUIT" });
}

void MainMenu_Update(GameState& currentState) {
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);
    float worldMouseX = (float)mouseX - (1600 / 2.0f);
    float worldMouseY = -((float)mouseY - (900 / 2.0f));

    for (auto& btn : buttons) {
        if (IsPointInRect(worldMouseX, worldMouseY, btn.x, btn.y, btn.width, btn.height)) {
            btn.isHovered = true;
            if (AEInputCheckTriggered(AEVK_LBUTTON)) currentState = btn.actionState;
        }
        else {
            btn.isHovered = false;
        }
    }
}

void MainMenu_Draw() {
    for (const auto& btn : buttons) {
        float r = btn.isHovered ? btn.r + 0.3f : btn.r;

        // Render Mesh
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetColorToMultiply(r, btn.g, btn.b, 1.0f);
        AEMtx33 scale, trans, transform;
        AEMtx33Scale(&scale, btn.width, btn.height);
        AEMtx33Trans(&trans, btn.x, btn.y);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(pSquareMesh, AE_GFX_MDM_TRIANGLES);

        // Render Text
        float textX = (btn.x / 800.0f) - (strlen(btn.text) * 0.015f);
        float textY = (btn.y / 450.0f) - 0.025f;
        AEGfxPrint(fontId, btn.text, textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void MainMenu_Unload() {
    AEGfxMeshFree(pSquareMesh);
    AEGfxDestroyFont(fontId);
}

//// ---------------------------------------------------------------------------
//// Includes
//// ---------------------------------------------------------------------------
//#include <crtdbg.h> // Memory leak detection
//#include <vector>   // For managing the list of buttons
//#include <iostream> // For console output
//#include "AEEngine.h"
//
//// ---------------------------------------------------------------------------
//// Structs and Enums
//// ---------------------------------------------------------------------------
//
//enum GameState {
//	STATE_MENU,
//	STATE_GAME,
//	STATE_CONTROLS,
//	STATE_CREDITS,
//	STATE_QUIT
//};
//
//struct MenuButton {
//	float x, y;             // World Position
//	float width, height;    // Dimensions
//	float r, g, b;          // Base Color
//	GameState actionState;  // State to switch to when clicked
//	bool isHovered;         // Is mouse over this button?
//	const char* text;       // Text label
//};
//
//// ---------------------------------------------------------------------------
//// Globals
//// ---------------------------------------------------------------------------
//
//AEGfxVertexList* pSquareMesh = nullptr;
//s8 fontId = 0; // Handle for the text font
//
//// ---------------------------------------------------------------------------
//// Helper Functions
//// ---------------------------------------------------------------------------
//
//// Creates the standard 1x1 normalized square mesh
//void CreateSquareMesh()
//{
//	AEGfxMeshStart();
//
//	// Triangle 1
//	AEGfxTriAdd(
//		-0.5f, -0.5f, 0xFFFFFFFF, 0.0f, 1.0f,
//		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
//		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
//	);
//	// Triangle 2
//	AEGfxTriAdd(
//		0.5f, -0.5f, 0xFFFFFFFF, 1.0f, 1.0f,
//		0.5f, 0.5f, 0xFFFFFFFF, 1.0f, 0.0f,
//		-0.5f, 0.5f, 0xFFFFFFFF, 0.0f, 0.0f
//	);
//
//	pSquareMesh = AEGfxMeshEnd();
//}
//
//// Checks if a point (mouse) is inside a rectangle (button)
//bool IsPointInRect(float pX, float pY, float rectX, float rectY, float rectW, float rectH)
//{
//	float halfW = rectW / 2.0f;
//	float halfH = rectH / 2.0f;
//
//	return (pX >= rectX - halfW && pX <= rectX + halfW &&
//		pY >= rectY - halfH && pY <= rectY + halfH);
//}
//
//// Draws a button with color and transformation
//void DrawButton(float x, float y, float w, float h, float r, float g, float b)
//{
//	// Set Color
//	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
//	AEGfxSetColorToMultiply(r, g, b, 1.0f);
//	AEGfxSetBlendMode(AE_GFX_BM_BLEND);
//
//	// Create Transformation Matrix
//	AEMtx33 scale, trans, transform;
//	AEMtx33Scale(&scale, w, h);
//	AEMtx33Trans(&trans, x, y);
//	AEMtx33Concat(&transform, &trans, &scale);
//
//	// Send to GPU
//	AEGfxSetTransform(transform.m);
//	AEGfxMeshDraw(pSquareMesh, AE_GFX_MDM_TRIANGLES);
//}
//
//// ---------------------------------------------------------------------------
//// Main
//// ---------------------------------------------------------------------------
//
//int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
//	_In_opt_ HINSTANCE hPrevInstance,
//	_In_ LPWSTR    lpCmdLine,
//	_In_ int       nCmdShow)
//{
//	// Enable memory leak check
//	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
//
//	// 1. Initialize System
//	const int SCREEN_WIDTH = 1600;
//	const int SCREEN_HEIGHT = 900;
//	AESysInit(hInstance, nCmdShow, SCREEN_WIDTH, SCREEN_HEIGHT, 1, 60, false, NULL);
//	AESysSetWindowTitle("No Thoughts Vibes Only - Main Menu");
//
//	// 2. Initialize Resources
//	CreateSquareMesh();
//	AESysReset();
//
//	// Load Font - Try to load Arial. Size 30.
//	// NOTE: If this fails to load text, ensure you have "Arial.ttf" in your folder 
//	// or try integer 0 if your engine setup uses default fonts differently.
//	fontId = AEGfxCreateFont("../Assets/buggy-font.ttf", 30);
//
//	// 3. Setup Buttons
//	std::vector<MenuButton> buttons;
//	// Start (Green)
//	buttons.push_back({ 0.0f, 150.0f, 300.0f, 80.0f, 0.0f, 0.7f, 0.0f, STATE_GAME, false, "START" });
//	// Controls (Blue)
//	buttons.push_back({ 0.0f, 50.0f, 300.0f, 80.0f, 0.0f, 0.4f, 0.8f, STATE_CONTROLS, false, "CONTROLS" });
//	// Credits (Purple)
//	buttons.push_back({ 0.0f, -50.0f, 300.0f, 80.0f, 0.6f, 0.0f, 0.6f, STATE_CREDITS, false, "CREDITS" });
//	// Quit (Red)
//	buttons.push_back({ 0.0f, -150.0f, 300.0f, 80.0f, 0.8f, 0.0f, 0.0f, STATE_QUIT, false, "QUIT" });
//
//	int gGameRunning = 1;
//	GameState currentState = STATE_MENU;
//
//	// -----------------------------------------------------------------------
//	// Game Loop
//	// -----------------------------------------------------------------------
//
//	while (gGameRunning)
//	{
//		AESysFrameStart();
//
//		// Update Input
//		AEInputUpdate();
//
//		// Quit Logic (Escape or Close Window)
//		if (AEInputCheckTriggered(AEVK_ESCAPE) || !AESysDoesWindowExist())
//			gGameRunning = 0;
//
//		// -------------------------------------------------------------------
//		// Input & Logic
//		// -------------------------------------------------------------------
//
//		// Get Mouse Position
//		s32 mouseX, mouseY;
//		AEInputGetCursorPosition(&mouseX, &mouseY);
//
//		// Convert Screen Coordinates (Top-Left 0,0) to World Coordinates (Center 0,0)
//		float worldMouseX = (float)mouseX - (SCREEN_WIDTH / 2.0f);
//		float worldMouseY = -((float)mouseY - (SCREEN_HEIGHT / 2.0f));
//
//		if (currentState == STATE_MENU)
//		{
//			for (auto& btn : buttons)
//			{
//				// Check Collision
//				if (IsPointInRect(worldMouseX, worldMouseY, btn.x, btn.y, btn.width, btn.height))
//				{
//					btn.isHovered = true;
//					if (AEInputCheckTriggered(AEVK_LBUTTON))
//					{
//						currentState = btn.actionState;
//					}
//				}
//				else
//				{
//					btn.isHovered = false;
//				}
//			}
//		}
//		else if (currentState == STATE_QUIT)
//		{
//			gGameRunning = 0;
//		}
//		else
//		{
//			// In Game/Credits/Controls: Click or Space to return to Menu
//			if (AEInputCheckTriggered(AEVK_LBUTTON) || AEInputCheckTriggered(AEVK_SPACE))
//			{
//				currentState = STATE_MENU;
//			}
//		}
//
//		// -------------------------------------------------------------------
//		// Rendering
//		// -------------------------------------------------------------------
//
//		AEGfxSetBackgroundColor(0.2f, 0.2f, 0.2f); // Dark Grey Background
//		AEGfxSetRenderMode(AE_GFX_RM_COLOR);
//
//		// Reset color to white before text drawing to avoid tinting
//		AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
//
//		if (currentState == STATE_MENU)
//		{
//			for (const auto& btn : buttons)
//			{
//				// 1. Draw Button Mesh
//				// Lighten color if hovered
//				float r = btn.isHovered ? btn.r + 0.3f : btn.r;
//				float g = btn.isHovered ? btn.g + 0.3f : btn.g;
//				float b = btn.isHovered ? btn.b + 0.3f : btn.b;
//
//				DrawButton(btn.x, btn.y, btn.width, btn.height, r, g, b);
//
//				// 2. Draw Text
//				// Map World Position to Screen Coords (-1.0 to 1.0) for Text
//				float textX = btn.x / (SCREEN_WIDTH / 2.0f);
//				float textY = btn.y / (SCREEN_HEIGHT / 2.0f);
//
//				// Center the text roughly based on length
//				float textOffset = (float)strlen(btn.text) * 0.015f;
//				textX -= textOffset;
//				textY -= 0.025f; // Slight Y adjustment for vertical centering
//
//				// Draw Text: FontID, Text, X, Y, Scale, R, G, B, A
//				// REMOVED (s8*) cast here to fix the error
//				AEGfxPrint(fontId, btn.text, textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
//			}
//		}
//		else if (currentState == STATE_GAME)
//		{
//			AEGfxSetBackgroundColor(0.0f, 0.5f, 0.0f); // Green
//			AEGfxPrint(fontId, "GAME STATE - Click to Return", -0.4f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
//		}
//		else if (currentState == STATE_CONTROLS)
//		{
//			AEGfxSetBackgroundColor(0.0f, 0.2f, 0.5f); // Blue
//			AEGfxPrint(fontId, "CONTROLS SCREEN - Click to Return", -0.45f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
//		}
//		else if (currentState == STATE_CREDITS)
//		{
//			AEGfxSetBackgroundColor(0.5f, 0.0f, 0.5f); // Purple
//			AEGfxPrint(fontId, "CREDITS SCREEN - Click to Return", -0.4f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
//		}
//
//		AESysFrameEnd();
//	}
//
//	// -----------------------------------------------------------------------
//	// Cleanup
//	// -----------------------------------------------------------------------
//
//	AEGfxDestroyFont(fontId);
//	AEGfxMeshFree(pSquareMesh);
//	AESysExit();
//
//	return 0;
//}