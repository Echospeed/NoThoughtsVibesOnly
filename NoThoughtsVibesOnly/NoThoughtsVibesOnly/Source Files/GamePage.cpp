#include "pch.h"
#include "GamePage.h"
#include "StateManager.h"
#include "ExpUI.h"
#include "MiniMap.h"
#include "Player.h"
#include "NPC.h"
#include "GameObjectType.h"
#include <vector>
#include <iostream>
#include "Bullet.h"


// ---------------------------------------------------------------------------
// World Settings
// ---------------------------------------------------------------------------
const f32 WORLD_WIDTH = 2000.0f;
const f32 WORLD_HEIGHT = 2000.0f;
const f32 GRID_SIZE = 100.0f;
int availableBullets = 0;

// ---------------------------------------------------------------------------
// Global/Static Object Vectors $DO NOT MOVE, MUST INITIALISE BEFORE USE$
// ---------------------------------------------------------------------------
std::vector<GameObject*> objects;

// ---------------------------------------------------------------------------
// Global/Static GameObjects and Variables
// ---------------------------------------------------------------------------
GameObject* pPlayer{nullptr};

// ---------------------------------------------------------------------------
// AMMO TEXT
// ---------------------------------------------------------------------------
TextRenderer ammoText;
s8 gameFont = 0; // Handle for the font
char ammoBuffer[500]; // Buffer to hold the text string "Ammo: 50"

// Camera position
static f32 sCamX = 0.0f;
static f32 sCamY = 0.0f;
const f32 CAM_SPEED = 5.0f; // Higher = tighter, Lower = smoother/looser

void Game_Load()
{
    Meshes::CreateTriangleMesh();
    Meshes::CreateSquareLeftOriginMesh();
    Meshes::CreateSquareCenterOriginMesh();
    Meshes::CreateCircleMesh();
    // Load the font
    gameFont = AEGfxCreateFont("Assets/buggy-font.ttf", 30);
    LoadTextRenderer(ammoText, gameFont);
}

void Game_Init()
{
    // Initialize Player
    pPlayer = new Player();

    // Initilize Camera
    sCamX = pPlayer->transform.position.x;
    sCamY = pPlayer->transform.position.y;

    // Initialize Ammo UI (Top Left corner usually)
    // Position x=-700, y=400 puts it in top-left area relative to camera center if UI follows camera
    // But since we draw UI *after* resetting camera, we use screen coordinates logic.
    InitTextRenderer(ammoText, "Ammo: 100%", 1.0f, 1.0f, 1.0f, 1.0f); // White text

    // Initialize Enemies
    for (int i = 0; i < 5; ++i) {
        NPC* n = new NPC;
        n->ObjectType = NP;
        n->type = NPC_WALK;
        n->target = pPlayer;
    }

    for (int i = 0; i < 5; ++i) {
        NPC* n = new NPC;
        n->ObjectType = NP;
        n->type = NPC_RANGER;
        n->target = pPlayer;
        for(int j =0; j < 3; ++j) 
        {
            Bullet* bullet = new Bullet();
			bullet->startPos = n; 
            bullet->owner = BulletOwner::ENEMY;
            // doesnt have a type need to set a enum for enemy bullets and player bullet separately and also create a interaction for ranger npc and player
		}
    }

    for (int i = 0; i < 5; ++i) {
        NPC* n = new NPC;
        n->ObjectType = NP;
        n->type = NPC_MELEE; 
        n->target = pPlayer;
    }

    for (int i = 0; i < 500; ++i)
    {
        Bullet* bullet = new Bullet();
        bullet->startPos = pPlayer;
        bullet->isActive = false;   // start inactive
        bullet->owner = BulletOwner::PLAYER;
    }

    for(auto& obj : objects) {
        obj->Start();
	}

	//std::cout << "Game_Init is running!\n";
}

