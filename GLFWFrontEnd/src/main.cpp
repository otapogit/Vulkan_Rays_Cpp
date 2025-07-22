
#include <stdio.h> // fprintf, stderr

#include <GLFW/glfw3.h>
#include <Renderer/RenderingApp.h>
#include "PGUPV.h"

using namespace PGUPV;

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
// this function will be called internally by GLFW whenever an error occur.
void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s (%d)\n", description, error);
}

int main() {
	// tell GLFW to call error_callback if an internal error ever occur at some point inside GLFW functions.
	glfwSetErrorCallback(error_callback);

	// initialize all the internal state of GLFW
	if (!glfwInit()) {
		return -1;
	}

	// create the window
	int resx = 640, resy = 480;
	GLFWwindow* window = glfwCreateWindow(resx, resy, "GLFW: Creating a window.", NULL, NULL);

	// check if the opening of the window failed whatever reason and clean up
	if (!window) {
		glfwTerminate();
		return -2;
	}

	// in principle we can have multiple windows, 
	// so we set the newly created on as "current"
	glfwMakeContextCurrent(window);

	// Enable v-sync for now, if possible
	glfwSwapInterval(1);

	VulkanRenderApp app(1920,1080);
	app.initRt();

	// main loop
	while (!glfwWindowShouldClose(window)) {
		// listen for events (keyboard, mouse, etc.). ignored for now, but useful later
		glfwPollEvents();

		// make it close on pressing escape
		if (glfwGetKey(window, GLFW_KEY_ESCAPE))
			glfwSetWindowShouldClose(window, GLFW_TRUE);

		//App.loop();

		// swap buffers (replace the old image with a new one)
		// this won't have any visible effect until we add actual drawing
		glfwSwapBuffers(window);
	}

	// clean up
	glfwTerminate();

	return 0;
}