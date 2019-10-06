#pragma once

#define COLOR_WHITE		Col(1.0f, 1.0f, 1.0f)
#define COLOR_BLACK		Col(0.0f, 0.0f, 0.0f)
#define COLOR_RED		Col(1.0f, 0.0f, 0.0f)
#define COLOR_GREEN		Col(0.0f, 1.0f, 0.0f)
#define COLOR_BLUE		Col(0.0f, 0.0f, 1.0f)
#define COLOR_YELLOW	Col(1.0f, 1.0f, 0.0f)
#define COLOR_MAGENTA	Col(1.0f, 0.0f, 1.0f)

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

static inline unsigned int ColorToU32(Color color)
{
	int r = (int)(color.r * 255);
	int g = (int)(color.g * 255);
	int b = (int)(color.b * 255);
	int a = (int)(color.a * 255);

	unsigned int result = ((r & 0xff) << 0) | ((g & 0xff) << 8) | ((b & 0xff) << 16) | (a & 0xff) << 24;
	return result;
}