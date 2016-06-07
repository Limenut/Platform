#pragma once

#include "Tilemap.h"
#include "Spritesheet.h"
#include "Animation.h"
#include <map>

enum Direction
{
	NONE,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

enum AnimState
{
	NO_ANIMATION,
	IDLE,
	MOVE,
	JUMP
};

struct intVector
{
	int x;
	int y;
};

struct doubleVector
{
	double x;
	double y;
};

class Character
{
public:
	Character();
	bool move(double deltaTime, const Tilemap& map);	//returns: whether character's rectangle has moved
	void moveTo(double x, double y);
	void jumpivate();
	double scanDistance(double edge, const Tilemap& map, Direction direction, intVector firstTile, intVector lastTile);
	double scanBoundary(Direction direction, const Tilemap& map);
	void render(const Window& window);
	void animate(double deltaTime);

	Spritesheet* sprites;
	std::map<AnimState, Animation> anims;
	Animation *currentAnim;
	unsigned currentFrame;
	double frameCounter;

	doubleVector velocity;
	double gravity;
	double runSpeed;
	double airSpeed;
	double jumpVelocity;
	double jumpTimeMax;
	double startJumpVector;
	double terminalVelocity;
	doubleVector position;
	doubleVector origin;

	SDL_Rect rect;

	bool airBorne;
	bool freeFall;
};