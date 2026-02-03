#include "pch.h"
#include "NPC.h"
#include "NPCType.h"
#include "Player.h"
#include "GamePage.h"
#include <Math.h>
#include <iostream>
#include "Bullet.h"

void NPC::Start() //Initialize enemy properties
{
	f32 rX = ((f32)rand() / RAND_MAX) * WORLD_WIDTH - WORLD_WIDTH / 2.0f;
	f32 rY = ((f32)rand() / RAND_MAX) * WORLD_HEIGHT - WORLD_HEIGHT / 2.0f;


	while (sqrt((rX - target->transform.position.x) * (rX - target->transform.position.x)
		+ (rY - target->transform.position.y) * (rY - target->transform.position.y)) < 100.0f)
	{
		rX = ((f32)rand() / RAND_MAX) * WORLD_WIDTH - WORLD_WIDTH / 2.0f;
		rY = ((f32)rand() / RAND_MAX) * WORLD_HEIGHT - WORLD_HEIGHT / 2.0f;
	}


	transform.position = { rX, rY };
	transform.scale = { 30.0f, 30.0f };
	transform.rotation = 0.0f;

	if (this->type == NPC_WALK)
	{
		spriteRenderer.colour.r = 1.0f;
		spriteRenderer.colour.g = 1.0f;
		spriteRenderer.colour.b = 0.0f;
		spriteRenderer.meshType = MESH_CIRCLE;
		this->baseColour.r = 1.0f;
		this->baseColour.g = 1.0f;
		this->baseColour.b = 0.0f;
	}
	else if (this->type == NPC_MELEE)
	{
		spriteRenderer.colour.r = 0.0f;
		spriteRenderer.colour.g = 0.0f;
		spriteRenderer.colour.b = 1.0f;
		spriteRenderer.meshType = MESH_TRIANGLE;
		this->baseColour.r = 0.0f;
		this->baseColour.g = 0.0f;
		this->baseColour.b = 1.0f;
	}
	else if(this->type == NPC_RANGER)
	{
		spriteRenderer.colour.r = 0.0f;
		spriteRenderer.colour.g = 1.0f;
		spriteRenderer.colour.b = 1.0f;
		spriteRenderer.meshType = MESH_SQUARE;
		this->baseColour.r = 0.0f;
		this->baseColour.g = 1.0f;
		this->baseColour.b = 1.0f;
	}

	//std::cout << "NPC - Start: NPC Initialized at Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void NPC::Update(f32 deltaTime) //Update enemy each frame
{
	if (!isVisibleToPlayer)
		return;

	if (health <= 0.0f)
	{
		isVisibleToPlayer = false;
		isActive = false;              // 👈 THIS IS THE KEY
		spriteRenderer.colour.a = 0.0f;
		return;
	}


	if (!isVisibleToPlayer) return; // skip all logic


	if (this->type == NPC_WALK)
	{
		WalkNPCs(deltaTime);
	}
	else if (this->type == NPC_MELEE)
	{
		BomberNPCs(deltaTime);
	}
	else if (this->type == NPC_RANGER)
	{
		RangerNPCs(deltaTime);
	}
	//std::cout << "NPC - Update: NPC Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void NPC::BomberNPCs(f32 deltaTime)
{
	if (!target || health <= 0.0f) return;

	AEVec2 direction {};
	AEVec2 displacement = {target->transform.position.x - transform.position.x, target->transform.position.y - transform.position.y };
	AEVec2Normalize(&direction, &displacement);
	this->transform.position.x += direction.x * speed * deltaTime;
	this->transform.position.y += direction.y * speed * deltaTime;

	// Wall bounce
	f32 halfWorldWidth = WORLD_WIDTH / 2.0f;
	f32 halfWorldHeight = WORLD_HEIGHT / 2.0f;
	f32 halfWidth = transform.scale.x / 2.0f;
	f32 halfHeight = transform.scale.y / 2.0f;

	if (transform.position.x > halfWorldWidth - halfWidth) { transform.position.x = halfWorldWidth - halfWidth; velocity.x = -velocity.x; }
	if (transform.position.x < -halfWorldWidth + halfWidth) { transform.position.x = -halfWorldWidth + halfWidth; velocity.x = -velocity.x; }
	if (transform.position.y > halfWorldHeight - halfHeight) { transform.position.y = halfWorldHeight - halfHeight; velocity.y = -velocity.y; }
	if (transform.position.y < -halfWorldHeight + halfHeight) { transform.position.y = -halfWorldHeight + halfHeight; velocity.y = -velocity.y; }
	//std::cout << "NPC - MeleeNPC: Is running\n";
	if (!isVisibleToPlayer) return; // skip all logic

}

void NPC::RangerNPCs(f32 deltaTime)
{
	if (!target || health <= 0.0f) return;

	// Move like before
	AEVec2 dirVec = { target->transform.position.x - transform.position.x,
					  target->transform.position.y - transform.position.y };
	f32 dist = sqrtf(dirVec.x * dirVec.x + dirVec.y * dirVec.y);

	if (dist < 250.0f)
	{
		velocity.x = -(dirVec.x / dist) * speed;
		velocity.y = -(dirVec.y / dist) * speed;
	}
	else
	{
		velocity.x *= 0.95f;
		velocity.y *= 0.95f;
	}

	transform.position.x += velocity.x * deltaTime;
	transform.position.y += velocity.y * deltaTime;

	// Wall bounce
	f32 halfW = WORLD_WIDTH / 2.0f, halfH = WORLD_HEIGHT / 2.0f;
	f32 hw = transform.scale.x / 2.0f, hh = transform.scale.y / 2.0f;

	if (transform.position.x > halfW - hw) { transform.position.x = halfW - hw; velocity.x = -velocity.x; }
	if (transform.position.x < -halfW + hw) { transform.position.x = -halfW + hw; velocity.x = -velocity.x; }
	if (transform.position.y > halfH - hh) { transform.position.y = halfH - hh; velocity.y = -velocity.y; }
	if (transform.position.y < -halfH + hh) { transform.position.y = -halfH + hh; velocity.y = -velocity.y; }

	// 🔫 Ranger shooting
	fireCooldown -= deltaTime;
	if (fireCooldown <= 0.0f)
	{
		for (auto& obj : objects)
		{
			Bullet* b = dynamic_cast<Bullet*>(obj);
			if (b && !b->isActive && b->startPos == this)
			{
				b->transform.position = transform.position;

				AEVec2 dir = { target->transform.position.x - transform.position.x,
							   target->transform.position.y - transform.position.y };
				f32 mag = sqrtf(dir.x * dir.x + dir.y * dir.y);
				b->dir.x = dir.x / mag;
				b->dir.y = dir.y / mag;

				b->isActive = true;
				b->lifeTime = b->maxLifeTime;
				b->spriteRenderer.colour.a = 1.0f;
				b->owner = BulletOwner::ENEMY;
				break; // fire 1 bullet
			}
		}
		fireCooldown = fireRate;
	}
}

void NPC::WalkNPCs(f32 deltaTime)
{
	if (!target || health <= 0.0f) return;


	// Wall bounce
	f32 halfWorldWidth = WORLD_WIDTH / 2.0f;
	f32 halfWorldHeight = WORLD_HEIGHT / 2.0f;
	f32 halfWidth = transform.scale.x / 2.0f;
	f32 halfHeight = transform.scale.y / 2.0f;

	if (transform.position.x > halfWorldWidth - halfWidth) { transform.position.x = halfWorldWidth - halfWidth; velocity.x = -velocity.x; }
	if (transform.position.x < -halfWorldWidth + halfWidth) { transform.position.x = -halfWorldWidth + halfWidth; velocity.x = -velocity.x; }
	if (transform.position.y > halfWorldHeight - halfHeight) { transform.position.y = halfWorldHeight - halfHeight; velocity.y = -velocity.y; }
	if (transform.position.y < -halfWorldHeight + halfHeight) { transform.position.y = -halfWorldHeight + halfHeight; velocity.y = -velocity.y; }
	//std::cout << "NPC - WalkNPC: Is running\n";
}