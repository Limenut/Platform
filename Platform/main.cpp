#include "Window.h"
#include "Tilemap.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <cmath>

#ifdef main
#undef main
#endif

using namespace std;
using namespace std::chrono;

const int SCREEN_WIDTH = 32*32;
const int SCREEN_HEIGHT = 32*18;
Window mainWindow;
Tilemap gameMap;

enum Direction
{
	NONE,
	LEFT,
	RIGHT,
	UP,
	DOWN
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

int min(int a, int b)
{
	return a < b ? a : b;
}

double min(double a, double b)
{
	return a < b ? a : b;
}

double max(double a, double b)
{
	return a > b ? a : b;
}


class Character
{
public:
	Character();
	void move(double deltaTime);
	void jump();
	double scanDistance(double edge, const Tilemap& map, Direction direction, intVector firstTile, intVector lastTile);
	double scanBoundary(Direction direction, const Tilemap& map);

	doubleVector velocity;
	double gravity;
	double runSpeed;
	double airSpeed;
	double jumpVelocity;
	double jumpHeight;
	double jumpHeightMax;
	double terminalVelocity;
	doubleVector position;
	doubleVector origin;

	SDL_Rect rect;

	bool airBorne;
	bool freeFall;
};

Character::Character()
{
	velocity.x = 0.0;
	velocity.y = 0.0;
	gravity = 0.0;
	runSpeed = 0.0;
	jumpVelocity = 0.0;
	jumpHeight = 0.0;
	jumpHeightMax = 0.0;
	terminalVelocity = 0.0;
	position.x = 0.0;
	position.y = 0.0;
	origin.x = 0.0;
	origin.y = 0.0;
	//bounds.left = 0.0;
	//bounds.right = 0.0;
	//bounds.up = 0.0;
	//bounds.down = 0.0;
	rect.x = 0;
	rect.y = 0;
	rect.w = 0;
	rect.h = 0;
	airBorne = false;
	freeFall = false;
}

void Character::move(double deltaTime)
{
	////////////////Y_AXIS///////////////////////////
	double downBound = scanBoundary(DOWN, gameMap);

	
	if (!airBorne)	//grounded
	{
		if (downBound > 0.0001)	//fall through
		{
			airBorne = true;
			freeFall = true;
		}
	}

	if (airBorne)	//airborne
	{
		if (freeFall && velocity.y < terminalVelocity)	//gravity
		{
			velocity.y = min(velocity.y + gravity * deltaTime, terminalVelocity);
		}

		if (velocity.y < 0.0)	//rising
		{		
			if (!freeFall)	//actively jumping
			{

				if (jumpHeight >= jumpHeightMax)	//max jump
				{
					position.y += jumpHeight - jumpHeightMax;
					jumpHeight = 0.0;
					freeFall = true;
				}

				jumpHeight -= velocity.y * deltaTime;
			}	

			double upBound = scanBoundary(UP, gameMap);
			position.y += max(velocity.y * deltaTime, -upBound);

			upBound = scanBoundary(UP, gameMap);
			if (upBound < 0.0001) //hit ceiling
			{
				jumpHeight = 0.0;
				freeFall = true;
				velocity.y = 0.0;
			}
		}
		else //falling
		{
			position.y += min(velocity.y * deltaTime, downBound);

			//landing
			if (downBound < 0.0001)
			{
				jumpHeight = 0.0;
				airBorne = false;
				freeFall = false;
				velocity.y = 0.0;
			}
		}
	}

	rect.y = int(position.y - origin.y); //truncation is fine

	///////////////////////X-Axis//////////////////////////
	if (velocity.x < 0.0)
	{
		position.x += max(velocity.x * deltaTime, -scanBoundary(LEFT, gameMap));
	}
	else if (velocity.x > 0.0)
	{
		position.x += min(velocity.x * deltaTime, scanBoundary(RIGHT, gameMap));
	}

	rect.x = int(position.x - origin.x);
}

void Character::jump()
{
	airBorne = true;
	velocity.y = -jumpVelocity;
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
	int x1 = rect.x / map.tileRes;
	int x2 = (rect.x + rect.w - 1) / map.tileRes;
	int y1 = rect.y / map.tileRes;
	int y2 = (rect.y + rect.h - 1) / map.tileRes;

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
		edge = position.x - origin.x + rect.w;
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
		edge = position.y - origin.y + rect.h;
		tile1 = { x1,y2 };
		tile2 = { x2,y2 };
		break;
	}
	default: return 0.0;
	}

	//get maximum distance scanner can travel direction
	return scanDistance(edge, map, direction, tile1, tile2);
}


