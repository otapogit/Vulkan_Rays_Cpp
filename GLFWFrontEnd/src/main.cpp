#include <GL/glew.h>
#include <GL/wglew.h>
#include <stdio.h> // fprintf, stderr
#include <stdlib.h>
#include <iostream>

#include "OBJloader.cpp"

#include <GLFW/glfw3.h>
#ifdef _WIN32
#include <GLFW/glfw3native.h>  // Importante: debes incluir esto
#endif
#include <Renderer/VulkanRenderer.h>
#include "core_fpcamera.h"
//#include "PGUPV.h"
#include <windows.h>

#include <GL/gl.h>

#include <3rdParty/stb_image_write.h>

//using namespace PGUPV;

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN

#include "3rdParty/stb_image.h"
#include "glfwcamera.h"

// Función para manejar errores de GLFW
void error_callback(int error, const char* description) {
    fprintf(stderr, "Error: %s (%d)\n", description, error);
}

static struct Vertex {
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

void initRT();
void initOBJ();
void CreateCamera(glm::vec3 pos);
void CreateCamera(glm::vec3 pos, float FOV, float znear, float zfar);

// Función para cargar y crear una textura OpenGL desde un archivo de imagen
GLuint loadTexture(const char* imagePath) {
    int width, height, channels;
    unsigned char* data = stbi_load(imagePath, &width, &height, &channels, 0);

    if (!data) {
        std::cerr << "Error: No se pudo cargar la imagen: " << imagePath << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configurar parámetros de la textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Determinar el formato de la imagen
    GLenum format;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;
    else {
        std::cerr << "Error: Formato de imagen no soportado" << std::endl;
        stbi_image_free(data);
        return 0;
    }

    // Cargar la textura en OpenGL
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    //glGenerateMipmap(GL_TEXTURE_2D);

    // Liberar memoria de la imagen
    stbi_image_free(data);

    std::cout << "Textura cargada: " << imagePath << " (" << width << "x" << height << ", " << channels << " canales)" << std::endl;

    return textureID;
}

GLuint createTextureFromData(uint8_t* data, int width, int height, int channels) {
    if (!data) {
        std::cerr << "Error: Datos de imagen nulos" << std::endl;
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Configurar parámetros de la textura
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLint internalFormat;
    GLenum format;
    if (channels == 1) {
        internalFormat = GL_R8;
        format = GL_RED;
    }
    else if (channels == 3) {
        internalFormat = GL_RGB8;
        format = GL_RGB;
    }
    else if (channels == 4) {
        internalFormat = GL_RGBA8;
        format = GL_RGBA;
    }
    else {
        std::cerr << "Error: Formato de imagen no soportado" << std::endl;
        return 0;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    std::cout << "Textura creada desde datos del renderer: " << width << "x" << height << ", " << channels << " canales" << std::endl;

    return textureID;
}

void checkGLError(const char* operation) {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR) {
        std::cerr << "Error OpenGL en " << operation << ": " << error << std::endl;
    }
}

// Función para dibujar un cuadrado que ocupe toda la pantalla con textura
void renderTexturedQuad(GLuint textureID) {

    if (!glIsTexture(textureID)) {
        std::cerr << "Error: Textura inválida: " << textureID << std::endl;
        return;
    }

    //std::cout << "textureID valid? " << glIsTexture(textureID) << std::endl;
    GLint boundTex = 0;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &boundTex);
    //std::cout << "GL_TEXTURE_BINDING_2D: " << boundTex << std::endl;

    // Activar textura
    glEnable(GL_TEXTURE_2D);
    //checkGLError("glEnable");
    glBindTexture(GL_TEXTURE_2D, textureID);
    checkGLError("bind texture in quad");
    // Dibujar cuadrado que ocupa toda la pantalla
    glBegin(GL_QUADS);
    // Vértice inferior izquierdo
    glTexCoord2f(0.0f, 0.0f);
    glVertex2f(-1.0f, 1.0f);
    

    // Vértice inferior derecho
    glTexCoord2f(1.0f, 0.0f);
    glVertex2f(1.0f, 1.0f);

    // Vértice superior derecho
    glTexCoord2f(1.0f, 1.0f);
    glVertex2f(1.0f, -1.0f);

    // Vértice superior izquierdo
    glTexCoord2f(0.0f, 1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
    glDisable(GL_TEXTURE_2D);

}

VulkanRenderer m_Renderer;
int m_windowwidth, m_windowheight;
CameraFirstPerson* m_pCamera;

int main(int argc, char* argv[]) {


    // Configurar callback de errores de GLFW
    glfwSetErrorCallback(error_callback);

    // Inicializar GLFW
    if (!glfwInit()) {
        std::cerr << "Error: No se pudo inicializar GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Crear ventana
    m_windowwidth = 800; m_windowheight = 800;
    GLFWwindow* window = glfwCreateWindow(m_windowwidth, m_windowheight, "OpenGL: Textura en Cuadrado", NULL, NULL);


    //HWND hwnd = glfwGetWin32Window(window);
    //HDC hdc = GetDC(hwnd);

    // Verificar si se pudo crear la ventana
    if (!window) {
        std::cerr << "Error: No se pudo crear la ventana" << std::endl;
        glfwTerminate();
        return -2;
    }

    // Hacer la ventana actual
    glfwMakeContextCurrent(window);


    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Error: No se pudo inicializar GLEW" << std::endl;
        glfwTerminate();
        return -4;
    }
    bool hasMemoryObject = false;
    bool hasMemoryObjectWin32 = false;
    bool hasSemaphore = false;
    bool hasSemaphoreWin32 = false;

    GLint nExtensions;
    glGetIntegerv(GL_NUM_EXTENSIONS, &nExtensions);

    for (GLint i = 0; i < nExtensions; ++i) {
        const char* ext = (const char*)glGetStringi(GL_EXTENSIONS, i);
        if (strcmp(ext, "GL_EXT_memory_object") == 0) hasMemoryObject = true;
        if (strcmp(ext, "GL_EXT_memory_object_win32") == 0) hasMemoryObjectWin32 = true;
        if (strcmp(ext, "GL_EXT_semaphore") == 0) hasSemaphore = true;
        if (strcmp(ext, "GL_EXT_semaphore_win32") == 0) hasSemaphoreWin32 = true;
    }

    if (hasMemoryObject && hasMemoryObjectWin32 && hasSemaphore && hasSemaphoreWin32) {
        std::cout << "Interoperabilidad Vulkan-OpenGL soportada (aunque no reporten GL_EXT_external_objects directamente)\n";
    }
    else {
        std::cerr << "Faltan extensiones para interoperabilidad Vulkan-OpenGL\n";
    }

    // Activar v-sync
    glfwSwapInterval(1);

    CreateCamera(glm::vec3(1.f, 5.f, 1.f));
    CameraGLFWController::setupCallbacks(window, m_pCamera);

    m_Renderer.init();
    initOBJ();
   // initRT();
    m_Renderer.setOutputResolution(m_windowwidth, m_windowheight);
    m_Renderer.save(false, window);
    m_Renderer.render();

    glfwMakeContextCurrent(window);
    
    // Calcula el tamaño del buffer necesario (RGBA, 4 bytes por píxel)
    size_t bufferSize = m_windowwidth * m_windowheight * (4 + 1);
    uint8_t* imageBuffer = new uint8_t[bufferSize];

    // Copia los bytes del resultado del renderer
    size_t bytesWritten = m_Renderer.copyResultBytes(imageBuffer, bufferSize);

    if (bytesWritten == 0) {
        std::cerr << "Error: No se pudieron copiar los bytes del resultado" << std::endl;
        delete[] imageBuffer;
        glfwTerminate();
        return -5;
    }

    printf("Datos copiados al bufer\n");
    // Crea la textura OpenGL usando los datos copiados
    GLuint textureID = createTextureFromData(imageBuffer, m_windowwidth, m_windowheight, 4);


    checkGLError("glBindTexture");
    // Configurar parámetros

    
    if (!glIsTexture(textureID)) {
        std::cerr << "Textura no válida después de getResultTextureId" << std::endl;
    }

    printf("texture created at: %zd\n", textureID);
    std::cout << "Textura cargada" << std::endl;

    // Configurar OpenGL
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Fondo negro

    std::cout << "Aplicación iniciada. Presiona ESC para salir." << std::endl;

    glm::mat4 prevmat = m_pCamera->GetVPMatrix();

    float lasttime = 0.0f;

    // Bucle principal
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lasttime;
        lasttime = currentFrame;

        m_pCamera->Update(deltaTime);

        // Procesar eventos
        glfwPollEvents();

        // Cerrar con ESC
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }

        // Limpiar pantalla
        glClear(GL_COLOR_BUFFER_BIT);
        //printf("Cleared screen\n");

        // Dibujar el cuadrado con textura
        renderTexturedQuad(textureID);
        //printf("Rendered quad\n");
        m_Renderer.setCamera(m_pCamera->GetVPMatrix(), glm::mat4(1.0f));
        m_Renderer.copyResultBytes(imageBuffer, bufferSize);
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_windowwidth, m_windowheight, GL_RGBA, GL_UNSIGNED_BYTE, imageBuffer);
        checkGLError("glTexSubImage2D");

        //Rerenderizar vulkan
        m_Renderer.render();

        // Intercambiar buffers
        glfwSwapBuffers(window);
    }
    delete[] imageBuffer;

