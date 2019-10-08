#include <stdio.h>
#include <assert.h>
#include "asteroids.h"
#include "input.h"
#include "utils.h"
#include "render.h"
#include "rect.h"
#include "debugrender.h"
#include "collision.h"

#define FIXED_DELTA_TIME		(1.0f/60.0f)

struct CircleEntity
{
	GUID guid;
	Vector2 pos;
	Vector2 vel;

	float radius;
};

typedef CircleEntity ExhaustParticles;

struct Ship
{
	GUID guid;
	Vector2 pos;
	Collider collider;

	Vector2 vel;
	Vector2 facing;
	int exhaustIdx;
	Vector2 size;
};

struct Bullet
{
	GUID guid;
	Vector2 pos;
	Collider collider;

	float radius;
	Vector2 vel;
	double tDestroy;
};

struct Asteroid
{
	GUID guid;
	Vector2 pos;
	Collider collider;

	float radius;
	Vector2 vel;
	int edges;
	float rot;
	float rotSpeed;
};

enum EntityType
{
	SHIP = 1,
	BULLET,
	EXHAUST_PARTICLE,
	ASTEROID,
	ENTITIES_TYPE_MAX,
};

#define EXHAUST_PARTICLES_MAX	32
#define BULLETS_MAX				16
#define ASTEROIDS_MAX			64
#define ASTEROID_MIN_SPEED		60
#define ASTEROID_MAX_SPEED		ASTEROID_MIN_SPEED + 40
#define BULLET_LIFETIME			1
struct Entities
{
	Ship ship;
	Bullet bullets[BULLETS_MAX];
	int bulletCount;
	Asteroid asteroids[ASTEROIDS_MAX];
	int asteroidsCount;

	ExhaustParticles exhaustParticles[EXHAUST_PARTICLES_MAX];
};

struct GuidDescriptor
{
	EntityType entityType;
	void* data;
};

#define MAX_ENTITIES  (1/*ship*/ + BULLETS_MAX + ASTEROIDS_MAX + EXHAUST_PARTICLES_MAX + 1/*dummy*/)
struct Game
{
	Rect screenRect;
	int level;
	double tCurr;
	double tLastSpawn;
	double spawnInterval;
	GuidDescriptor guidTable[MAX_ENTITIES];
};


// Globals
static Entities entities;
static Game game;
static GUID gguid = 1;

static void SpawnAsteroids(int count, Asteroid* asteroids_p);
static void DestroyOldBullets(Bullet* bullets_p);
static void DestroyOffScreenAsteroids(Asteroid* asteroids_p);
static void ShipCollision(Collider* collider, Collider* otherCollider);
static void AsteroidCollision(Collider* collider, Collider* otherCollider);
static void BulletCollision(Collider* collider, Collider* otherCollider);

static GUID GetNextGUID()
{
	return gguid++;
}

static GUID AddToGUIDTable(EntityType entityType, void* entityData)
{
	GuidDescriptor desc = { entityType, entityData};
	GUID guid = GetNextGUID();
	assert(guid < MAX_ENTITIES);
	assert(game.guidTable[guid].data == NULL);//making sure we are not overwriting
	game.guidTable[guid] = desc;
	return guid;
}

static void SwapGuidDescriptors(GUID guid1, GUID guid2)
{
	GuidDescriptor tmp = game.guidTable[guid1];
	game.guidTable[guid1] = game.guidTable[guid2];
	game.guidTable[guid2] = tmp;
}

