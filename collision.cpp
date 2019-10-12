#include <assert.h>
#include "collision.h"
#include "debugrender.h"

#define MAX_COLLISION_LAYERS	3
#define MAX_COLLIDERS			64

#define COLLISION_CIRLE_CIRCLE	0x1
#define COLLISION_BOX_BOX		0x2
#define COLLISION_CIRCLE_BOX	0x3

typedef unsigned int CID; // Collider ID

struct ColliderData
{
	CID cID;
	Collider* collider;
};

struct Collisions
{
	CID gcID;
	ColliderData colliders[MAX_COLLIDERS];
	int collidersCount;

	int matrix[MAX_COLLISION_LAYERS][MAX_COLLISION_LAYERS];
};

struct CallbackData
{
	void(*func)(Collider*, Collider*);
	Collider* collider1;
	Collider* collider2;
};

struct CollisionCallback
{
	CallbackData queue[MAX_COLLIDERS];
	int count;
	unsigned int callbackInQueueBm[MAX_COLLIDERS / 32];
};
static Collisions collisions;
static CollisionCallback collisionCallback;

static bool InCallbackQueue(ColliderData collider);
static void AddToCallbackQueue(ColliderData collider1, ColliderData collider2);
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
	collisionCallback.count = 0;
}

void Collsions_NewFrame()
{
	collisions.gcID = 0;
	collisions.collidersCount = 0;
	collisionCallback.count = 0;
	for (int i = 0; i < MAX_COLLIDERS / 32; i++)
	{
		collisionCallback.callbackInQueueBm[i] = 0U;
	}
}

void Collisions_AddCollider(Collider* collider)
{
	int count = collisions.collidersCount;
	collisions.colliders[count].cID = collisions.gcID++;
	collisions.colliders[count].collider = collider;
	collisions.collidersCount = count + 1;
}

void Collisions_DebugShowColliders()
{
	for (int i = 0; i < collisions.collidersCount; i++)
	{
		Collider* collider = collisions.colliders[i].collider;
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
		ColliderData collider1 = collisions.colliders[i];
		for (int j = i + 1; j < collisions.collidersCount; j++)
		{
			ColliderData collider2 = collisions.colliders[j];
			bool collisionAllowed = collisions.matrix[collider1.collider->layer][collider2.collider->layer];
			if (collisionAllowed)
			{
				int collisionType = collider1.collider->colliderType | collider2.collider->colliderType;
				switch (collisionType)
				{
				case COLLISION_CIRLE_CIRCLE:
					if (CircleCircleCollision(collider1.collider, collider2.collider))
					{
						// Add to Queue (dont call callback in here, since we usually destroy things at calllback and this is a loop)
						if (!InCallbackQueue(collider1)) AddToCallbackQueue(collider1, collider2);
						if (!InCallbackQueue(collider2)) AddToCallbackQueue(collider2, collider1);
					}
					break;
				case COLLISION_BOX_BOX:
				case COLLISION_CIRCLE_BOX: // falling through on purpose until implemented...
					InvalidDefaultCase;
				}
			}
		}
	}

	// Call callback functions (if any)
	for (int i = 0; i < collisionCallback.count; i++)
	{
		Collider* collider1 = collisionCallback.queue[i].collider1;
		Collider* collider2 = collisionCallback.queue[i].collider2;
		(*collisionCallback.queue[i].func)(collider1, collider2);
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

static bool InCallbackQueue(ColliderData collider)
{
	unsigned int bm = collisionCallback.callbackInQueueBm[collider.cID / 32];
	return (bm & (1 << collider.cID % 32)) > 0;	
}

static void AddToCallbackQueue(ColliderData collider1, ColliderData collider2)
{
	CallbackData data = { collider1.collider->collisionCallback , collider1.collider, collider2.collider};
	collisionCallback.queue[collisionCallback.count++] = data;
	collisionCallback.callbackInQueueBm[collider1.cID / 32] |= 1U << (collider1.cID % 32);
	assert(collisionCallback.count <= MAX_COLLIDERS);
}