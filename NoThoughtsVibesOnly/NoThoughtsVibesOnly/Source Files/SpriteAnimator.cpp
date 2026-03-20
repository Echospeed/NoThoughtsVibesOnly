#include "pch.hpp"
#include "SpriteAnimator.hpp"
#include <vector>

Animation::Animation() : frameDuration(0.0f), isLooping(false) {}

Animation::Animation(float duration, bool loop) : frameDuration(duration), isLooping(loop) {}

Animation::~Animation()
{

}

void Animation::AddFrames(int x, int y, int width, int height)
{
	frames.push_back({ x, y, width, height });
}


Animator::Animator() : currentAnimation(nullptr), currentFrameIndex(0), elapsedTime(0.0f) {}

Animator::~Animator()
{

}

void Animator::Play(Animation* anim)
{
	if (currentAnimation != anim)
	{
		currentAnimation = anim;
		currentFrameIndex = 0;
		elapsedTime = 0.0f;
	}
}

void Animator::Update(float deltaTime)
{
	if (!currentAnimation || currentAnimation->frames.empty()) return;

	elapsedTime += deltaTime;

	if (elapsedTime >= currentAnimation->frameDuration)
	{
		elapsedTime -= currentAnimation->frameDuration;
		currentFrameIndex++;

		if (currentFrameIndex >= currentAnimation->frames.size())
		{
			if (currentAnimation->isLooping)
			{
				currentFrameIndex = 0;
			}
			else
			{
				currentFrameIndex = static_cast<s8>(currentAnimation->frames.size()) - 1;
			}
		}
	}
}

Rect Animator::GetCurrentFrameRect() const
{
	if (!currentAnimation || currentAnimation->frames.empty())
	{
		return { 0, 0, 0, 0 };
	}
	return currentAnimation->frames[currentFrameIndex];
}