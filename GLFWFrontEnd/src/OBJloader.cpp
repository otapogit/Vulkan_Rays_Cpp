#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

class OBJLoader {
public:
    bool loadOBJ(const std::string& filepath) {
        // Limpiar vectores anteriores
        vertices.clear();
        normals.clear();
        texCoords.clear();
        indices.clear();
        normals.clear();

        // Crear el importador de Assimp
        Assimp::Importer importer;

        // Cargar el archivo con post-procesamiento m�s completo
        /*const aiScene* scene = importer.ReadFile(filepath,
            aiProcess_Triangulate |      // Convertir a tri�ngulos
            //aiProcess_GenNormals            // Generar normales si no las tiene
            aiProcess_FlipUVs |               // Voltear coordenadas UV (com�n en OBJ)
            aiProcess_JoinIdenticalVertices | // Unir v�rtices id�nticos
            aiProcess_ValidateDataStructure  // Validar estructura de datos
        );*/

        const aiScene* scene = importer.ReadFile(filepath,
            0
        );

        printf("Imported mesh at %s\n", filepath);

        // Verificar que la carga fue exitosa
        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Error al cargar el archivo OBJ: " << importer.GetErrorString() << std::endl;
            return false;
        }

        // Procesar todos los meshes en la escena
        processNode(scene->mRootNode, scene);

        std::cout << "Archivo OBJ cargado exitosamente!" << std::endl;
        std::cout << "Vertices: " << vertices.size() << std::endl;
        std::cout << "Normales: " << normals.size() << std::endl;
        std::cout << "Coordenadas de textura: " << texCoords.size() << std::endl;
        std::cout << "Indices: " << indices.size() << std::endl;
        std::cout << "Tri�ngulos: " << indices.size() / 3 << std::endl;

