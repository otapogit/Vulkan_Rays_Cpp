#include "core_fpcamera.h"
#include <GLFW/glfw3.h>
bool CameraFirstPerson::GLFWCameraHandler(CameraMovement& movement, int Key, int Action, int Mods) {

	bool Press = Action != GLFW_RELEASE;

	bool Handled = true;

	switch (Key) {
	case GLFW_KEY_W:
		movement.Backward = Press;
		break;
	case GLFW_KEY_S:
		movement.Forward = Press;
		break;
	case GLFW_KEY_A:
		movement.StrafeLeft = Press;
		break;
	case GLFW_KEY_D:
		movement.StrafeRight = Press;
		break;
	case GLFW_KEY_O:
		movement.Down = Press;
		break;
	case GLFW_KEY_L:
		movement.Up = Press;
		break;
	case GLFW_KEY_P:
		movement.FastSpeed = Press;
		break;
	}

	return Handled;
}

// Implementación del handler de mouse
bool CameraFirstPerson::GLFWMouseButtonHandler(int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            // Cambiar a modo orbital
            m_cameraMode = CameraMode::Orbital;

            // Calcular el target orbital basado en la dirección actual de la cámara
            glm::vec3 forward = glm::mat3_cast(m_cameraOrientation) * glm::vec3(0, 0, -1);
            m_orbitTarget = m_cameraPos + forward * m_orbitDistance;

            // Calcular ángulos iniciales de órbita
            glm::vec3 toCamera = m_cameraPos - m_orbitTarget;
            m_orbitDistance = glm::length(toCamera);

            if (m_orbitDistance > 0.0f) {
                toCamera = glm::normalize(toCamera);
                m_orbitYaw = atan2(toCamera.x, toCamera.z);
                m_orbitPitch = asin(toCamera.y);
            }

            m_mouseState.m_buttonPressed = true;
        }
        else if (action == GLFW_RELEASE) {
            m_mouseState.m_buttonPressed = false;
        }
        return true;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Cambiar a modo primera persona
            m_cameraMode = CameraMode::FirstPerson;
            m_mouseState.m_buttonPressed = true;
        }
        else if (action == GLFW_RELEASE) {
            m_mouseState.m_buttonPressed = false;
        }
        return true;
    }

    return false;
}

CameraFirstPerson::CameraFirstPerson(const glm::vec3& Pos, const glm::vec3& Target,
    const glm::vec3& Up, float FOV, float width, float height, float znear, float zfar) {
    m_cameraPos = Pos;
    m_up = Up;
    float ar = width / height;

    // FIX 1: Convertir FOV a radianes
    m_projection = glm::perspective(glm::radians(FOV), ar, znear, zfar);

    // FIX 2: Calcular quaternion de orientación correctamente
    glm::vec3 direction = glm::normalize(Target - Pos);
    glm::vec3 right = glm::normalize(glm::cross(direction, Up));
    glm::vec3 up = glm::normalize(glm::cross(right, direction));

    // Crear matriz de rotación y convertir a quaternion
    glm::mat3 rotationMatrix;
    rotationMatrix[0] = right;
    rotationMatrix[1] = up;
    rotationMatrix[2] = -direction; // Importante: negativo porque miramos hacia -Z

    m_cameraOrientation = glm::quat_cast(rotationMatrix);
}

/*
glm::mat4 CameraFirstPerson::GetViewMatrix() const {
    // FIX 3: Usar la función lookAt directamente es más confiable
    glm::vec3 forward = glm::normalize(glm::mat3_cast(m_cameraOrientation) * glm::vec3(0, 0, -1));
    glm::vec3 target = m_cameraPos + forward;
    return glm::lookAt(m_cameraPos, target, m_up);
}*/
glm::mat4 CameraFirstPerson::GetViewMatrix() const {
    glm::vec3 forward = glm::mat3_cast(m_cameraOrientation) * glm::vec3(0, 0, -1);
    glm::vec3 up = glm::mat3_cast(m_cameraOrientation) * glm::vec3(0, 1, 0);
    glm::vec3 target = m_cameraPos + forward;
    return glm::lookAt(m_cameraPos, target, up);
}

