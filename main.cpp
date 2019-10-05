#include <stdio.h>
#include <stdlib.h>
#include <GLFW/glfw3.h>
#include "vector.h"
#include "color.h"
#include "render.h"
#include "input.h"
#include "asteroids.h"
#include "utils.h"
#include <time.h>

// TODO:
// [ ] Asteroids!
// [ ] Text
// [ ] Menus
// [ ] Decelerate
// [ ] Drawing layers
// [ ] UI
// [ ] Collision

#define WINDOW_SIZE			1000

static void GlfwErrorCallback(int error, const char* description)
{
	fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

static void SetProjectionMatrix()
{
	// ignore model view matrix - dont need it
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// load a different projection matrix
	// this will let us use a screen space coordinates that are then transformed to clip space by opengl
	// Pcl= Mproj x P
	// where,
	//  Pcl: point in clip space
	//  Mproj: Projection maxtrix
	//  P: point in screen space
	glMatrixMode(GL_PROJECTION);
	float v = 2.0f / WINDOW_SIZE;
	float Mproj[] =
	{
		v,  0,  0,  0,
		0,  v,  0,  0,
		0,  0,  1,  0,
		-1, -1,  0,  1,
	};
	glLoadMatrixf(Mproj);
}

int main(void)
{
	glfwSetErrorCallback(GlfwErrorCallback);
	if (!glfwInit()) return -1;

	GLFWwindow* window = glfwCreateWindow(WINDOW_SIZE, WINDOW_SIZE, "AsteroidsGL", NULL, NULL);
	if (window == NULL) return 1;

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1); // Enable vsync
	
	SetProjectionMatrix();

	srand(time(NULL)); // Initialize random seed

	GameInput_Init();
	GameInput_BindButton(BUTTON_SPACE, GLFW_KEY_SPACE);
	GameInput_BindButton(BUTTON_UP_ARROW, GLFW_KEY_UP);
	GameInput_BindButton(BUTTON_LEFT_ARROW, GLFW_KEY_LEFT);
	GameInput_BindButton(BUTTON_RIGHT_ARROW, GLFW_KEY_RIGHT);
	
	Renderer_Init(2048);

	GameStart(WINDOW_SIZE, WINDOW_SIZE);

	ButtonState buttonStates[MAX_BUTTONS];
	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
		glClear(GL_COLOR_BUFFER_BIT);

		for (int i = 0; i < MAX_BUTTONS; i++)
		{
			buttonStates[i] = (ButtonState)glfwGetKey(window, GameInput_GetBinding(i)); // TODO: Remove casting

		}

		GameInput_NewFrame(buttonStates);
		Renderer_NewFrame();

		GameUpdate();

		Renderer_Render();

		glfwMakeContextCurrent(window);
		glfwSwapBuffers(window);
	}

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}