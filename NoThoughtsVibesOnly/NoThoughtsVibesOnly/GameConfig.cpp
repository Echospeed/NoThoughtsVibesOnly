// ============================================================================
// GameConfig.cpp - Game Configuration Loader Implementation
// ============================================================================
// Defines the static storage and Load() function declared in GameConfig.hpp.
// Split from the header so inline variables are not needed (C++14 compatible).
//
// ADDED vs original:
//   gameplay_config.json loader now also reads:
//     playerBulletPoolSize, enemyBulletPoolSize,
//     scorePerWaveNormal,   scorePerWaveEndless,
//     invulnDuration,       invulnCooldown
// ============================================================================

#include "pch.hpp"
#include "GameConfig.hpp"

// ============================================================================
// Internal helpers
// ============================================================================
namespace
{
    // Open a JSON file and parse it into doc.
    bool OpenDoc(const std::string& path, rapidjson::Document& doc)
    {
        std::ifstream ifs(path);
        if (!ifs.is_open())
        {
            //std::cout << "[GameConfig] WARNING: '" << path << "' not found - using defaults.\n";
            return false;
        }
        rapidjson::IStreamWrapper isw(ifs);
        doc.ParseStream(isw);
        if (doc.HasParseError())
        {
            //std::cout << "[GameConfig] WARNING: Parse error in '" << path << "' - using defaults.\n";
            return false;
        }
        return true;
    }

    float GetF(const rapidjson::Value& v, const char* key, float def)
    {
        return (v.HasMember(key) && v[key].IsNumber()) ? v[key].GetFloat() : def;
    }
    int GetI(const rapidjson::Value& v, const char* key, int def)
    {
        return (v.HasMember(key) && v[key].IsInt()) ? v[key].GetInt() : def;
    }
    std::string GetS(const rapidjson::Value& v, const char* key, const std::string& def)
    {
        return (v.HasMember(key) && v[key].IsString()) ? v[key].GetString() : def;
    }

    void ReadNpcType(const rapidjson::Value& v, NpcTypeConfig& out)
    {
        out.health = GetF(v, "health", out.health);
        out.speed = GetF(v, "speed", out.speed);
        out.scaleX = GetF(v, "scaleX", out.scaleX);
        out.scaleY = GetF(v, "scaleY", out.scaleY);
        out.fireRate = GetF(v, "fireRate", out.fireRate);
        out.retreatDistance = GetF(v, "retreatDistance", out.retreatDistance);
        out.velocityDamping = GetF(v, "velocityDamping", out.velocityDamping);
        out.changeDirInterval = GetF(v, "changeDirInterval", out.changeDirInterval);
        out.orbitDistance = GetF(v, "orbitDistance", out.orbitDistance);
        out.orbitMargin = GetF(v, "orbitMargin", out.orbitMargin);
        out.explosionDamage = GetF(v, "explosionDamage", out.explosionDamage);
        out.explosionBurst = GetI(v, "explosionBurstCount", out.explosionBurst);
        out.bulletVolleyCount = GetI(v, "bulletVolleyCount", out.bulletVolleyCount);
        out.burstCount = GetI(v, "burstCount", out.burstCount);
        out.xpReward = GetF(v, "xpReward", out.xpReward);
        out.baseHeal = GetF(v, "baseHeal", out.baseHeal);
        out.colourR = GetF(v, "colourR", out.colourR);
        out.colourG = GetF(v, "colourG", out.colourG);
        out.colourB = GetF(v, "colourB", out.colourB);
        out.texture = GetS(v, "texture", out.texture);

        // bulletsAssigned uses different JSON keys for ranger vs boss
        if (v.HasMember("bulletsPerRanger") && v["bulletsPerRanger"].IsInt())
            out.bulletsAssigned = v["bulletsPerRanger"].GetInt();
        if (v.HasMember("bulletsPerBoss") && v["bulletsPerBoss"].IsInt())
            out.bulletsAssigned = v["bulletsPerBoss"].GetInt();
    }

} // anonymous namespace

// ============================================================================
// Static storage
// ============================================================================
static PlayerConfig    s_Player;
static NpcConfig       s_Npc;
static PowerUpConfig   s_PowerUp;
static WaveSpawnConfig s_Wave;
static GameplayConfig  s_Gameplay;
static bool            s_Loaded = false;

