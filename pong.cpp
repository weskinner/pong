#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL_ttf.h"
#include <iostream>
using namespace std;

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;
const int SCREEN_BPP = 32;

const int FRAMES_PER_SECOND = 30;

const int PADDLE_HEIGHT = 80;
const int PADDLE_WIDTH = 20;
const int PADDLE_SPEED = 8;

const int BALL_WIDTH = 20;
const int BALL_HEIGHT = 20;
const int BALL_SPEED = 8;

SDL_Surface *screen;
TTF_Font *font = NULL;
SDL_Event event;

SDL_Color fontColor = {0xFF,0xFF,0xFF};

bool init();
bool clean_up();

class Paddle
{
public:
	int x,y,xVel,yVel;
	Paddle();
	void show();
	void move();
	void handle_input();
};

class Opponent
{
public:
	int x,y,xVel,yVel;
	Opponent();
	void move(int ballX, int ballY);
	void show();
};

class Ball
{
public:
	int x,y,xVel,yVel;
	SDL_Surface *skin;
	Ball();
	void show();
	void move(Paddle*,Opponent*);
	bool opp_win();
	bool player_win();
};

class Game
{
public:
	Paddle me;
	Opponent opp;
	Ball ball;
	string message;
	SDL_Surface *messageSurface;

	Game();

	void handle_input();
	void move();
	void render();
};

int main(int argc, char *argv[])
{
	bool quit = false;

	if(!init())
	{
		return 1;
	}

	Game game;

	//game loop
	Uint32 startTime;
	Uint32 currentTime;
	const int timePerFrame = 1000 / FRAMES_PER_SECOND;
	while(quit == false)
	{
		startTime = SDL_GetTicks();

		// events
		if(SDL_PollEvent(&event))
		{
			game.handle_input();

			if(event.type == SDL_QUIT)
			{
				quit = true;
			}
		}

		// frame logic
		game.move();

		// render
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

		game.render();

		if(SDL_Flip(screen) == -1)
		{
			return 1;
		}

		// fps regulation
		currentTime = SDL_GetTicks() - startTime;
		if(currentTime < timePerFrame)
		{
			SDL_Delay(timePerFrame - currentTime);
		}
	}

	if(!clean_up())
	{
		return 1;
	}
	return 0;
}

bool init()
{
	if(SDL_Init(SDL_INIT_EVERYTHING) == -1)
	{
		cout << "Error in SDL_Init()";
		return false;
	}

	screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE);
	if(screen == NULL)
	{
		cout << "Error setting up screen in SDL_SetVideoMode()";
		return false;
	}

	TTF_Init();

	font = TTF_OpenFont("brewmastermodern.ttf",72);
	if(font == NULL)
	{
		cout << "Error loading font";
		return false;
	}

	return true;
}

bool clean_up()
{
	TTF_Quit();
	SDL_Quit();

	return true;
}

Game::Game()
{
	message = "";
}

void Game::handle_input()
{
	me.handle_input();
}

void Game::move()
{
	if(ball.player_win())
	{
		message = "Player Wins!";
	}
	else if(ball.opp_win())
	{
		message = "Opponent Wins.";
	}
	else
	{
		me.move();
		ball.move(&me,&opp);
		opp.move(ball.x, ball.y);	
	}
}

void Game::render()
{
	if(message != "")
	{
		messageSurface = TTF_RenderText_Solid(font, message.c_str(), fontColor);
		SDL_Rect messageLoc;
		messageLoc.x = SCREEN_WIDTH/2 - (messageSurface->w / 2);
		messageLoc.y = 100;
		SDL_BlitSurface(messageSurface, NULL, screen, &messageLoc);
	}

	ball.show();
	me.show();
	opp.show();
}

Ball::Ball()
{
	SDL_Surface *temp = IMG_Load("dot.bmp");
	skin = SDL_DisplayFormat(temp);
	SDL_FreeSurface(temp);
	SDL_SetColorKey(skin, SDL_SRCCOLORKEY, SDL_MapRGB(screen->format, 0, 0xFF, 0xFF));

	x = SCREEN_WIDTH - (SCREEN_WIDTH/4);
	y = (SCREEN_HEIGHT / 2) - (BALL_HEIGHT/2);
	xVel = -BALL_SPEED;
	yVel = BALL_SPEED;
}

