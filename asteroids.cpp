#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "asteroids.h"
#include "input.h"
#include "utils.h"
#include "render.h"
#include "rect.h"
#include "debugrender.h"
#include "collision.h"
#include "guid.h"

#define FIXED_DELTA_TIME		(1.0f/60.0f)

#define STARS_MAX 64
#define STARS_MIN_SIZE 2
#define STARS_MAX_SIZE 4
struct Star
{
	Vector2 pos;
	float size;
	Color32 color;
	bool fading;
};

struct Particle
{
	GUID guid;
	Vector2 pos;

	Vector2 vel;
	float radius;
	int circleEdges;
	Color32 color;
};

#define SHIP_SPEED 5.0f
#define BOOST_SPEED_FACTOR 2.5f
struct Ship
{
	GUID guid;
	Vector2 pos;
	Collider collider;

	Vector2 vel;
	Vector2 facing;
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
	Color32 color;
};

struct ParticleShare
{
	int index;
	int startIndex;
	int count;	
};

enum EntityType
{
	SHIP = 0,
	BULLET,
	ASTEROID,
	PARTICLE,
	//ENTITY_TYPE_MAX,
};

enum ParticleType
{
	EXHAUST_PARTICLE = 0,
	ASTEROID_PARTICLE,
	PARTICLE_TYPE_MAX,
};

#define PARTICLES_MAX			64
#define BULLETS_MAX				16
#define ASTEROIDS_MAX			64
#define ASTEROID_MIN_SPEED		60
#define ASTEROID_MAX_SPEED		ASTEROID_MIN_SPEED + 40
#define ASTEROID_MIN_SIZE		20
#define BULLET_LIFETIME			1
struct Entities
{
	Ship ship;
	Bullet bullets[BULLETS_MAX];
	int bulletCount;
	Asteroid asteroids[ASTEROIDS_MAX];
	int asteroidsCount;

	// Particles
	Particle particles[PARTICLES_MAX];
	ParticleShare particleShare[PARTICLE_TYPE_MAX];
	int particleCount;
};

#define MAX_ENTITIES  (1/*ship*/ + BULLETS_MAX + ASTEROIDS_MAX + PARTICLES_MAX + 1/*dummy*/)
struct Game
{
	Rect screenRect;
	int level;
	double tCurr;
	double tLastSpawn;
	double spawnInterval;
	unsigned long frameCounter;
	bool paused;
};


// Globals
static Entities entities;
static Game game;
static Star stars[STARS_MAX];

static void ReserveParticles(Entities* entities_p, ParticleType particleType, int count);
static Particle* GetParticle(Entities* entities_p, ParticleType particleType);
static void SpawnAsteroidsOffscreen(int count, Asteroid* asteroids_p);
static void DestroyOldBullets(Bullet* bullets_p);
static void DestroyOffScreenAsteroids(Asteroid* asteroids_p);
static void ShipCollision(Collider* collider, Collider* otherCollider);
static void AsteroidCollision(Collider* collider, Collider* otherCollider);
static void BulletCollision(Collider* collider, Collider* otherCollider);


static void InitParticles()
{
	memset(&entities.particleShare[0], 0, sizeof(entities.particleShare));
	entities.particleCount = 0;
}

static void EntitiesInit()
{
	entities = {0};

	Guid_Init(MAX_ENTITIES);

	InitParticles();

	Ship* ship_p = &entities.ship;
	ship_p->guid = ship_p->collider.guid = Guid_AddToGUIDTable(SHIP, ship_p);
	ship_p->facing = VECTOR2_UP;
	ship_p->pos = 500.0f * VECTOR2_ONE;
	ship_p->size = 30.0f * V2(1.0f, 1.2f);
	ship_p->vel = VECTOR2_ZERO;
	//ship_p->collider.colliderType = COLLIDER_BOX;	
	//ship_p->collider.box.localRect = RectNew(ship_p->pos - 0.5f*ship_p->size, 0.5f*ship_p->size);
	ship_p->collider.posRef = &ship_p->pos;
	ship_p->collider.colliderType = COLLIDER_CIRCLE;
	ship_p->collider.circle.localPos = VECTOR2_ZERO;
	ship_p->collider.circle.radius = 15.0f;
	ship_p->collider.collisionCallback = &ShipCollision;
	ship_p->collider.layer = 0;
	ReserveParticles(&entities, EXHAUST_PARTICLE, 32);

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
		bullet.guid = bullet.collider.guid = Guid_AddToGUIDTable(BULLET, &bullets_p[i]);
		bullets_p[i] = bullet;
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
		asteroid.guid = asteroid.collider.guid = Guid_AddToGUIDTable(ASTEROID, &asteroids_p[i]);
		asteroids_p[i] = asteroid; 
	}
	ReserveParticles(&entities, ASTEROID_PARTICLE, 32);

	Particle* particles_p = &entities.particles[0];
	Particle particle; particle.radius = 5.0f; particle.pos = -10.0 * VECTOR2_ONE; particle.circleEdges = 4;
	for (int i = 0; i < PARTICLES_MAX; i++)
	{
		particle.guid = Guid_AddToGUIDTable(PARTICLE, &particles_p[i]);;
		particles_p[i] = particle;
	}

	entities.bulletCount = 0;
	entities.asteroidsCount = 0;

	Guid_CheckTable();
}

