#include <stdio.h>
#include <assert.h>
#include "asteroids.h"
#include "input.h"
#include "utils.h"
#include "render.h"
#include "rect.h"
#include "debugrender.h"

#define FIXED_DELTA_TIME		(1.0f/60.0f)

struct CircleEntity
{
	float radius;

	Vector2 pos;
	Vector2 vel;
};

typedef CircleEntity ExhaustParticles;
typedef CircleEntity Bullet;

struct Ship
{
	Vector2 size;

	Vector2 pos;
	Vector2 facing;
	Vector2 vel;
	int bulletIdx;
	int exhaustIdx;
};


struct Asteroid
{
	float radius;
	int edges;

	Vector2 pos;
	Vector2 vel;
	float rot;
	float rotSpeed;
};

struct Game
{
	Rect screenRect;
	int level;
	double tCurr;
	double tLastSpawn;
	double spawnInterval;
};


#define EXHAUST_PARTICLES_MAX	32
#define BULLETS_MAX				16
#define ASTEROIDS_MAX			128
#define ASTEROID_MIN_SPEED		60
#define ASTEROID_MAX_SPEED		ASTEROID_MIN_SPEED + 40
struct Entities
{
	Ship ship;
	Bullet bullets[BULLETS_MAX];
	ExhaustParticles exhaustParticles[EXHAUST_PARTICLES_MAX];

	Asteroid asteroids[ASTEROIDS_MAX];
	int asteroidsCount;
};

static Entities entities;
static Game game;

static void EntitiesStart()
{
	Ship* ship_p = &entities.ship;
	ship_p->facing = VECTOR2_UP;
	ship_p->pos = 500.0f * VECTOR2_ONE;
	ship_p->size = 30.0f * V2(1.0f, 1.2f);
	ship_p->vel = VECTOR2_ZERO;
	ship_p->bulletIdx = 0;

	Bullet* bullets_p = &entities.bullets[0];
	Bullet defBullet; defBullet.radius = 8.0f; defBullet.pos = -10.0 * VECTOR2_ONE;
	for (int i = 0; i < BULLETS_MAX; i++) bullets_p[i] = defBullet;

	ExhaustParticles* exhaustParticles_p = &entities.exhaustParticles[0];
	ExhaustParticles exhaust; exhaust.radius = 5.0f; exhaust.pos = -10.0 * VECTOR2_ONE;
	for (int i = 0; i < EXHAUST_PARTICLES_MAX; i++) exhaustParticles_p[i] = exhaust;

	Asteroid* asteroids_p = &entities.asteroids[0];
	Asteroid asteroid; asteroid.radius = 7.0f; asteroid.pos = -10.0 * VECTOR2_ONE; asteroid.rot = 0.0f;
	for (int i = 0; i < ASTEROIDS_MAX; i++) asteroids_p[i] = asteroid;
}

static void GameInit(int screenWidth, int screenHeight)
{
	game = { 0 };
	game.level = 0;
	game.tCurr = 0.0f;
	game.spawnInterval = 3.0f;

	game.screenRect = RectNew(VECTOR2_ZERO, V2(screenWidth, screenHeight));

}

static void SpawnAsteroids(int count, Asteroid* asteroids_p);
static void DestroyOffScreenAsteroids(Asteroid* asteroids_p);

