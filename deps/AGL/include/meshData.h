#pragma once

// STL
#include <map>
#include <filesystem>
#include <functional>
#include <vector>
// dependencies (glm)
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
// AGL
#include "boundingVolumes.h"
// AGLcommon
#include <cadCommon.h>
#include <meshCommon.h>

#include <GL/glew.h>

namespace autis
{
	class Mesh;
	class Model;

	using MeshIndex = uint32_t;
	const GLenum MeshIndexGLType{ GL_UNSIGNED_INT };
	const MeshIndex INVALID_MESH_INDEX = INVALID_UNSIGNED_INT_ID_VALUE;
	const MeshIndex FIRST_MESH_INDEX = 0;

	/**
	* @brief Definición del cable (Edge)
	*
	* Ej:
	*
	* // la malla tiene 100 vértices
	*
	* auto e = std::vector<Edge>{ 1001, {80, 81, 82, 83, 84, 85}, {80, 83, 85}};
	*
	*/
	struct Edge
	{
		// La id es relevante para el picking y la selección
		EdgeId id = INVALID_EDGE_ID;
		std::vector<MeshIndex> indices; // Índices de la malla que forman el edge
		std::vector<MeshIndex> selectableVertices; // Índices de la malla (y del edge)
		                                           // que se pueden seleccionar
	};

	/**
	 * @brief Definición de la cara (Face)
	 *
	 * Ej:
	 *
	 * // la malla tiene 100 vértices y tres caras
	*
	* auto fs = std::vector<Face>{ {0, {<indices-triangulos-cara-0>}, {1, {indices-triangulos-cara-1}, ...};
	*/
	struct Face
	{
		// La id es relevante para el picking y la selección
		FaceId id = INVALID_FACE_ID;
		std::vector<MeshIndex> indices; // Índices de la malla que forman la cara (cada 3 forman un triángulo)
	};

	struct MeshData
	{
		// Id opcional y no serializable
		MeshId id = INVALID_MESH_ID;
		std::string name;

		std::vector<glm::vec3> pos, normals;
		std::vector<MeshIndex> inds;
		std::vector<glm::vec2> uv;
		std::vector<Edge> edges;
		std::vector<Face> faces;

		static size_t fromMeshIndexToArrayIndex(const MeshIndex index) { return static_cast<size_t>(index); }
		static MeshIndex fromArrayIndexToMeshIndex(const size_t index) { return MeshIndex(static_cast<uint32_t>(index)); }
	};

	/**
	Estructura en memoria para almacenar la geometra de varias mallas.
	*/
	struct MeshesData
	{
		std::vector<MeshData> meshes;
	};

	/**
	Estructura con meta informacin sobre una malla
	*/
	struct MeshDataInfo
	{
		// Índice de la malla en el vector MeshesData::meshes
		MeshIndex index{ std::numeric_limits<uint32_t>::max() };
		BoundingBox boundingBox;
		bool hasUVs{ false };
		BoundingBox uvBoundingBox;
		uint32_t numVertices, numIndices, numEdges, numFaces, numSelectableVertices;
	};

	/**
	Estructura con meta informacin sobre varias mallas
	*/
	struct MeshesDataInfo
	{
		BoundingBox globalBoundingBox;
		std::vector<MeshDataInfo> meshes;
	};

};