static void EntitiesInit()
{
	entities = {0};

	Ship* ship_p = &entities.ship;
	ship_p->guid = ship_p->collider.guid = AddToGUIDTable(SHIP, ship_p);
	ship_p->facing = VECTOR2_UP;
	ship_p->pos = 500.0f * VECTOR2_ONE;
	ship_p->size = 30.0f * V2(1.0f, 1.2f);
	ship_p->vel = VECTOR2_ZERO;
	ship_p->collider.colliderType = COLLIDER_BOX;
	ship_p->collider.posRef = &ship_p->pos;
	ship_p->collider.box.localRect = RectNew(ship_p->pos - 0.5f*ship_p->size, 0.5f*ship_p->size);
	ship_p->collider.collisionCallback = &ShipCollision;
	ship_p->collider.layer = 0;

	Bullet* bullets_p = &entities.bullets[0];
	Bullet bullet; bullet.radius = 8.0f; bullet.pos = -10.0 * VECTOR2_ONE;
	bullet.collider.colliderType = COLLIDER_CIRCLE;
	bullet.collider.circle.localPos = VECTOR2_ZERO;
	bullet.collider.circle.radius = bullet.radius;
	bullet.collider.collisionCallback = &BulletCollision;
	bullet.collider.layer = 1;
	for (int i = 0; i < BULLETS_MAX; i++) 
	{
		bullet.collider.posRef = &bullets_p[i].pos;
		bullet.guid = bullet.collider.guid = AddToGUIDTable(BULLET, &bullets_p[i]);
		bullets_p[i] = bullet;
	}

	ExhaustParticles* exhaustParticles_p = &entities.exhaustParticles[0];
	ExhaustParticles exhaust; exhaust.radius = 5.0f; exhaust.pos = -10.0 * VECTOR2_ONE;
	for (int i = 0; i < EXHAUST_PARTICLES_MAX; i++) 
	{ 
		exhaust.guid = AddToGUIDTable(EXHAUST_PARTICLE, &exhaustParticles_p[i]);;
		exhaustParticles_p[i] = exhaust; 
	}

	Asteroid* asteroids_p = &entities.asteroids[0];
	Asteroid asteroid; asteroid.radius = 7.0f; asteroid.pos = -10.0 * VECTOR2_ONE; asteroid.rot = 0.0f;
	asteroid.collider.colliderType = COLLIDER_CIRCLE;
	asteroid.collider.circle.localPos = VECTOR2_ZERO;
	asteroid.collider.circle.radius = asteroid.radius;
	asteroid.collider.collisionCallback = &AsteroidCollision;
	asteroid.collider.layer = 2;
	for (int i = 0; i < ASTEROIDS_MAX; i++) 
	{ 
		asteroid.collider.posRef = &asteroids_p[i].pos;
		asteroid.guid = asteroid.collider.guid = AddToGUIDTable(ASTEROID, &asteroids_p[i]);
		asteroids_p[i] = asteroid; 
	}

	entities.bulletCount = 0;
	entities.asteroidsCount = 0;
}

static void GameInit(int screenWidth, int screenHeight)
{
	game = { 0 };
	game.level = 0;
	game.tCurr = 0.0f;
	game.spawnInterval = 1.0f;

	game.screenRect = RectNew(VECTOR2_ZERO, V2(screenWidth, screenHeight));

	// first guid in the table is a dummy
	GuidDescriptor dummyDesc = { ENTITIES_TYPE_MAX, NULL };
	game.guidTable[0] = dummyDesc;
}

void GameStart(int screenWidth, int screenHeight)
{
	GameInit(screenWidth, screenHeight);
	EntitiesInit();
								 //              Ship     Bullets  Asteroids
	int collisionMatrix[3][3] = {/*Ship*/		{0,       0,       1,},
								 /*Bullets*/	{0,       0,       1,},
								 /*Asteroids*/	{1,       1,       0,}, };
	Collsions_Init(collisionMatrix, 3);

	// sanity check for the guidTable
	for (int i = 1; i < MAX_ENTITIES; i++)
	{
		void* data = game.guidTable[i].data; 
		assert(data != NULL);						// should not be null
		for (int j = i + 1; j < MAX_ENTITIES; j++)
		{
			assert(data != game.guidTable[j].data); // ... and should be different from others 
		}
	}
}

