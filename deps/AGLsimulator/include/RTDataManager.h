#pragma once

#include <memory>

#include <glm/vec3.hpp>
#include <bvh.h>

namespace autis {
	class ShaderDataScenegraph;
	class Program;

	class SSBO_RT_VertexTable;
	class SSBO_RT_IndexTable;
	class SSBO_RT_TriangleIndexTable;
	class SSBO_RT_NodeTable;
	class SSBO_RT_MeshInfoTable;

	class RTDataManager {
	public:
		RTDataManager();
		void update(const ShaderDataScenegraph& data);
		void update(const MultiBVH& data);
		void connectToProgram(Program& p);
		const SSBO_RT_VertexTable& getVertexTable() const { return *ssboVertexTable; }
		const SSBO_RT_IndexTable& getIndexTable() const { return *ssboIndexTable; }
		const SSBO_RT_TriangleIndexTable& getTriangleIndexTable() const { return *ssboTriangleIndexTable; }
		const SSBO_RT_MeshInfoTable& getMeshInfoTable() const  { return *ssboMeshInfoTable; }
	private:
		std::shared_ptr<SSBO_RT_VertexTable> ssboVertexTable;
		std::shared_ptr<SSBO_RT_IndexTable> ssboIndexTable;
		std::shared_ptr<SSBO_RT_TriangleIndexTable> ssboTriangleIndexTable;
		std::shared_ptr<SSBO_RT_NodeTable> ssboNodeTable;
		std::shared_ptr<SSBO_RT_MeshInfoTable> ssboMeshInfoTable;
	};
};
