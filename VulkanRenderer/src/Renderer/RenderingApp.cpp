#include "Renderer/RenderingApp.h"
#include "VulkanRenderer.cpp"
#include <vector>
#include "core_fpcamera.h"

struct Vertex {
	Vertex(const glm::vec3& p, const glm::vec2& t, const glm::vec3& n) {
		Pos = p;
		Tex = t;
		Nrm = n;
	}
	glm::vec3 Pos;
	glm::vec2 Tex;
	glm::vec3 Nrm;
};

static uint32_t createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, VulkanRenderer* renderer) {
	std::vector<glm::vec3> vertData = {};
	std::vector<glm::vec2> uvData = {};
	std::vector<glm::vec3> normData = {};

	for (int i = 0; i < vertices.size(); i++) {
		vertData.push_back(vertices[i].Pos);
		uvData.push_back(vertices[i].Tex);
		normData.push_back(vertices[i].Nrm);
	}

	return renderer->defineMesh(vertData, normData, uvData, indices);
}

VulkanRenderApp::VulkanRenderApp(int WindowWidth, int WindowHeight) {
	m_windowWidth = WindowWidth;
	m_windowHeight = WindowHeight;
}
VulkanRenderApp::~VulkanRenderApp() {
	// Limpia recursos aquí
	if (m_pCamera) {
		delete m_pCamera;
		m_pCamera = nullptr;
	}
	// Otros recursos que necesites limpiar
}

