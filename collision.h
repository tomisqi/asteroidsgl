#pragma once
#include "asteroids.h"
#include "vector.h"
#include "rect.h"
#include "guid.h"


enum ColliderType
{
	COLLIDER_CIRCLE =	0x1 << 0,
	COLLIDER_BOX =		0x1 << 1,
};


struct Collider
{
	ColliderType colliderType;
	GUID guid;
	Vector2* posRef;
	int layer;
	union
	{
		struct
		{
			Vector2 localPos;
			float radius;
		} circle;
		struct
		{
			Rect localRect;
		} box;
	};
	void (*collisionCallback)(Collider*, Collider*);
};

void Collisions_Init(int collisionMatrix[][3], int collisionLayers);
void Collisions_Clear();
void Collisions_NewFrame();
void Collisions_AddCollider(Collider* collider);
void Collisions_CheckCollisions();
void Collisions_DebugShowColliders();