// camera_glfw.h
#pragma once
#include "core_fpcamera.h"
#include <GLFW/glfw3.h>

class CameraGLFWController {
public:
    static void setupCallbacks(GLFWwindow* window, CameraFirstPerson* camera);
    static void cleanup();

    // Callbacks estáticos
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

private:
    static CameraFirstPerson* s_camera;
    static bool s_firstMouse;
    static bool s_cursorCaptured;
};


