#include <stdint.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <assert.h>
#include "render.h"
#include "utils.h"

static DrawList drawList;
static unsigned long frameCounter;

void Renderer_Init(int maxVertCount)
{
	assert(maxVertCount < UINT16_MAX);
	memset(&drawList, 0, sizeof(drawList));

	drawList.vertBuffer = (DrawVert*)malloc(sizeof(DrawVert) * maxVertCount);
	drawList.idxBuffer = (DrawIdx*)malloc(sizeof(DrawIdx) * maxVertCount);
	drawList.maxVertCount = maxVertCount;

	frameCounter = 0;
}

void Renderer_NewFrame()
{
	drawList.vertCount = 0;
	frameCounter++;
}

void Renderer_Render()
{
	glVertexPointer(2, GL_FLOAT, sizeof(DrawVert), (uint8_t*)drawList.vertBuffer + OFFSET_OF(DrawVert, vert));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(DrawVert), (uint8_t*)drawList.vertBuffer + OFFSET_OF(DrawVert, color32));
	glDrawElements(GL_TRIANGLES, drawList.vertCount, GL_UNSIGNED_SHORT, drawList.idxBuffer);
}

void DrawCircleWStartAngle(Vector2 pos, float radius, Color32 color32, int edgeCount, float startAngle)
{
	ReservedDrawData drawData = PushVerts(&drawList, edgeCount * 3);
	DrawIdx elemIdx = drawData.idxBuffer[0];
	assert(drawList.vertCount <= drawList.maxVertCount);

	float theta = 360.0f / edgeCount;
	Vector2 point0 = pos;
	Vector2 v = radius * Rotate(VECTOR2_RIGHT, startAngle);
	Vector2 point1 = point0 + v;
	for (int i = 0; i < edgeCount; i++)
	{
		v = Rotate(v, theta);
		Vector2 point2 = point0 + v;

		drawData.vertBuffer[i * 3 + 0].vert = point0; drawData.vertBuffer[i * 3 + 0].color32 = color32; drawData.idxBuffer[i * 3 + 0] = elemIdx + 0;
		drawData.vertBuffer[i * 3 + 1].vert = point1; drawData.vertBuffer[i * 3 + 1].color32 = color32; drawData.idxBuffer[i * 3 + 1] = elemIdx + 1;
		drawData.vertBuffer[i * 3 + 2].vert = point2; drawData.vertBuffer[i * 3 + 2].color32 = color32; drawData.idxBuffer[i * 3 + 2] = elemIdx + 2;
		elemIdx += 3;

		point1 = point2;
	}
}

void DrawCircle(Vector2 pos, float radius, Color32 color32, int edgeCount)
{
	DrawCircleWStartAngle(pos, radius, color32, edgeCount, 0.0f);
}

void DrawTriangle(Vector2 point1, Vector2 point2, Vector2 point3, Color32 color32)
{
	ReservedDrawData drawData = PushVerts(&drawList, 3);
	DrawIdx elemIdx = drawData.idxBuffer[0];
	assert(drawList.vertCount <= drawList.maxVertCount);

	drawData.vertBuffer[0].vert = point1; drawData.vertBuffer[0].color32 = color32; drawData.idxBuffer[0] = elemIdx + 0;
	drawData.vertBuffer[1].vert = point2; drawData.vertBuffer[1].color32 = color32; drawData.idxBuffer[1] = elemIdx + 1;
	drawData.vertBuffer[2].vert = point3; drawData.vertBuffer[2].color32 = color32; drawData.idxBuffer[2] = elemIdx + 2;
}

//#define TIP_LENGTH 10.0f
//void DrawVectorImmediate(Vector2 v, Vector2 pos)
//{
//	glBegin(GL_LINES);
//	{
//		Vector2 end = pos + v;
//		Vector2 tip = TIP_LENGTH * Normalize(pos - end);
//		Vector2 tip1 = end + Rotate(tip, 45.0f);
//		Vector2 tip2 = end + Rotate(tip, -45.0f);
//
//		glVertex2f(pos.x, pos.y);
//		glVertex2f(end.x, end.y);
//		glVertex2f(end.x, end.y);
//		glVertex2f(tip1.x, tip1.y);
//		glVertex2f(end.x, end.y);
//		glVertex2f(tip2.x, tip2.y);
//	}
//	glEnd();
//}

