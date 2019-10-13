#pragma once
#include "color.h"

void DebugRenderer_Init(int maxVertCount);
void DebugRenderer_NewFrame();
void DebugRenderer_Render();

void Debug_DrawVector(Vector2 v, Vector2 pos, Color32 color32);
void Debug_DrawRect(struct Rect rect, Color32 color32 = COL32_WHITE);
void Debug_DrawCross(Vector2 pos, Color32 color32 = COL32_WHITE);
void Debug_DrawCircle(Vector2 pos, float radius, Color32 color32 = COL32_WHITE);