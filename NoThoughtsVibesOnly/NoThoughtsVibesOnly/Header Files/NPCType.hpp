#pragma once
// Author: John Chiow
#include "AEEngine.h"

enum NPCType
{
	NPC_MELEE,  // Close-range attacker
	NPC_RANGER, // Ranged NPC attack from a distance
	NPC_WALK,   // Simple wandering NPCs
	NPC_BOSS    // BOSS - Powerful enemy with special abilities
};
