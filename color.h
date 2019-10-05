#pragma once

#define COLOR_WHITE		Col(1.0f, 1.0f, 1.0f)
#define COLOR_BLACK		Col(0.0f, 0.0f, 0.0f)
#define COLOR_RED		Col(1.0f, 0.0f, 0.0f)
#define COLOR_GREEN		Col(0.0f, 1.0f, 0.0f)
#define COLOR_BLUE		Col(0.0f, 0.0f, 1.0f)
#define COLOR_YELLOW	Col(1.0f, 1.0f, 0.0f)

struct Color
{
	float r;
	float g;
	float b;
	float a;
};

static inline Color Col(float r, float g, float b, float a = 1.0f)
{
	Color result = {r, g, b, a};
	return result;
}

static inline Color Col(int r, int g, int b, int a = 255)
{
	return Col((float)r / 255, (float)g / 255, (float)b / 255, (float)a / 255);
}