void Ball::move(Paddle *me, Opponent *opp)
{
	// player collision
	if((y < me->y+PADDLE_HEIGHT && y > me->y) && (x < me->x + PADDLE_WIDTH))
	{
		xVel = -xVel;

		int paddleCenter = me->y + (PADDLE_HEIGHT/2);
		int paddleSectionSize = (PADDLE_HEIGHT/2)/3;
		// upper middle
		if(y <= paddleCenter - paddleSectionSize && y >= (paddleCenter - paddleSectionSize * 2))
		{
			yVel -= 1;
		}
		// upper
		else if(y < (paddleCenter - paddleSectionSize * 2))
		{
			yVel -= 2;
		}
		// lower middle
		else if(y >= paddleCenter + paddleSectionSize && y <= (paddleCenter + paddleSectionSize * 2))
		{
			xVel += 1;
		}
		// lower
		else if(y > (paddleCenter + paddleSectionSize * 2))
		{	
			xVel +=2;
		}
	}

	// opponent collision
	if((y < opp->y+PADDLE_HEIGHT && y > opp->y) && (x+BALL_WIDTH > opp->x))
	{
		xVel = -xVel;

		int paddleCenter = opp->y + (PADDLE_HEIGHT/2);
		int paddleSectionSize = (PADDLE_HEIGHT/2)/3;
		// upper middle
		if(y <= paddleCenter - paddleSectionSize && y >= (paddleCenter - paddleSectionSize * 2))
		{
			yVel -= 1;
		}
		// upper
		else if(y < (paddleCenter - paddleSectionSize * 2))
		{
			yVel -= 2;
		}
		// lower middle
		else if(y >= paddleCenter + paddleSectionSize && y <= (paddleCenter + paddleSectionSize * 2))
		{
			xVel += 1;
		}
		// lower
		else if(y > (paddleCenter + paddleSectionSize * 2))
		{	
			xVel +=2;
		}
	}

	// wall collisions
	
	if(y < 0)
	{
		yVel = -yVel;
	}
	if(y + BALL_HEIGHT > SCREEN_HEIGHT)
	{
		yVel = -yVel;
	}

	x += xVel;
	y += yVel;
}

bool Ball::opp_win()
{
	if(x < 0)
	{
		return true;
	}
	return false;
}

bool Ball::player_win()
{
	if(x + BALL_WIDTH > SCREEN_WIDTH)
	{
		return true;
	}
	return false;
}

void Ball::show()
{
	SDL_Rect ballPos;
	ballPos.x = x;
	ballPos.y = y;
	ballPos.w = BALL_WIDTH;
	ballPos.h = BALL_HEIGHT;
	SDL_BlitSurface(skin, NULL, screen, &ballPos);
}

Paddle::Paddle()
{
	x = 20;
	y = (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
	xVel = 0;
	yVel = 0;
}

void Paddle::handle_input()
{
	if(event.type == SDL_KEYDOWN)
	{
		switch(event.key.keysym.sym)
		{
			case SDLK_UP: yVel -= PADDLE_SPEED; break;
			case SDLK_DOWN: yVel += PADDLE_SPEED; break;
		}
	}
	else if(event.type == SDL_KEYUP)
	{
		switch(event.key.keysym.sym)
		{
			case SDLK_UP: yVel += PADDLE_SPEED; break;
			case SDLK_DOWN: yVel -= PADDLE_SPEED; break;
		}
	}
}

void Paddle::show()
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = PADDLE_WIDTH;
	rect.h = PADDLE_HEIGHT;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0x00,0x00,0xFF));
}

void Paddle::move()
{
	y += yVel;
	if(y < 0 || (y + PADDLE_HEIGHT) > SCREEN_HEIGHT)
	{
		y -= yVel;
	}
}

Opponent::Opponent()
{
	x = SCREEN_WIDTH - PADDLE_WIDTH - 20;
	y = (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2);
	xVel = 0;
	yVel = PADDLE_SPEED;
}

void Opponent::move(int ballX, int ballY)
{
	if(ballX > (SCREEN_WIDTH/2))
	{
		if(ballY + (BALL_HEIGHT/2) > y + (PADDLE_HEIGHT/2))
		{
			yVel = PADDLE_SPEED;
		}
		else
		{
			yVel = -PADDLE_SPEED;
		}	
	}
	else
	{
		if(y + PADDLE_HEIGHT/2 > (SCREEN_HEIGHT/2) + PADDLE_HEIGHT/2)
		{
			yVel = -PADDLE_SPEED;
		}
		else if(y + PADDLE_HEIGHT/2 < (SCREEN_HEIGHT/2) - PADDLE_HEIGHT/2)
		{
			yVel = PADDLE_SPEED;
		}
		else
		{
			yVel = 0;
		}
	}
	

	y += yVel;
	if(y < 0 || (y + PADDLE_HEIGHT) > SCREEN_HEIGHT)
	{
		y -= yVel;
	}
}

void Opponent::show()
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = PADDLE_WIDTH;
	rect.h = PADDLE_HEIGHT;
	SDL_FillRect(screen, &rect, SDL_MapRGB(screen->format, 0xFF,0x00,0x00));
}