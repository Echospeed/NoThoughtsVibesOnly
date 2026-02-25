// ============================================================================
// GameUI.cpp - In-Game HUD Rendering
// ============================================================================
// Centralises all HUD drawing so Game_Draw() stays clean.
// Call Init() once in Game_Init(), then call the draw methods from Game_Draw().
//
// DRAW ORDER (from Game_Draw):
// ----------------------------------------------------------------------------
//   WORLD SPACE (before AEGfxSetCamPosition(0,0)):
//     gameUI.DrawAllHealthBars();     <- floats above entities in the world
//
//   SCREEN SPACE (after AEGfxSetCamPosition(0,0)):
//     gameUI.DrawHealthText();
//     gameUI.DrawXPBar();
//     gameUI.DrawCurrentStats();
//     gameUI.DrawWaveInfo();
//     gameUI.DrawWaveTimer();         <- only shown during wave breaks
//     gameUI.DrawPowerUpScreen();     <- only shown during level-up
// ============================================================================

#include "pch.hpp"
#include "GameUI.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "WaveSystem.hpp"
#include "PowerUpSystem.hpp"
#include "GamePage.hpp"
#include "TextRenderer.hpp"
#include "Transform.hpp"

extern GameObject* pPlayer; // Declared in GamePage.cpp

// ============================================================================
// Init
// ============================================================================
void GameUI::Init(s8 font, WaveSystem* waves, PowerUpSystem* powerups)
{
    gameFont = font;
    waveSystem = waves;
    powerUpSystem = powerups;
}

// ============================================================================
// DrawRect (private helper)
// ============================================================================
// Draws a solid coloured rectangle at world/screen position (x,y) with size (w,h).
// ============================================================================
void GameUI::DrawRect(f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a)
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(r, g, b, a);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    Transform tf;
    tf.SetPosition(x, y);
    tf.SetScale(w, h);
    tf.Apply();

    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// DrawText (private helper)
// ============================================================================
// Draws a temporary text string at (x, y) with the given scale and colour.
// Constructs and frees a TextRenderer each call (suitable for HUD text).
// ============================================================================
void GameUI::DrawText(const char* text, f32 x, f32 y, f32 scale, f32 r, f32 g, f32 b)
{
    TextRenderer t;
    LoadTextRenderer(t, gameFont);
    InitTextRenderer(t, text, { scale, scale }, r, g, b);
    DrawTextRenderer(t, { x, y }, scale);
    FreeTextRenderer(t);
}

// ============================================================================
// IsMouseOverBox (private helper)
// ============================================================================
// Point-in-AABB test using raw pixel mouse position, converted to screen space.
// Used for power-up card mouse-over in DrawPowerUpScreen().
// ============================================================================
bool GameUI::IsMouseOverBox(f32 boxX, f32 boxY, f32 boxW, f32 boxH)
{
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    // Convert pixel -> screen-center-relative
    const f32 worldX = static_cast<f32>(mouseX) - 800.0f;
    const f32 worldY = 450.0f - static_cast<f32>(mouseY);

    const f32 halfW = boxW / 2.0f;
    const f32 halfH = boxH / 2.0f;

    return (worldX >= boxX - halfW && worldX <= boxX + halfW &&
        worldY >= boxY - halfH && worldY <= boxY + halfH);
}

