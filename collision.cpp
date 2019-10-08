#include <assert.h>
#include "collision.h"
#include "debugrender.h"

#define MAX_COLLISION_LAYERS	3
#define MAX_COLLIDERS			64

#define COLLISION_CIRLE_CIRCLE	0x1
#define COLLISION_BOX_BOX		0x2
#define COLLISION_CIRCLE_BOX	0x3

struct Collisions
{
	Collider* colliders[MAX_COLLIDERS];
	int collidersCount;
	int matrix[MAX_COLLISION_LAYERS][MAX_COLLISION_LAYERS];
};

Collisions collisions;

static bool CircleCircleCollision(Collider* collider1, Collider* collider2);

void Collsions_Init(int collisionMatrix[][3], int collisionLayers)
{
	assert(collisionLayers <= MAX_COLLISION_LAYERS);
	collisions = { 0 };
	collisions.collidersCount = 0;
	for (int i = 0; i < collisionLayers; i++)
	{
		for (int j = 0; j < collisionLayers; j++)
		{
			collisions.matrix[i][j] = collisionMatrix[i][j];
		}
	}
}

void Collsions_NewFrame()
{
	collisions.collidersCount = 0;
}

void Collisions_AddCollider(Collider* collider)
{
	int count = collisions.collidersCount;
	collisions.colliders[count] = collider;
	collisions.collidersCount = count + 1;
}

void Collisions_DebugShowColliders()
{
	for (int i = 0; i < collisions.collidersCount; i++)
	{
		Collider* collider = collisions.colliders[i];
		switch (collider->colliderType)
		{
		case COLLIDER_CIRCLE:
			Debug_DrawCircle(*collider->posRef + collider->circle.localPos, collider->circle.radius, COLOR_GREEN);
			break;
		case COLLIDER_BOX:  // falling through on purpose until implemented...
		InvalidDefaultCase;
		}
	}
}

void Collisions_CheckCollisions()
{
	for (int i = 0; i < collisions.collidersCount; i++)
	{
		Collider* collider1 = collisions.colliders[i];
		for (int j = i + 1; j < collisions.collidersCount; j++)
		{
			Collider* collider2 = collisions.colliders[j];
			int collisionType = collider1->colliderType | collider2->colliderType;
			bool collisionAllowed = collisions.matrix[collider1->layer][collider2->layer];
			switch (collisionType)
			{
			case COLLISION_CIRLE_CIRCLE:
				if (collisionAllowed && CircleCircleCollision(collider1, collider2))
				{
					(*collider1->collisionCallback)(collider1, collider2);
					(*collider2->collisionCallback)(collider2, collider1);
				}
				break;
			case COLLISION_BOX_BOX:
			case COLLISION_CIRCLE_BOX: // falling through on purpose until implemented...
			InvalidDefaultCase;
			}
		}
	}
}

static bool CircleCircleCollision(Collider* collider1, Collider* collider2)
{
	Vector2 pos1 = *collider1->posRef + collider1->circle.localPos;
	Vector2 pos2 = *collider2->posRef + collider2->circle.localPos;

	float radius1 = collider1->circle.radius;
	float radius2 = collider2->circle.radius;

	return Distance(pos1, pos2) < (radius1 + radius2);
}