    // Limpiar recursos
    glDeleteTextures(1, &textureID);
    glfwTerminate();

    return 0;
}

void initOBJ() {

    OBJLoader loader;
    // Cargar el archivo OBJ
    if (loader.loadOBJOptimized("../GLFWFrontEnd/OBJ/free_car_001.obj")) {
        loader.analyzeVertexDuplication();
        
        // Obtener los datos
        auto vertices = loader.getVertices();
        auto normals = loader.getNormals();
        auto indices = loader.getIndices();
        std::vector<glm::vec2> uvs = {glm::vec2(1.0f)};

        // Datos entrelazados (vértice + normal)
        //auto interleavedData = loader.getInterleavedData();

        uint32_t vanId = m_Renderer.defineMesh(vertices, normals, uvs, indices);
        m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.f, 0.f)), glm::vec3(1.0f), vanId);
        // Mostrar estadísticas
        loader.printStats();

    }
    else {
        std::cerr << "Error al cargar el archivo OBJ" << std::endl;
        exit(1);
    }

}

void initRT(){
    



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
    uint32_t frontMeshId = createMesh(frontMesh, frontIndices, &m_Renderer);

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
    uint32_t rightMeshId = createMesh(rightMesh, rightIndices, &m_Renderer);

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
    uint32_t leftMeshId = createMesh(leftMesh, leftIndices, &m_Renderer);

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
    uint32_t topMeshId = createMesh(topMesh, topIndices, &m_Renderer);

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
    uint32_t bottomMeshId = createMesh(bottomMesh, bottomIndices, &m_Renderer);

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
    uint32_t backMeshId = createMesh(backMesh, backIndices, &m_Renderer);

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
    uint32_t diagonalMeshId = createMesh(diagonalMesh, diagonalIndices, &m_Renderer);


    uint32_t meshid = m_Renderer.defineMesh(vertData, normData, uvData, indices);
    printf("Mesh Defined\n");

    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(1.0f,0.0f,0.0f), meshid);
    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(0.0f,1.0f,0.0f), topMeshId);
    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(0.0f,0.0f,1.0f), bottomMeshId);
    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(1.0f,1.0f,1.0f), diagonalMeshId);
    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(1.0f,1.0f,0.0f), frontMeshId);
    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(1.0f), rightMeshId);
    m_Renderer.addMesh(glm::translate(glm::mat4(1.0f), glm::vec3(1.4f, 0.f, 0.f)), glm::vec3(1.0f), leftMeshId);

    m_Renderer.setCamera(m_pCamera->GetVPMatrix(), glm::mat4(1.0f));
    //m_Renderer.setCamera(glm::mat4(1.0f), glm::mat4(1.0f));

    printf("Rendered everything\n");
}

void CreateCamera(glm::vec3 pos) {

    float FOV = 45.0f;
    float znear = 0.1f;
    float zfar = 1000.0f;
    CreateCamera(pos, FOV, znear, zfar);
    printf("Created camera");
}

void CreateCamera(glm::vec3 pos, float FOV, float znear, float zfar) {
    if ((m_windowwidth == 0) || (m_windowheight == 0)) {
        printf("Invalid window dims");
        exit(1);
    }

    glm::vec3 Target(0.0f, 0.0f, 0.0f);
    glm::vec3 Up(0.0f, 1.0f, 0.0f);

    m_pCamera = new CameraFirstPerson(pos, Target, Up, FOV, m_windowwidth, m_windowheight, znear, zfar);
}