        return true;
    }

    // Versi�n alternativa que intenta optimizar v�rtices duplicados
    bool loadOBJOptimized(const std::string& filepath) {
        vertices.clear();
        normals.clear();
        texCoords.clear();
        indices.clear();

        Assimp::Importer importer;

        // CON aiProcess_JoinIdenticalVertices para optimizar
        const aiScene* scene = importer.ReadFile(filepath,
            aiProcess_Triangulate |
            aiProcess_GenNormals |
            aiProcess_JoinIdenticalVertices  // Este flag reduce v�rtices duplicados
        );

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "Error al cargar el archivo OBJ: " << importer.GetErrorString() << std::endl;
            return false;
        }

        std::cout << "=== INFORMACI�N DE CARGA OPTIMIZADA ===" << std::endl;
        std::cout << "N�mero de meshes en la escena: " << scene->mNumMeshes << std::endl;

        processNode(scene->mRootNode, scene);

        std::cout << "\n=== ESTAD�STICAS OPTIMIZADAS ===" << std::endl;
        std::cout << "V�rtices procesados: " << vertices.size() << std::endl;
        std::cout << "Normales: " << normals.size() << std::endl;
        std::cout << "Coordenadas de textura: " << texCoords.size() << std::endl;
        std::cout << "�ndices: " << indices.size() << std::endl;
        std::cout << "Tri�ngulos: " << indices.size() / 3 << std::endl;

        return true;
    }

    void analyzeVertexDuplication() const {
        std::cout << "\n=== AN�LISIS DE DUPLICACI�N DE V�RTICES ===" << std::endl;

        std::vector<glm::vec3> uniquePositions;

        int duplicateCount = 0;

        for (const auto& vertex : vertices) {
            bool found = false;
            for (const auto& unique : uniquePositions) {
                if (glm::distance(vertex, unique) < 0.0001f) { // Tolerancia peque�a
                    found = true;
                    duplicateCount++;
                    break;
                }
            }
            if (!found) {

                uniquePositions.push_back(vertex);
                //uniqueVertices.push_back(vertex);
            }
        }



        std::cout << "Posiciones �nicas: " << uniquePositions.size() << std::endl;
        std::cout << "V�rtices duplicados: " << duplicateCount << std::endl;
        std::cout << "Total de v�rtices: " << vertices.size() << std::endl;
        std::cout << "Ratio de duplicaci�n: " << (float)duplicateCount / vertices.size() * 100.0f << "%" << std::endl;
    }

    // M�todos getter para acceder a los datos
    const std::vector<glm::vec3>& getVertices() const {
        return vertices;
    }

    const std::vector<glm::vec3>& getNormals() const {
        return normals;
    }

    const std::vector<glm::vec2>& getTexCoords() const {
        return texCoords;
    }

    const std::vector<unsigned int>& getIndices() const {
        return indices;
    }

    // M�todos para obtener los datos como arrays de floats (�til para OpenGL/Vulkan)
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

    std::vector<float> getTexCoordsAsFloats() const {
        std::vector<float> result;
        result.reserve(texCoords.size() * 2);

        for (const auto& texCoord : texCoords) {
            result.push_back(texCoord.x);
            result.push_back(texCoord.y);
        }
        return result;
    }

    // M�todo para obtener datos entrelazados (v�rtice + normal + texCoord)
    std::vector<float> getInterleavedData() const {
        std::vector<float> result;
        result.reserve(vertices.size() * 8); // 3 para v�rtice + 3 para normal + 2 para texCoord

        for (size_t i = 0; i < vertices.size(); i++) {
            // V�rtice
            result.push_back(vertices[i].x);
            result.push_back(vertices[i].y);
            result.push_back(vertices[i].z);

            // Normal
            result.push_back(normals[i].x);
            result.push_back(normals[i].y);
            result.push_back(normals[i].z);

            // Coordenadas de textura
            result.push_back(texCoords[i].x);
            result.push_back(texCoords[i].y);
        }
        return result;
    }

    // M�todo para obtener solo v�rtice + normal (como el original)
    std::vector<float> getVertexNormalData() const {
        std::vector<float> result;
        result.reserve(vertices.size() * 6); // 3 para v�rtice + 3 para normal

        for (size_t i = 0; i < vertices.size(); i++) {
            // V�rtice
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

    // M�todo para imprimir estad�sticas
    void printStats() const {
        std::cout << "\n=== Estad�sticas del modelo ===" << std::endl;
        std::cout << "N�mero de v�rtices: " << vertices.size() << std::endl;
        std::cout << "N�mero de normales: " << normals.size() << std::endl;
        std::cout << "N�mero de coordenadas de textura: " << texCoords.size() << std::endl;
        std::cout << "N�mero de �ndices: " << indices.size() << std::endl;
        std::cout << "N�mero de tri�ngulos: " << indices.size() / 3 << std::endl;

        // Informaci�n del bounding box
        if (!vertices.empty()) {
            glm::vec3 minBounds = vertices[0];
            glm::vec3 maxBounds = vertices[0];

            for (const auto& vertex : vertices) {
                minBounds.x = std::min(minBounds.x, vertex.x);
                minBounds.y = std::min(minBounds.y, vertex.y);
                minBounds.z = std::min(minBounds.z, vertex.z);

                maxBounds.x = std::max(maxBounds.x, vertex.x);
                maxBounds.y = std::max(maxBounds.y, vertex.y);
                maxBounds.z = std::max(maxBounds.z, vertex.z);
            }

            std::cout << "Bounding Box:" << std::endl;
            std::cout << "  Min: (" << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << ")" << std::endl;
            std::cout << "  Max: (" << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")" << std::endl;

            glm::vec3 size = maxBounds - minBounds;
            std::cout << "  Tama�o: (" << size.x << ", " << size.y << ", " << size.z << ")" << std::endl;
        }
    }

private:
    void processNode(aiNode* node, const aiScene* scene) {
        // Procesar todos los meshes del nodo
        for (unsigned int i = 0; i < node->mNumMeshes; i++) {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            processMesh(mesh, scene);
        }

        // Procesar recursivamente todos los nodos hijos
        for (unsigned int i = 0; i < node->mNumChildren; i++) {
            processNode(node->mChildren[i], scene);
        }
    }

    void processMesh(aiMesh* mesh, const aiScene* scene) {
        unsigned int baseIndex = vertices.size();

        // Procesar v�rtices, normales y coordenadas de textura
        for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
            // Extraer posiciones de v�rtices
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

            // Extraer coordenadas de textura (solo el primer conjunto)
            glm::vec2 texCoord(0.0f);
            if (mesh->mTextureCoords[0]) {
                texCoord = glm::vec2(
                    mesh->mTextureCoords[0][i].x,
                    mesh->mTextureCoords[0][i].y
                );
            }
            texCoords.push_back(texCoord);
        }

        // Procesar �ndices
        for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
            aiFace face = mesh->mFaces[i];
            // Asegurar que es un tri�ngulo (deber�a ser as� por aiProcess_Triangulate)
            if (face.mNumIndices == 3) {
                for (unsigned int j = 0; j < face.mNumIndices; j++) {
                    indices.push_back(baseIndex + face.mIndices[j]);
                }
            }
            else {
                std::cerr << "Advertencia: Face con " << face.mNumIndices << " v�rtices encontrada" << std::endl;
            }
        }
    }

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec3> uniqueVertices;

    std::vector<unsigned int> indices;
    std::vector<glm::vec2> texCoords;

};