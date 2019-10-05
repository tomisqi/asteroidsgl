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

void RenderJob_Init(int maxTriangleCount, int maxCircleCount, int maxRectCount, int maxVectorCount);

void RenderJob_NewFrame();

void RenderJob_Render();

void* RenderJob_Push(RenderShape shape);