#pragma once
#include "rect.h"

#define InvalidCodePath assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

enum SceneE
{
	MAIN_MENU,
	GAME,
};

struct Game
{
	SceneE scene;
	Rect screenRect;
	bool doQuit;
	float deltaT;
};

extern Game game;

void GameStart(int screenWidth, int screenHeight, float deltaT);
void GameUpdate();