glm::mat4 CameraFirstPerson::GetVPMatrix() const {
    glm::mat4 View = GetViewMatrix();
    return m_projection * View;
}

void CameraFirstPerson::Update(float dt) {
    if (m_mouseState.m_buttonPressed && (m_mouseState.m_pos != m_oldMousePos)) {
        CalcCameraOrientation();
    }
    m_oldMousePos = m_mouseState.m_pos;

    // Solo aplicar movimiento de velocidad en modo primera persona
    if (m_cameraMode == CameraMode::FirstPerson) {
        CalcVelocity(dt);
        m_cameraPos += m_velocity * dt;
    }
    // En modo orbital, la posición se calcula en UpdateOrbitalCamera()
}


void CameraFirstPerson::CalcVelocity(float dt) {
    glm::vec3 Acceleration = CalcAcceleration();
    if (Acceleration == glm::vec3(0.0f)) {
        m_velocity -= m_velocity * std::min(dt * m_damping, 1.0f);
    }
    else {
        m_velocity += Acceleration * m_acceleration * dt;
        float MaxSpeed = m_movement.FastSpeed ? m_maxSpeed * m_fastCoef : m_maxSpeed;
        if (glm::length(m_velocity) > MaxSpeed) {
            m_velocity = glm::normalize(m_velocity) * MaxSpeed;
        }
    }
}

glm::vec3 CameraFirstPerson::CalcAcceleration() {
    // FIX 4: Obtener vectores de dirección correctamente
    glm::mat3 rotMat = glm::mat3_cast(m_cameraOrientation);
    glm::vec3 right = rotMat[0];
    glm::vec3 up = rotMat[1];
    glm::vec3 forward = -rotMat[2]; // Negativo porque miramos hacia -Z

    glm::vec3 Acceleration = glm::vec3(0.0f);
    if (m_movement.Forward) {
        Acceleration += forward;
    }
    if (m_movement.Backward) {
        Acceleration -= forward;
    }
    if (m_movement.StrafeLeft) {
        Acceleration -= right;
    }
    if (m_movement.StrafeRight) {
        Acceleration += right;
    }
    if (m_movement.Up) {
        Acceleration += up;
    }
    if (m_movement.Down) {
        Acceleration -= up;
    }

    return Acceleration;
}

void CameraFirstPerson::CalcCameraOrientation() {
    glm::vec2 DeltaMouse = m_mouseState.m_pos - m_oldMousePos;

    if (glm::length(DeltaMouse) < 0.01f) return;

    float baseSensitivity = 0.001f;
    float sensitivity = m_mouseSpeed * baseSensitivity;
    float smoothingFactor = 0.5f;
    sensitivity *= smoothingFactor;

    if (m_cameraMode == CameraMode::FirstPerson) {
        // Código original para primera persona
        float yawDelta = -DeltaMouse.x * sensitivity;
        float pitchDelta = -DeltaMouse.y * sensitivity;

        float maxRotation = glm::radians(5.0f);
        yawDelta = glm::clamp(yawDelta, -maxRotation, maxRotation);
        pitchDelta = glm::clamp(pitchDelta, -maxRotation, maxRotation);

        glm::mat3 rotMatrix = glm::mat3_cast(m_cameraOrientation);
        glm::vec3 rightAxis = rotMatrix[0];

        glm::quat yawQuat = glm::angleAxis(yawDelta, glm::vec3(0, 1, 0));
        glm::quat pitchQuat = glm::angleAxis(pitchDelta, rightAxis);

        m_cameraOrientation = yawQuat * m_cameraOrientation;
        m_cameraOrientation = m_cameraOrientation * pitchQuat;
        m_cameraOrientation = glm::normalize(m_cameraOrientation);

    }
    else if (m_cameraMode == CameraMode::Orbital) {
        // Modo orbital
        float yawDelta = -DeltaMouse.x * sensitivity * 9.0f;   // Más sensible para órbita
        float pitchDelta = -DeltaMouse.y * sensitivity * 9.0f;

        m_orbitYaw += yawDelta;
        m_orbitPitch += pitchDelta;

        // Limitar el pitch para evitar que la cámara se voltee
        float maxPitch = glm::radians(89.0f);
        m_orbitPitch = glm::clamp(m_orbitPitch, -maxPitch, maxPitch);

        UpdateOrbitalCamera();
    }
}



