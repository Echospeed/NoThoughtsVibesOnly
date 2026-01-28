#pragma once
#include <vector>
#include "Player.h"
#include "Enemy.h"

void DrawMinimap(const Player& player, const std::vector<Enemy>& enemies, float camX, float camY);