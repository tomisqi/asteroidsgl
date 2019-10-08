#pragma once

#define InvalidCodePath assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

typedef int GUID;

void GameStart(int screenWidth, int screenHeight);
void GameUpdate();
