#include <stdint.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <assert.h>
#include "render.h"

#define OFFSET_OF(_TYPE, _MEMBER) ((size_t)&(((_TYPE*)0)->_MEMBER)) 

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

static DrawList drawList;
static long frameCounter;

void Renderer_Init(int maxVertCount)
{
	assert(maxVertCount < UINT16_MAX);
	memset(&drawList, 0, sizeof(drawList));

	drawList.vertBuffer = (DrawVert*)malloc(sizeof(DrawVert) * maxVertCount);
	drawList.idxBuffer = (DrawIdx*)malloc(sizeof(DrawIdx) * maxVertCount);
	drawList.maxVertCount = maxVertCount;

	frameCounter = 0;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
}

void Renderer_NewFrame()
{
	drawList.vertCount = 0;
	frameCounter++;
}

void Renderer_Render()
{
	glVertexPointer(2, GL_FLOAT, sizeof(DrawVert), (uint8_t*)drawList.vertBuffer + OFFSET_OF(DrawVert, vert));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(DrawVert), (uint8_t*)drawList.vertBuffer + OFFSET_OF(DrawVert, color));
	glDrawElements(GL_TRIANGLES, drawList.vertCount, GL_UNSIGNED_SHORT, drawList.idxBuffer);
}

static ReservedDrawData PushVerts(int count)
{
	DrawVert* vertBuff = &drawList.vertBuffer[drawList.vertCount];
	DrawIdx* idxBuff = &drawList.idxBuffer[drawList.vertCount];
	idxBuff[0] = drawList.vertCount;

	drawList.vertCount += count;
	assert(drawList.vertCount <= drawList.maxVertCount);

	ReservedDrawData resDrawData = {vertBuff, idxBuff};
	return resDrawData;
}

void DrawCircleWStartAngle(Vector2 pos, float radius, Color color, int edgeCount, float startAngle)
{
	ReservedDrawData drawData = PushVerts(edgeCount * 3);
	unsigned int colorU32 = ColorToU32(color);

	float theta = 360.0f / edgeCount;
	Vector2 point0 = pos;
	Vector2 v = radius * Rotate(VECTOR2_RIGHT, startAngle);
	Vector2 point1 = point0 + v;
	DrawIdx elemIdx = drawData.idxBuffer[0];
	for (int i = 0; i < edgeCount; i++)
	{
		v = Rotate(v, theta);
		Vector2 point2 = point0 + v;

		drawData.vertBuffer[i * 3 + 0].vert = point0; drawData.vertBuffer[i * 3 + 0].color = colorU32; drawData.idxBuffer[i * 3 + 0] = elemIdx + 0;
		drawData.vertBuffer[i * 3 + 1].vert = point1; drawData.vertBuffer[i * 3 + 1].color = colorU32; drawData.idxBuffer[i * 3 + 1] = elemIdx + 1;
		drawData.vertBuffer[i * 3 + 2].vert = point2; drawData.vertBuffer[i * 3 + 2].color = colorU32; drawData.idxBuffer[i * 3 + 2] = elemIdx + 2;
		elemIdx += 3;

		point1 = point2;
	}
}

void DrawCircle(Vector2 pos, float radius, Color color, int edgeCount)
{
	DrawCircleWStartAngle(pos, radius, color, edgeCount, 0.0f);
}

void DrawTriangle(Vector2 point1, Vector2 point2, Vector2 point3, Color color)
{
	ReservedDrawData drawData = PushVerts(3);
	unsigned int colorU32 = ColorToU32(color);
	DrawIdx elemIdx = drawData.idxBuffer[0];
	drawData.vertBuffer[0].vert = point1; drawData.vertBuffer[0].color = colorU32; drawData.idxBuffer[0] = elemIdx + 0;
	drawData.vertBuffer[1].vert = point2; drawData.vertBuffer[1].color = colorU32; drawData.idxBuffer[1] = elemIdx + 1;
	drawData.vertBuffer[2].vert = point3; drawData.vertBuffer[2].color = colorU32; drawData.idxBuffer[2] = elemIdx + 2;
}


#define TIP_LENGTH 10.0f
void DrawVectorImmediate(Vector2 v, Vector2 pos)
{
	glBegin(GL_LINES);
	{
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
	glEnd();
}

