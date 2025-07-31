#include "gltfloader.h"

 bool GLTFHelper::ExtractMeshAttributes(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
    std::vector<glm::vec3>& outVertices,
    std::vector<glm::vec3>& outNormals,
    std::vector<glm::vec2>& outUVs,
    std::vector<uint32_t>& outIndices) {
    auto getBufferViewData = [&](int accessorIndex) -> const unsigned char* {
        const auto& accessor = model.accessors[accessorIndex];
        const auto& view = model.bufferViews[accessor.bufferView];
        const auto& buffer = model.buffers[view.buffer];
        return &buffer.data[view.byteOffset + accessor.byteOffset];
        };

    // === INDICES ===
    if (primitive.indices < 0) {
        std::cerr << "Mesh has no indices, skipping." << std::endl;
        return false;
    }

    const auto& indexAccessor = model.accessors[primitive.indices];
    const auto& indexView = model.bufferViews[indexAccessor.bufferView];
    const auto& indexBuffer = model.buffers[indexView.buffer];
    const unsigned char* indexData = &indexBuffer.data[indexView.byteOffset + indexAccessor.byteOffset];

    outIndices.resize(indexAccessor.count);

    for (size_t i = 0; i < indexAccessor.count; ++i) {
        switch (indexAccessor.componentType) {
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
            outIndices[i] = ((const uint16_t*)indexData)[i];
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
            outIndices[i] = ((const uint32_t*)indexData)[i];
            break;
        case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
            outIndices[i] = ((const uint8_t*)indexData)[i];
            break;
        default:
            std::cerr << "Unsupported index type\n";
            return false;
        }
    }

    // === POSITION ===
    if (primitive.attributes.find("POSITION") == primitive.attributes.end()) {
        std::cerr << "Mesh missing POSITION attribute\n";
        return false;
    }
    const float* positionData = reinterpret_cast<const float*>(getBufferViewData(primitive.attributes.at("POSITION")));

    // === NORMAL ===
    const float* normalData = nullptr;
    if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
        normalData = reinterpret_cast<const float*>(getBufferViewData(primitive.attributes.at("NORMAL")));
    }

    // === TEXCOORD_0 ===
    const float* uvData = nullptr;
    if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
        uvData = reinterpret_cast<const float*>(getBufferViewData(primitive.attributes.at("TEXCOORD_0")));
    }

    size_t vertexCount = model.accessors[primitive.attributes.at("POSITION")].count;
    outVertices.reserve(vertexCount);
    outNormals.reserve(vertexCount);
    outUVs.reserve(vertexCount);

    for (size_t i = 0; i < vertexCount; ++i) {
        glm::vec3 pos(positionData[i * 3 + 0], positionData[i * 3 + 1], positionData[i * 3 + 2]);
        outVertices.push_back(pos);

        if (normalData) {
            glm::vec3 n(normalData[i * 3 + 0], normalData[i * 3 + 1], normalData[i * 3 + 2]);
            outNormals.push_back(n);
        }
        else {
            outNormals.push_back(glm::vec3(0.0f));  // dummy normal
        }

        if (uvData) {
            glm::vec2 uv(uvData[i * 2 + 0], uvData[i * 2 + 1]);
            outUVs.push_back(uv);
        }
        else {
            outUVs.push_back(glm::vec2(0.0f));  // dummy UV
        }
    }

    return true;
}