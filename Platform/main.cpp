#include "Window.h"
#include "Tilemap.h"
#include <iostream>
#include <chrono>
#include <vector>

#ifdef main
#undef main
#endif

using namespace std;
using namespace std::chrono;

const int SCREEN_WIDTH = 32*32;
const int SCREEN_HEIGHT = 32*18;
Window mainWindow;
Tilemap gameMap;

double min(double a, double b)
{
	return a < b ? a : b;
}

double max(double a, double b)
{
	return a > b ? a : b;
}

enum Direction
{
	NONE,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

struct doubleVector
{
	double x;
	double y;
};

struct doubleBounds
{
	double left;
	double right;
	double up;
	double down;
};

class Character
{
public:
	Character();
	void move(double deltaTime);
	void jump();

	doubleVector velocity;
	double gravity;
	double runSpeed;
	double airSpeed;
	double jumpVelocity;
	double jumpHeight;
	double jumpHeightMax;
	doubleVector position;
	doubleVector origin;
	doubleBounds bounds;

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
	position.x = 0.0;
	position.y = 0.0;
	origin.x = 0.0;
	origin.y = 0.0;
	bounds.left = 0.0;
	bounds.right = 0.0;
	bounds.up = 0.0;
	bounds.down = 0.0;
	rect.x = 0;
	rect.y = 0;
	rect.w = 0;
	rect.h = 0;
	airBorne = false;
	freeFall = false;
}

void Character::move(double deltaTime)
{
	if (airBorne)
	{
		if (velocity.y < 0.0)
		{
			position.y += min(velocity.y * deltaTime, bounds.up);
		}
		if (velocity.y > 0.0)
		{
			position.y += min(velocity.y * deltaTime, bounds.down);
		}
		
		if (freeFall)
		{
			velocity.y += gravity * deltaTime;
		}
		else //elevation0
		{
			jumpHeight -= velocity.y * deltaTime;
			if (jumpHeight >= jumpHeightMax)
			{
				position.y += jumpHeight - jumpHeightMax;

				jumpHeight = 0.0;
				freeFall = true;
			}
		}
	}

	if (velocity.x < 0.0)
	{
		position.x += min(velocity.x * deltaTime, bounds.left);
	}
	else if (velocity.x > 0.0)
	{
		position.x += min(velocity.x * deltaTime, bounds.right);
	}

	//landing
	if (position.y > SCREEN_HEIGHT || bounds.down < 0.0001)
	{
		airBorne = false;
		freeFall = false;
		//position.y = SCREEN_HEIGHT;
	}

	rect.x = int(position.x - origin.x + 0.5);
	//rect.y = SCREEN_HEIGHT - int(position.y - origin.y + 0.5); //invert y-axis for rendering
	rect.y = int(position.y - origin.y + 0.5);
}

void Character::jump()
{
	airBorne = true;
	velocity.y = -jumpVelocity;
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

double scanDistance(doubleVector pos, Direction direction, const Tilemap& map)
{
	double distance;

	int xi = int((pos.x - 0.5) / map.tileRes);
	int yi = int((pos.y - 0.5) / map.tileRes);
	while (
		xi >= 0 
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
		
	}

	switch (direction)
	{
	case LEFT:	distance = pos.x - (xi+1)*map.tileRes;	break;
	case RIGHT:	distance = xi*map.tileRes - pos.x;		break;
	case UP:	distance = pos.y - (yi+1)*map.tileRes;	break;
	case DOWN:	distance = yi*map.tileRes - pos.y;		break;
	}
	
	return max(distance, 0.0);
}

void scanBoundaries(Character& scanner, const Tilemap& map)
{
	//double leftD, rightD, upD, downD;

	//topleft, topright, bottomleft, bottomright
	doubleVector tl, tr, bl, br;

	tl.x = bl.x = scanner.position.x - scanner.origin.x;					//left
	tr.x = br.x = scanner.position.x - scanner.origin.x + scanner.rect.w;	//right
	tl.y = tr.y = scanner.position.y - scanner.origin.y;					//top
	bl.y = br.y = scanner.position.y - scanner.origin.y + scanner.rect.h;	//bottom
	
	scanner.bounds.left = min(scanDistance(tl, LEFT, map), scanDistance(bl, LEFT, map));
	scanner.bounds.right = min(scanDistance(tr, RIGHT, map), scanDistance(br, RIGHT, map));
	scanner.bounds.up = min(scanDistance(tl, UP, map), scanDistance(tr, UP, map));
	scanner.bounds.down = min(scanDistance(bl, DOWN, map), scanDistance(br, DOWN, map));

	cout << scanner.bounds.left << endl;
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
	Player.position.y = SCREEN_HEIGHT - 100;
	Player.gravity = 5000.0;
	Player.runSpeed = 500.0;
	Player.jumpVelocity = 1200.0;
	Player.jumpHeightMax = 200;

	Player.rect.w = 32;
	Player.rect.h = 64;
	Player.rect.x = int(Player.position.x + 0.5);
	Player.rect.y = int(Player.position.y + 0.5);
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

		scanBoundaries(Player, gameMap);
		Player.move(frameTime);

	
		//rendering block
		SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 0, 255);
		SDL_RenderClear(mainWindow.ren);

		gameMap.render(&mainWindow);

		SDL_SetRenderDrawColor(mainWindow.ren, 255, 0, 0, 255);
		SDL_RenderFillRect(mainWindow.ren, &Player.rect);
		SDL_RenderPresent(mainWindow.ren);

		SDL_Delay(50);
	}

	close();
}