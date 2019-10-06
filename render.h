#pragma once
#include "vector.h"
#include "color.h"

void Renderer_Init(int maxVertCount);
void Renderer_NewFrame();
void Renderer_Render();

void DrawCircle(Vector2 pos, float radius, Color color, int edgeCount = 8);
void DrawCircleWStartAngle(Vector2 pos, float radius, Color color, int edgeCount, float startAngle);
void DrawTriangle(Vector2 point1, Vector2 point2, Vector2 point3, Color color);
void DrawVectorImmediate(Vector2 v, Vector2 pos);