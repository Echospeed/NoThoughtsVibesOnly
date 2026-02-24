#include "pch.hpp"
#include "GameUI.hpp"
#include "Player.hpp"
#include "NPC.hpp"
#include "WaveSystem.hpp"
#include "PowerUpSystem.hpp"
#include "GamePage.hpp"     // for extern gamePageObj, pPlayer
#include "TextRenderer.hpp"
extern GameObject* pPlayer;
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
// DrawRect - Helper
// ============================================================================
void GameUI::DrawRect(f32 x, f32 y, f32 w, f32 h, f32 r, f32 g, f32 b, f32 a)
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetColorToMultiply(r, g, b, a);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEMtx33 tf, sc, tr;
    AEMtx33Scale(&sc, w, h);
    AEMtx33Trans(&tr, x, y);
    AEMtx33Concat(&tf, &tr, &sc);
    AEGfxSetTransform(tf.m);
    AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
}

// ============================================================================
// DrawText - Helper
// ============================================================================
void GameUI::DrawText(const char* text, f32 x, f32 y, f32 scale, f32 r, f32 g, f32 b)
{
    TextRenderer t;
    LoadTextRenderer(t, gameFont);
    InitTextRenderer(t, text, {scale,scale},r, g, b);
    DrawTextRenderer(t, { x, y }, scale);
    FreeTextRenderer(t);
}

// ============================================================================
//   NEW: IsMouseOverBox - Check if mouse is inside a box
// ============================================================================
bool GameUI::IsMouseOverBox(f32 boxX, f32 boxY, f32 boxW, f32 boxH)
{
    s32 mouseX, mouseY;
    AEInputGetCursorPosition(&mouseX, &mouseY);

    // Convert screen coords to world coords (assuming 0,0 center, 1600x900 window)
    f32 worldX = (mouseX - 800.0f);   // Center X
    f32 worldY = (450.0f - mouseY);   // Flip Y (screen Y goes down, world Y goes up)

    // Check if inside box (box is center-origin)
    f32 halfW = boxW / 2.0f;
    f32 halfH = boxH / 2.0f;

    return (worldX >= boxX - halfW && worldX <= boxX + halfW &&
        worldY >= boxY - halfH && worldY <= boxY + halfH);
}

