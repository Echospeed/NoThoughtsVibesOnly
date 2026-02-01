#include "NPC.h"
#include "NPCType.h"
#include "Player.h"
#include "GamePage.h"
#include <Math.h>
#include <iostream>

void NPC::Start() //Initialize enemy properties
{
	float rX = (rand() % static_cast<int>(2000.0f - (2000.0f/ 2.0f)));
	float rY = (rand() % static_cast<int>(2000.0f - (2000.0f / 2.0f)));

	transform.position = { rX, rY };
	transform.scale = { 15.0f, 15.0f };
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

	std::cout << "NPC - Start: NPC Initialized at Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void NPC::Update(f32 deltaTime) //Update enemy each frame
{
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

	if (health <= 0)
	{
		this->spriteRenderer.colour.a = 0.0f;
	}
	std::cout << "NPC - Update: NPC Position (" << this->transform.position.x << ',' << this->transform.position.y << ")\n";
}

void NPC::BomberNPCs(f32 deltaTime)
{
	std::cout << "NPC - MeleeNPC: Is running\n";
}

void NPC::RangerNPCs(f32 deltaTime)
{
	if (!target) return;


	AEVec2 dir;
	dir.x = target->transform.position.x - this->transform.position.x;
	dir.y = target->transform.position.y - this->transform.position.y;

	f32 dist = ((dir.x * dir.x) + (dir.y * dir.y));

	dist = sqrt(dist);

	if (dist < 250.0f)
	{
		velocity.x = -(dir.x / dist) * speed;
		velocity.y = -(dir.y / dist) * speed;
	}
	else
	{
		velocity.x *= 0.95f;
		velocity.y *= 0.95f;
	}

	this->transform.position.x += velocity.x * deltaTime;
	this->transform.position.y += velocity.y * deltaTime;

	// Wall bounce
	float halfWorldWidth = WORLD_WIDTH / 2.0f;
	float halfWorldHeight = WORLD_HEIGHT / 2.0f;
	float halfWidth = transform.scale.x / 2.0f;
	float halfHeight = transform.scale.y / 2.0f;

	if (transform.position.x > halfWorldWidth - halfWidth) { transform.position.x = halfWorldWidth - halfWidth; velocity.x = -velocity.x; }
	if (transform.position.x < -halfWorldWidth + halfWidth) { transform.position.x = -halfWorldWidth + halfWidth; velocity.x  = -velocity.x; }
	if (transform.position.y > halfWorldHeight - halfHeight) { transform.position.y = halfWorldHeight - halfHeight; velocity.y = -velocity.y; }
	if (transform.position.y < -halfWorldHeight + halfHeight) { transform.position.y = -halfWorldHeight + halfHeight; velocity.y = -velocity.y; }
	std::cout << "NPC - RangerNPC: Is running\n";
}

void NPC::WalkNPCs(f32 deltaTime)
{
	std::cout << "NPC - WalkNPC: Is running\n";
}