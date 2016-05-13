#include "Window.h"
#include <iostream>
#include <chrono>
#include <vector>

#ifdef main
#undef main
#endif

using namespace std;
using namespace std::chrono;

const int SCREEN_WIDTH = 1000;
const int SCREEN_HEIGHT = 800;
Window mainWindow;

struct doubleVector
{
	double x;
	double y;
};

enum Direction
{
	NONE,
	LEFT,
	RIGHT,
	DOWN,
	UP
};

class Character
{
public:
	Character();
	void move(double deltaTime);

	doubleVector velocity;
	double gravity;
	double runSpeed;
	double airSpeed;
	double jumpVelocity;
	double jumpHeight;
	double jumpHeightMax;
	doubleVector position;
	doubleVector origin;

	SDL_Rect rect;

	bool airBorne;
	bool freeFall;
	Direction primDirection;
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
	rect.x = 0;
	rect.y = 0;
	rect.w = 0;
	rect.h = 0;
	airBorne = false;
	freeFall = false;
	primDirection = NONE;
}

void Character::move(double deltaTime)
{
	if (airBorne)
	{
		position.y += velocity.y * deltaTime;
		if (freeFall)
		{
			velocity.y -= gravity * deltaTime;
		}
		else
		{
			jumpHeight += velocity.y * deltaTime;
			if (jumpHeight >= jumpHeightMax)
			{
				position.y -= jumpHeight - jumpHeightMax;

				jumpHeight = 0.0;
				freeFall = true;
			}
		}
	}



	position.x += velocity.x * deltaTime;

	rect.x = int(position.x - origin.x + 0.5);
	rect.y = SCREEN_HEIGHT - int(position.y - origin.y + 0.5);
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


	return true;
}

void close()
{
	SDL_Quit();
}

int main()
{
	init();

	Character Player;
	Player.position.x = 100;
	Player.position.y = 0;
	Player.gravity = 5000.0;
	Player.runSpeed = 500.0;
	Player.jumpVelocity = 1200.0;
	Player.jumpHeightMax = 200;

	Player.rect.w = 32;
	Player.rect.h = 64;
	Player.origin.x = (double)(Player.rect.w / 2);
	Player.origin.y = -(double)Player.rect.h;

	Player.rect.x = int(Player.position.x + 0.5);
	Player.rect.y = int(Player.position.y + 0.5);

	const Uint8 *keystate = SDL_GetKeyboardState(NULL);

	double frameTime;
	system_clock::time_point lastTime = system_clock::now();
	while (true)
	{
		frameTime = duration_cast<microseconds>(system_clock::now() - lastTime).count() / 1000000.0;
		if (frameTime > 0.1) frameTime = 0.1;	//at low framerates game becomes frame dependent to avoid collision errors etc.
		lastTime = system_clock::now();


		SDL_PumpEvents();

		if (keystate[SDL_SCANCODE_RIGHT] && !keystate[SDL_SCANCODE_LEFT]) Player.velocity.x = Player.runSpeed;
		else if (keystate[SDL_SCANCODE_LEFT] && !keystate[SDL_SCANCODE_RIGHT]) Player.velocity.x = -Player.runSpeed;
		else if (!keystate[SDL_SCANCODE_LEFT] && !keystate[SDL_SCANCODE_RIGHT]) Player.velocity.x = 0.0;



		if (keystate[SDL_SCANCODE_UP] && !Player.freeFall)
		{
			Player.velocity.y = Player.jumpVelocity;
			Player.airBorne = true;
		}
		else if (Player.airBorne)
		{
			Player.freeFall = true;
		}

		Player.move(frameTime);

		if (Player.position.y <= 0.0)
		{
			Player.airBorne = false;
			Player.freeFall = false;
			Player.position.y = 0.0;
		}



		SDL_SetRenderDrawColor(mainWindow.ren, 0, 0, 0, 255);
		SDL_RenderClear(mainWindow.ren);

		SDL_SetRenderDrawColor(mainWindow.ren, 255, 0, 0, 255);
		SDL_RenderFillRect(mainWindow.ren, &Player.rect);
		SDL_RenderPresent(mainWindow.ren);

		//SDL_Delay(1);
	}

	close();
}
//boo