// ============================================================================
// DrawAllHealthBars - WORLD SPACE, call BEFORE camera reset
// ============================================================================
void GameUI::DrawAllHealthBars()
{
    AEGfxSetRenderMode(AE_GFX_RM_COLOR);
    AEGfxSetBlendMode(AE_GFX_BM_NONE);
    AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

    AEMtx33 transform, scale, trans;

    // === Player health bar ===
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (player && player->isActive)
    {
        f32 barX = player->transform.position.x;
        f32 barY = player->transform.position.y + 80.0f;
        f32 barWidth = 60.0f;
        f32 barHeight = 8.0f;
        f32 ratio = player->health / player->maxHealth;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        // Red background
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
        AEMtx33Scale(&scale, barWidth, barHeight);
        AEMtx33Trans(&trans, barX, barY);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // Green fill
        if (ratio > 0.01f)
        {
            f32 greenWidth = barWidth * ratio;
            f32 greenX = barX - (barWidth / 2.0f) + (greenWidth / 2.0f);
            AEGfxSetColorToMultiply(0.0f, 1.0f, 0.0f, 1.0f);
            AEMtx33Scale(&scale, greenWidth, barHeight);
            AEMtx33Trans(&trans, greenX, barY);
            AEMtx33Concat(&transform, &trans, &scale);
            AEGfxSetTransform(transform.m);
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    // === Enemy health bars ===
    for (GameObject* obj : gamePageObj)
    {
        if (obj->ObjectType != NP || !obj->isActive) continue;
        NPC* npc = dynamic_cast<NPC*>(obj);
        if (!npc || !npc->isVisibleToPlayer) continue;

        f32 barX = npc->transform.position.x;
        f32 barY = npc->transform.position.y + 40.0f;
        f32 barWidth = 30.0f;
        f32 barHeight = 4.0f;
        f32 ratio = npc->health / 100.0f;
        if (ratio < 0.0f) ratio = 0.0f;
        if (ratio > 1.0f) ratio = 1.0f;

        // Red background
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
        AEMtx33Scale(&scale, barWidth, barHeight);
        AEMtx33Trans(&trans, barX, barY);
        AEMtx33Concat(&transform, &trans, &scale);
        AEGfxSetTransform(transform.m);
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // Orange fill
        if (ratio > 0.01f)
        {
            f32 fillWidth = barWidth * ratio;
            f32 fillX = barX - (barWidth / 2.0f) + (fillWidth / 2.0f);
            AEGfxSetColorToMultiply(1.0f, 0.5f, 0.0f, 1.0f);
            AEMtx33Scale(&scale, fillWidth, barHeight);
            AEMtx33Trans(&trans, fillX, barY);
            AEMtx33Concat(&transform, &trans, &scale);
            AEGfxSetTransform(transform.m);
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }
    }

    AEGfxSetBlendMode(AE_GFX_BM_BLEND);
}

// ============================================================================
// DrawHealthText - SCREEN SPACE, call AFTER camera reset
// ============================================================================
void GameUI::DrawHealthText()
{
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (!player) return;

    char buf[64];
    TextRenderer playerHealth;
    sprintf_s(buf, "HP: %.0f / %.0f", player->health, player->maxHealth);
    DrawText(buf, -500.0f, -400.0f, 1.2f, 1.0f, 1.0f, 1.0f);
}

// ============================================================================
// DrawXPBar - SCREEN SPACE
// ============================================================================
void GameUI::DrawXPBar()
{
    if (!powerUpSystem) return;

    const PlayerStats& stats = powerUpSystem->GetStats();

    f32 barX = -500.0f;
    f32 barY = 300.0f;
    f32 barW = 250.0f;
    f32 barH = 20.0f;

    // Dark background
    DrawRect(barX, barY, barW, barH, 0.2f, 0.2f, 0.2f, 0.9f);

    // Green XP fill
    f32 fill = (stats.expToNextLevel > 0.0f) ? stats.currentExp / stats.expToNextLevel : 0.0f;
    if (fill > 0.01f)
    {
        f32 fw = barW * fill;
        f32 fx = barX - (barW / 2.0f) + (fw / 2.0f);
        DrawRect(fx, barY, fw, barH - 4.0f, 0.0f, 0.85f, 0.2f, 0.95f);
    }

    // Label: "LVL 2   45 / 130 XP"
    char buf[64];
    sprintf_s(buf, "LVL %u   %d / %d XP", stats.level, (int)stats.currentExp, (int)stats.expToNextLevel);
    DrawText(buf, barX, barY + 28.0f, 0.9f, 1.0f, 1.0f, 0.3f);
}

// ============================================================================
// DrawCurrentStats - SCREEN SPACE, top-center (same spot as wave timer)
// Only shows when NOT in a wave break (timer takes priority)
// ============================================================================
void GameUI::DrawCurrentStats()
{
    if (!powerUpSystem) return;
    if (waveSystem && waveSystem->IsInBreak()) return; // hide when timer is showing

    const PlayerStats& stats = powerUpSystem->GetStats();

    // Top-center position (same as wave timer)
    f32 panelX = 0.0f;
    f32 panelY = 380.0f;
    f32 panelW = 395.0f;
    f32 panelH = 120.0f;

    // Background box (matches wave timer style)
    DrawRect(panelX, panelY, panelW, panelH, 0.1f, 0.1f, 0.15f, 0.85f);

    // Header
    DrawText("CURRENT STATS", panelX, panelY + 38.0f, 0.6f, 0.8f, 0.8f, 0.8f);

    // Speed
    char spd[32];
    sprintf_s(spd, "SPD: %.0f", stats.GetTotalSpeed());
    DrawText(spd, panelX - 100.0f, panelY + 8.0f, 0.5f, 0.4f, 0.8f, 1.0f);   // Blue

    // Bullet damage
    char dmg[32];
    sprintf_s(dmg, "DMG: %.0f", stats.GetTotalBulletDamage());
    DrawText(dmg, panelX + 100.0f, panelY + 8.0f, 0.5f, 1.0f, 0.5f, 0.2f);    // Orange

    // AoE
    char aoe[48];
    sprintf_s(aoe, "AOE: r=%.0f  d=%.0f/s", stats.GetTotalAoeRadius(), stats.GetTotalAoeDamage());
    DrawText(aoe, panelX, panelY - 22.0f, 0.5f, 0.4f, 1.0f, 0.4f);           // Green
}

// ============================================================================
// DrawWaveInfo - SCREEN SPACE
// ============================================================================
void GameUI::DrawWaveInfo()
{
    if (!waveSystem) return;

    f32 infoX = 400.0f;
    f32 infoY = 400.0f;

    char waveBuffer[64];
    sprintf_s(waveBuffer, "WAVE %u", waveSystem->GetCurrentWave());
    DrawText(waveBuffer, infoX, infoY, 1.3f, 1.0f, 0.3f, 0.3f);

    char roundBuffer[64];
    sprintf_s(roundBuffer, "Round %u", waveSystem->GetCurrentRound());
    DrawText(roundBuffer, infoX, infoY - 35.0f, 0.9f, 0.7f, 0.7f, 0.7f);

    int activeEnemies = 0;
    for (GameObject* obj : gamePageObj)
        if (obj && obj->isActive && obj->ObjectType == NP)
            activeEnemies++;

    char enemyBuffer[64];
    sprintf_s(enemyBuffer, "%d Enemies", activeEnemies);
    DrawText(enemyBuffer, infoX, infoY - 70.0f, 0.9f, 1.0f, 1.0f, 0.3f);
}

// ============================================================================
// DrawWaveTimer - SCREEN SPACE
// ============================================================================
void GameUI::DrawWaveTimer()
{
    if (!waveSystem || !waveSystem->IsInBreak()) return;

    f32 timeRemaining = waveSystem->GetBreakTimeRemaining();
    f32 timerX = 0.0f;
    f32 timerY = 400.0f;
    f32 boxW = 395.0f;
    f32 boxH = 120.0f;

    // Background box
    DrawRect(timerX, timerY, boxW, boxH, 0.1f, 0.1f, 0.15f, 0.85f);

    // Label
    DrawText("NEXT WAVE IN:", timerX, timerY + 25.0f, 1.0f, 1.0f, 0.8f, 0.8f);

    // Countdown (yellow → orange when < 2s)
    char timerBuffer[32];
    sprintf_s(timerBuffer, "%.1f", timeRemaining);
    f32 g = timeRemaining > 2.0f ? 1.0f : 0.3f;
    DrawText(timerBuffer, timerX, timerY - 35.0f, 2.0f, 1.0f, g, 0.0f);
}

// ============================================================================
// DrawPowerUpScreen - SCREEN SPACE, full overlay
// ============================================================================
void GameUI::DrawPowerUpScreen()
{
    if (!powerUpSystem) return;

    PowerUp* choices = powerUpSystem->GetPowerUpChoices();
    const PlayerStats& stats = powerUpSystem->GetStats();

    // Dark overlay
    DrawRect(0.0f, 0.0f, 3000.0f, 3000.0f, 0.0f, 0.0f, 0.0f, 0.85f);

    // Title
    DrawText("LEVEL UP!  Choose a Power-Up", 0.0f, 320.0f, 1.5f, 1.0f, 1.0f, 0.2f);

    f32         boxX[3] = { -370.0f, 0.0f, 370.0f };
    const char* keys[3] = { "[1]",   "[2]", "[3]" };
    f32 boxW = 280.0f, boxH = 220.0f;

    //   CHECK FOR MOUSE CLICKS
    bool mouseClicked = AEInputCheckTriggered(AEVK_LBUTTON);
    Player* player = dynamic_cast<Player*>(pPlayer);

    for (int i = 0; i < 3; ++i)
    {
        // Box background
        //DrawRect(boxX[i], 40.0f, 280.0f, 220.0f, 0.12f, 0.12f, 0.18f, 0.95f);
        bool isHovered = IsMouseOverBox(boxX[i], 40.0f, boxW, boxH);

        //   HIGHLIGHT ON HOVER
        f32 boxR = isHovered ? 0.18f : 0.12f;
        f32 boxG = isHovered ? 0.18f : 0.12f;
        f32 boxB = isHovered ? 0.25f : 0.18f;

        DrawRect(boxX[i], 40.0f, boxW, boxH, boxR, boxG, boxB, 0.95f);

        // HANDLE CLICK
        if (isHovered && mouseClicked && player)
        {
            powerUpSystem->ApplyPowerUp(choices[i].type, player);
            return; // Exit after selection
        }

        // Key prompt
        DrawText(keys[i], boxX[i], 140.0f, 1.0f, 1.0f, 0.7f, 0.0f);

        // Power-up name
        DrawText(choices[i].name, boxX[i], 80.0f, 0.7f, 0.2f, 1.0f, 1.0f);

        // Description
        DrawText(choices[i].description, boxX[i], 30.0f, 0.5f, 0.9f, 0.9f, 0.9f);

        // Current stat
        char statBuf[64] = "";
        if (choices[i].type == POWERUP_SPEED)
            sprintf_s(statBuf, "Current: %.0f spd", stats.GetTotalSpeed());
        else if (choices[i].type == POWERUP_BULLET_DAMAGE)
            sprintf_s(statBuf, "Current: %.0f dmg", stats.GetTotalBulletDamage());
        else if (choices[i].type == POWERUP_AOE_DAMAGE)
        {
            // Draw radius on first line, damage on second line
            char radiusBuf[32];
            sprintf_s(radiusBuf, "r=%.0f", stats.GetTotalAoeRadius());
            DrawText(radiusBuf, boxX[i], -35.0f, 0.7f, 0.55f, 0.55f, 0.55f);

            char damageBuf[32];
            sprintf_s(damageBuf, "dmg=%.0f/s", stats.GetTotalAoeDamage());
            DrawText(damageBuf, boxX[i], -60.0f, 0.7f, 0.55f, 0.55f, 0.55f);
            continue; // skip the DrawText below since we already drew it
        }

        DrawText(statBuf, boxX[i], -35.0f, 0.7f, 0.55f, 0.55f, 0.55f);
    }

    // Footer
    DrawText("Press 1, 2 or 3 to pick", 0.0f, -200.0f, 1.1f, 0.65f, 0.65f, 0.65f);
}