static void GameInit(int screenWidth, int screenHeight)
{
	game = { 0 };
	game.level = 0;
	game.tCurr = 0.0f;
	game.spawnInterval = 1.0f;
	game.paused = false;

	game.screenRect = RectNew(VECTOR2_ZERO, V2(screenWidth, screenHeight));
}

static void StarsInit()
{
	for (int i = 0; i < STARS_MAX; i++)
	{
		stars[i].pos = V2(GetRandomValue(10.0f, (int)game.screenRect.size.x), 
						  GetRandomValue(10.0f, (int)game.screenRect.size.y));
		stars[i].size = GetRandomValue(STARS_MIN_SIZE, STARS_MAX_SIZE);
		stars[i].color = ColorHSVToColor32(0, 0, 0.5f + 0.5f*GetRandomFloat01());
	}
}

void GameStart(int screenWidth, int screenHeight)
{
	GameInit(screenWidth, screenHeight);
	EntitiesInit();
	StarsInit();
											  // Ship     Bullets  Asteroids
	int collisionMatrix[3][3] = {/*Ship*/		{0,       0,       1,},
								 /*Bullets*/	{0,       0,       1,},
								 /*Asteroids*/	{1,       1,       0,}, };
	Collisions_Init(collisionMatrix, 3);
}