void CameraFirstPerson::UpdateOrbitalCamera() {
    // Usar coordenadas esféricas completas para permitir rotación completa
    float cosYaw = cos(m_orbitYaw);
    float sinYaw = sin(m_orbitYaw);
    float cosPitch = cos(m_orbitPitch);
    float sinPitch = sin(m_orbitPitch);

    // Calcular posición usando coordenadas esféricas estándar
    // Con pitch que va de -π/2 a π/2 y yaw que va de 0 a 2π
    glm::vec3 offset;
    offset.x = m_orbitDistance * cosPitch * sinYaw;
    offset.y = m_orbitDistance * sinPitch;
    offset.z = m_orbitDistance * cosPitch * cosYaw;

    m_cameraPos = m_orbitTarget + offset;

    // Calcular la orientación para mirar hacia el target
    glm::vec3 direction = glm::normalize(m_orbitTarget - m_cameraPos);

    // Para el vector up, necesitamos manejarlo de manera especial para evitar gimbal lock
    glm::vec3 worldUp = glm::vec3(0, 1, 0);
    glm::vec3 right;
    glm::vec3 up;

    // Si estamos muy cerca de mirar directamente hacia arriba o abajo,
    // usar un vector de referencia diferente para evitar gimbal lock
    if (abs(glm::dot(direction, worldUp)) > 0.99f) {
        // Usar el eje Z como referencia cuando miramos casi verticalmente
        glm::vec3 tempRef = glm::vec3(0, 0, 1);
        right = glm::normalize(glm::cross(tempRef, direction));
        up = glm::normalize(glm::cross(direction, right));
    }
    else {
        // Comportamiento normal
        right = glm::normalize(glm::cross(worldUp, direction));
        up = glm::normalize(glm::cross(direction, right));
    }

    // Crear matriz de rotación y convertir a quaternion
    glm::mat3 rotationMatrix;
    rotationMatrix[0] = right;
    rotationMatrix[1] = up;
    rotationMatrix[2] = -direction;

    m_cameraOrientation = glm::quat_cast(rotationMatrix);
}

void CameraFirstPerson::SetOrbitTarget(const glm::vec3& target) {
    m_orbitTarget = target;
    if (m_cameraMode == CameraMode::Orbital) {
        UpdateOrbitalCamera();
    }
}
void CameraFirstPerson::SetOrbitDistance(float distance) {
    m_orbitDistance = distance;
    if (m_cameraMode == CameraMode::Orbital) {
        UpdateOrbitalCamera();
    }
}

void CameraFirstPerson::SetCameraMode(CameraMode mode) {
    if (m_cameraMode != mode) {
        m_cameraMode = mode;
        if (mode == CameraMode::Orbital) {
            // Configurar parámetros iniciales del modo orbital
            glm::vec3 forward = glm::mat3_cast(m_cameraOrientation) * glm::vec3(0, 0, -1);
            m_orbitTarget = m_cameraPos + forward * m_orbitDistance;
            UpdateOrbitalCamera();
        }
    }
}

void CameraFirstPerson::GLFWScrollHandler(double xoffset, double yoffset) {
    if (m_cameraMode == CameraMode::Orbital) {
        float zoomSpeed = 1.0f;
        m_orbitDistance -= yoffset * zoomSpeed;
        m_orbitDistance = glm::clamp(m_orbitDistance, 0.5f, 100.0f); // Limitar distancia
        UpdateOrbitalCamera();
    }
}

void CameraFirstPerson::SetUpVector() {
    // FIX 6: Simplificar - mantener el up vector original
    // Esta función causaba problemas, mejor no recalcular constantemente
}



