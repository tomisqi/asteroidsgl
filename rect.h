#pragma once
#include "vector.h"

struct Rect
{
	Vector2 pos;
	Vector2 size;
};

static inline Rect RectNew(Vector2 pos, Vector2 size)
{
	Rect rect = { pos, size };
	return rect;
}

static inline bool RectContains(Rect rect, Vector2 pos)
{
	return (pos.x >= rect.pos.x) && 
		   (pos.y >= rect.pos.y) && 
		   (pos.x < rect.pos.x + rect.size.x) && 
		   (pos.y < rect.pos.y + rect.size.y);
}

static inline Vector2 RectBottomLeft(Rect rect)
{
	return rect.pos;
}

static inline Vector2 RectBottomRight(Rect rect)
{
	return rect.pos + V2(0.0f, rect.size.y);
}

static inline Vector2 RectTopLeft(Rect rect)
{
	return rect.pos + V2(rect.size.x, 0.0f);
}

static inline Vector2 RectTopRight(Rect rect)
{
	return rect.pos + rect.size;
}

static inline Vector2 RectCenter(Rect rect)
{
	Vector2 center = V2(rect.pos.x + rect.size.x / 2, rect.pos.y + rect.size.y / 2);
	return center;
}