void GameUpdate()
{
#if 0
	DrawTriangle(V2(400, 400), V2(410, 400), V2(405, 390), COL32_WHITE);
	DrawTriangle(V2(400, 393), V2(410, 393), V2(405, 403), COL32_WHITE);
	DrawTriangle(V2(400, 393), V2(410, 393), V2(405, 403), ColorHSVToColor32(235.0f/360.0f, 0.42f, 0.49f));
	//DrawCircle(V2(600, 600), 100.0f, ColorHSVToColor32(235.0f / 360.0f, 0.42f, 1.1f));
	return;
#endif
	Ship* ship_p = &entities.ship;
	Bullet* bullets_p = &entities.bullets[0];
	Asteroid* asteroids_p = &entities.asteroids[0];
	Particle* particles_p = &entities.particles[0];

	float shipRotSpeed = 0.0f;
	float shipSpeed = 0.0f;
	bool shoot = false;
	static bool paused = false;

	/// --- Read input ---
	if (GameInput_Button(BUTTON_RIGHT_ARROW))	shipRotSpeed = -5.0f;
	if (GameInput_Button(BUTTON_LEFT_ARROW))	shipRotSpeed = 5.0f;
	if (GameInput_Button(BUTTON_UP_ARROW))		shipSpeed = SHIP_SPEED;
	if (GameInput_Button(BUTTON_DOWN_ARROW))	shipSpeed = -SHIP_SPEED;
	if (GameInput_Button(BUTTON_LSHIFT))		shipSpeed *= BOOST_SPEED_FACTOR;
	if (GameInput_ButtonDown(BUTTON_C))			shoot = true;
	if (GameInput_ButtonDown(BUTTON_ENTER))     game.paused = !game.paused;

	/// --- Handle collisions ---
	//Collisions_DebugShowColliders();
	Collisions_CheckCollisions();
	Collisions_NewFrame();

	/// --- Destroying ---
	DestroyOldBullets(bullets_p);
	DestroyOffScreenAsteroids(asteroids_p);

	/// --- Spawning ---
	if ((game.tCurr - game.tLastSpawn) > game.spawnInterval)
	{
		SpawnAsteroidsOffscreen(1, asteroids_p);
		game.tLastSpawn = game.tCurr;
	}
	if (shoot)
	{
		assert(entities.bulletCount < BULLETS_MAX);
		Bullet* bullet_p = &bullets_p[entities.bulletCount];
		bullet_p->vel = 500.0f * ship_p->facing + ship_p->vel;
		bullet_p->pos = ship_p->pos + ship_p->size.y*ship_p->facing;
		bullet_p->tDestroy = game.tCurr + BULLET_LIFETIME;
		entities.bulletCount++;
	}
	if (fabs(shipSpeed) > 0.0f)
	{
		Particle* particle_p = GetParticle(&entities, EXHAUST_PARTICLE);
		particle_p->vel = -50.0f * ship_p->facing;
		particle_p->vel = Rotate(particle_p->vel, GetRandomValue(-45, 45));
		particle_p->pos = ship_p->pos;
		particle_p->color = fabs(shipSpeed) > SHIP_SPEED ? COL32_YELLOW : COL32_WHITE;
		particle_p->circleEdges = 4;
	}
	
	/// --- Physics ---
	if (!game.paused)
	{
		ship_p->facing = Rotate(ship_p->facing, shipRotSpeed);
		ship_p->vel += shipSpeed * ship_p->facing;
		if (shipSpeed == 0.0f) 	ship_p->vel = 0.99f * ship_p->vel;
		ship_p->pos += FIXED_DELTA_TIME * ship_p->vel;
		ship_p->pos.x = Wrapf(ship_p->pos.x, 0.0f, game.screenRect.size.x);
		ship_p->pos.y = Wrapf(ship_p->pos.y, 0.0f, game.screenRect.size.y);
		Collisions_AddCollider(&ship_p->collider);

		for (int i = 0; i < entities.bulletCount; i++)
		{
			Bullet* bullet_p = &bullets_p[i];
			bullet_p->pos += FIXED_DELTA_TIME * bullet_p->vel;
			bullet_p->pos.x = Wrapf(bullet_p->pos.x, 0.0f, game.screenRect.size.x);
			bullet_p->pos.y = Wrapf(bullet_p->pos.y, 0.0f, game.screenRect.size.y);
			Collisions_AddCollider(&bullet_p->collider);
		}
		for (int i = 0; i < PARTICLES_MAX; i++)
		{
			Particle* particle_p = &particles_p[i];
			particle_p->pos += FIXED_DELTA_TIME * particle_p->vel;
		}
		for (int i = 0; i < entities.asteroidsCount; i++)
		{
			Asteroid* asteroid_p = &asteroids_p[i];
			asteroid_p->pos += FIXED_DELTA_TIME * asteroid_p->vel;
			asteroid_p->rot += FIXED_DELTA_TIME * asteroid_p->rotSpeed;
			Collisions_AddCollider(&asteroid_p->collider);
		}
	}

	/// --- Drawing 
	int rndIndex = GetRandomValue(0, 63);
	for (int i = 0; i < STARS_MAX; i++)
	{
		Star* star_p = &stars[i];
		DrawCircle(star_p->pos, star_p->size, star_p->color, 4);
		if (i == rndIndex)
		{
			ColorHSV colorHSV = Color32ToHSV(star_p->color);
			colorHSV.v = 0.5f + fabs(0.5f*sinf(game.tCurr));
			star_p->color = ColorHSVToColor32(colorHSV);
		}
	}

	Vector2 point1 = ship_p->pos + ship_p->size.y*ship_p->facing;
	Vector2 point2 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, +90.0f);
	Vector2 point3 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, -90.0f);
	Vector2 point4 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, +135.0f);
	Vector2 point5 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, -135.0f);
	DrawTriangle(point1, point2, point3, COL32(20, 89, 255));
	DrawTriangle(ship_p->pos, point2, point4, COL32(20, 89, 255));
	DrawTriangle(ship_p->pos, point3, point5, COL32(20, 89, 255));

	for (int i = 0; i < entities.bulletCount; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		DrawCircle(bullet_p->pos, bullet_p->radius, COL32_RED);
	}
	for (int i = 0; i < entities.asteroidsCount; i++)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		DrawCircleWStartAngle(asteroid_p->pos, asteroid_p->radius, asteroid_p->color, asteroid_p->edges, asteroid_p->rot);
		//Debug_DrawVector(50.0f*Normalize(asteroid_p->vel), asteroid_p->pos, COLOR_GREEN);
	}
	for (int i = 0; i < entities.particleCount; i++)
	{
		Particle* particle_p = &particles_p[i];
		DrawCircle(particle_p->pos, particle_p->radius, particle_p->color, particle_p->circleEdges);
	}

	//Debug_DrawVector(50.0f*ship_p->facing, ship_p->pos, COLOR_GREEN);

	if (!game.paused) game.tCurr += FIXED_DELTA_TIME;
	game.frameCounter++;
}

