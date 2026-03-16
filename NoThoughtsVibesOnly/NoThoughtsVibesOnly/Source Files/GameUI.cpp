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
#include <iomanip>
#include "Collider.hpp"
#include <functional>
#include "Leaderboard.hpp"


bool isOverlayActive = false;
extern GameObject* pPlayer; // Declared in GamePage.cpp

// ============================================================================
// Animated Health Bar State
// ============================================================================
// s_DisplayedPlayerHP lerps toward the real HP value each frame, creating
// a smooth "damage drain" ghost bar effect behind the true green fill.
// Initialised to 100 to match the player's starting health.
// ============================================================================
static f32 s_DisplayedPlayerHP = 100.0f;
static f32 s_DisplayedPlayerMaxHP = 100.0f;
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
    const f32 dt = (f32)AEFrameRateControllerGetFrameTime();

    // -------------------------------------------------------------------------
    // Player health bar (animated fill)
    // -------------------------------------------------------------------------
    const Player* player = dynamic_cast<Player*>(pPlayer);
    if (player && player->isActive)
    {
        // Smoothly lerp displayed HP toward real HP each frame.
        // lerpSpeed of 6 gives a ~0.17s drain for a full-bar hit.
        s_DisplayedPlayerMaxHP = player->maxHealth;
        const f32 lerpSpeed = 6.0f;
        s_DisplayedPlayerHP += (player->health - s_DisplayedPlayerHP) * lerpSpeed * dt;

        const f32 barX = player->transform.position.x;
        const f32 barY = player->transform.position.y + 80.0f;
        const f32 barW = 60.0f;
        const f32 barH = 8.0f;
        const f32 trueRatio = AEClamp(player->health / player->maxHealth, 0.0f, 1.0f);
        const f32 displayRatio = AEClamp(s_DisplayedPlayerHP / s_DisplayedPlayerMaxHP, 0.0f, 1.0f);

        // 1. Red background (represents empty health)
        AEGfxSetColorToMultiply(1.0f, 0.0f, 0.0f, 1.0f);
        tf.SetPosition(barX, barY);
        tf.SetScale(barW, barH);
        tf.Apply();
        AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);

        // 2. Pale yellow ghost bar - shows the lagging displayed value.
        //    Only drawn when it has drifted ahead of the true value.
        if (displayRatio > trueRatio + 0.01f)
        {
            const f32 ghostW = barW * displayRatio;
            const f32 ghostX = barX - (barW / 2.0f) + (ghostW / 2.0f);
            AEGfxSetColorToMultiply(1.0f, 0.9f, 0.3f, 1.0f);
            tf.SetPosition(ghostX, barY);
            tf.SetScale(ghostW, barH);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }

        // 3. Green fill - true current health, anchored to the left edge.
        if (trueRatio > 0.01f)
        {
            const f32 greenW = barW * trueRatio;
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

    playerHealth = TextRenderer(gameFont, 1.0f, { -500.0f, 400.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    playerHealth << "HP: " << std::fixed << std::setprecision(0) << player->health << " / " << player->maxHealth;
    playerHealth.Draw();

    // ============================================================================
    // Ammo Pip Icons
    // ============================================================================
    // Draws one small square icon per bullet in the magazine instead of text.
    //   Yellow  = loaded round
    //   Grey    = empty slot (spent this magazine)
    //   Orange  = reloading (all pips pulse the same colour)
    // Capped at 30 visible pips so they never overflow the screen width.
    // ============================================================================
    if (powerUpSystem)
    {
        const int totalAmmo = (int)powerUpSystem->GetStats().bulletCount;
        const int currentAmmo = player->GetAmmoInMagazine();
        const bool reloading = player->IsReloading();

        // Clamp to a sane display max so pips don't overflow screen
        const int displayMax = (totalAmmo > 30) ? 30 : totalAmmo;

        const f32 pipSize = 8.0f;
        const f32 pipGap = 4.0f;
        const f32 pipStartX = -500.0f - (displayMax * (pipSize + pipGap)) / 2.0f;
        const f32 pipY = -430.0f;

        Transform tf;
        AEGfxSetRenderMode(AE_GFX_RM_COLOR);
        AEGfxSetBlendMode(AE_GFX_BM_BLEND);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        for (int i = 0; i < displayMax; ++i)
        {
            const f32 pipX = pipStartX + i * (pipSize + pipGap);
            bool filled = !reloading && (i < currentAmmo);

            if (reloading)
            {
                // Pips pulse orange during reload
                AEGfxSetColorToMultiply(1.0f, 0.45f, 0.0f, 0.6f);
            }
            else if (filled)
            {
                AEGfxSetColorToMultiply(1.0f, 0.9f, 0.0f, 1.0f); // Yellow
            }
            else
            {
                AEGfxSetColorToMultiply(0.3f, 0.3f, 0.3f, 0.8f); // Empty: grey
            }

            tf.SetPosition(pipX, pipY);
            tf.SetScale(pipSize, pipSize);
            tf.Apply();
            AEGfxMeshDraw(Meshes::pSquareCOriMesh, AE_GFX_MDM_TRIANGLES);
        }

        // Reset render state
        AEGfxSetColorToMultiply(1.0f, 1.0f, 1.0f, 1.0f);
        AEGfxSetColorToAdd(0.0f, 0.0f, 0.0f, 0.0f);

        // -------------------------------------------------------------------------
        // Reload Progress Bar
        // -------------------------------------------------------------------------
        // A thin bar just below the ammo pips. Only visible while reloading.
        // Fills left-to-right orange -> white as reload completes.
        // -------------------------------------------------------------------------
        if (reloading)
        {
            const f32 progress = player->GetReloadProgress(); // 0.0 -> 1.0
            const f32 barW = displayMax * (pipSize + pipGap);
            const f32 barH = 5.0f;
            const f32 barX = pipStartX + barW / 2.0f;
            const f32 barY = pipY - pipSize - 4.0f;

            // Dark background track
            DrawRect(barX, barY, barW, barH, 0.15f, 0.15f, 0.15f, 0.9f);

            // Animated fill: starts orange, turns white near completion
            if (progress > 0.01f)
            {
                const f32 fillW = barW * progress;
                const f32 fillX = barX - barW / 2.0f + fillW / 2.0f;
                const f32 g = 0.45f + progress * 0.55f; // orange -> white
                DrawRect(fillX, barY, fillW, barH - 1.0f, 1.0f, g, 0.0f, 1.0f);
            }

            // "RELOADING" label above the bar
            TextRenderer reloadLabel(gameFont, 0.55f, { barX, barY + 14.0f }, { 1.0f, 0.6f, 0.1f, 1.0f });
            reloadLabel << "RELOADING...";
            reloadLabel.Draw();
        }
    }
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
    playerXP = TextRenderer(gameFont, 0.8f, { barX, barY + 28.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    playerXP << "LVL " << stats.level << "   " << (int)stats.currentExp << " / " << (int)stats.expToNextLevel << " XP";
    playerXP.Draw();
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

    playerStats = TextRenderer(gameFont, 0.5f, { panelX, panelY + (panelH * 0.4f) }, { 0.6f, 0.8f, 0.8f, 0.8f });
    playerStats << "CURRENT STATS";
    playerStats.Draw();

    playerStats = TextRenderer(gameFont, 0.5f, { panelX, panelY }, { 1.0f, 1.0f, 1.0f, 1.0f });
    playerStats << "SPD: " << (float)stats.GetTotalSpeed() << "   DMG: " << (float)stats.GetTotalBulletDamage();
    playerStats.Draw();
    playerStats = TextRenderer(gameFont, 0.5f, { panelX, panelY - 30.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    playerStats << "AOE: r=" << (float)stats.GetTotalAoeRadius() << "  d=" << (float)stats.GetTotalAoeDamage() << "/s";
    playerStats.Draw();

    // -------------------------------------------------------------------------
    // Live score panel - shows current run score and best score below stats
    // -------------------------------------------------------------------------
    {
        const int wavesCleared = waveSystem ? waveSystem->GetCurrentWave() - 1 : 0;
        const int currentScore = wavesCleared * 100;

        const auto& entries = Leaderboard::GetEntries();
        const int bestScore = entries.empty() ? 0 : entries[0].score;

        const f32 hsX = panelX;
        const f32 hsY = panelY - panelH / 2.0f - 20.0f;
        const f32 hsW = panelW;
        const f32 hsH = 55.0f;

        DrawRect(hsX, hsY, hsW, hsH, 0.1f, 0.1f, 0.15f, 0.85f);

        // Current score in white
        TextRenderer scoreText(gameFont, 0.5f, { hsX, hsY + 12.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        scoreText << "SCORE: " << currentScore << " pts";
        scoreText.Draw();

        // Best score in yellow, turns green if player is beating it
        const bool beating = currentScore > bestScore && bestScore > 0;
        Colour bestCol = beating ? Colour{ 0.2f, 1.0f, 0.2f, 1.0f } : Colour{ 1.0f, 1.0f, 0.2f, 1.0f };
        TextRenderer bestText(gameFont, 0.5f, { hsX, hsY - 14.0f }, bestCol);
        bestText << "BEST:  " << bestScore << " pts" << (beating ? "  <NEW RECORD!>" : "");
        bestText.Draw();
    }
}

// ============================================================================
// DrawWaveInfo - SCREEN SPACE
// ============================================================================
// Shows current wave, round number, and active enemy count (top-right area).
// ============================================================================
void GameUI::DrawWaveInfo()
{
    if (!waveSystem) return;

    const f32 infoX = 500.0f;
    const f32 infoY = 400.0f;

    waveInfo = TextRenderer(gameFont, 1.0f, { infoX, infoY }, { 1.0f, 0.3f, 0.3f, 1.0f });
    waveInfo << "WAVE " << waveSystem->GetCurrentWave();
    waveInfo.Draw();

    waveRound = TextRenderer(gameFont, 1.0f, { infoX, infoY - 35.0f }, { 0.7f, 0.7f, 0.7f, 1.0f });
    waveRound << "Round " << waveSystem->GetCurrentRound();
    waveRound.Draw();

    // Count active NPCs for the enemy counter display
    int activeEnemies = 0;
    for (const GameObject* obj : gamePageObj)
        if (obj && obj->isActive && obj->ObjectType == NP)
            ++activeEnemies;

    activeEnemyCount = TextRenderer(gameFont, 1.0f, { infoX, infoY - 70.0f }, { 1.0f, 1.0f, 0.3f, 1.0f });
    activeEnemyCount << activeEnemies << " Enemies";
    activeEnemyCount.Draw();
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

    waveTimer = TextRenderer(gameFont, 1.0f, { timerX, timerY + 25.0f }, { 1.0f, 1.0f, 0.8f, 0.8f });
    waveTimer << " NEXT WAVE IN";
    waveTimer.Draw();

    // Countdown digits turn orange when time is short
    const f32 g = (timeRemaining > 2.0f) ? 1.0f : 0.3f;
    waveTimer = TextRenderer(gameFont, 1.0f, { timerX, timerY - 35.0f }, { 1.0f, g, 0.0f, 0.8f });
    waveTimer.SetMaxPixelWidth(395.0f);
    waveTimer << std::fixed << std::setprecision(1) << timeRemaining;
    waveTimer.Draw();
}

// ============================================================================
// DrawAbilities - SCREEN SPACE
// ============================================================================
void GameUI::DrawAbilities()
{
    Player* player = dynamic_cast<Player*>(pPlayer);
    if (!player) return;

    // Positioned at the bottom center-ish
    const f32 boxX = 0.0f;
    const f32 boxY = -350.0f;
    const f32 boxW = 80.0f;
    const f32 boxH = 80.0f;

    const Ability& ability = player->invulnAbility;

    // 1. Draw Background Box based on state
    if (ability.IsActive())
    {
        // Gold / Yellow when active
        DrawRect(boxX, boxY, boxW, boxH, 1.0f, 0.8f, 0.0f, 1.0f);
    }
    else if (ability.IsOnCooldown())
    {
        // Dark gray and faded when on cooldown
        DrawRect(boxX, boxY, boxW, boxH, 0.3f, 0.3f, 0.3f, 0.8f);

        // Draw the countdown timer
        abilityTimerText = TextRenderer(gameFont, 0.8f, { boxX, boxY }, { 1.0f, 1.0f, 1.0f, 1.0f });
        abilityTimerText << (int)ceil(ability.GetCooldownRemaining()) << "s";
        abilityTimerText.Draw();
    }
    else
    {
        // Bright Blue when ready to use
        DrawRect(boxX, boxY, boxW, boxH, 0.2f, 0.6f, 1.0f, 0.9f);
    }

    // 2. Draw Label
    abilityKeyText = TextRenderer(gameFont, 0.5f, { boxX, boxY - 60.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
    abilityKeyText << "[E] INVULN";
    abilityKeyText.Draw();
}

// ============================================================================
// DrawPowerUpScreen - SCREEN SPACE
// ============================================================================
// Full-screen overlay showing three power-up cards using Buttons.
// Buttons handle hover and click automatically.
// ============================================================================

void GameUI::DrawPowerUpScreen()
{
    if (!powerUpSystem || !powerUpSystem->IsWaitingForUpgrade())
        return;

    isOverlayActive = true;

    const PowerUp* choices = powerUpSystem->GetPowerUpChoices();
    const PlayerStats& stats = powerUpSystem->GetStats();
    Player* player = dynamic_cast<Player*>(pPlayer);

    // Dark overlay
    DrawRect(0.0f, 0.0f, 3000.0f, 3000.0f, 0.0f, 0.0f, 0.0f, 0.85f);

    powerUpTitle = TextRenderer(gameFont, 1.0f, { 0.0f, 320.0f }, { 1.0f, 1.0f, 0.2f, 1.0f });
    powerUpTitle << "LEVEL UP!  Choose a Power-Up";
    powerUpTitle.Draw();

    // 4 cards evenly spaced across the screen (expanded from 3 to include Lifesteal)
    const f32  boxX[4] = { -555.0f, -185.0f, 185.0f, 555.0f };
    const char* keys[4] = { "[1]",   "[2]",   "[3]",  "[4]" };
    const f32  boxW = 280.0f;
    const f32  boxH = 220.0f;
    const f32  boxY = 40.0f;

    const bool mouseClicked = AEInputCheckTriggered(AEVK_LBUTTON);

    for (int i = 0; i < 4; ++i) // Iterate all 4 power-up choices
    {
        SquareCollider cardCollider{}; // zero-initializes all members
        cardCollider.position = { boxX[i], boxY };
        cardCollider.scale = { boxW, boxH };

        Mouse mouse{};
        GetMouseWorldPosition(mouse.position.x, mouse.position.y);

        const bool isHovered = isOverlapping(cardCollider, mouse);

        // Draw card background 
        const f32 bg = isHovered ? 0.22f : 0.12f;
        const f32 bb = isHovered ? 0.30f : 0.18f;
        DrawRect(boxX[i], boxY, boxW, boxH, bg, bg, bb, 0.95f);

        // Highlight border on hover
        if (isHovered)
            DrawRect(boxX[i], boxY, boxW + 4.0f, boxH + 4.0f, 0.4f, 0.8f, 1.0f, 0.5f);

        //Apply upgrade on mouse click (was never hooked up before)
        if (isHovered && mouseClicked && player)
        {
            powerUpSystem->ApplyPowerUp(choices[i].type, player);
            // Suppress shooting for one frame so this click doesn't also fire a bullet
            player->suppressShootOneFrame = true;
            isOverlayActive = false;
            return; // overlay gone, stop drawing
        }

        // Card title: key + upgrade name
        TextRenderer cardTitle = TextRenderer(gameFont, 0.65f, { boxX[i], boxY + 80.0f }, { 1.0f, 1.0f, 0.4f, 1.0f });
        cardTitle.SetMaxPixelWidth(boxW);
        cardTitle << keys[i] << " " << choices[i].name;
        cardTitle.Draw();

        // Description
        descriptionText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY + 20.0f }, { 1.0f, 1.0f, 1.0f, 1.0f });
        descriptionText.SetMaxPixelWidth(boxW);
        descriptionText << choices[i].description;
        descriptionText.Draw();

        // Current stat context
        if (choices[i].type == POWERUP_SPEED)
        {
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 55.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "Current: " << (float)stats.GetTotalSpeed() << " spd";
            statBuffText.Draw();
        }
        else if (choices[i].type == POWERUP_BULLET_DAMAGE)
        {
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 40.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "Current: " << (float)stats.GetTotalBulletDamage() << " dmg";
            statBuffText.Draw();
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 60.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "Bullets: " << stats.bulletCount << "  ->  " << stats.bulletCount + 5;
            statBuffText.Draw();
        }
        else if (choices[i].type == POWERUP_AOE_DAMAGE)
        {
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 25.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "Current:";
            statBuffText.Draw();
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 45.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "r=" << (float)stats.GetTotalAoeRadius();
            statBuffText.Draw();
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 65.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "dmg=" << (float)stats.GetTotalAoeDamage() << "/s";
            statBuffText.Draw();
        }
        else if (choices[i].type == POWERUP_LIFESTEAL)
        {
            // Shows total heal per kill: base 5 HP + accumulated lifestealBonus
            statBuffText = TextRenderer(gameFont, 0.5f, { boxX[i], boxY - 40.0f }, { 0.55f, 0.55f, 0.55f, 0.8f });
            statBuffText << "Current: +" << (5.0f + stats.lifestealBonus) << " HP/kill";
            statBuffText.Draw();
        }
    }

    powerUpTitle = TextRenderer(gameFont, 0.8f, { 0.0f, -200.0f }, { 0.65f, 0.65f, 0.65f, 1.0f });
    powerUpTitle << "Click a card  or  Press 1, 2, 3, 4";
    powerUpTitle.Draw();
}