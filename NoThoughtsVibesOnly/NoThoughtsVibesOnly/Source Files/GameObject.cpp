// Implements the rendering logic using the Global Functions (AEGfxSetTransform, AEGfxMeshDraw) listed in the documentation
#include "pch.h"
#include "GameObject.h"
#include "GamePage.h"

GameObject::GameObject(AEVec2 pos, AEVec2 scale, f32 rot, AEGfxTexture* texture, f32 width, f32 height) //Constructor
{
	Init(pos, scale, rot, texture, width, height);

	objects.push_back(this);
}

GameObject::~GameObject() //Destructor
{
    Free();
}

void GameObject::Init(AEVec2 pos, AEVec2 scale, f32 rot, AEGfxTexture* texture, f32 width, f32 height)
{
	// Progammer Defined Values
	transform.position = { pos.x, pos.y };
    transform.scale = { scale.x, scale.y };
	transform.rotation = rot;

	//Programmer Defined Sprite Renderer Values
	spriteRenderer.texture = texture;
	spriteRenderer.width = width;
	spriteRenderer.height = height;
}

void GameObject::Start() {}
void GameObject::Update(f32 deltaTime) { Draw(); }

void GameObject::Draw()
{
	DrawSpriteRenderer(spriteRenderer, transform);
}

void GameObject::Free()
{
	FreeSpriteRenderer(spriteRenderer);
}