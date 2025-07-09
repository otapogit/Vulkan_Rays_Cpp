#include <core/core.h>
#include <core/core_shader.h>

#include <core/core_simple_mesh.h>
#include <core/core_glfw.h>
#include <core/core_rt.h>
#include <core/core_vertex.h>
#include <iostream>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <vulkan/vulkan_core.h>
//#include <GLFW/glfw3.h>
#include "VulkanRenderer.cpp"

#include "core_fpcamera.h"
#include <array>

#define WINDOW_HEIGHT 1080
#define WINDOW_WIDTH 1920


class VulkanApp /* : public core::GLFWcallbacks */{
public:

	VulkanApp(int WindowWidth, int WindowHeight) {
		m_windowWidth = WindowWidth;
		m_windowHeight = WindowHeight;
	}

	~VulkanApp() {

		m_raytracer.cleanup();

		m_texture1.Destroy(m_device);
		m_texture2.Destroy(m_device);

		m_vkcore.FreeCommandBuffers((uint32_t)m_cmdBufs.size(), m_cmdBufs.data());

		vkDestroyShaderModule(m_vkcore.GetDevice(), m_vs, NULL);
		vkDestroyShaderModule(m_vkcore.GetDevice(), m_fs, NULL);
		vkDestroyRenderPass(m_vkcore.GetDevice(), m_renderPass, NULL);
		//delete m_pipeline;
		m_mesh.Destroy(m_device);
		m_mesh1.Destroy(m_device);
		for (int i = 0; i < m_uniformBuffers.size(); i++) {
			m_uniformBuffers[i].Destroy(m_device);
		}
		
	}

