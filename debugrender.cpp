#include <stdint.h>
#include <string.h>
#include <GLFW/glfw3.h>
#include <stdlib.h>
#include <assert.h>
#include "render.h" 
#include "utils.h"
#include "rect.h"

static DrawList debugDrawList;

void DebugRenderer_Init(int maxVertCount)
{
	assert(maxVertCount < UINT16_MAX);
	memset(&debugDrawList, 0, sizeof(debugDrawList));

	debugDrawList.vertBuffer = (DrawVert*)malloc(sizeof(DrawVert) * maxVertCount);
	debugDrawList.idxBuffer = (DrawIdx*)malloc(sizeof(DrawIdx) * maxVertCount);
	debugDrawList.maxVertCount = maxVertCount;
}

void DebugRenderer_NewFrame()
{
	debugDrawList.vertCount = 0;
}

void DebugRenderer_Render()
{
	glVertexPointer(2, GL_FLOAT, sizeof(DrawVert), (uint8_t*)debugDrawList.vertBuffer + OFFSET_OF(DrawVert, vert));
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(DrawVert), (uint8_t*)debugDrawList.vertBuffer + OFFSET_OF(DrawVert, color));
	glDrawElements(GL_LINES, debugDrawList.vertCount, GL_UNSIGNED_SHORT, debugDrawList.idxBuffer);
}

#define TIP_LENGTH 10.0f
void Debug_DrawVector(Vector2 v, Vector2 pos, Color color)
{
	ReservedDrawData drawData = PushVerts(&debugDrawList, 6);
	DrawIdx elemIdx = drawData.idxBuffer[0];
	assert(debugDrawList.vertCount <= debugDrawList.maxVertCount);

	unsigned int colorU32 = ColorToU32(color);

	Vector2 end = pos + v;
	Vector2 tip = TIP_LENGTH * Normalize(pos - end);
	Vector2 tip1 = end + Rotate(tip, 45.0f);
	Vector2 tip2 = end + Rotate(tip, -45.0f);

	drawData.vertBuffer[0].vert = pos; drawData.vertBuffer[0].color = colorU32; drawData.idxBuffer[0] = elemIdx + 0;
	drawData.vertBuffer[1].vert = end; drawData.vertBuffer[1].color = colorU32; drawData.idxBuffer[1] = elemIdx + 1;
	drawData.vertBuffer[2].vert = end; drawData.vertBuffer[2].color = colorU32; drawData.idxBuffer[2] = elemIdx + 2;
	drawData.vertBuffer[3].vert = tip1; drawData.vertBuffer[3].color = colorU32; drawData.idxBuffer[3] = elemIdx + 3;
	drawData.vertBuffer[4].vert = end; drawData.vertBuffer[4].color = colorU32; drawData.idxBuffer[4] = elemIdx + 4;
	drawData.vertBuffer[5].vert = tip2; drawData.vertBuffer[5].color = colorU32; drawData.idxBuffer[5] = elemIdx + 5;
}


void Debug_DrawRect(Rect rect, Color color)
{
	ReservedDrawData drawData = PushVerts(&debugDrawList, 8);
	DrawIdx elemIdx = drawData.idxBuffer[0];
	assert(debugDrawList.vertCount <= debugDrawList.maxVertCount);

	unsigned int colorU32 = ColorToU32(color);

	Vector2 point1 = RectBottomLeft(rect);
	Vector2 point2 = RectBottomRight(rect);
	Vector2 point3 = RectTopRight(rect);
	Vector2 point4 = RectTopLeft(rect);

	drawData.vertBuffer[0].vert = point1; drawData.vertBuffer[0].color = colorU32; drawData.idxBuffer[0] = elemIdx + 0;
	drawData.vertBuffer[1].vert = point2; drawData.vertBuffer[1].color = colorU32; drawData.idxBuffer[1] = elemIdx + 1;
	drawData.vertBuffer[2].vert = point2; drawData.vertBuffer[2].color = colorU32; drawData.idxBuffer[2] = elemIdx + 2;
	drawData.vertBuffer[3].vert = point3; drawData.vertBuffer[3].color = colorU32; drawData.idxBuffer[3] = elemIdx + 3;
	drawData.vertBuffer[4].vert = point3; drawData.vertBuffer[4].color = colorU32; drawData.idxBuffer[4] = elemIdx + 4;
	drawData.vertBuffer[5].vert = point4; drawData.vertBuffer[5].color = colorU32; drawData.idxBuffer[5] = elemIdx + 5;
	drawData.vertBuffer[6].vert = point4; drawData.vertBuffer[6].color = colorU32; drawData.idxBuffer[6] = elemIdx + 6;
	drawData.vertBuffer[7].vert = point1; drawData.vertBuffer[7].color = colorU32; drawData.idxBuffer[7] = elemIdx + 7;
}

