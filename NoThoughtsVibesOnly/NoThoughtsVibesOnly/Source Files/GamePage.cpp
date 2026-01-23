#include "pch.h"
#include <vector>
#include "ExpUI.h"

// creating simple Exp struct
Experience PlayerExp;

//The modular level file where game logic is implemented.
// Static members specific to this file (Level scope)
static std::vector<GameObject> sEnemyList;
//static AEGfxVertexList* spEnemyMesh;
void Game_Load()
{
	/* --- Load Resources ---*/
	// Creating a triangle mesh using AEGfx functions
	//AEGfxMeshStart();
	//AEGfxTriAdd(-15.0f, -15.0f, 0xFFFFFFFF, 0.0f, 0.0f,
	//	15.0f, -15.0f, 0xFFFFFFFF, 0.0f, 0.0f,
	//	0.0f, 20.0f, 0xFFFFFFFF, 0.0f, 0.0f);
	//spEnemyMesh = AEGfxMeshEnd();
	Meshes::CreateTriangleMesh();
	Meshes::CreateSquareLeftOriginMesh();

}	

void Game_Init()
{
	/* --- Initialize Game Page Variable ---*/
	sEnemyList.clear();
	// Spawn objects into the modular level
	for (int i = 0; i < 3; ++i) {
		GameObject enemy;
		enemy.pos = { (float)(i * 60 - 60), 0.0f };
		enemy.pMesh = Meshes::pTriangleMesh;
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

	//exp gain test currently tied to G key for testing
	if (AEInputCheckCurr(AEVK_G)) {
		printf("G pressed: %f\n", PlayerExp.CurrentExp += 30.0f * dt);
		if (PlayerExp.CurrentExp >= PlayerExp.MaxExp)
		{
			PlayerExp.CurrentExp = PlayerExp.MaxExp;
		}
		PlayerExp.ExpPercentage = PlayerExp.CurrentExp / PlayerExp.MaxExp;
	}	

}

void Game_Draw()
{
	/* --- Render ---*/
	for (auto& enemy : sEnemyList) {
		enemy.Draw();
	}
	AEMtx33 rot{ 0 }, scale{ 0 }, trans{ 0 }, transform{ 0 };
	CreateSquare(Meshes::pSquareLOriMesh, &transform, &scale, &rot, &trans,
		-700.0f, 400.0f, 200.0f, 30.0f, 0.0f,
		1.0f, 1.0f, 1.0f, 1.0f);
	CreateSquare(Meshes::pSquareLOriMesh, &transform, &scale, &rot, &trans,
		-700.0f, 400.0f, PlayerExp.ExpPercentage * 200.0f, 30.0f, 0.0f,
		1.0f, 0.0f, 1.0f, 1.0f);

}

void Game_Free()
{
	/* ---  Free Memory ---*/
	sEnemyList.clear();
}

void Game_Unload()
{
	/* --- Unload Resources ---*/
	Meshes::FreeMeshes();
}