void VulkanRenderApp::initRt() {
	CreateCamera(glm::vec3(1.f, 5.f, 1.f));

	VulkanRenderer Renderer;

	Renderer.init();



	// Vértices únicos (eliminamos duplicados)
	std::vector<Vertex> vertices = {
		Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f},{1.0f,1.0f,1.0f}), // 0
		Vertex({ 1.0f, -1.0f, 0.0f}, {0.0f, 1.0f},{1.0f,1.0f,1.0f}), // 1
		Vertex({ 0.0f,  0.0f, 0.0f}, {1.0f, 1.0f},{1.0f,1.0f,1.0f}), // 2
		Vertex({ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f},{1.0f,1.0f,1.0f}), // 3
		Vertex({-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f},{1.0f,1.0f,1.0f})  // 4
	};

	// Índices para formar los triángulos
	std::vector<uint32_t> indices = {
		0, 1, 2,  // Primer triángulo
		3, 4, 2,   // Segundo triángulo
	};

	std::vector<glm::vec3> vertData = {};
	std::vector<glm::vec2> uvData = {};
	std::vector<glm::vec3> normData = {};
	for (int i = 0; i < vertices.size(); i++) {
		vertData.push_back(vertices[i].Pos);
		uvData.push_back(vertices[i].Tex);
		normData.push_back(vertices[i].Nrm);
	}


	// 1. MALLA FRONTAL GRANDE (ocupa la mayor parte de la pantalla frontal)
	std::vector<Vertex> frontMesh = {
		Vertex({-8.0f, -6.0f, 0.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}), // 0
		Vertex({ 8.0f, -6.0f, 0.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}), // 1
		Vertex({ 8.0f,  6.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}), // 2
		Vertex({-8.0f,  6.0f, 0.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f})  // 3
	};
	std::vector<uint32_t> frontIndices = {
		0, 1, 2,  // Primer triángulo
		0, 2, 3   // Segundo triángulo
	};
	uint32_t frontMeshId = createMesh(frontMesh, frontIndices, &Renderer);

	// 2. MALLA LATERAL DERECHA (plano vertical a la derecha)
	std::vector<Vertex> rightMesh = {
		Vertex({10.0f, -6.0f, -8.0f}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}), // 0
		Vertex({10.0f, -6.0f,  8.0f}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}), // 1
		Vertex({10.0f,  6.0f,  8.0f}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}), // 2
		Vertex({10.0f,  6.0f, -8.0f}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f})  // 3
	};
	std::vector<uint32_t> rightIndices = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t rightMeshId = createMesh(rightMesh, rightIndices, &Renderer);

	// 3. MALLA LATERAL IZQUIERDA (plano vertical a la izquierda)
	std::vector<Vertex> leftMesh = {
		Vertex({-10.0f, -6.0f,  8.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}), // 0
		Vertex({-10.0f, -6.0f, -8.0f}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}), // 1
		Vertex({-10.0f,  6.0f, -8.0f}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}), // 2
		Vertex({-10.0f,  6.0f,  8.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f})  // 3
	};
	std::vector<uint32_t> leftIndices = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t leftMeshId = createMesh(leftMesh, leftIndices, &Renderer);

	// 4. MALLA SUPERIOR (plano horizontal arriba)
	std::vector<Vertex> topMesh = {
		Vertex({-8.0f, 8.0f, -8.0f}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}), // 0
		Vertex({ 8.0f, 8.0f, -8.0f}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}), // 1
		Vertex({ 8.0f, 8.0f,  8.0f}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}), // 2
		Vertex({-8.0f, 8.0f,  8.0f}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f})  // 3
	};
	std::vector<uint32_t> topIndices = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t topMeshId = createMesh(topMesh, topIndices, &Renderer);

	// 5. MALLA INFERIOR (plano horizontal abajo)
	std::vector<Vertex> bottomMesh = {
		Vertex({-8.0f, -8.0f,  8.0f}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}), // 0
		Vertex({ 8.0f, -8.0f,  8.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}), // 1
		Vertex({ 8.0f, -8.0f, -8.0f}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}), // 2
		Vertex({-8.0f, -8.0f, -8.0f}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f})  // 3
	};
	std::vector<uint32_t> bottomIndices = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t bottomMeshId = createMesh(bottomMesh, bottomIndices, &Renderer);

	// 6. MALLA TRASERA (plano vertical al fondo)
	std::vector<Vertex> backMesh = {
		Vertex({ 8.0f, -6.0f, -12.0f}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}), // 0
		Vertex({-8.0f, -6.0f, -12.0f}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}), // 1
		Vertex({-8.0f,  6.0f, -12.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}), // 2
		Vertex({ 8.0f,  6.0f, -12.0f}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f})  // 3
	};
	std::vector<uint32_t> backIndices = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t backMeshId = createMesh(backMesh, backIndices, &Renderer);

	// 7. MALLA DIAGONAL (plano inclinado para variedad)
	std::vector<Vertex> diagonalMesh = {
		Vertex({-4.0f, -4.0f,  2.0f}, {0.0f, 0.0f}, {0.5f, 0.5f, 0.7f}), // 0
		Vertex({ 4.0f, -2.0f,  4.0f}, {1.0f, 0.0f}, {0.5f, 0.5f, 0.7f}), // 1
		Vertex({ 4.0f,  4.0f,  6.0f}, {1.0f, 1.0f}, {0.5f, 0.5f, 0.7f}), // 2
		Vertex({-4.0f,  2.0f,  4.0f}, {0.0f, 1.0f}, {0.5f, 0.5f, 0.7f})  // 3
	};
	std::vector<uint32_t> diagonalIndices = {
		0, 1, 2,
		0, 2, 3
	};
	uint32_t diagonalMeshId = createMesh(diagonalMesh, diagonalIndices, &Renderer);


	uint32_t meshid = Renderer.defineMesh(vertData, normData, uvData, indices);
	printf("Mesh Defined\n");

	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), meshid);
	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), topMeshId);
	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), bottomMeshId);
	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), diagonalMeshId);
	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), frontMeshId);
	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), rightMeshId);
	Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.001f, 0.f, 0.f)), glm::vec3(1.0f), leftMeshId);

	Renderer.setCamera(m_pCamera->GetVPMatrix(), glm::mat4(1.0f));
	//Renderer.setCamera(glm::mat4(1.0f), glm::mat4(1.0f));
	Renderer.setOutputResolution(800, 800);

	Renderer.save(true);

	Renderer.render();
	printf("Rendered everything\n");


}

void VulkanRenderApp::loop() {

}

 void VulkanRenderApp::CreateCamera(glm::vec3 pos) {

	float FOV = 45.0f;
	float znear = 0.1f;
	float zfar = 1000.0f;
	CreateCamera(pos, FOV, znear, zfar);
	printf("Created camera");
}

void VulkanRenderApp::CreateCamera(glm::vec3 pos, float FOV, float znear, float zfar) {
	if ((m_windowWidth == 0) || (m_windowHeight == 0)) {
		printf("Invalid window dims");
		exit(1);
	}

	glm::vec3 Target(0.0f, 0.0f, 0.0f);
	glm::vec3 Up(0.0f, 1.0f, 0.0f);

	m_pCamera = new CameraFirstPerson(pos, Target, Up, FOV, m_windowWidth, m_windowHeight, znear, zfar);
}