	void Init(const char* pAppName) {

		m_vkcore.Init(pAppName, WINDOW_WIDTH, WINDOW_HEIGHT);
		
		m_device = m_vkcore.GetDevice();
		m_numImages = 1;

		m_pQueue = m_vkcore.GetQueue();

		m_renderPass = m_vkcore.CreateSimpleRenderPass();
		m_frameBuffers = m_vkcore.CreateFrameBuffers(m_renderPass);

		//CreateShaders();

		CreateVertexBuffer();
		CreateVertexBuffer2();
		//LoadTexture();
		//CreateUniformBuffers();

		//CreatePipeline();
		CreateCamera(glm::vec3(10.f,10.f,10.f));

		std::vector<core::SimpleMesh> meshes = { m_mesh,m_mesh1 };

		if (rt_active) {
			//Raytracer
			m_raytracer.initRayTracing(m_vkcore.GetSelectedPhysicalDevice(), &m_device);
			m_raytracer.setup(m_vkcore.GetCommandPool(), &m_vkcore);


			m_raytracer.createRtDescriptorSet();
			//ver como pasar las matrices de modelo
			m_raytracer.createBottomLevelAS(meshes);
			m_raytracer.createTopLevelAS();

			m_raytracer.createMvpDescriptorSet();

			glm::mat4 WVP = glm::affineInverse(m_pCamera->GetVPMatrix());


			m_raytracer.UpdateMvpMatrix(WVP);

			VkShaderModule rgen = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "shaders/raytrace.rgen");
			VkShaderModule rmiss = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "shaders/raytrace.rmiss");
			VkShaderModule rchit = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "shaders/raytrace.rchit");

			m_raytracer.createRtPipeline(rgen, rmiss, rchit);
			m_raytracer.createRtShaderBindingTable();
			m_raytracer.UpdateAccStructure();

			vkDestroyShaderModule(m_vkcore.GetDevice(), rgen, nullptr);
			vkDestroyShaderModule(m_vkcore.GetDevice(), rmiss, nullptr);
			vkDestroyShaderModule(m_vkcore.GetDevice(), rchit, nullptr);

			m_raytracer.render(800, 800, true, "Test1.png");
		}

		//CreateCommandBuffers();
		//RecordCommandBuffers();


	}


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

	void initRt() {


		CreateCamera(glm::vec3(1.f,5.f,1.f));

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
		
		Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.00f, 0.f, 0.f)), glm::vec3(1.0f), meshid);
		Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.00f, 0.f, 0.f)), glm::vec3(1.0f), topMeshId);
		Renderer.addMesh(glm::translate(glm::mat4(1.0f),glm::vec3(.00f,0.f,0.f)), glm::vec3(1.0f), bottomMeshId);
		Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.00f, 0.f, 0.f)), glm::vec3(1.0f), diagonalMeshId);
		Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.00f, 0.f, 0.f)), glm::vec3(1.0f), frontMeshId);
		Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.00f, 0.f, 0.f)), glm::vec3(1.0f), rightMeshId);
		Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(.00f, 0.f, 0.f)), glm::vec3(1.0f), leftMeshId);

		Renderer.setCamera(m_pCamera->GetVPMatrix(), glm::mat4(1.0f));
		//Renderer.setCamera(glm::mat4(1.0f), glm::mat4(1.0f));
		Renderer.setOutputResolution(800, 800);
		


		Renderer.render();
		printf("Rendered everything\n");


	}

	uint32_t createMesh(const std::vector<Vertex>& vertices, const std::vector<uint32_t>& indices, VulkanRenderer* renderer) {
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

	void RenderScene() {
		/*uint32_t ImageIndex = m_pQueue->AcquireNextImage();
		UpdateUniformBuffers(ImageIndex);
		m_pQueue->SubmitAsync(m_cmdBufs[ImageIndex]);
		
		m_pQueue->Present(ImageIndex);
		*/

		//uint32_t ImageIndex = 0;

		//UpdateUniformBuffers(ImageIndex);
		//m_pQueue->SubmitSync(m_cmdBufs[ImageIndex]);
		m_pQueue->WaitIdle();
	}
	/*
	void Execute() {
		float CurTime = (float)glfwGetTime();

		while (!glfwWindowShouldClose(m_pWindow)) {
			float Time = (float)glfwGetTime();
			float dt = Time - CurTime;
			m_pCamera->Update(dt);
			//RenderScene();
			CurTime = Time;
			glfwPollEvents();
		}

		//Fix temporal, guardar imagenes en disco es una operacion muy costosa 2 MB
		//m_vkcore.SaveOffscreenImage("output.png");
		
		glfwTerminate();
	}

	//Callback
	void Key(GLFWwindow* pWindow, int Key, int Scancode, int Action, int Mods) {
		if ((Key == GLFW_KEY_ESCAPE) && (Action == GLFW_PRESS)) {

			glfwSetWindowShouldClose(pWindow, GLFW_TRUE);
		}

		bool handled = m_pCamera->GLFWCameraHandler(m_pCamera->m_movement, Key, Action, Mods);


	}
	void MouseMove(GLFWwindow* pWindow, double xpos, double ypos) {
		
		m_pCamera->m_mouseState.m_pos.x = (float)xpos / (float)WINDOW_WIDTH;
		m_pCamera->m_mouseState.m_pos.y = (float)ypos / (float)WINDOW_HEIGHT;
	}
	void MouseButton(GLFWwindow* pWindow, int Button, int Action, int Mods) {
	
		if (Button == GLFW_MOUSE_BUTTON_LEFT) {
			m_pCamera->m_mouseState.m_buttonPressed = (Action == GLFW_PRESS);
		}
	}
	*/

	//igual esto que en ele shader
	struct UniformData {
		glm::mat4 MVP;
	};


private:
	void CreateUniformBuffers() {
		m_uniformBuffers = m_vkcore.CreateUniformBuffers(sizeof(UniformData));
	}

	void CreateCommandBuffers() {
		m_cmdBufs.resize(m_numImages);
		m_vkcore.CreateCommandBuffer(m_numImages, m_cmdBufs.data());

	}

	void LoadTexture() {
		m_mesh.m_pTex = new core::VulkanTexture;
		m_vkcore.CreateTexture("Textures/carlos.jpg", *(m_mesh.m_pTex));
		//Cuidado al copiar pointers xq dara error al borrarlos
		m_mesh1.m_pTex = new core::VulkanTexture;
		m_vkcore.CreateTexture("Textures/hqdefault.jpg", *(m_mesh1.m_pTex));
	}


	void CreatePipeline() {

		//m_pipeline = new core::GraphicsPipeline(m_vkcore.GetDevice(), m_pWindow, m_renderPass,m_numImages,m_uniformBuffers,sizeof(UniformData), m_vs, m_fs);
	}

	void CreateShaders() {
		m_vs = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "test.vert");
		m_fs = core::CreateShaderModuleFromText(m_vkcore.GetDevice(), "test.frag");
	}


	void CreateVertexBuffer() {
		struct Vertex {
			Vertex(const glm::vec3& p, const glm::vec2& t) {
				Pos = p;
				Tex = t;
			}
			glm::vec3 Pos;
			glm::vec2 Tex;
		};

		// Vértices únicos (eliminamos duplicados)
		std::vector<Vertex> vertices = {
			Vertex({-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f}), // 0
			Vertex({ 1.0f, -1.0f, 0.0f}, {0.0f, 1.0f}), // 1
			Vertex({ 0.0f,  0.0f, 0.0f}, {1.0f, 1.0f}), // 2
			Vertex({ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f}), // 3
			Vertex({-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f})  // 4
		};

		// Índices para formar los triángulos
		std::vector<uint32_t> indices = {
			0, 1, 2,  // Primer triángulo
			3, 4, 2,   // Segundo triángulo
		};

		// Crear vertex buffer
		m_mesh.m_vertexBufferSize = sizeof(vertices[0]) * vertices.size();
		m_mesh.m_vb = m_vkcore.CreateVertexBuffer(vertices.data(), m_mesh.m_vertexBufferSize, rt_active);

		// Crear index buffer
		m_mesh.m_indexBufferSize = sizeof(indices[0]) * indices.size();
		m_mesh.m_indexbuffer = m_vkcore.CreateIndexBuffer(indices.data(), m_mesh.m_indexBufferSize, rt_active);
		m_mesh.m_indexType = VK_INDEX_TYPE_UINT32;

		m_mesh.vertexcount = indices.size(); // Número de índices, no vértices
	}

	void CreateVertexBuffer2() {
		struct Vertex {
			Vertex(const glm::vec3& p, const glm::vec2& t) {
				Pos = p;
				Tex = t;
			}
			glm::vec3 Pos;
			glm::vec2 Tex;
		};

		float size = 5.0f; // Radio desde el centro

		// Vértices únicos para un cuadrado
		std::vector<core::VertexObj> vertices = {
			core::VertexObj({-size, -4.0f, -size}, {0.0f, 0.0f}), // 0: Esquina inferior-izquierda
			core::VertexObj({ size, -4.0f, -size}, {1.0f, 0.0f}), // 1: Esquina inferior-derecha  
			core::VertexObj({ size, -4.0f,  size}, {1.0f, 1.0f}), // 2: Esquina superior-derecha
			core::VertexObj({-size, -4.0f,  size}, {0.0f, 1.0f})  // 3: Esquina superior-izquierda
		};

		// Índices para formar dos triángulos que crean un cuadrado
		std::vector<uint32_t> indices = {
			0, 1, 2,  // Primer triángulo: inferior-izq, inferior-der, superior-der
			0, 2, 3   // Segundo triángulo: inferior-izq, superior-der, superior-izq

		};
		//Parece que el error se ha solucionado solo

		// Crear vertex buffer
		m_mesh1.m_vertexBufferSize = sizeof(vertices[0]) * vertices.size();
		m_mesh1.m_vb = m_vkcore.CreateVertexBuffer(vertices.data(), m_mesh1.m_vertexBufferSize, rt_active);

		// Crear index buffer
		m_mesh1.m_indexBufferSize = sizeof(indices[0]) * indices.size();
		m_mesh1.m_indexbuffer = m_vkcore.CreateIndexBuffer(indices.data(), m_mesh1.m_indexBufferSize, rt_active);
		m_mesh1.m_indexType = VK_INDEX_TYPE_UINT32;

		m_mesh1.vertexcount = indices.size(); // Número de índices, no vértices
	}

	void UpdateUniformBuffers(uint32_t ImageIndex) {
		/*static float foo = 0.0f;
		glm::mat4 Rotate = glm::mat4(1.0);
		Rotate = glm::rotate(Rotate, glm::radians(foo), glm::normalize(glm::vec3(0.0f, 0.0f, 1.0f)));
		foo += 0.005f;
		*/
		glm::mat4 WVP = m_pCamera->GetVPMatrix();

		//glm::mat4 WVP = Rotate;
		m_uniformBuffers[ImageIndex].Update(m_device, &WVP, sizeof(WVP));
	}

	void RecordCommandBuffers() {
		VkClearColorValue ClearColor = { 1.0f,0.6f,0.6f,0.0f };

		/*
		VkImageSubresourceRange ImageRange = {};

		VkImageMemoryBarrier PresentToClearBarrier = {};

		VkImageMemoryBarrier ClearToPresentBarrier = {};

		Ahora solo soporta 1 textura, para soportar multiples hacer un array de samplers y elegir una de esas mediante pushconstants de cada una

		*/

		std::array<VkClearValue, 2> ClearValue = {
			VkClearValue{},  // Se inicializa a cero
			VkClearValue{}   // Se inicializa a cero
		};
		ClearValue[0].color = ClearColor;
		ClearValue[1].depthStencil = { 1.0f, 0 };

		VkRenderPassBeginInfo RenderPassBeginInfo = {};
		RenderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassBeginInfo.pNext = NULL;
		RenderPassBeginInfo.renderPass = m_renderPass;
		VkRect2D rect;
		rect.offset = { 0,0 };
		rect.extent = { WINDOW_WIDTH,WINDOW_HEIGHT-25 };
		RenderPassBeginInfo.renderArea = rect;
		RenderPassBeginInfo.clearValueCount = (uint32_t)ClearValue.size();
		RenderPassBeginInfo.pClearValues = ClearValue.data();



		//m_pipeline->UpdateTexture(m_mesh.m_pTex);

		for (uint i = 0; i < m_cmdBufs.size(); i++) {

			//PresentToClearBarrier.image = m_vkcore.GetImage(i);
			//ClearToPresentBarrier.image = m_vkcore.GetImage(i);

			core::BeginCommandBuffer(m_cmdBufs[i], VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT);
			/*
			vkCmdPipelineBarrier(m_cmdBufs[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, //dependency flags
				0, NULL, // memory barriers
				0, NULL, //BufferMemory barriers
				1, &PresentToClearBarrier); // Image memory barriers

			vkCmdClearColorImage(m_cmdBufs[i], m_vkcore.GetImage(i), VK_IMAGE_LAYOUT_GENERAL, &ClearColor, 1, &ImageRange);

			vkCmdPipelineBarrier(m_cmdBufs[i], VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
				0, 0, NULL, 0, NULL, 1, &ClearToPresentBarrier);
			*/

			RenderPassBeginInfo.framebuffer = m_frameBuffers[i];

			vkCmdBeginRenderPass(m_cmdBufs[i], &RenderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

			//m_pipeline->Bind(m_cmdBufs[i],(int)i);

			//m_pipeline->DrawMeshIndexed(m_cmdBufs[i], m_mesh);
			//m_pipeline->DrawMeshIndexed(m_cmdBufs[i], m_mesh1);

			vkCmdEndRenderPass(m_cmdBufs[i]);

			VkResult res = vkEndCommandBuffer(m_cmdBufs[i]);
			CHECK_VK_RESULT(res, "vkEndCommandBuffer\n");
		}
		
		printf("Command buffers recorded\n");
	}

	void CreateCamera(glm::vec3 pos) {
		
		float FOV = 45.0f;
		float znear = 0.1f;
		float zfar = 1000.0f;
		CreateCamera(pos, FOV, znear, zfar);
		printf("Created camera");
	}

	void CreateCamera(glm::vec3 pos,float FOV, float znear, float zfar) {
		if ((m_windowWidth == 0) || (m_windowHeight == 0)) {
			printf("Invalid window dims");
			exit(1);
		}

		glm::vec3 Target(0.0f, 0.0f, 0.0f);
		glm::vec3 Up(0.0f, 1.0f, 0.0f);

		m_pCamera = new CameraFirstPerson(pos, Target, Up, FOV, m_windowWidth, m_windowHeight, znear, zfar);
	}

	float m_windowWidth, m_windowHeight;

	core::VulkanQueue* m_pQueue;
	core::VulkanCore m_vkcore;
	VkDevice m_device = NULL;
	int m_numImages = 0;
	std::vector<VkCommandBuffer> m_cmdBufs;
	//GLFWwindow* m_pWindow;
	VkRenderPass m_renderPass;
	std::vector<VkFramebuffer> m_frameBuffers;
	VkShaderModule m_vs;
	VkShaderModule m_fs;
	//core::GraphicsPipeline* m_pipeline = NULL;
	core::SimpleMesh m_mesh, m_mesh1;
	core::VulkanTexture m_texture1, m_texture2;
	CameraFirstPerson* m_pCamera = NULL;

	std::vector<core::BufferMemory> m_uniformBuffers;

	bool rt_active = true;

	///RAYTRACING
	core::Raytracer m_raytracer;
};