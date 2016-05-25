#include "Window.h"
#include "Character.h"
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
	Player.gravity = 5000.0;
	Player.runSpeed = 15*32;
	Player.jumpVelocity = 25*32;
	Player.jumpTimeMax = 0.14;
	Player.terminalVelocity = 30*32;

	//minjump = 0.5(jumpVelocity^2/gravity)	= 64 = 2 blocks
	//maxjump = 0.5(jumpVelocity^2/gravity) + jumpVelocity*jumpTimeMax	= 176 = 5.5 blocks

	Player.rect.w = 32;
	Player.rect.h = 64;
	Player.origin.x = (double)(Player.rect.w / 2);
	Player.origin.y = (double)Player.rect.h;

	Player.moveTo(48, SCREEN_HEIGHT - 32);

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

		if (!Player.freeFall)
		{
			if ((keystate[SDL_SCANCODE_UP] || keystate[SDL_SCANCODE_W]))
			{
				Player.jumpivate();
			}
			else if (Player.airBorne)
			{
				Player.freeFall = true;
			}
		}

		Player.move(frameTime, gameMap);

		//cout << duration_cast<microseconds>(system_clock::now() - lastTime).count() << "\t\t";
	
		//rendering block
		SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 0, 255);
		SDL_RenderClear(mainWindow.ren);

		gameMap.render(&mainWindow);

		if (checkMapCollision(Player, gameMap)) SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 255, 255);
		else SDL_SetRenderDrawColor(mainWindow.ren, 255, 0, 0, 255);

		SDL_RenderFillRect(mainWindow.ren, &Player.rect);
		SDL_RenderPresent(mainWindow.ren);

		//cout << duration_cast<microseconds>(system_clock::now() - lastTime).count() << endl;
		SDL_Delay(1);
	}

	close();
}