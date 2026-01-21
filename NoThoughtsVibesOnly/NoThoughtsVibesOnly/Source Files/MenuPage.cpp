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
	float r, g, b;
	int actionID;
	bool isHovered;
};

// ---------------------------------------------------------------------------
// Static Variables
// ---------------------------------------------------------------------------

static MenuSubState subState = SUB_MAIN;
static s8 fontId = 0;
static std::vector<Button> buttons;

// Transformation matrices
static AEMtx33 transform, scale, rotate, translate;

const float SCREEN_W = 1600.0f;
const float SCREEN_H = 900.0f;

// ---------------------------------------------------------------------------
// Helper Functions
// ---------------------------------------------------------------------------

void AddButton(const char* text, float x, float y, int actionID) {
	Button b;
	b.text = text;
	b.x = x;
	b.y = y;
	b.width = 300.0f;
	b.height = 60.0f;
	b.r = 0.3f; b.g = 0.3f; b.b = 0.3f;
	b.actionID = actionID;
	b.isHovered = false;
	buttons.push_back(b);
}

void SetupButtons() {
	buttons.clear();

	if (subState == SUB_MAIN) {
		AddButton("START", 0.0f, 100.0f, 1);
		AddButton("CONTROLS", 0.0f, 0.0f, 2);
		AddButton("CREDITS", 0.0f, -100.0f, 3);
		AddButton("QUIT", 0.0f, -200.0f, 4);
	}
	else if (subState == SUB_CONTROLS || subState == SUB_CREDITS) {
		AddButton("BACK", 0.0f, -300.0f, 5);
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
	// Ensure you have arial.ttf in your project folder!
	fontId = AEGfxCreateFont("arial.ttf", 30);
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
			btn.r = 0.5f; btn.g = 0.5f; btn.b = 0.5f; // Highlight

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
			btn.r = 0.3f; btn.g = 0.3f; btn.b = 0.3f; // Reset
		}
	}
}

void Main_Draw()
{
	AEGfxSetRenderMode(AE_GFX_RM_COLOR);
	AEGfxSetBlendMode(AE_GFX_BM_BLEND);

	// --- FIX: Removed (s8*) casts below ---
	if (subState == SUB_CONTROLS) {
		AEGfxPrint(fontId, "CONTROLS: WASD to Move, SPACE to Jump", -0.4f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	else if (subState == SUB_CREDITS) {
		AEGfxPrint(fontId, "CREDITS: Made with Alpha Engine", -0.35f, 0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
	}
	else {
		AEGfxPrint(fontId, "MAIN MENU", -0.12f, 0.6f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f);
	}

	for (const auto& btn : buttons)
	{
		CreateSquare(Meshes::pSquareCOriMesh, &transform, &scale, &rotate, &translate,
			btn.x, btn.y, btn.width, btn.height, 0.0f,
			btn.r, btn.g, btn.b, 1.0f);

		float textX = (btn.x / (SCREEN_W / 2.0f)) - (strlen(btn.text) * 0.015f);
		float textY = (btn.y / (SCREEN_H / 2.0f)) - 0.02f;

		// --- FIX: Removed (s8*) cast here ---
		AEGfxPrint(fontId, btn.text, textX, textY, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f);
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