void GameUpdate()
{
	Ship* ship_p = &entities.ship;
	Bullet* bullets_p = &entities.bullets[0];
	ExhaustParticles* exhaustParticles_p = &entities.exhaustParticles[0];
	Asteroid* asteroids_p = &entities.asteroids[0];

	float shipRotSpeed = 0.0f;
	float shipSpeed = 0.0f;
	bool shoot = false;

	/// --- Read input ---
	if (GameInput_Button(BUTTON_RIGHT_ARROW)) shipRotSpeed = -5.0f;
	if (GameInput_Button(BUTTON_LEFT_ARROW)) shipRotSpeed = 5.0f;
	if (GameInput_Button(BUTTON_UP_ARROW)) shipSpeed = 5.0f;
	if (GameInput_ButtonDown(BUTTON_SPACE)) shoot = true;

	/// --- Handle collisions ---
	Collisions_DebugShowColliders();
	Collisions_CheckCollisions();
	Collsions_NewFrame();

	/// --- Destroying ---
	DestroyOldBullets(bullets_p);
	DestroyOffScreenAsteroids(asteroids_p);

	/// --- Spawning ---
	if ((game.tCurr - game.tLastSpawn) > game.spawnInterval)
	{
		SpawnAsteroids(1, asteroids_p);
		game.tLastSpawn = game.tCurr;
	}
	
	/// --- Physics ---

	//Ship
	ship_p->facing = Rotate(ship_p->facing, shipRotSpeed);
	ship_p->vel += shipSpeed * ship_p->facing;
	if (shipSpeed == 0.0f) 	ship_p->vel = 0.99f * ship_p->vel;
	ship_p->pos += FIXED_DELTA_TIME * ship_p->vel;
	ship_p->pos.x = Wrapf(ship_p->pos.x, 0.0f, game.screenRect.size.x);
	ship_p->pos.y = Wrapf(ship_p->pos.y, 0.0f, game.screenRect.size.y);
	//Collisions_AddCollider(&ship_p->collider);

	// Bullets
	if (shoot)
	{
		assert(entities.bulletCount < BULLETS_MAX);
		Bullet* bullet_p = &bullets_p[entities.bulletCount];
		bullet_p->vel = 500.0f * ship_p->facing + ship_p->vel;
		bullet_p->pos = ship_p->pos + ship_p->size.y*ship_p->facing;
		bullet_p->tDestroy = game.tCurr + BULLET_LIFETIME;
		entities.bulletCount++;
	}
	for (int i = 0; i < entities.bulletCount; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		bullet_p->pos += FIXED_DELTA_TIME * bullet_p->vel;
		bullet_p->pos.x = Wrapf(bullet_p->pos.x, 0.0f, game.screenRect.size.x);
		bullet_p->pos.y = Wrapf(bullet_p->pos.y, 0.0f, game.screenRect.size.y);
		Collisions_AddCollider(&bullet_p->collider);
	}

	// Exhaust particles
	if (shipSpeed > 0.0f)
	{
		ExhaustParticles* exhaustParticle_p = &exhaustParticles_p[ship_p->exhaustIdx];
		exhaustParticle_p->vel = -50.0f * ship_p->facing;
		exhaustParticle_p->vel = Rotate(exhaustParticle_p->vel, GetRandomValue(-45, 45));
		exhaustParticle_p->pos = ship_p->pos;
		ship_p->exhaustIdx = (ship_p->exhaustIdx + 1) % EXHAUST_PARTICLES_MAX;
	}
	for (int i = 0; i < EXHAUST_PARTICLES_MAX; i++)
	{
		ExhaustParticles* exhaustParticle_p = &exhaustParticles_p[i];
		exhaustParticle_p->pos += FIXED_DELTA_TIME * exhaustParticle_p->vel;
	}

	// Asteroids
	for (int i = 0; i < entities.asteroidsCount; i++)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		asteroid_p->pos += FIXED_DELTA_TIME * asteroid_p->vel;
		asteroid_p->rot += FIXED_DELTA_TIME * asteroid_p->rotSpeed;
		Collisions_AddCollider(&asteroid_p->collider);
	}

	/// --- Drawing --- 

	// Ship
	Vector2 point1 = ship_p->pos + ship_p->size.y*ship_p->facing;
	Vector2 point2 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, 90.0f);
	Vector2 point3 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, -90.0f);
	DrawTriangle(point1, point2, point3, Col(20, 89, 255));

	// Bullets
	for (int i = 0; i < entities.bulletCount; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		DrawCircle(bullet_p->pos, bullet_p->radius, COLOR_MAGENTA);
	}

	// ExhaustParticles
	for (int i = 0; i < EXHAUST_PARTICLES_MAX; i++)
	{
		ExhaustParticles* exhaustParticle_p = &exhaustParticles_p[i];
		DrawCircle(exhaustParticle_p->pos, exhaustParticle_p->radius, COLOR_WHITE, 4);
	}

	// Asteroids
	for (int i = 0; i < entities.asteroidsCount; i++)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		DrawCircleWStartAngle(asteroid_p->pos, asteroid_p->radius, Col(255, 119, 0), asteroid_p->edges, asteroid_p->rot);
		//Debug_DrawVector(50.0f*Normalize(asteroid_p->vel), asteroid_p->pos, COLOR_GREEN);
	}

	Debug_DrawVector(50.0f*ship_p->facing, ship_p->pos, COLOR_GREEN);

	game.tCurr += FIXED_DELTA_TIME;
}

