#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <string>

class OBJLoader {
private:
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;

public:
    bool loadOBJ(const std::string& filepath) {
        // Limpiar vectores anteriores
        vertices.clear();
        normals.clear();
        indices.clear();

        // Crear el importador de Assimp
        Assimp::Importer importer;

        // Cargar el archivo con post-procesamiento
        const aiScene* scene = importer.ReadFile(filepath,
            aiProcess_Triangulate |           // Convertir a triángulos
            aiProcess_FlipUVs |              // Voltear coordenadas UV
            aiProcess_GenNormals |           // Generar normales si no las tiene
            aiProcess_CalcTangentSpace);     // Calcular tangentes

        // Verificar que la carga fue exitosa
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Error al cargar el archivo OBJ: " << importer.GetErrorString() << std::endl;
            return false;
        }

        // Procesar todos los meshes en la escena
        for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
            processMesh(scene->mMeshes[i]);
        }

        std::cout << "Archivo OBJ cargado exitosamente!" << std::endl;
        std::cout << "Vertices: " << vertices.size() << std::endl;
        std::cout << "Normales: " << normals.size() << std::endl;
        std::cout << "indices: " << indices.size() << std::endl;

        return true;
    }

private:
    void processMesh(aiMesh* mesh) {
        unsigned int baseIndex = vertices.size();

        // Procesar vértices y normales
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // Extraer posiciones de vértices
            glm::vec3 vertex(
                mesh->mVertices[i].x,
                mesh->mVertices[i].y,
                mesh->mVertices[i].z
            );
            vertices.push_back(vertex);

            // Extraer normales (si existen)
            glm::vec3 normal(0.0f);
            if (mesh->HasNormals()) {
                normal = glm::vec3(
                    mesh->mNormals[i].x,
                    mesh->mNormals[i].y,
                    mesh->mNormals[i].z
                );
            }
            normals.push_back(normal);
        }

        // Procesar índices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++) {
                indices.push_back(baseIndex + face.mIndices[j]);
            }
        }
    }

public:
    // Métodos getter para acceder a los datos
    const std::vector<glm::vec3>& getVertices() const {
        return vertices;
    }

    const std::vector<glm::vec3>& getNormals() const {
        return normals;
    }

    const std::vector<unsigned int>& getIndices() const {
        return indices;
    }

    // Métodos para obtener los datos como arrays de floats (útil para OpenGL/Vulkan)
    std::vector<float> getVerticesAsFloats() const {
        std::vector<float> result;
        result.reserve(vertices.size() * 3);

        for (const auto& vertex : vertices) {
            result.push_back(vertex.x);
            result.push_back(vertex.y);
            result.push_back(vertex.z);
        }
        return result;
    }

    std::vector<float> getNormalsAsFloats() const {
        std::vector<float> result;
        result.reserve(normals.size() * 3);

        for (const auto& normal : normals) {
            result.push_back(normal.x);
            result.push_back(normal.y);
            result.push_back(normal.z);
        }
        return result;
    }

    // Método para obtener datos entrelazados (vértice + normal)
    std::vector<float> getInterleavedData() const {
        std::vector<float> result;
        result.reserve(vertices.size() * 6); // 3 para vértice + 3 para normal

        for (size_t i = 0; i < vertices.size(); i++) {
            // Vértice
            result.push_back(vertices[i].x);
            result.push_back(vertices[i].y);
            result.push_back(vertices[i].z);

            // Normal
            result.push_back(normals[i].x);
            result.push_back(normals[i].y);
            result.push_back(normals[i].z);
        }
        return result;
    }

    // Método para imprimir estadísticas
    void printStats() const {
        std::cout << "\n=== Estadisticas del modelo ===" << std::endl;
        std::cout << "Numero de vertices: " << vertices.size() << std::endl;
        std::cout << "Numero de normales: " << normals.size() << std::endl;
        std::cout << "Numero de indices: " << indices.size() << std::endl;
        std::cout << "Numero de triangulos: " << indices.size() / 3 << std::endl;
    }
};