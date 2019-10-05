#include "asteroids.h"
#include "input.h"
#include "utils.h"
#include "render.h"
#include <stdio.h>

#define NO_EXHAUST_PARTICLES 32
#define NO_BULLETS 16


#define FIXED_DELTA_TIME	1.0f/60.0f

struct Screen
{
	int width;
	int height;
};

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


struct Entities
{
	Ship ship;
	Bullet bullets[NO_BULLETS];
	ExhaustParticles exhaustParticles[NO_EXHAUST_PARTICLES];
};

static Entities entities;
static Screen screen;

static void EntitiesStart()
{
	Ship* ship_p = &entities.ship;
	ship_p->facing = VECTOR2_LEFT;
	ship_p->pos = 500.0f * VECTOR2_ONE;
	ship_p->size = 30.0f * V2(1.0f, 1.2f);
	ship_p->vel = VECTOR2_ZERO;
	ship_p->bulletIdx = 0;

	Bullet* bullets_p = &entities.bullets[0];
	Bullet defBullet; defBullet.radius = 10.0f; defBullet.pos = -10.0 * VECTOR2_ONE;
	for (int i = 0; i < NO_BULLETS; i++) bullets_p[i] = defBullet;

	ExhaustParticles* exhaustParticles_p = &entities.exhaustParticles[0];
	ExhaustParticles exhaust; exhaust.radius = 5.0f; exhaust.pos = -10.0 * VECTOR2_ONE;
	for (int i = 0; i < NO_EXHAUST_PARTICLES; i++) exhaustParticles_p[i] = exhaust;
}

void GameStart(int screenWidth, int screenHeight)
{
	EntitiesStart();
	screen.width = screenWidth;
	screen.height = screenHeight;
}

void GameUpdate()
{
	Ship* ship_p = &entities.ship;
	Bullet* bullets_p = &entities.bullets[0];
	ExhaustParticles* exhaustParticles_p = &entities.exhaustParticles[0];

	float shipRotSpeed = 0.0f;
	float shipSpeed = 0.0f;
	bool shoot = false;

	/// Read input

	if (GameInput_Button(BUTTON_RIGHT_ARROW)) shipRotSpeed = -5.0f;
	if (GameInput_Button(BUTTON_LEFT_ARROW)) shipRotSpeed = 5.0f;
	if (GameInput_Button(BUTTON_UP_ARROW)) shipSpeed = 5.0f;
	if (GameInput_ButtonDown(BUTTON_SPACE)) shoot = true;
	
	/// Physics

	//Ship
	ship_p->facing = Rotate(ship_p->facing, shipRotSpeed);
	ship_p->vel += shipSpeed * ship_p->facing;
	ship_p->pos += FIXED_DELTA_TIME * ship_p->vel;
	ship_p->pos.x = Wrapf(ship_p->pos.x, 0.0f, screen.width);
	ship_p->pos.y = Wrapf(ship_p->pos.y, 0.0f, screen.height);

	// Bullets
	if (shoot)
	{
		Bullet* bullet_p = &bullets_p[ship_p->bulletIdx];
		bullet_p->vel = 500.0f * ship_p->facing + ship_p->vel;
		bullet_p->pos = ship_p->pos + ship_p->size.y*ship_p->facing;
		ship_p->bulletIdx = (ship_p->bulletIdx + 1) % NO_BULLETS;
	}
	for (int i = 0; i < NO_BULLETS; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		bullet_p->pos += FIXED_DELTA_TIME * bullet_p->vel;
	}

	if (shipSpeed > 0.0f)
	{
		ExhaustParticles* exhaustParticle_p = &exhaustParticles_p[ship_p->exhaustIdx];
		exhaustParticle_p->vel = -50.0f * ship_p->facing;
		exhaustParticle_p->vel = Rotate(exhaustParticle_p->vel, GetRandomValue(-45, 45));
		exhaustParticle_p->pos = ship_p->pos;
		ship_p->exhaustIdx = (ship_p->exhaustIdx + 1) % NO_EXHAUST_PARTICLES;
	}
	for (int i = 0; i < NO_EXHAUST_PARTICLES; i++)
	{
		ExhaustParticles* exhaustParticle_p = &exhaustParticles_p[i];
		exhaustParticle_p->pos += FIXED_DELTA_TIME * exhaustParticle_p->vel;
	}

	/// Push to render jobs

	// Ship
	Triangle* triangle_p = (Triangle*)RenderJob_Push(TRIANGLE);
	triangle_p->point1 = ship_p->pos + ship_p->size.y*ship_p->facing;
	triangle_p->point2 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, 90.0f);
	triangle_p->point3 = ship_p->pos + (ship_p->size.x / 2.0f)*Rotate(ship_p->facing, -90.0f);
	triangle_p->color = COLOR_GREEN;

	// Bullets
	for (int i = 0; i < NO_BULLETS; i++)
	{
		Bullet* bullet_p = &bullets_p[i];
		Circle* circle_p = (Circle*)RenderJob_Push(CIRCLE);
		circle_p->pos = bullet_p->pos;
		circle_p->radius = bullet_p->radius;
		circle_p->color = COLOR_RED;
	}

	// ExhaustParticles
	for (int i = 0; i < NO_EXHAUST_PARTICLES; i++)
	{
		ExhaustParticles* exhaustParticle_p = &exhaustParticles_p[i];
		Circle* circle_p = (Circle*)RenderJob_Push(CIRCLE);
		circle_p->pos = exhaustParticle_p->pos;
		circle_p->radius = exhaustParticle_p->radius;
		circle_p->color = COLOR_WHITE;
	}


	PosVector2* posVector_p = (PosVector2*)RenderJob_Push(VECTOR);
	posVector_p->pos = ship_p->pos;
	posVector_p->vector = 50.0f*ship_p->facing;
//	DrawVector2(ship.vel, ship.pos);
}