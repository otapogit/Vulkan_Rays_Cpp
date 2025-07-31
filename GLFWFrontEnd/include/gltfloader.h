#pragma once
#include <glm/ext.hpp>
#include <vector>
#include <windows.data.json.h>
#include <tiny_gltf.h>
#include <stdio.h> // fprintf, stderr
#include <stdlib.h>
#include <iostream>


class GLTFHelper {
public:
    bool ExtractMeshAttributes(const tinygltf::Model& model, const tinygltf::Primitive& primitive,
        std::vector<glm::vec3>& outVertices,
        std::vector<glm::vec3>& outNormals,
        std::vector<glm::vec2>& outUVs,
        std::vector<uint32_t>& outIndices);
};