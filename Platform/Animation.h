#pragma once


class Animation
{
public:
	Animation();
	unsigned animate(double deltaTime, unsigned currentFrame, double &counter);

	unsigned startFrame;
	unsigned endFrame;

	bool running;
	bool loop;
	bool pong;
	bool animForward;

	double frameDelay;
};

