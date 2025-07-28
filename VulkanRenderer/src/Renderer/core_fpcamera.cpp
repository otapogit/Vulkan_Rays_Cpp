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
    CalcVelocity(dt);
    m_cameraPos += m_velocity * dt;
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

    // Sensibilidad del mouse reducida y configurable
    float baseSensitivity = 0.001f; // Sensibilidad base muy baja
    float sensitivity = m_mouseSpeed * baseSensitivity;

    // Opcional: Aplicar un factor de suavizado adicional
    float smoothingFactor = 0.5f; // Valor entre 0.1 y 1.0
    sensitivity *= smoothingFactor;

    // Calcular los ángulos de rotación
    float yawDelta = -DeltaMouse.x * sensitivity;   // Rotación horizontal (Y axis)
    float pitchDelta = -DeltaMouse.y * sensitivity; // Rotación vertical (X axis)

    // Limitar la velocidad máxima de rotación por frame
    float maxRotation = glm::radians(5.0f); // Máximo 5 grados por frame
    yawDelta = glm::clamp(yawDelta, -maxRotation, maxRotation);
    pitchDelta = glm::clamp(pitchDelta, -maxRotation, maxRotation);

    // Solo rotar si hay movimiento significativo del mouse
    if (glm::length(DeltaMouse) > 0.01f) {
        // Obtener el eje derecho (right) de la cámara para el pitch
        glm::mat3 rotMatrix = glm::mat3_cast(m_cameraOrientation);
        glm::vec3 rightAxis = rotMatrix[0];  // Eje derecho de la cámara

        // Yaw: rotación alrededor del eje Y mundial (arriba)
        glm::quat yawQuat = glm::angleAxis(yawDelta, glm::vec3(0, 1, 0));

        // Pitch: rotación alrededor del eje derecho (right) de la cámara
        glm::quat pitchQuat = glm::angleAxis(pitchDelta, rightAxis);

        // Aplicar rotaciones en el orden correcto
        // 1. Primero aplicar yaw (rotación mundial)
        m_cameraOrientation = yawQuat * m_cameraOrientation;

        // 2. Luego aplicar pitch (rotación local)
        m_cameraOrientation = m_cameraOrientation * pitchQuat;

        // Normalizar para evitar acumulación de errores
        m_cameraOrientation = glm::normalize(m_cameraOrientation);
    }

    // Opcional: Limitar el pitch para evitar que la cámara se voltee completamente
    // Puedes descomentar esto si quieres limitar la rotación vertical
    /*
    glm::vec3 forward = glm::mat3_cast(m_cameraOrientation) * glm::vec3(0, 0, -1);
    float currentPitch = asin(-forward.y);
    float maxPitch = glm::radians(89.0f); // Limitar a 89 grados

    if (currentPitch > maxPitch || currentPitch < -maxPitch) {
        // Recalcular sin el exceso de pitch
        glm::vec3 right = glm::mat3_cast(m_cameraOrientation) * glm::vec3(1, 0, 0);
        glm::vec3 up = glm::vec3(0, 1, 0);
        forward = glm::normalize(glm::cross(up, right));
        forward.y = glm::clamp(forward.y, -sin(maxPitch), sin(maxPitch));
        forward = glm::normalize(forward);

        right = glm::normalize(glm::cross(forward, up));
        up = glm::cross(right, forward);

        glm::mat3 rotMatrix;
        rotMatrix[0] = right;
        rotMatrix[1] = up;
        rotMatrix[2] = -forward;

        m_cameraOrientation = glm::quat_cast(rotMatrix);
    }
    */
}

void CameraFirstPerson::SetUpVector() {
    // FIX 6: Simplificar - mantener el up vector original
    // Esta función causaba problemas, mejor no recalcular constantemente
}



