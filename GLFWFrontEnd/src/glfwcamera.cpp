// camera_glfw.cpp
#include "glfwcamera.h"
#include <iostream>

// Variables estáticas
CameraFirstPerson* CameraGLFWController::s_camera = nullptr;
bool CameraGLFWController::s_firstMouse = true;
bool CameraGLFWController::s_cursorCaptured = false;

void CameraGLFWController::setupCallbacks(GLFWwindow* window, CameraFirstPerson* camera) {
    s_camera = camera;

    // Configurar callbacks
    glfwSetKeyCallback(window, keyCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Configurar modo de cursor inicial
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    std::cout << "Controles de cámara configurados:" << std::endl;
    std::cout << "- WASD: Movimiento" << std::endl;
    std::cout << "- O/L: Subir/Bajar" << std::endl;
    std::cout << "- P: Velocidad rápida" << std::endl;
    std::cout << "- Click derecho + mouse: Mirar alrededor" << std::endl;
    std::cout << "- Tab: Capturar/liberar cursor" << std::endl;
}

void CameraGLFWController::cleanup() {
    s_camera = nullptr;
}

void CameraGLFWController::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (!s_camera) return;

    // Manejar captura de cursor con Tab
    if (key == GLFW_KEY_TAB && action == GLFW_PRESS) {
        s_cursorCaptured = !s_cursorCaptured;
        if (s_cursorCaptured) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            s_firstMouse = true; // Reset para evitar saltos
        }
        else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            s_camera->m_mouseState.m_buttonPressed = false;
        }
        return;
    }

    // Usar el handler existente de la cámara
    s_camera->GLFWCameraHandler(s_camera->m_movement, key, action, mods);
}

void CameraGLFWController::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!s_camera) return;

    glm::vec2 currentPos(static_cast<float>(xpos), static_cast<float>(ypos));

    if (s_cursorCaptured || s_camera->m_mouseState.m_buttonPressed) {
        if (s_firstMouse) {
            s_camera->m_oldMousePos = currentPos; // Importante: establecer la posición anterior
            s_camera->m_mouseState.m_pos = currentPos;
            s_firstMouse = false;
        }
        else {
            s_camera->m_oldMousePos = s_camera->m_mouseState.m_pos; // Guardar posición anterior
            s_camera->m_mouseState.m_pos = currentPos;
        }
    }
}

void CameraGLFWController::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (!s_camera) return;

    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            s_camera->m_mouseState.m_buttonPressed = true;
            s_firstMouse = true; // Reset para evitar saltos

            // Obtener posición actual del cursor
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            s_camera->m_mouseState.m_pos = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
        }
        else if (action == GLFW_RELEASE) {
            s_camera->m_mouseState.m_buttonPressed = false;
        }
    }
}

void CameraGLFWController::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (!s_camera) return;

    // Opcional: Ajustar velocidad de movimiento con scroll
    float speedChange = static_cast<float>(yoffset) * 0.5f;
    s_camera->m_maxSpeed = std::max(0.1f, s_camera->m_maxSpeed + speedChange);

    std::cout << "Velocidad de cámara: " << s_camera->m_maxSpeed << std::endl;
}