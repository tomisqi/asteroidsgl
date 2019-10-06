#pragma once
#include "vector.h"
#include "color.h"

struct DrawVert
{
	Vector2 vert;
	unsigned int color;
};

typedef unsigned short DrawIdx;

struct DrawList
{
	DrawVert* vertBuffer;
	DrawIdx* idxBuffer;
	unsigned short vertCount;
	unsigned short maxVertCount;
};

struct ReservedDrawData
{
	DrawVert* vertBuffer;
	DrawIdx* idxBuffer;
};

void Renderer_Init(int maxVertCount);
void Renderer_NewFrame();
void Renderer_Render();

static inline ReservedDrawData PushVerts(DrawList* drawList, int count)
{
	DrawVert* vertBuff = &drawList->vertBuffer[drawList->vertCount];
	DrawIdx* idxBuff = &drawList->idxBuffer[drawList->vertCount];
	idxBuff[0] = drawList->vertCount;

	drawList->vertCount += count;

	ReservedDrawData resDrawData = { vertBuff, idxBuff };
	return resDrawData;
}

void DrawCircle(Vector2 pos, float radius, Color color, int edgeCount = 8);
void DrawCircleWStartAngle(Vector2 pos, float radius, Color color, int edgeCount, float startAngle);
void DrawTriangle(Vector2 point1, Vector2 point2, Vector2 point3, Color color);
//void DrawVectorImmediate(Vector2 v, Vector2 pos);