#include "render.h"
#include <stdint.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <assert.h>

struct RenderList
{
	int count;
	int size;
	void* elements;
};

struct RenderJob
{
	union
	{
		RenderList shapes[MAX_SHAPES];
		struct 
		{
			RenderList triangles;
			RenderList circles;
			RenderList rects;
			RenderList vectors;
		};
	};
};

static RenderJob render;
static long frameCounter;

void RenderJob_Init(int maxTriangleCount, int maxCircleCount, int maxRectCount, int maxVectors)
{
	memset(&render, 0, sizeof(render));
	render.triangles.size = maxTriangleCount;
	render.circles.size = maxCircleCount;
	render.rects.size = maxRectCount;
	render.vectors.size = maxVectors;

	render.triangles.elements = malloc(sizeof(Triangle) * maxTriangleCount);
	render.circles.elements = malloc(sizeof(Circle) * maxCircleCount);
	render.rects.elements = malloc(sizeof(Rect) * maxRectCount);
	render.vectors.elements = malloc(sizeof(PosVector2) * maxVectors);

	frameCounter = 0;
}

void RenderJob_NewFrame()
{
	for (int i = 0; i < MAX_SHAPES; i++)
	{
		render.shapes[i].count = 0;
	}
	frameCounter++;
}

void RenderJob_Render()
{
	// Triangles
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_TRIANGLES);
	{
		Triangle* triangles_p = (Triangle*)render.triangles.elements;
		for (int i = 0; i < render.triangles.count; i++)
		{
			Triangle* triangle_p = &triangles_p[i];
			glColor4f(triangle_p->color.r, triangle_p->color.g, triangle_p->color.b, 1.0f);
			glVertex2f(triangle_p->point1.x, triangle_p->point1.y);
			glVertex2f(triangle_p->point2.x, triangle_p->point2.y);
			glVertex2f(triangle_p->point3.x, triangle_p->point3.y);
		}
	}
	glEnd();

	// Circles
#define NO_TRIANGLES 4
	float theta = 360.0f / NO_TRIANGLES;
	glBegin(GL_TRIANGLES);
	{
		Circle* circles_p = (Circle*)render.circles.elements;
		for (int i = 0; i < render.circles.count; i++)
		{
			Circle* circle_p = &circles_p[i];
			glColor4f(circle_p->color.r, circle_p->color.g, circle_p->color.b, circle_p->color.a);

			Vector2 point0 = circle_p->pos;
			Vector2 v = circle_p->radius * VECTOR2_RIGHT;
			Vector2 point1 = point0 + v;
			for (int i = 0; i <= NO_TRIANGLES; i++)
			{
				glVertex2f(point0.x, point0.y);
				glVertex2f(point1.x, point1.y);
				v = Rotate(v, theta);
				point1 = point0 + v;
				glVertex2f(point1.x, point1.y);
			}
		}
	}
	glEnd();


	// Vectors
#define TIP_LENGTH 10.0f
	glBegin(GL_LINES);
	{
		PosVector2* vectors = (PosVector2*)render.vectors.elements;
		for (int i = 0; i < render.vectors.count; i++)
		{
			PosVector2* posVector = &vectors[i];
			Vector2 pos = posVector->pos;
			Vector2 v = posVector->vector;
			Vector2 end = pos + v;
			Vector2 tip = TIP_LENGTH * Normalize(pos - end);
			Vector2 tip1 = end + Rotate(tip, 45.0f);
			Vector2 tip2 = end + Rotate(tip, -45.0f);

			glVertex2f(pos.x, pos.y);
			glVertex2f(end.x, end.y);
			glVertex2f(end.x, end.y);
			glVertex2f(tip1.x, tip1.y);
			glVertex2f(end.x, end.y);
			glVertex2f(tip2.x, tip2.y);

		}
	}
	glEnd();
}

void* RenderJob_Push(RenderShape shape)
{
	int elemSize = 0;
	RenderList* renderList_p = nullptr;
	switch (shape)
	{
	case TRIANGLE:
		elemSize = sizeof(Triangle);
		renderList_p = &render.triangles;
		break;
	case CIRCLE:
		elemSize = sizeof(Circle);
		renderList_p = &render.circles;
		break;
	case RECT:
		elemSize = sizeof(Rect);
		renderList_p = &render.rects;
		break;
	case VECTOR:
		elemSize = sizeof(PosVector2);
		renderList_p = &render.vectors;
		break;
	default:
		break;
	}
	assert(renderList_p != nullptr);
	if (renderList_p->count < renderList_p->size)
	{
		void* result = (uint8_t*)renderList_p->elements + elemSize * renderList_p->count;
		renderList_p->count++;
		return result;
	}
	assert(false);
	return nullptr;
}

