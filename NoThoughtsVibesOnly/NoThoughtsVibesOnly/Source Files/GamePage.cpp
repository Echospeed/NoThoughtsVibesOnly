#include "pch.h"
#include <vector>
//The modular level file where game logic is implemented.
// Static members specific to this file (Level scope)
static std::vector<GameObject> sEnemyList;
static AEGfxVertexList* spEnemyMesh;
void Game_Load()
{
	/* --- Load Resources ---*/
	// Creating a triangle mesh using AEGfx functions
	AEGfxMeshStart();
	AEGfxTriAdd(-15.0f, -15.0f, 0xFFFFFFFF, 0.0f, 0.0f,
		15.0f, -15.0f, 0xFFFFFFFF, 0.0f, 0.0f,
		0.0f, 20.0f, 0xFFFFFFFF, 0.0f, 0.0f);
	spEnemyMesh = AEGfxMeshEnd();
}	

void Game_Init()
{
	/* --- Initialize Game Page Variable ---*/
	sEnemyList.clear();
	// Spawn objects into the modular level
	for (int i = 0; i < 3; ++i) {
		GameObject enemy;
		enemy.pos = { (float)(i * 60 - 60), 0.0f };
		enemy.pMesh = spEnemyMesh;
		enemy.color = 0xFFFF0000;
		sEnemyList.push_back(enemy);
	}
}

void Game_Update()
{
	/* --- Logic ---*/
	float dt = (float)AEFrameRateControllerGetFrameTime();

	// Check Input for state transitions
	if (AEInputCheckTriggered(AEVK_R)) current = STATE_RESTART;

	for (auto& enemy : sEnemyList) {
		enemy.Update(dt);
	}
}

void Game_Draw()
{
	/* --- Render ---*/
	for (auto& enemy : sEnemyList) {
		enemy.Draw();
	}
}

void Game_Free()
{
	/* ---  Free Memory ---*/
	sEnemyList.clear();
}

void Game_Unload()
{
	/* --- Unload Resources ---*/
	AEGfxMeshFree(spEnemyMesh);
}