static void SpawnAsteroidsOffscreen(int count, Asteroid* asteroids_p)
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
		//Vector2 spawnPoint = 0.5f*game.screenRect.size + 30.0f * VECTOR2_UP;
		Vector2 spawnPoint = spawnPoints[spawnIdx];
		Vector2 destPoint = spawnPoints[destIdx];
		asteroid_p->pos = spawnPoint;
		asteroid_p->radius = GetRandomValue(ASTEROID_MIN_SIZE, 80);
		asteroid_p->rotSpeed = GetRandomSign()*GetRandomValue(45, 75);
		asteroid_p->vel = GetRandomValue(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED) * Normalize(destPoint - spawnPoint);
		asteroid_p->edges = GetRandomValue(5, 9);
		asteroid_p->collider.circle.radius = 0.8f*asteroid_p->radius;
		asteroid_p->color = ColorHSVToColor32(33.0f/360.0f, 1.0f, GetRandomValue(30,90)/100.0f);
		spawnIdx = (spawnIdx + 1) % 4;
		destIdx = (destIdx + 1) % 4;
	}

	entities.asteroidsCount += count;
}

static void DestroyBullet(Bullet* bullet_p)
{
	assert(entities.bulletCount >= 0);

	// Move it past bulletCount
	Bullet* last_p = &entities.bullets[entities.bulletCount - 1];
	Bullet tmp = *bullet_p;
	*bullet_p = *last_p;
	*last_p = tmp;

	// keep correct ref to pos and guid
	bullet_p->collider.posRef = &bullet_p->pos;
	last_p->collider.posRef = &last_p->pos;
	Guid_SwapGuidDescriptors(bullet_p->guid, last_p->guid);

	entities.bulletCount--;
}

static void DestroyAsteroid(Asteroid* asteroid_p)
{
	assert(entities.asteroidsCount >= 0);

	// Move it past bulletCount
	Asteroid* last_p = &entities.asteroids[entities.asteroidsCount - 1];
	Asteroid tmp = *asteroid_p;
	*asteroid_p = *last_p;
	*last_p = tmp;

	// keep correct ref to pos and guid
	asteroid_p->collider.posRef = &asteroid_p->pos;
	last_p->collider.posRef = &last_p->pos;
	Guid_SwapGuidDescriptors(asteroid_p->guid, last_p->guid);

	entities.asteroidsCount--;
}

static void DestroyOldBullets(Bullet* bullets_p)
{
	int bulletCount = entities.bulletCount;

	for (int i = bulletCount - 1; i >= 0; i--)
	{
		Bullet* bullet_p = &bullets_p[i];
		if (game.tCurr > bullet_p->tDestroy)
		{
			DestroyBullet(bullet_p);
		}
	}
}

static void DestroyOffScreenAsteroids(Asteroid* asteroids_p)
{
	Vector2 screenCenter = RectCenter(game.screenRect);

	int asteroidCount = entities.asteroidsCount;
	for (int i = asteroidCount - 1; i >= 0; i--)
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
			DestroyAsteroid(asteroid_p);
		}
	}
}

static void ReserveParticles(Entities* entities_p, ParticleType particleType, int count)
{
	assert(entities.particleCount + count <= PARTICLES_MAX);
	entities.particleShare[particleType].startIndex = entities.particleCount;
	entities.particleShare[particleType].count = count;
	entities.particleCount += count;
}

static Particle* GetParticle(Entities* entities_p, ParticleType particleType)
{
	ParticleShare* particleShare_p = &entities.particleShare[particleType];
	Particle* paticle_p = &entities_p->particles[particleShare_p->startIndex + particleShare_p->index];
	particleShare_p->index = (particleShare_p->index + 1) % particleShare_p->count;
	return paticle_p;
}

