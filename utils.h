#pragma once
#include <stdlib.h>

static inline float Wrapf(float x, float min, float max)
{
	if (x > max) return min;
	if (x < min) return max;
	return x;
}

static inline float Clampf(float x, float min, float max)
{
	if (x > max) return max;
	if (x < min) return min;
	return x;
}

static inline int GetRandomValue(int min, int max)
{
	if (min > max)
	{
		int tmp = max;
		int max = min;
		int min = tmp;
	}

	return min + rand() % (abs(max - min) + 1);
}

static inline int GetRandomSign()
{
	int signs[2] = { -1, 1 };
	int randVal = GetRandomValue(0, 1);
	return signs[randVal];
}