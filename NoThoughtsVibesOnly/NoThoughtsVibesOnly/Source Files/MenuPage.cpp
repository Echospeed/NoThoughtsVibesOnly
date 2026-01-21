#include "MenuPage.h"
#include "Util.h"
#include "AEEngine.h"
#include <vector>
#include <iostream>

// ---------------------------------------------------------------------------
// Structs & Enums
// ---------------------------------------------------------------------------

enum MenuSubState {
	SUB_MAIN,
	SUB_CONTROLS,
	SUB_CREDITS
};

struct Button {
	const char* text;
	float x, y;
	float width, height;

	// Colors
	float r, g, b;				// Current Color
	float baseR, baseG, baseB;	// Base Color

	int actionID;
	bool isHovered;
};

// ---------------------------------------------------------------------------
// Static Variables
// ---------------------------------------------------------------------------

static MenuSubState subState = SUB_MAIN;
static s8 fontId = 0;
static std::vector<Button> buttons;

// Matrices
static AEMtx33 transform, scale, rotate, translate;

const float SCREEN_W = 1600.0f;
const float SCREEN_H = 900.0f;

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------

// Centers text horizontally at a specific World X, Y
void DrawTextCentered(const char* text, float worldX, float worldY, float r = 1.0f, float g = 1.0f, float b = 1.0f)
{
	// 1. Convert World Y to NDC Y (-1 to 1)
	float ndcY = worldY / (SCREEN_H / 2.0f);

	// 2. Convert World X to NDC X
	float ndcX = worldX / (SCREEN_W / 2.0f);

	// 3. Calculate Offset
	// For Buggy Font Size 30, each character is roughly 0.037 units wide in NDC space
	float charWidthNDC = 0.037f;
	float totalWidth = (float)strlen(text) * charWidthNDC;
	float startX = ndcX - (totalWidth / 2.0f);

	AEGfxPrint(fontId, text, startX, ndcY, 1.0f, r, g, b, 1.0f);
}

void AddButton(const char* text, float x, float y, int actionID, float r, float g, float b) {
	Button btn;
	btn.text = text;
	btn.x = x;
	btn.y = y;
	btn.width = 300.0f;
	btn.height = 60.0f;

	btn.r = btn.baseR = r;
	btn.g = btn.baseG = g;
	btn.b = btn.baseB = b;

	btn.actionID = actionID;
	btn.isHovered = false;
	buttons.push_back(btn);
}

void SetupButtons() {
	buttons.clear();

	// Note: X is 0.0f for all buttons to keep them centered in the window
	if (subState == SUB_MAIN) {
		AddButton("START", 0.0f, 100.0f, 1, 0.0f, 0.6f, 0.0f);
		AddButton("CONTROLS", 0.0f, 0.0f, 2, 0.0f, 0.3f, 0.7f);
		AddButton("CREDITS", 0.0f, -100.0f, 3, 0.5f, 0.0f, 0.5f);
		AddButton("QUIT", 0.0f, -200.0f, 4, 0.7f, 0.0f, 0.0f);
	}
	else if (subState == SUB_CONTROLS || subState == SUB_CREDITS) {
		AddButton("BACK", 0.0f, -300.0f, 5, 0.8f, 0.4f, 0.0f);
	}
}

bool IsMouseInRect(float mx, float my, float x, float y, float w, float h) {
	return (mx >= x - w / 2 && mx <= x + w / 2 &&
		my >= y - h / 2 && my <= y + h / 2);
}

// ---------------------------------------------------------------------------
// State Functions
// ---------------------------------------------------------------------------

void Main_Load()
{
	fontId = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
	Meshes::CreateSquareCenterOriginMesh();
}

void Main_Init()
{
	subState = SUB_MAIN;
	SetupButtons();
	AEGfxSetBackgroundColor(0.1f, 0.1f, 0.15f);
}

void Main_Update()
{
	s32 mouseX, mouseY;
	AEInputGetCursorPosition(&mouseX, &mouseY);

	float worldX = (float)mouseX - SCREEN_W / 2.0f;
	float worldY = -((float)mouseY - SCREEN_H / 2.0f);

	for (auto& btn : buttons)
	{
		if (IsMouseInRect(worldX, worldY, btn.x, btn.y, btn.width, btn.height))
		{
			btn.isHovered = true;
			// Brighten color on hover
			btn.r = (btn.baseR + 0.2f > 1.0f) ? 1.0f : btn.baseR + 0.2f;
			btn.g = (btn.baseG + 0.2f > 1.0f) ? 1.0f : btn.baseG + 0.2f;
			btn.b = (btn.baseB + 0.2f > 1.0f) ? 1.0f : btn.baseB + 0.2f;

			if (AEInputCheckTriggered(AEVK_LBUTTON))
			{
				switch (btn.actionID)
				{
				case 1: next = STATE_PLAYING; break;
				case 2: subState = SUB_CONTROLS; SetupButtons(); break;
				case 3: subState = SUB_CREDITS; SetupButtons(); break;
				case 4: next = STATE_QUIT; break;
				case 5: subState = SUB_MAIN; SetupButtons(); break;
				}
			}
		}
		else
		{
			btn.isHovered = false;
			btn.r = btn.baseR; btn.g = btn.baseG; btn.b = btn.baseB;
		}
	}
}

void Main_Draw()
{
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	// 1. Draw Headers
	if (subState == SUB_CONTROLS) {
		DrawTextCentered("CONTROLS", 0.0f, 300.0f);
		DrawTextCentered("WASD to Move", 0.0f, 200.0f);
	}
	else if (subState == SUB_CREDITS) {
		DrawTextCentered("CREDITS", 0.0f, 300.0f);
		DrawTextCentered("Made with Alpha Engine", 0.0f, 200.0f);
		DrawTextCentered("By NoThoughtsVibesOnly", 0.0f, 150.0f);
	}
	else {
		// Main Title
		DrawTextCentered("<Insert game name here>", 0.0f, 300.0f);
	}

	// 2. Draw Buttons
	for (const auto& btn : buttons)
	{
		// Draw Mesh
		CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rotate, &translate,
			btn.x, btn.y, btn.width, btn.height, 0.0f,
			btn.r, btn.g, btn.b, 1.0f);

		// Draw Text
		DrawTextCentered(btn.text, btn.x, btn.y - 17.5f);
	}
}

void Main_Free()
{
	buttons.clear();
}

void Main_Unload()
{
	AEGfxDestroyFont(fontId);
	Meshes::FreeMeshes();
}