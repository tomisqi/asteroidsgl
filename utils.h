#pragma once
#include <stdlib.h>

static inline float Wrapf(float x, float min, float max)
{
	if (x > max) return min;
	if (x < min) return max;
	return x;
}

static inline float Clampf(int x, float min, float max)
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