bool init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		return false;
	}

	//Set texture filtering to linear
	if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
	{
		printf("Warning: Linear texture filtering not enabled!");
	}

	if (!mainWindow.init("Platform", SCREEN_WIDTH, SCREEN_HEIGHT)) return false;

	//Initialize PNG loading
	int imgFlags = IMG_INIT_PNG;
	if (!(IMG_Init(imgFlags) & imgFlags))
	{
		printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
		return false;
	}

	return true;
}

void close()
{
	IMG_Quit();
	SDL_Quit();
}

bool checkMapCollision(Character& scanner, const Tilemap& map)
{
	int x1 = scanner.rect.x / map.tileRes;
	int x2 = (scanner.rect.x + scanner.rect.w - 1) / map.tileRes;
	int y1 = scanner.rect.y / map.tileRes;
	int y2 = (scanner.rect.y + scanner.rect.h - 1) / map.tileRes;

	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			if (x < 0 || x >= map.horiTiles || y < 0 || y >= map.vertiTiles) continue;
			if (map.getTile(x, y) == 1) return true;
		}
	}
	return false;
}

int main()
{
	init();

	Spritesheet levelSprites("testpic.png", 32, &mainWindow);
	gameMap.sprites = &levelSprites;
	gameMap.loadFile("testmap.map");
	gameMap.update(&mainWindow);

	Character Player;
	Player.position.x = 100;
	Player.position.y = SCREEN_HEIGHT - 90;
	Player.gravity = 5000.0;
	Player.runSpeed = 500.0;
	Player.jumpVelocity = 800.0;
	Player.jumpHeightMax = 128.0;
	Player.terminalVelocity = 1024.0;

	Player.rect.w = 32;
	Player.rect.h = 64;
	Player.rect.x = int(Player.position.x);
	Player.rect.y = int(Player.position.y);
	Player.origin.x = (double)(Player.rect.w / 2);
	Player.origin.y = (double)Player.rect.h;

	const Uint8 *keystate = SDL_GetKeyboardState(NULL);
	SDL_Event e;
	bool quit = false;

	double frameTime = 0.0;
	system_clock::time_point lastTime = system_clock::now();
	while (!quit)
	{
		frameTime = duration_cast<microseconds>(system_clock::now() - lastTime).count() / 1000000.0;
		if (frameTime > 0.1) frameTime = 0.1;	//at low framerates game becomes frame dependent to avoid collision errors etc.
		lastTime = system_clock::now();


		//event block
		SDL_PumpEvents();
		while (SDL_PollEvent(&e))
		{
			if (e.type == SDL_WINDOWEVENT)
			{
				mainWindow.handleEvents(&e);
			}
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}
			if (e.type == SDL_KEYDOWN)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_a:
				case SDLK_LEFT: 
				{
					Player.velocity.x = -Player.runSpeed;
					break;
				}
				case SDLK_d:
				case SDLK_RIGHT:	
				{
					Player.velocity.x = Player.runSpeed;
					break;
				}
				default: break;
				}
			}
			else if (e.type == SDL_KEYUP)
			{
				switch (e.key.keysym.sym)
				{
				case SDLK_a:
				case SDLK_LEFT:
				{
					if (keystate[SDL_SCANCODE_RIGHT] || keystate[SDL_SCANCODE_D]) Player.velocity.x = Player.runSpeed;
					else Player.velocity.x = 0.0;
					break;
				}
				case SDLK_d:
				case SDLK_RIGHT:
				{
					if (keystate[SDL_SCANCODE_LEFT] || keystate[SDL_SCANCODE_A]) Player.velocity.x = -Player.runSpeed;
					else Player.velocity.x = 0.0;
					break;
				}
				default: break;
				}
			}
		}

		if (!Player.freeFall && (keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W]))
		{
			Player.jump();
		}
		else if (Player.airBorne)
		{
			Player.freeFall = true;
		}

		Player.move(frameTime);

	
		//rendering block
		SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 0, 255);
		SDL_RenderClear(mainWindow.ren);

		gameMap.render(&mainWindow);

		if (checkMapCollision(Player, gameMap)) SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 255, 255);
		else SDL_SetRenderDrawColor(mainWindow.ren, 255, 0, 0, 255);

		SDL_RenderFillRect(mainWindow.ren, &Player.rect);
		SDL_RenderPresent(mainWindow.ren);

		SDL_Delay(50);
	}

	close();
}