#include "Window.h"
#include "Character.h"
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

/*bool checkMapCollision(Character& scanner, const Tilemap& map)
{
	int x1 = scanner.hitbox.x / map.tileRes;
	int x2 = (scanner.hitbox.x + scanner.hitbox.w - 1) / map.tileRes;
	int y1 = scanner.hitbox.y / map.tileRes;
	int y2 = (scanner.hitbox.y + scanner.hitbox.h - 1) / map.tileRes;

	for (int x = x1; x <= x2; x++)
	{
		for (int y = y1; y <= y2; y++)
		{
			if (x < 0 || x >= map.horiTiles || y < 0 || y >= map.vertiTiles) continue;
			if (map.getTile(x, y) == 1) return true;
		}
	}
	return false;
}*/

int main()
{
	init();

	Spritesheet levelSprites("testpic.png", 32, 32, &mainWindow);
	gameMap.sprites = &levelSprites;
	gameMap.loadFile("tornila.map");
	gameMap.update(&mainWindow);

	

	Character Player;
	Player.gravity = 5000.0;
	Player.runSpeed = 10*32;
	Player.jumpVelocity = 25*32;
	Player.jumpTimeMax = 0.14;
	Player.terminalVelocity = 30*32;

	//minjump = 0.5(jumpVelocity^2/gravity)	= 64 = 2 blocks
	//maxjump = 0.5(jumpVelocity^2/gravity) + jumpVelocity*jumpTimeMax	= 176 = 5.5 blocks

	Player.hitbox.w = 32;
	Player.hitbox.h = 52;
	Player.origin.x = (double)(Player.hitbox.w / 2);
	Player.origin.y = (double)Player.hitbox.h;

	Player.padding[0] = 8;
	Player.padding[1] = 12;
	Player.padding[2] = 8;
	Player.padding[3] = 0;

	Player.moveTo(112, SCREEN_HEIGHT - 128);

	Spritesheet playerSprites("runnyC.png", 48, 64, &mainWindow);
	Player.sprites = &playerSprites;

	Player.anims[IDLE] = Animation(7, 0, 0.0);
	Player.anims[MOVE] = Animation(0, 8, 0.01875);
	Player.anims[JUMP] = Animation(1, 0, 0.0);
	Player.anims[FALL] = Animation(6, 0, 0.0);
	Player.changeAnim(IDLE);



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

		if (Player.move(frameTime, gameMap))
		{
			mainWindow.follow((int)Player.position.x, (int)Player.position.y, gameMap.horiTiles*gameMap.tileRes, gameMap.vertiTiles*gameMap.tileRes);
			gameMap.update(&mainWindow);
		}
		Player.animate(frameTime);
	
		//rendering block
		SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 0, 255);
		SDL_RenderClear(mainWindow.ren);

		gameMap.render(&mainWindow);

		//if (checkMapCollision(Player, gameMap)) SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 255, 255);
		//else SDL_SetRenderDrawColor(mainWindow.ren, 255, 0, 0, 255);

		Player.render(mainWindow);
		SDL_RenderPresent(mainWindow.ren);

		SDL_Delay(1);
	}

	close();
}