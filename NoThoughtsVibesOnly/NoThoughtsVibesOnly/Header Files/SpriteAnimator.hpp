#pragma once
// Author: Chia Wenjie
#include "pch.hpp"
#include "AEEngine.h"
#include <vector>

struct Rect
{
	int x, y;
	int width, height;
};

class Animation
{
public:
	std::vector<Rect> frames;
	float frameDuration;
	bool isLooping;

	Animation();

	Animation(float duration, bool loop);


~Animation();

void AddFrames(int x, int y, int width, int height);
};

class Animator
{
private:
	Animation* currentAnimation;
	int currentFrameIndex;
	float elapsedTime;

public:

	Animator();

	~Animator();

	void Play(Animation* anim);

	void Update(float deltaTime);

	Rect GetCurrentFrameRect() const;
};