// ============================================================================
// DrawAllHealthBars - WORLD SPACE
// ============================================================================
// Draws health bars floating above the player and all visible, active NPCs.
// Must be called BEFORE AEGfxSetCamPosition(0, 0).
// ============================================================================
void GameUI::DrawAllHealthBars()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_NONE);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    Transform tf;

    // -------------------------------------------------------------------------
    // Player health bar
    // -------------------------------------------------------------------------
    const Player* player = dynamic_cast<Player*>(pPlayer);
    if (player && player->isActive)
    {
        const f32 barX = player->transform.position.x;
        const f32 barY = player->transform.position.y + 80.0f;
        const f32 barW = 60.0f;
        const f32 barH = 8.0f;
        const f32 ratio = AEClamp(player->health / player->maxHealth, 0.0f, 1.0f);

        // Red background (empty health)
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
        tf.SetPosition(barX, barY);
        tf.SetScale(barW, barH);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // Green fill (current health), offset to anchor to left edge
        if (ratio > 0.01f)
        {
            const f32 greenW = barW * ratio;
            const f32 greenX = barX - (barW / 2.0f) + (greenW / 2.0f);
            AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
            tf.SetPosition(greenX, barY);
            tf.SetScale(greenW, barH);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    // -------------------------------------------------------------------------
    // Enemy health bars
    // -------------------------------------------------------------------------
    for (GameObject* obj : gamePageObj)
    {
        if (obj->ObjectType != NP || !obj->isActive) continue;

        const NPC* npc = dynamic_cast<NPC*>(obj);
        if (!npc || !npc->isVisibleToPlayer) continue;

        const f32 barX = npc->transform.position.x;
        const f32 barY = npc->transform.position.y + 40.0f;
        const f32 barW = 30.0f;
        const f32 barH = 4.0f;
        const f32 ratio = AEClamp(npc->health / 100.0f, 0.0f, 1.0f);

        // Red background
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
        tf.SetPosition(barX, barY);
        tf.SetScale(barW, barH);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // Orange fill
        if (ratio > 0.01f)
        {
            const f32 fillW = barW * ratio;
            const f32 fillX = barX - (barW / 2.0f) + (fillW / 2.0f);
            AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 1.0f);
            tf.SetPosition(fillX, barY);
            tf.SetScale(fillW, barH);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
}

// ============================================================================
// DrawHealthText - SCREEN SPACE
// ============================================================================
void GameUI::DrawHealthText()
{
    const Player* player = dynamic_cast<Player*>(pPlayer);
    if (!player) return;

    char buf[64];
    sprintf_s(buf, "HP: %.0f / %.0f", player->health, player->maxHealth);
    DrawText(buf, -500.0f, -400.0f, 1.2f, 1.0f, 1.0f, 1.0f);
}

// ============================================================================
// DrawXPBar - SCREEN SPACE
// ============================================================================
// Draws a horizontal XP progress bar with level and XP text above it.
// ============================================================================
void GameUI::DrawXPBar()
{
    if (!powerUpSystem) return;

    const PlayerStats& stats = powerUpSystem->GetStats();

    const f32 barX = -500.0f;
    const f32 barY = 300.0f;
    const f32 barW = 250.0f;
    const f32 barH = 20.0f;

    // Dark panel background
    DrawRect(barX, barY, barW, barH, 0.2f, 0.2f, 0.2f, 0.9f);

    // Green XP fill
    const f32 fill = (stats.expToNextLevel > 0.0f)
        ? stats.currentExp / stats.expToNextLevel : 0.0f;

    if (fill > 0.01f)
    {
        const f32 fw = barW * fill;
        const f32 fx = barX - (barW / 2.0f) + (fw / 2.0f);
        DrawRect(fx, barY, fw, barH - 4.0f, 0.0f, 0.85f, 0.2f, 0.95f);
    }

    // Level and XP text
    char buf[64];
    sprintf_s(buf, "LVL %u   %d / %d XP",
        stats.level, (int)stats.currentExp, (int)stats.expToNextLevel);
    DrawText(buf, barX, barY + 28.0f, 0.9f, 1.0f, 1.0f, 0.3f);
}

// ============================================================================
// DrawCurrentStats - SCREEN SPACE
// ============================================================================
// Always-visible compact panel showing current SPD / DMG / AOE values.
// Hidden during wave break to reduce visual clutter.
// ============================================================================
void GameUI::DrawCurrentStats()
{
    if (!powerUpSystem) return;
    if (waveSystem && waveSystem->IsInBreak()) return;

    const PlayerStats& stats = powerUpSystem->GetStats();

    const f32 panelX = 0.0f;
    const f32 panelY = 380.0f;
    const f32 panelW = 395.0f;
    const f32 panelH = 120.0f;

    DrawRect(panelX, panelY, panelW, panelH, 0.1f, 0.1f, 0.15f, 0.85f);
    DrawText("CURRENT STATS", panelX, panelY + 38.0f, 0.6f, 0.8f, 0.8f, 0.8f);

    char spd[32]; sprintf_s(spd, "SPD: %.0f", stats.GetTotalSpeed());
    DrawText(spd, panelX - 100.0f, panelY + 8.0f, 0.5f, 0.4f, 0.8f, 1.0f);

    char dmg[32]; sprintf_s(dmg, "DMG: %.0f", stats.GetTotalBulletDamage());
    DrawText(dmg, panelX + 100.0f, panelY + 8.0f, 0.5f, 1.0f, 0.5f, 0.2f);

    char aoe[48]; sprintf_s(aoe, "AOE: r=%.0f  d=%.0f/s",
        stats.GetTotalAoeRadius(), stats.GetTotalAoeDamage());
    DrawText(aoe, panelX, panelY - 22.0f, 0.5f, 0.4f, 1.0f, 0.4f);
}

// ============================================================================
// DrawWaveInfo - SCREEN SPACE
// ============================================================================
// Shows current wave, round number, and active enemy count (top-right area).
// ============================================================================
void GameUI::DrawWaveInfo()
{
    if (!waveSystem) return;

    const f32 infoX = 400.0f;
    const f32 infoY = 400.0f;

    char waveBuf[64];
    sprintf_s(waveBuf, "WAVE %u", waveSystem->GetCurrentWave());
    DrawText(waveBuf, infoX, infoY, 1.3f, 1.0f, 0.3f, 0.3f);

    char roundBuf[64];
    sprintf_s(roundBuf, "Round %u", waveSystem->GetCurrentRound());
    DrawText(roundBuf, infoX, infoY - 35.0f, 0.9f, 0.7f, 0.7f, 0.7f);

    // Count active NPCs for the enemy counter display
    int activeEnemies = 0;
    for (const GameObject* obj : gamePageObj)
        if (obj && obj->isActive && obj->ObjectType == NP)
            ++activeEnemies;

    char enemyBuf[64];
    sprintf_s(enemyBuf, "%d Enemies", activeEnemies);
    DrawText(enemyBuf, infoX, infoY - 70.0f, 0.9f, 1.0f, 1.0f, 0.3f);
}

// ============================================================================
// DrawWaveTimer - SCREEN SPACE
// ============================================================================
// Large countdown shown during the break between waves.
// Only visible when waveSystem->IsInBreak() is true.
// ============================================================================
void GameUI::DrawWaveTimer()
{
    if (!waveSystem || !waveSystem->IsInBreak()) return;

    const f32 timeRemaining = waveSystem->GetBreakTimeRemaining();
    const f32 timerX = 0.0f;
    const f32 timerY = 400.0f;

    DrawRect(timerX, timerY, 395.0f, 120.0f, 0.1f, 0.1f, 0.15f, 0.85f);
    DrawText("NEXT WAVE IN:", timerX, timerY + 25.0f, 1.0f, 1.0f, 0.8f, 0.8f);

    // Countdown digits turn orange when time is short
    char timerBuf[32];
    sprintf_s(timerBuf, "%.1f", timeRemaining);
    const f32 g = (timeRemaining > 2.0f) ? 1.0f : 0.3f;
    DrawText(timerBuf, timerX, timerY - 35.0f, 2.0f, 1.0f, g, 0.0f);
}

// ============================================================================
// DrawPowerUpScreen - SCREEN SPACE
// ============================================================================
// Full-screen overlay showing three power-up cards.
// Cards highlight on hover; click or press 1/2/3 to choose.
// ============================================================================
void GameUI::DrawPowerUpScreen()
{
    if (!powerUpSystem) return;

    PowerUp* choices = powerUpSystem->GetPowerUpChoices();
    const PlayerStats& stats = powerUpSystem->GetStats();

    // Semi-transparent full-screen dark overlay
    DrawRect(0.0f, 0.0f, 3000.0f, 3000.0f, 0.0f, 0.0f, 0.0f, 0.85f);
    DrawText("LEVEL UP!  Choose a Power-Up", 0.0f, 320.0f, 1.5f, 1.0f, 1.0f, 0.2f);

    // Three card positions
    const f32 boxX[3] = { -370.0f, 0.0f, 370.0f };
    const char* keys[3] = { "[1]", "[2]", "[3]" };
    const f32 boxW = 280.0f;
    const f32 boxH = 220.0f;

    const bool mouseClicked = AEInputCheckTriggered(AEVK_LBUTTON);
    Player* player = dynamic_cast<Player*>(pPlayer);

    for (int i = 0; i < 3; ++i)
    {
        const bool isHovered = IsMouseOverBox(boxX[i], 40.0f, boxW, boxH);

        // Card background (brighter on hover)
        const f32 bg = isHovered ? 0.18f : 0.12f;
        DrawRect(boxX[i], 40.0f, boxW, boxH, bg, bg, isHovered ? 0.25f : 0.18f, 0.95f);

        // Mouse-click selection
        if (isHovered && mouseClicked && player)
        {
            powerUpSystem->ApplyPowerUp(choices[i].type, player);
            return;
        }

        // Card content
        DrawText(keys[i], boxX[i], 140.0f, 1.0f, 1.0f, 0.7f, 0.0f);
        DrawText(choices[i].name, boxX[i], 80.0f, 0.7f, 0.2f, 1.0f, 1.0f);
        DrawText(choices[i].description, boxX[i], 30.0f, 0.5f, 0.9f, 0.9f, 0.9f);

        // Current stat value for context
        char statBuf[64] = "";
        if (choices[i].type == POWERUP_SPEED)
        {
            sprintf_s(statBuf, "Current: %.0f spd", stats.GetTotalSpeed());
            DrawText(statBuf, boxX[i], -35.0f, 0.7f, 0.55f, 0.55f, 0.55f);
        }
        else if (choices[i].type == POWERUP_BULLET_DAMAGE)
        {
            sprintf_s(statBuf, "Current: %.0f dmg", stats.GetTotalBulletDamage());
            DrawText(statBuf, boxX[i], -35.0f, 0.7f, 0.55f, 0.55f, 0.55f);
        }
        else if (choices[i].type == POWERUP_AOE_DAMAGE)
        {
            char rBuf[32]; sprintf_s(rBuf, "r=%.0f", stats.GetTotalAoeRadius());
            char dBuf[32]; sprintf_s(dBuf, "dmg=%.0f/s", stats.GetTotalAoeDamage());
            DrawText(rBuf, boxX[i], -35.0f, 0.7f, 0.55f, 0.55f, 0.55f);
            DrawText(dBuf, boxX[i], -60.0f, 0.7f, 0.55f, 0.55f, 0.55f);
        }
    }

    DrawText("Press 1, 2 or 3 to pick", 0.0f, -200.0f, 1.1f, 0.65f, 0.65f, 0.65f);
}