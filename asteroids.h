#pragma once

#define InvalidCodePath assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

void GameStart(int screenWidth, int screenHeight);
void GameUpdate();