void GameStart(int screenWidth, int screenHeight)
{
	GameInit(screenWidth, screenHeight);
	EntitiesStart();
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

	/// Read input

	if (GameInput_Button(BUTTON_RIGHT_ARROW)) shipRotSpeed = -5.0f;
	if (GameInput_Button(BUTTON_LEFT_ARROW)) shipRotSpeed = 5.0f;
	if (GameInput_Button(BUTTON_UP_ARROW)) shipSpeed = 5.0f;
	if (GameInput_ButtonDown(BUTTON_SPACE)) shoot = true;

	/// Destroying

	DestroyOffScreenAsteroids(asteroids_p);

	/// Spawning

	if ((game.tCurr - game.tLastSpawn) > game.spawnInterval)
	{
		SpawnAsteroids(1, asteroids_p);
		game.tLastSpawn = game.tCurr;
	}
	
	/// Physics

	//Ship
	ship_p->facing = Rotate(ship_p->facing, shipRotSpeed);
	ship_p->vel += shipSpeed * ship_p->facing;
	if (shipSpeed == 0.0f) 	ship_p->vel = 0.99f * ship_p->vel;
	ship_p->pos += FIXED_DELTA_TIME * ship_p->vel;
	ship_p->pos.x = Wrapf(ship_p->pos.x, 0.0f, game.screenRect.size.x);
	ship_p->pos.y = Wrapf(ship_p->pos.y, 0.0f, game.screenRect.size.y);

	// Bullets
	if (shoot)
	{
		Bullet* bullet_p = &bullets_p[ship_p->bulletIdx];
		bullet_p->vel = 500.0f * ship_p->facing + ship_p->vel;
		bullet_p->pos = ship_p->pos + ship_p->size.y*ship_p->facing;
		ship_p->bulletIdx = (ship_p->bulletIdx + 1) % BULLETS_MAX;
	}
	for (int i = 0; i < BULLETS_MAX; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		bullet_p->pos += FIXED_DELTA_TIME * bullet_p->vel;
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
	}

	/// Drawing

	// Ship
	Vector2 point1 = ship_p->pos + ship_p->size.y*ship_p->facing;
	Vector2 point2 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, 90.0f);
	Vector2 point3 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, -90.0f);
	DrawTriangle(point1, point2, point3, Col(20, 89, 255));

	// Bullets
	for (int i = 0; i < BULLETS_MAX; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		DrawCircle(bullet_p->pos, bullet_p->radius, COLOR_RED);
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
		DrawCircleWStartAngle(asteroid_p->pos, asteroid_p->radius, Col(255, 123, 0), asteroid_p->edges, asteroid_p->rot);
		//Debug_DrawVector(50.0f*Normalize(asteroid_p->vel), asteroid_p->pos, COLOR_GREEN);
	}

	Debug_DrawVector(50.0f*ship_p->facing, ship_p->pos, COLOR_GREEN);

	game.tCurr += FIXED_DELTA_TIME;
}

static void SpawnAsteroids(int count, Asteroid* asteroids_p)
{
	Rect screenRect = game.screenRect;
	assert(entities.asteroidsCount + count <= ASTEROIDS_MAX);
	Vector2 randPos = V2(GetRandomValue(-10.0f, screenRect.size.x + 10.0f), GetRandomValue(-10.0f, screenRect.size.y + 10.0f));
	Vector2 spawnPoints[4] = {	V2(randPos.x, -10.0f),
								V2(randPos.x, screenRect.size.y + 10.0f),
								V2(-10.0f, randPos.y), 
								V2(screenRect.size.x + 10.0f, randPos.y) };

	
	for (int i = entities.asteroidsCount; i < entities.asteroidsCount + count; i++)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		Vector2 spawnPoint = spawnPoints[i%4];
		asteroid_p->pos = spawnPoint;
		asteroid_p->radius = GetRandomValue(20, 70);
		asteroid_p->rotSpeed = GetRandomSign()*GetRandomValue(45, 75);
		asteroid_p->vel = GetRandomValue(ASTEROID_MIN_SPEED, ASTEROID_MAX_SPEED) * Normalize(screenRect.center - spawnPoint);
		asteroid_p->edges = GetRandomValue(5, 9);
	}

	entities.asteroidsCount += count;
}

static void DestroyOffScreenAsteroids(Asteroid* asteroids_p)
{
	int asteroidCount = entities.asteroidsCount;

	for (int i = entities.asteroidsCount - 1; i >= 0; i--)
	{
		Asteroid* asteroid_p = &asteroids_p[i];
		bool offScreenAndMovingAway = false;

		if (!RectContains(game.screenRect, asteroid_p->pos))	
		{
			Vector2 velNorm = Normalize(asteroid_p->vel);
			Vector2 toCenterNorm = Normalize(game.screenRect.center - asteroid_p->pos);
			if (Dot(velNorm, toCenterNorm) < 0.0f)
			{			
				offScreenAndMovingAway = true;
			}
		}

		if (offScreenAndMovingAway)
		{
			// Move it past asteroidCount
			Asteroid* tmp = &asteroids_p[i];
			asteroids_p[i] = asteroids_p[asteroidCount - 1];
			asteroids_p[asteroidCount - 1] = *tmp;

			asteroidCount--;
		}
	}
	assert(asteroidCount >= 0);
	entities.asteroidsCount = asteroidCount;
}