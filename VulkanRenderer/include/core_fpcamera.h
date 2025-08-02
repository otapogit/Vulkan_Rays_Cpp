// core_fpcamera.h
#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
//#include <glm/gtx/quaternion.hpp>

enum class CameraMode {
    FirstPerson,
    Orbital
};

struct CameraMovement {
    bool Forward = false;
    bool Backward = false;
    bool StrafeLeft = false;
    bool StrafeRight = false;
    bool Up = false;
    bool Down = false;
    bool FastSpeed = false;
};

struct MouseState {
    glm::vec2 m_pos = glm::vec2(0.0f);
    bool m_buttonPressed = false;
};

class CameraFirstPerson {
public:
    glm::vec3 m_cameraPos;
    glm::quat m_cameraOrientation;
    glm::vec3 m_up;
    glm::mat4 m_projection;

    // Variables de movimiento
    glm::vec3 m_velocity = glm::vec3(0.0f);
    CameraMovement m_movement;

    // Variables de mouse
    MouseState m_mouseState;
    glm::vec2 m_oldMousePos = glm::vec2(0.0f);

    // Parámetros de movimiento
    float m_maxSpeed = 10.0f;
    float m_acceleration = 20.0f;
    float m_damping = 5.0f;
    float m_fastCoef = 3.0f;
    float m_mouseSpeed = 1.0f;

    // Variables para modo orbital
    CameraMode m_cameraMode = CameraMode::FirstPerson;
    glm::vec3 m_orbitTarget = glm::vec3(0.0f);
    float m_orbitDistance = 10.0f;
    float m_orbitYaw = 0.0f;
    float m_orbitPitch = 0.0f;


    // Constructor
    CameraFirstPerson(const glm::vec3& Pos, const glm::vec3& Target,
        const glm::vec3& Up, float FOV, float width, float height, float znear, float zfar);

    // Handlers de entrada
    bool GLFWCameraHandler(CameraMovement& movement, int Key, int Action, int Mods);
    bool GLFWMouseButtonHandler(int button, int action, int mods);
    void GLFWScrollHandler(double xoffset, double yoffset);

    // Funciones principales
    glm::mat4 GetViewMatrix() const;
    glm::mat4 GetVPMatrix() const;
    void Update(float dt);

    // Funciones de movimiento
    void CalcVelocity(float dt);
    glm::vec3 CalcAcceleration();
    void CalcCameraOrientation();

    // Funciones de modo orbital
    void SetOrbitTarget(const glm::vec3& target);
    void SetOrbitDistance(float distance);
    void UpdateOrbitalCamera();
    void SetCameraMode(CameraMode mode);
    CameraMode GetCameraMode() const { return m_cameraMode; }

    // Utilidades
    void SetUpVector();
    void SetMouseState(const MouseState& state) { m_mouseState = state; }
    void SetMousePosition(const glm::vec2& pos) { m_mouseState.m_pos = pos; }

    // Getters
    glm::vec3 GetPosition() const { return m_cameraPos; }
    glm::quat GetOrientation() const { return m_cameraOrientation; }
    glm::vec3 GetOrbitTarget() const { return m_orbitTarget; }
    float GetOrbitDistance() const { return m_orbitDistance; }
};
