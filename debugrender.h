#pragma once

void DebugRenderer_Init(int maxVertCount);
void DebugRenderer_NewFrame();
void DebugRenderer_Render();

void Debug_DrawVector(Vector2 v, Vector2 pos, Color color);
void Debug_DrawRect(struct Rect rect, Color color = COLOR_WHITE);
void Debug_DrawCross(Vector2 pos, Color color = COLOR_WHITE);