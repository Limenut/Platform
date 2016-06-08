#include "Animation.h"

Animation::Animation()
{
	startFrame = 0;
	endFrame = 0;

	running = true;
	loop = true;
	pong = false;
	animForward = true;

	frameDelay = 0.1;
}

Animation::Animation(unsigned start, unsigned count, double delay) : Animation()
{
	startFrame = start;

	if (count <= 1)
	{
		endFrame = startFrame;
		running = false; 
	}
	else
	{
		endFrame = start + count - 1;
		frameDelay = delay;
	}
}

unsigned Animation::animate(double deltaTime, unsigned currentFrame, double &counter)
{
	if (!running) return currentFrame;

	counter += deltaTime;

	while (counter >= frameDelay)
	{
		counter -= frameDelay;

		if (animForward)
		{
			currentFrame++;
		}
		else
		{
			currentFrame--;
		}

		if (currentFrame > endFrame)
		{
			if (pong)
			{
				animForward = false;
				currentFrame -= 2;
			}
			else
			{
				currentFrame = startFrame;
			}
		}
		else if (currentFrame < startFrame)
		{
			if (pong)
			{
				animForward = true;
				currentFrame += 2;
			}
			else
			{
				currentFrame = endFrame;
			}
		}
	}
	return currentFrame;
}