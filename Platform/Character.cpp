#include "Character.h"

int min(int a, int b)
{
	return a < b ? a : b;
}


Character::Character()
{
	currentFrame = 0;
	frameCounter = 0.0;
	facingRight = true;
	currentState = IDLE;
	currentAnim = nullptr;
	velocity.x = 0.0;
	velocity.y = 0.0;
	gravity = 0.0;
	runSpeed = 0.0;
	jumpVelocity = 0.0;
	startJumpVector = 0.0;
	terminalVelocity = 0.0;
	position.x = 0.0;
	position.y = 0.0;
	origin.x = 0.0;
	origin.y = 0.0;
	hitbox.x = 0;
	hitbox.y = 0;
	hitbox.w = 0;
	hitbox.h = 0;
	airBorne = false;
	freeFall = false;

	for (int i = 0; i < 4; i++)
	{
		padding[i] = 0;
	}
}

bool Character::move(double deltaTime, const Tilemap& map)
{
	bool moved = false;	//character has moved a whole pixel *clapclap*
	////////////////Y_AXIS///////////////////////////
	static double startHeight = position.y;
	double downBound = scanBoundary(DOWN, map);

	if (!airBorne)	//grounded
	{
		if (downBound > 0.0)	//fall through
		{
			airBorne = true;
			freeFall = true;
			changeAnim(FALL);
		}
	}

	if (airBorne)	//airborne
	{
		double targetPos;
		static double airTime = 0.0;
		static double fallTime = 0.0;

		airTime += deltaTime;
		if (!freeFall)	//actively jump
		{
			if (airTime >= jumpTimeMax)	//max jump
			{
				freeFall = true;
				deltaTime = airTime - jumpTimeMax;	//freefall for the remaining time
			}
			else targetPos = startHeight - jumpVelocity*airTime;
		}

		if (freeFall)	//freefall
		{
			fallTime += deltaTime;
			targetPos = startHeight - startJumpVector*airTime + 0.5*gravity*fallTime*fallTime;

			if (targetPos - position.y > terminalVelocity*deltaTime)	//terminal velocity
			{
				targetPos = position.y + terminalVelocity*deltaTime;
			}
		}


		double upBound = scanBoundary(UP, map);

		double dist = targetPos - position.y;

		if (dist < 0)
		{	
			if (-dist > upBound)	//hit ceiling
			{
				position.y -= upBound;
				freeFall = true;
				startHeight = position.y;
				startJumpVector = 0.0;
				airTime = 0.0;
				fallTime = 0.0;
				changeAnim(FALL);
			}
			else
			{
				position.y = targetPos;
				changeAnim(JUMP);
			}
		}
		else
		{
			if (targetPos - position.y > downBound)	//landing
			{
				position.y += downBound;
				airBorne = false;
				freeFall = false;
				startHeight = position.y;
				startJumpVector = 0.0;
				airTime = 0.0;
				fallTime = 0.0;
				changeAnim(IDLE);
			}
			else
			{
				position.y = targetPos;
				changeAnim(FALL);
			}
		}
		

		if (hitbox.y != int(position.y - origin.y))
		{
			hitbox.y = int(position.y - origin.y);  //truncation is fine
			moved = true;
		}
		
	}

	///////////////////////X-Axis//////////////////////////
	int targetDist;
	if (velocity.x < 0.0)
	{
		facingRight = false;
		position.x += fmax(velocity.x * deltaTime, -scanBoundary(LEFT, map));
		if (hitbox.x != int(position.x - origin.x))
		{
			hitbox.x = int(position.x - origin.x);
			moved = true;
		}
		if (currentState == IDLE)
		{
			changeAnim(MOVE);
		}
	}
	else if (velocity.x > 0.0)
	{
		facingRight = true;
		position.x += fmin(velocity.x * deltaTime, scanBoundary(RIGHT, map));
		if (hitbox.x != int(position.x - origin.x))
		{
			hitbox.x = int(position.x - origin.x);
			moved = true;
		}
		if (currentState == IDLE)
		{
			changeAnim(MOVE);
		}
	}
	else if (!airBorne)
	{
		changeAnim(IDLE);
	}

	return moved;
}

void Character::moveTo(double x, double y)
{
	position.x = x;
	position.y = y;
	hitbox.x = int(position.x - origin.x);
	hitbox.y = int(position.y - origin.y);
}

