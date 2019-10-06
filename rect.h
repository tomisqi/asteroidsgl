#pragma once
#include "vector.h"

struct Rect
{
	Vector2 pos;
	Vector2 size;
	Vector2 center;
};

static inline Rect NewRect(Vector2 pos, Vector2 size)
{
	Vector2 center = V2(pos.x + size.x / 2, pos.y + size.y / 2);
	Rect rect = { pos, size, center};
	return rect;
}

static inline bool Contains(Rect rect, Vector2 pos)
{
	return (pos.x >= rect.pos.x) && 
		   (pos.y >= rect.pos.y) && 
		   (pos.x < rect.pos.x + rect.size.x) && 
		   (pos.y < rect.pos.y + rect.size.y);
}
