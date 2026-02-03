#pragma once
#include "GameObject.h"
#include "NPC.h"
#include "GameObjectType.h"

class Bullet : public GameObject {
public:
    void Start();
    void Update(f32 deltaTime);
    void Activate(GameObject* shooter, AEVec2 direction, BulletOwner newOwner);

    GameObject* startPos = nullptr;   // who fired
    AEVec2 dir = { 0.0f, 0.0f };
    f32 speed = 1000.0f;
    f32 lifeTime = 0.0f;
    f32 maxLifeTime = 2.0f;
    BulletOwner owner = BulletOwner::PLAYER; // PLAYER or ENEMY
    
private:
    void HideBullet(); //Bullet *obj
};
