#pragma once

// AGLcommon
#include <meshData.h>

namespace autis
{
	struct MeshData;

	void flipNormals(MeshData& mesh);
	void flipNormalsFiltered(MeshData& mesh, std::vector<uint32_t> indices);
	void unifyNormals(MeshData& mesh, std::vector<uint32_t> indices);
	glm::vec3 getCentroid(MeshData& mesh);

	std::vector <uint32_t> getAdjacentVertices(const MeshData& m, uint32_t tri);
	std::vector<std::vector<uint32_t>> getVerticesToTriangles(const MeshData& m);
	void iterateOnTriangles(const MeshData& m, std::function<void(const MeshData& m,
		MeshIndex triangle)> op);
	void iterateOnTriangles(const MeshData& m, std::function<void(const MeshData& m,
		MeshIndex i1, MeshIndex i2, MeshIndex i3)> op);

	/**
	\class ADAT2Metadata
	Clase encargada de calcular los metadatos de una serie de mallas cargadas desde un adat2.
	*/
	class ADAT2Metadata
	{
	public:
		/**
		Calcula los metadatos asociados a las mallas indicadas
		\param meshes informacin geomtrica cargada desde un adat2
		*/
		static MeshesDataInfo compute(const MeshesData& meshes);
		static MeshDataInfo compute(const MeshData& meshes);
		static const std::string extension;
	};

	bool loadOBJToMesh(const std::filesystem::path& filename, MeshesData& meshes);
	bool saveMeshToOBJ(const std::filesystem::path& filename, const MeshesData& meshes);
	bool savePolylinesToOBJ(const std::filesystem::path& filename, const std::vector<std::vector<glm::vec3>>& pls);
	bool saveMeshToCSV(const std::filesystem::path& filename, const MeshesData& meshes);
	void meshToMeshData(const Mesh& mesh, MeshData& meshData);
	void modelToMeshesData(const Model& model, MeshesData& meshes);
	void meshesDataToModel(const MeshesData& meshes, Model& model);
	void meshesDataToMesh(Mesh& glmesh, const MeshData& mesh);
	void computeNormalsFromVertices(MeshesData& meshes);
	void computeNormalsFromVertices(MeshData& mesh);

	void flipTriangle(MeshData& mesh, uint32_t triangle, bool fixNormals = true);

	float computeArea(const MeshData& mesh);
	float computeArea(const MeshesData& meshes);

	template<typename F>
	std::vector<F> computeTriangleAreas(const MeshData& mesh) {
		std::vector<F> areas;
		if (!mesh.inds.empty())
			areas.resize(mesh.inds.size() / 3);
		iterateOnTriangles(mesh,
			[&areas](const MeshData& m, uint32_t tri) {
				auto i1 = m.inds[tri * 3];
				auto i2 = m.inds[tri * 3 + 1];
				auto i3 = m.inds[tri * 3 + 2];
				areas[tri] = static_cast<F>(0.5) * glm::length(glm::cross(m.pos[i2] - m.pos[i1], m.pos[i3] - m.pos[i1]));
			}
		);
		return areas;
	}

	/**
	Exportar un conjunto de mallas de un MeshData a otro nuevo
	*/
	MeshData exportFaces(const MeshData& mesh, const std::vector<uint32_t>& faces);

	/**
	 * @brief Combinar varias mallas en una
	 * @param meshes
	*/
	MeshData combineMeshes(const MeshesData& meshes);

	struct ExportFacesRequest {
		uint32_t meshIndex;
		uint32_t faceIndex;
	};
	MeshesData exportFaces(const std::vector<const MeshData*>& meshes, const std::vector<ExportFacesRequest>& faces);

}