static void SpawnAsteroids(int count, Asteroid* asteroids_p)
{
	Rect screenRect = game.screenRect;
	assert(entities.asteroidsCount + count <= ASTEROIDS_MAX);
	Vector2 randPos = V2(GetRandomValue(100.0f, screenRect.size.x - 100.0f), GetRandomValue(100.0f, screenRect.size.y - 100.0f));
	Vector2 spawnPoints[4] = {	V2(randPos.x, -10.0f),
								V2(randPos.x, screenRect.size.y + 10.0f),
								V2(-10.0f, randPos.y), 
								V2(screenRect.size.x + 10.0f, randPos.y) };
	int spawnIdx = GetRandomValue(0, 3);
	int destIdx = (spawnIdx + GetRandomValue(1, 3)) % 4;
	for (int i = entities.asteroidsCount; i < entities.asteroidsCount + count; i++)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		Vector2 spawnPoint = spawnPoints[spawnIdx];
		Vector2 destPoint = spawnPoints[destIdx];
		asteroid_p->pos = spawnPoint;
		asteroid_p->radius = GetRandomValue(20, 70);
		asteroid_p->rotSpeed = GetRandomSign()*GetRandomValue(45, 75);
		asteroid_p->vel = GetRandomValue(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED) * Normalize(destPoint - spawnPoint);
		asteroid_p->edges = GetRandomValue(5, 9);
		asteroid_p->collider.circle.radius = 0.8f*asteroid_p->radius;
		spawnIdx = (spawnIdx + 1) % 4;
		destIdx = (destIdx + 1) % 4;
	}

	entities.asteroidsCount += count;
}

static void DestroyOldBullets(Bullet* bullets_p)
{
	int bulletCount = entities.bulletCount;

	for (int i = entities.bulletCount - 1; i >= 0; i--)
	{
		Bullet* bullet_p = &bullets_p[i];
		if (game.tCurr > bullet_p->tDestroy)
		{
			// Move it past bulletCount
			Bullet tmp = bullets_p[i];
			bullets_p[i] = bullets_p[bulletCount - 1];
			bullets_p[bulletCount - 1] = tmp;

			// keep correct ref to pos and guid
			bullets_p[i].collider.posRef = &bullets_p[i].pos;
			bullets_p[bulletCount - 1].collider.posRef = &bullets_p[bulletCount - 1].pos;
			SwapGuidDescriptors(bullets_p[i].guid, bullets_p[bulletCount - 1].guid);

			bulletCount--;
		}
	}
	assert(bulletCount >= 0);
	entities.bulletCount = bulletCount;
}

static void DestroyOffScreenAsteroids(Asteroid* asteroids_p)
{
	Vector2 screenCenter = RectCenter(game.screenRect);

	int asteroidCount = entities.asteroidsCount;
	for (int i = entities.asteroidsCount - 1; i >= 0; i--)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		bool offScreenAndMovingAway = false;

		if (!RectContains(game.screenRect, asteroid_p->pos))	
		{
			Vector2 velNorm = Normalize(asteroid_p->vel);
			Vector2 toCenterNorm = Normalize(screenCenter - asteroid_p->pos);
			if (Dot(velNorm, toCenterNorm) < 0.0f)
			{			
				offScreenAndMovingAway = true;
			}
		}

		if (offScreenAndMovingAway)
		{
			// Move it past asteroidCount
			Asteroid tmp = asteroids_p[i];
			asteroids_p[i] = asteroids_p[asteroidCount - 1];
			asteroids_p[asteroidCount - 1] = tmp;

			// keep correct ref to pos and guid
			asteroids_p[i].collider.posRef = &asteroids_p[i].pos;
			asteroids_p[asteroidCount - 1].collider.posRef = &asteroids_p[asteroidCount - 1].pos;
			SwapGuidDescriptors(asteroids_p[i].guid, asteroids_p[asteroidCount - 1].guid);

			asteroidCount--;
		}
	}
	assert(asteroidCount >= 0);
	entities.asteroidsCount = asteroidCount;
}

static void ShipCollision(Collider* collider, Collider* otherCollider)
{
	printf("Ship collision! collider.guidRef=%d otherCollider.guidRef=%d\n", collider->guid, otherCollider->guid);
}

static void AsteroidCollision(Collider* collider, Collider* otherCollider)
{
	printf("Asteroid collision! collider.guidRef=%d otherCollider.guidRef=%d\n", collider->guid, otherCollider->guid);
	GuidDescriptor desc = game.guidTable[collider->guid];
	GuidDescriptor otherDesc = game.guidTable[otherCollider->guid];

	assert(desc.entityType == ASTEROID);
	Asteroid* asteroid_p = (Asteroid*)desc.data;

	switch (otherDesc.entityType)
	{
	case BULLET:
		asteroid_p->rotSpeed = 0; // TODO
		break;
	InvalidDefaultCase;
	}
}

static void BulletCollision(Collider* collider, Collider* otherCollider)
{
	printf("Bullet collision! collider.guidRef=%d otherCollider.guidRef=%d\n", collider->guid, otherCollider->guid);
}