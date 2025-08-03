#include "OBJloader.cpp"
#include <Renderer/VulkanRenderer.h>
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