void Character::jumpivate()
{
	airBorne = true;
	//velocity.y = -jumpVelocity;
	startJumpVector = jumpVelocity;
	changeAnim(JUMP);
}

double Character::scanDistance(double edge, const Tilemap& map, Direction direction, intVector firstTile, intVector lastTile)
{
	double distance;

	//indices of tile to be checked
	int xi;
	int yi;

	//to keep track of smallest value
	int minDist = 1000000;
	int distIndex;

	//for each occupied tile, shoot a ray in desired direction
	//insert smallest value in distance
	for (int i = firstTile.y; i <= lastTile.y; i++)
	{
		for (int j = firstTile.x; j <= lastTile.x; j++)
		{
			yi = i;
			xi = j;
			distIndex = 0;

			while (
				distIndex < minDist
				&& xi >= 0
				&& yi >= 0
				&& xi < map.horiTiles
				&& yi < map.vertiTiles
				&& map.getTile(xi, yi) != 1
				)
			{

				switch (direction)
				{
				case LEFT:	xi--;	break;
				case RIGHT:	xi++;	break;
				case UP:	yi--;	break;
				case DOWN:	yi++;	break;
				}
				distIndex++;
			}
			minDist = min(minDist, distIndex);
		}
	}

	switch (direction)
	{
	case LEFT:	distance = edge - (xi + 1)*map.tileRes;	break;
	case RIGHT:	distance = xi*map.tileRes - edge;		break;
	case UP:	distance = edge - (yi + 1)*map.tileRes;	break;
	case DOWN:	distance = yi*map.tileRes - edge;		break;
	}

	return signbit(distance) ? 0.0 : distance;
}

double Character::scanBoundary(Direction direction, const Tilemap& map)
{
	//scanner's shape is simplified: find every tile which scanner's hitbox overlaps with
	//get the first and last indices of these tiles in both axes
	int x1 =  hitbox.x / map.tileRes;
	int x2 = (hitbox.x + hitbox.w - 1) / map.tileRes;
	int y1 = hitbox.y / map.tileRes;
	int y2 = (hitbox.y + hitbox.h - 1) / map.tileRes;

	intVector tile1;
	intVector tile2;

	double edge; //position of the relevant edge of the hitbox
	switch (direction)
	{
	case LEFT:
	{
		edge = position.x - origin.x;
		tile1 = { x1,y1 };
		tile2 = { x1,y2 };
		break;
	}
	case RIGHT:
	{
		edge = position.x - origin.x + hitbox.w;
		tile1 = { x2,y1 };
		tile2 = { x2,y2 };
		break;
	}
	case UP:
	{
		edge = position.y - origin.y;
		tile1 = { x1,y1 };
		tile2 = { x2,y1 };
		break;
	}
	case DOWN:
	{
		edge = position.y - origin.y + hitbox.h;
		tile1 = { x1,y2 };
		tile2 = { x2,y2 };
		break;
	}
	default: return 0.0;
	}

	//get maximum distance scanner can travel direction
	return scanDistance(edge, map, direction, tile1, tile2);
}

void Character::render(const Window& window)
{
	SDL_Rect renderRect;
	renderRect.x = hitbox.x - padding[0] - window.offsetX;
	renderRect.y = hitbox.y - padding[1] - window.offsetY;
	renderRect.w = hitbox.w + padding[0] + padding[2];
	renderRect.h = hitbox.h + padding[1] + padding[3];

	//SDL_RenderFillRect(window.ren, &renderRect);
	//rc.h = rc.w = sprites->tileRes;

	if (currentFrame < sprites->frames.size())
	{
		if (facingRight)
		{
			SDL_RenderCopy(window.ren, sprites->frames[currentFrame], NULL, &renderRect);
		}
		else
		{
			SDL_RenderCopyEx(window.ren, sprites->frames[currentFrame], NULL, &renderRect, NULL, NULL, SDL_FLIP_HORIZONTAL);
		}
	}
}

void Character::animate(double deltaTime)
{
	if (currentAnim == nullptr) return;
	else currentFrame = currentAnim->animate(deltaTime, currentFrame, frameCounter);
	
}

void Character::changeAnim(AnimState state)
{
	if (state == currentState) return;
	if (!anims.count(state)) return; //animation not found
	currentState = state;
	currentAnim = &anims[state];

	currentFrame = currentAnim->startFrame;
	frameCounter = 0.0;
}