void Game_Update()
{
    f32 dt = (f32)AEFrameRateControllerGetFrameTime();

	//testObject.Update(dt);

    // Boundary Logic
    f32 halfWorldWidth = WORLD_WIDTH / 2.0f;
    f32 halfWorldHeight = WORLD_HEIGHT / 2.0f;
    f32 halfPlayerSizex = (pPlayer->transform.scale.x / 2.0f);
    f32 halfPlayerSizey = (pPlayer->transform.scale.y / 2.0f);

    if (pPlayer->transform.position.x > halfWorldWidth - halfPlayerSizex) pPlayer->transform.position.x = halfWorldWidth - halfPlayerSizex;
    if (pPlayer->transform.position.x < -halfWorldWidth + halfPlayerSizex) pPlayer->transform.position.x = -halfWorldWidth + halfPlayerSizex;
    if (pPlayer->transform.position.y > halfWorldHeight - halfPlayerSizey) pPlayer->transform.position.y = halfWorldHeight - halfPlayerSizey;
    if (pPlayer->transform.position.y < -halfWorldHeight + halfPlayerSizey) pPlayer->transform.position.y = -halfWorldHeight + halfPlayerSizey;

    // Smooth Camera Follow (Lerp)
    // This calculates the difference between player and camera, and moves a fraction of that distance
    sCamX += (pPlayer->transform.position.x - sCamX) * CAM_SPEED * dt;
    sCamY += (pPlayer->transform.position.y - sCamY) * CAM_SPEED * dt;

    // -----------------------------------------------------------------------
    // Game State Controls
    // -----------------------------------------------------------------------
    /*for (auto& obj : objects) {
        obj->Update(dt);
    }*/

    // --- LOGIC: Count Available Ammo ---
    availableBullets = 0;
    int totalBullets = 0;

    for (auto& obj : objects)
    {
        // 1. SAFETY CHECK: First, make sure it is actually a bullet (SHOT)
        if (obj->ObjectType == ObjectType::SHOT)
        {
            // 2. SAFE CAST: Now we know it's safe to treat it as a Bullet
            Bullet* b = (Bullet*)obj;

            // 3. LOGIC: Only count bullets that belong to the player
            if (b->owner == BulletOwner::PLAYER)
            {
                totalBullets++;

                // Count it if it is ready to be fired (inactive)
                if (!b->isActive)
                {
                    availableBullets++;
                }
            }
        }
    }

	// Check Player Health
    if(dynamic_cast<Player*>(pPlayer)->health <= 0.0f)
    {
        std::cout << "Player has died!\n";
        
        StateManagerChangeState(STATE_FINISH);
	}
    bool anyEnemiesLeft = false;

    for (GameObject* obj : objects)
    {
        // Check if the object is an NPC and is still active in the world
        if (obj != nullptr && obj->ObjectType == NP && obj->isActive)
        {
            anyEnemiesLeft = true;
            break; // We found one, so no need to keep checking the rest
        }
    }

    if (!anyEnemiesLeft)
    {
        std::cout << "All enemies cleared! Victory!\n";
        StateManagerChangeState(STATE_WIN);
    }

    for (auto& obj : objects)
    {
        if (!obj || !obj->isActive) continue;

        // NPC handling
        if (obj->ObjectType == NP)
        {
            NPC* npc = dynamic_cast<NPC*>(obj);

            // 🔒 Ghost NPCs do NOTHING
            if (!npc)continue;

            npc->Update(dt);
        }
        else
        {
            // Player, bullets, everything else
            obj->Update(dt);
        }
    }

    // Format the text string
    AEGfxSetCamPosition(0.0f, 0.0f);
    sprintf_s(ammoBuffer, "Ammo: %d / %d", availableBullets, 500);
    ammoText.text = ammoBuffer;


    //check if enemy count is 0 to finish level or press space key
    if (AEInputCheckTriggered(AEVK_SPACE)) // space key
    {
        StateManagerChangeState(STATE_FINISH);
    }
	//win condition: if all NPCs are inactive
    if (AEInputCheckTriggered(AEVK_RETURN)) // space key
    {
        StateManagerChangeState(STATE_WIN);
    }
    // Restart Level
    if (AEInputCheckTriggered(AEVK_R))
    {
        StateManagerChangeState(STATE_RESTART);
    }
    // Return to Main Menu
    if (AEInputCheckTriggered(AEVK_Q))
    {
        StateManagerChangeState(STATE_MENU);
    }

}

void Game_Draw()
{
    // Camera Follow
    AEGfxSetCamPosition(sCamX, sCamY);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    // Draw World
    AEMtx33 transform, s, t;

    // Border
    AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
    AEMtx33Scale(&s, WORLD_WIDTH + 20.0f, WORLD_HEIGHT + 20.0f);
    AEMtx33Trans(&t, 0.0f, 0.0f);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // Floor
    AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 1.0f);
    AEMtx33Scale(&s, WORLD_WIDTH, WORLD_HEIGHT);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

    // Grid
    AEGfxSetColorToMultiply(0.5f, 0.5f, 0.5f, 1.0f);
    for (f32 gx = -WORLD_WIDTH / 2; gx <= WORLD_WIDTH / 2; gx += GRID_SIZE)
    {
        AEMtx33Scale(&s, 2.0f, WORLD_HEIGHT);
        AEMtx33Trans(&t, gx, 0.0f);
        AEMtx33Concat(&transform, &t, &s);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }
    for (f32 gy = -WORLD_HEIGHT / 2; gy <= WORLD_HEIGHT / 2; gy += GRID_SIZE)
    {
        AEMtx33Scale(&s, WORLD_WIDTH, 2.0f);
        AEMtx33Trans(&t, 0.0f, gy);
        AEMtx33Concat(&transform, &t, &s);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
    }

    // AoE indicator
    f32 aoeRadius = 33.0f; // radius of effect
    AEVec2 playerPos = pPlayer->transform.position;

    AEMtx33Scale(&s, aoeRadius * 2.0f, aoeRadius * 2.0f); // diameter
    AEMtx33Trans(&t, playerPos.x, playerPos.y);
    AEMtx33Concat(&transform, &t, &s);
    AEGfxSetTransform(transform.m);

    // Semi-transparent black
    AEGfxSetColorToMultiply(0.0f, 0.0f, 0.0f, 0.5f);
    AEGfxMeshDraw(Meshes::pCircleMesh, AE_GFX_MDM_TRIANGLES);

    // Reset color if needed
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);

    // Draw Objects
    AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
    for (auto& obj : objects)
    {
        // Skip invisible NPCs
        if (obj->ObjectType == NP)
        {
            NPC* npc = dynamic_cast<NPC*>(obj);
            if (!npc || !npc->isVisibleToPlayer) continue;
        }
        obj->Draw();
    }

    // Draw MiniMap
    DrawMinimap(objects, sCamX, sCamY);
    DrawTextRenderer(ammoText, { -500.0f , 400.0f }, 1.0f);
}

void Game_Free()
{
    for (auto& obj : objects)
    {
        delete obj;
        obj = nullptr;
    }

    objects.clear();

    objects.shrink_to_fit();

    pPlayer = nullptr;
}

void Game_Unload()
{
    Meshes::FreeMeshes();
    AEGfxDestroyFont(gameFont); // <--- Add this to stop leaks!
}