#pragma once
#include "vector.h"
#include "color.h"

enum RenderShape
{
	TRIANGLE,
	CIRCLE,
	RECT,
	VECTOR,
	MAX_SHAPES,
};

struct Circle
{
	Vector2 pos;
	float radius;
	Color color;
};

struct Triangle
{
	Vector2 point1;
	Vector2 point2;
	Vector2 point3;
	Color color;
};

struct Rect
{
	Vector2 pos;
	Vector2 size;
	Color color;
};

struct PosVector2
{
	Vector2 pos;
	Vector2 vector;
};

void Renderer_Init(int maxVertCount);

void Renderer_NewFrame();

void Renderer_Render();

void DrawCircle(Vector2 pos, float radius, Color color, int edgeCount = 8);

void DrawTriangle(Vector2 point1, Vector2 point2, Vector2 point3, Color color);

void DrawVectorImmediate(Vector2 v, Vector2 pos);