static void ShipCollision(Collider* collider, Collider* otherCollider)
{
	//printf("Ship collision! collider.guidRef=%d otherCollider.guidRef=%d frame=%lu\n", collider->guid, otherCollider->guid, game.frameCounter);

	GuidDescriptor desc = Guid_GetDescriptor(collider->guid);
	GuidDescriptor otherDesc = Guid_GetDescriptor(otherCollider->guid);

	assert(desc.entityType == SHIP);
	Ship* ship_p = (Ship*)desc.data;

	switch (otherDesc.entityType)
	{
	case ASTEROID:
	{
		ship_p->pos = 500.0f * VECTOR2_ONE;
		ship_p->vel = VECTOR2_ZERO; // TODO
	} break;
	InvalidDefaultCase;
	}
}

static void AsteroidCollision(Collider* collider, Collider* otherCollider)
{
	//printf("Asteroid collision! collider.guidRef=%d otherCollider.guidRef=%d frame=%lu\n", collider->guid, otherCollider->guid, game.frameCounter);
	GuidDescriptor desc = Guid_GetDescriptor(collider->guid);
	GuidDescriptor otherDesc = Guid_GetDescriptor(otherCollider->guid);

	assert(desc.entityType == ASTEROID);
	Asteroid* asteroid_p = (Asteroid*)desc.data;

	switch (otherDesc.entityType)
	{
	case BULLET:
	{
		Bullet* bullet_p = (Bullet*)otherDesc.data;
		Vector2 normBulletVel = Normalize(bullet_p->vel);

		int particleCount = GetRandomValue(6, 8);
		for (int i = 0; i < particleCount; i++)
		{
			Particle* particle_p = GetParticle(&entities, ASTEROID_PARTICLE);
			//particle_p->vel = 500.0f * normBulletVel;
			particle_p->vel = 100.0f * Rotate(VECTOR2_RIGHT, GetRandomValue(-180, 180));
			particle_p->pos = asteroid_p->pos;
			particle_p->color = asteroid_p->color;
			particle_p->circleEdges = 4;
		}

		// Spawn smaller ones
		int count = (asteroid_p->radius / (ASTEROID_MIN_SIZE + 10));
		if (count > 1)
		{			
			ColorHSV colorHSV = Color32ToHSV(asteroid_p->color);
			colorHSV.v += GetRandomValue(5, 10)/100.0f;
			Color32 childColor = ColorHSVToColor32(colorHSV.h, colorHSV.s, colorHSV.v);

			for (int i = entities.asteroidsCount; i < entities.asteroidsCount + count; i++)
			{
				Asteroid* childAsteroid_p = &entities.asteroids[i];
				childAsteroid_p->pos = asteroid_p->pos;
				childAsteroid_p->radius = GetRandomValue(ASTEROID_MIN_SIZE, asteroid_p->radius);
				childAsteroid_p->rotSpeed = GetRandomSign()*GetRandomValue(45, 75);
				childAsteroid_p->vel = 1.2f*Magnitude(asteroid_p->vel) * Rotate(-1.0f*normBulletVel, GetRandomValue(-90, 90));
				childAsteroid_p->edges = GetRandomValue(5, 9);
				childAsteroid_p->collider.circle.radius = 0.8f*childAsteroid_p->radius;
				childAsteroid_p->color = childColor;
			}
			entities.asteroidsCount += count;
		}

		DestroyAsteroid(asteroid_p);
	} break;
	case SHIP:
		break;
	InvalidDefaultCase;
	}
}

static void BulletCollision(Collider* collider, Collider* otherCollider)
{
	//printf("Bullet collision! collider.guidRef=%d otherCollider.guidRef=%d frame=%lu\n", collider->guid, otherCollider->guid, game.frameCounter);
	GuidDescriptor desc = Guid_GetDescriptor(collider->guid);
	GuidDescriptor otherDesc = Guid_GetDescriptor(otherCollider->guid);

	assert(desc.entityType == BULLET);
	Bullet* bullet_p = (Bullet*)desc.data;

	switch (otherDesc.entityType)
	{
	case ASTEROID:
	{
		DestroyBullet(bullet_p);
	} break;
	InvalidDefaultCase;
	}
}