// ============================================================================
// GameConfig::Load
// ============================================================================
void GameConfig::Load()
{
    rapidjson::Document doc;

    // ---- player_config.json ----
    if (OpenDoc("Assets/player_config.json", doc))
    {
        s_Player.startHealth = GetF(doc, "startHealth", s_Player.startHealth);
        s_Player.maxHealth = GetF(doc, "maxHealth", s_Player.maxHealth);
        s_Player.baseSpeed = GetF(doc, "baseSpeed", s_Player.baseSpeed);
        s_Player.rotationSpeed = GetF(doc, "rotationSpeed", s_Player.rotationSpeed);
        s_Player.scaleX = GetF(doc, "scaleX", s_Player.scaleX);
        s_Player.scaleY = GetF(doc, "scaleY", s_Player.scaleY);
        s_Player.shootCooldown = GetF(doc, "shootCooldown", s_Player.shootCooldown);
        s_Player.shootSuppressCooldown = GetF(doc, "shootSuppressCooldown", s_Player.shootSuppressCooldown);
        s_Player.reloadDuration = GetF(doc, "reloadDuration", s_Player.reloadDuration);
        s_Player.startingAmmo = GetI(doc, "startingAmmo", s_Player.startingAmmo);
        s_Player.bulletLifeTime = GetF(doc, "bulletLifeTime", s_Player.bulletLifeTime);
        s_Player.bulletSpeed = GetF(doc, "bulletSpeed", s_Player.bulletSpeed);
        s_Player.spawnClearRadius = GetF(doc, "spawnClearRadius", s_Player.spawnClearRadius);
        s_Player.texture = GetS(doc, "texture", s_Player.texture);
    }

    // ---- npc_config.json ----
    if (OpenDoc("Assets/npc_config.json", doc))
    {
        // Set boss/ranger-specific defaults before reading
        s_Npc.boss.health = 1000.0f;
        s_Npc.boss.scaleX = 150.0f;
        s_Npc.boss.scaleY = 150.0f;
        s_Npc.boss.speed = 150.0f;
        s_Npc.boss.fireRate = 0.5f;
        s_Npc.boss.bulletVolleyCount = 8;
        s_Npc.boss.bulletsAssigned = 24;
        s_Npc.boss.burstCount = 40;
        s_Npc.boss.baseHeal = 15.0f;
        s_Npc.ranger.bulletsAssigned = 6;

        if (doc.HasMember("walk") && doc["walk"].IsObject())   ReadNpcType(doc["walk"], s_Npc.walk);
        if (doc.HasMember("melee") && doc["melee"].IsObject())  ReadNpcType(doc["melee"], s_Npc.melee);
        if (doc.HasMember("ranger") && doc["ranger"].IsObject()) ReadNpcType(doc["ranger"], s_Npc.ranger);
        if (doc.HasMember("boss") && doc["boss"].IsObject())   ReadNpcType(doc["boss"], s_Npc.boss);
    }

    // ---- powerup_config.json ----
    if (OpenDoc("Assets/powerup_config.json", doc))
    {
        s_PowerUp.baseSpeed = GetF(doc, "baseSpeed", s_PowerUp.baseSpeed);
        s_PowerUp.baseBulletDamage = GetF(doc, "baseBulletDamage", s_PowerUp.baseBulletDamage);
        s_PowerUp.baseAoeRadius = GetF(doc, "baseAoeRadius", s_PowerUp.baseAoeRadius);
        s_PowerUp.baseAoeDamage = GetF(doc, "baseAoeDamage", s_PowerUp.baseAoeDamage);
        s_PowerUp.startingBulletCount = GetI(doc, "startingBulletCount", s_PowerUp.startingBulletCount);
        s_PowerUp.expToFirstLevel = GetF(doc, "expToFirstLevel", s_PowerUp.expToFirstLevel);
        s_PowerUp.expScalePerLevel = GetF(doc, "expScalePerLevel", s_PowerUp.expScalePerLevel);
        s_PowerUp.speedUpgradeBonus = GetF(doc, "speedUpgradeBonus", s_PowerUp.speedUpgradeBonus);
        s_PowerUp.bulletDamageBonus = GetF(doc, "bulletDamageBonus", s_PowerUp.bulletDamageBonus);
        s_PowerUp.bulletCountBonus = GetI(doc, "bulletCountBonus", s_PowerUp.bulletCountBonus);
        s_PowerUp.aoeRadiusBonus = GetF(doc, "aoeRadiusBonus", s_PowerUp.aoeRadiusBonus);
        s_PowerUp.aoeDamageBonus = GetF(doc, "aoeDamageBonus", s_PowerUp.aoeDamageBonus);
        s_PowerUp.lifestealBonus = GetF(doc, "lifestealBonus", s_PowerUp.lifestealBonus);
        s_PowerUp.lifestealBaseHealPerKill = GetF(doc, "lifestealBaseHealPerKill", s_PowerUp.lifestealBaseHealPerKill);

        if (doc.HasMember("enemyScaling") && doc["enemyScaling"].IsObject())
        {
            const rapidjson::Value& es = doc["enemyScaling"];
            EnemyScalingConfig& out = s_PowerUp.enemyScaling;
            out.rangerBaseDamage = GetF(es, "rangerBaseDamage", out.rangerBaseDamage);
            out.rangerDamagePerRound = GetF(es, "rangerDamagePerRound", out.rangerDamagePerRound);
            out.meleeBaseSpeed = GetF(es, "meleeBaseSpeed", out.meleeBaseSpeed);
            out.meleeSpeedPerRound = GetF(es, "meleeSpeedPerRound", out.meleeSpeedPerRound);
            out.meleeSpeedCap = GetF(es, "meleeSpeedCap", out.meleeSpeedCap);
            out.meleeBaseDamage = GetF(es, "meleeBaseDamage", out.meleeBaseDamage);
            out.meleeDamagePerRound = GetF(es, "meleeDamagePerRound", out.meleeDamagePerRound);
        }
    }

    // ---- wave_config.json ----
    if (OpenDoc("Assets/wave_config.json", doc))
    {
        s_Wave.waveBreakDuration = GetF(doc, "waveBreakDuration", s_Wave.waveBreakDuration);
        s_Wave.baseEnemyCount = GetI(doc, "baseEnemyCount", s_Wave.baseEnemyCount);
        s_Wave.enemyCountPerRound = GetI(doc, "enemyCountPerRound", s_Wave.enemyCountPerRound);
        s_Wave.maxEnemyCount = GetI(doc, "maxEnemyCount", s_Wave.maxEnemyCount);
        s_Wave.earlyRoundThreshold = GetI(doc, "earlyRoundThreshold", s_Wave.earlyRoundThreshold);
        s_Wave.midRoundThreshold = GetI(doc, "midRoundThreshold", s_Wave.midRoundThreshold);
        s_Wave.midWalkerDivisor = GetI(doc, "midWalkerDivisor", s_Wave.midWalkerDivisor);
        s_Wave.midMeleeDivisor = GetI(doc, "midMeleeDivisor", s_Wave.midMeleeDivisor);
        s_Wave.midRangerDivisor = GetI(doc, "midRangerDivisor", s_Wave.midRangerDivisor);
        s_Wave.lateWalkerDivisor = GetI(doc, "lateWalkerDivisor", s_Wave.lateWalkerDivisor);
        s_Wave.lateMeleeDivisor = GetI(doc, "lateMeleeDivisor", s_Wave.lateMeleeDivisor);
        s_Wave.lateRangerDivisor = GetI(doc, "lateRangerDivisor", s_Wave.lateRangerDivisor);
        s_Wave.expRewardBase = GetF(doc, "expRewardBase", s_Wave.expRewardBase);
        s_Wave.expRewardPerRound = GetF(doc, "expRewardPerRound", s_Wave.expRewardPerRound);

        if (doc.HasMember("contactDamage") && doc["contactDamage"].IsObject())
        {
            const rapidjson::Value& cd = doc["contactDamage"];
            s_Wave.contactDamage.melee = GetF(cd, "melee", s_Wave.contactDamage.melee);
            s_Wave.contactDamage.boss = GetF(cd, "boss", s_Wave.contactDamage.boss);
            s_Wave.contactDamage.walkRanger = GetF(cd, "walkRanger", s_Wave.contactDamage.walkRanger);
        }
        if (doc.HasMember("knockbackForce") && doc["knockbackForce"].IsObject())
        {
            const rapidjson::Value& kb = doc["knockbackForce"];
            s_Wave.knockback.melee = GetF(kb, "melee", s_Wave.knockback.melee);
            s_Wave.knockback.boss = GetF(kb, "boss", s_Wave.knockback.boss);
            s_Wave.knockback.walkRanger = GetF(kb, "walkRanger", s_Wave.knockback.walkRanger);
        }
    }

    // ---- gameplay_config.json ----
    if (OpenDoc("Assets/gameplay_config.json", doc))
    {
        if (doc.HasMember("minimap") && doc["minimap"].IsObject())
        {
            const rapidjson::Value& mm = doc["minimap"];
            s_Gameplay.minimap.worldRefWidth = GetF(mm, "worldRefWidth", s_Gameplay.minimap.worldRefWidth);
            s_Gameplay.minimap.size = GetF(mm, "size", s_Gameplay.minimap.size);
            s_Gameplay.minimap.screenOffsetX = GetF(mm, "screenOffsetX", s_Gameplay.minimap.screenOffsetX);
            s_Gameplay.minimap.screenOffsetY = GetF(mm, "screenOffsetY", s_Gameplay.minimap.screenOffsetY);
            s_Gameplay.minimap.playerDotSize = GetF(mm, "playerDotSize", s_Gameplay.minimap.playerDotSize);
            s_Gameplay.minimap.npcDotSize = GetF(mm, "npcDotSize", s_Gameplay.minimap.npcDotSize);
        }
        if (doc.HasMember("world") && doc["world"].IsObject())
        {
            const rapidjson::Value& w = doc["world"];
            s_Gameplay.world.width = GetF(w, "width", s_Gameplay.world.width);
            s_Gameplay.world.height = GetF(w, "height", s_Gameplay.world.height);
        }
        if (doc.HasMember("bullet") && doc["bullet"].IsObject())
        {
            const rapidjson::Value& b = doc["bullet"];
            s_Gameplay.bullet.playerSpeed = GetF(b, "playerSpeed", s_Gameplay.bullet.playerSpeed);
            s_Gameplay.bullet.enemySpeed = GetF(b, "enemySpeed", s_Gameplay.bullet.enemySpeed);
            s_Gameplay.bullet.maxLifeTime = GetF(b, "maxLifeTime", s_Gameplay.bullet.maxLifeTime);
            s_Gameplay.bullet.enemyDamage = GetF(b, "enemyDamage", s_Gameplay.bullet.enemyDamage);
            s_Gameplay.bullet.defaultScale = GetF(b, "defaultScale", s_Gameplay.bullet.defaultScale);
        }

        // NEW fields - pool sizes, scoring, invulnerability timings
        s_Gameplay.playerBulletPoolSize = GetI(doc, "playerBulletPoolSize", s_Gameplay.playerBulletPoolSize);
        s_Gameplay.enemyBulletPoolSize = GetI(doc, "enemyBulletPoolSize", s_Gameplay.enemyBulletPoolSize);
        s_Gameplay.scorePerWaveNormal = GetI(doc, "scorePerWaveNormal", s_Gameplay.scorePerWaveNormal);
        s_Gameplay.scorePerWaveEndless = GetI(doc, "scorePerWaveEndless", s_Gameplay.scorePerWaveEndless);
        s_Gameplay.invulnDuration = GetF(doc, "invulnDuration", s_Gameplay.invulnDuration);
        s_Gameplay.invulnCooldown = GetF(doc, "invulnCooldown", s_Gameplay.invulnCooldown);
        s_Gameplay.noDamageBonus = GetI(doc, "noDamageBonus", s_Gameplay.noDamageBonus);
        s_Gameplay.bossKillBonus = GetI(doc, "bossKillBonus", s_Gameplay.bossKillBonus);
        s_Gameplay.speedBonusMax = GetF(doc, "speedBonusMax", s_Gameplay.speedBonusMax);
        s_Gameplay.waveParTimeSeconds = GetF(doc, "waveParTimeSeconds", s_Gameplay.waveParTimeSeconds);
        s_Gameplay.killScoreWalker = GetI(doc, "killScoreWalker", s_Gameplay.killScoreWalker);
        s_Gameplay.killScoreMelee = GetI(doc, "killScoreMelee", s_Gameplay.killScoreMelee);
        s_Gameplay.killScoreRanger = GetI(doc, "killScoreRanger", s_Gameplay.killScoreRanger);
        s_Gameplay.killScoreBoss = GetI(doc, "killScoreBoss", s_Gameplay.killScoreBoss);
    }

    s_Loaded = true;
    //std::cout << "[GameConfig] All configs loaded.\n";
}

// ============================================================================
// Accessors
// ============================================================================
const PlayerConfig& GameConfig::Player() { if (!s_Loaded) Load(); return s_Player; }
const NpcConfig& GameConfig::Npc() { if (!s_Loaded) Load(); return s_Npc; }
const PowerUpConfig& GameConfig::PowerUp() { if (!s_Loaded) Load(); return s_PowerUp; }
const WaveSpawnConfig& GameConfig::Wave() { if (!s_Loaded) Load(); return s_Wave; }
const GameplayConfig& GameConfig::Gameplay() { if (!s_Loaded) Load(); return s_Gameplay; }