#pragma once
#include <boundingVolumes.h>
#include <meshData.h>

namespace autis {

	using TriangleOrNodeIndex = uint32_t;

	// There are two types of nodes:
	// -leaves: references a single triangle
	// -internal: has two children (left, right)
	// To save space, I reuse the indices. 
	struct BVHNode {
		static constexpr uint32_t SPLIT_AXIS_MASK() { return 0xC0000000; }
		static constexpr uint32_t INFO_MASK() { return 0x3FFFFFF; };
		static constexpr uint32_t X_AXIS() { return 0; }
		static constexpr uint32_t Y_AXIS() { return 1; }
		static constexpr uint32_t Z_AXIS() { return 2; }

		BoundingBoxGLM<glm::vec3> boundingBox; // Axis-aligned bounding box
		uint32_t leaf_info;						// SPLIT_AXIS_MASK: leaf/split axis? 00: X, 01: Y, 10: Z, 11: leaf node
												//	INFO_MASK bits: available space for storing information
		TriangleOrNodeIndex leftOrFirst;         // Index of the left child node or triangle index
		TriangleOrNodeIndex rightOrLast;        // Index of the right child node or triangle index
		uint32_t padding{ 512 };
		bool isLeaf() const;
		uint32_t splitAxis() const;
		void storeInfo(uint32_t info);
		uint32_t getInfo() const;
		constexpr size_t availableInfoBits() const;
	};

	struct BVHFlags {
		// bit 0: 1 active, 0, ignore this mesh
		static constexpr uint32_t ACTIVE{ 1U };

		// bit 1: 1 compute intersections only in front faces, 0 both
		static constexpr uint32_t INTERSECT_FRONT_FACE_ONLY_BIT{ 1U << 1 };

		// bits 3 and 2: 
		// 01: light, 10: inspectable geometry, 11: other geometry (p.e., supports), 00: unknown
		static constexpr uint32_t NODE_TYPE_MASK{ 3U << 2 };
		static constexpr uint32_t LIGHT_SOURCE{ 1U << 2 };
		static constexpr uint32_t INSPECTABLE_MESH{ 2U << 2 };
		static constexpr uint32_t NON_INSPECTABLE_MESH{ 3U << 2 };

		static BVHFlags UnkownGeometry(bool inspectBothSides = false);
		static BVHFlags LightSource(bool inspectBothSides = false);
		static BVHFlags InspectableGeometry(bool inspectBothSides = false);
		static BVHFlags NonInspectableGeometry(bool inspectBothSides = false);

		// by default, active, unknown geometry type, only front faces
		uint32_t flags{ INTERSECT_FRONT_FACE_ONLY_BIT | ACTIVE}; 
		bool isUnknownMesh() const { return (flags & NODE_TYPE_MASK) == 0; }
		bool isLightSource() const { return (flags & LIGHT_SOURCE) == LIGHT_SOURCE; };
		bool isInspectableMesh() const { return (flags & INSPECTABLE_MESH) == INSPECTABLE_MESH; };
		bool isNonInspectableMesh() const { return (flags & NON_INSPECTABLE_MESH) == NON_INSPECTABLE_MESH; };
		bool isActive() const { return (flags & ACTIVE) == ACTIVE; };
	};

	struct BVH {
		std::vector<BVHNode> nodes; // the root is the first node
		std::vector<uint32_t> triangleIndices;
		MeshData mesh;
		glm::mat4 worldXform;
		BVHFlags flags;
		glm::vec4 payload;  // space for user-defined data
	};


	inline bool operator==(const BVHNode& lhs, const BVHNode& rhs) {
		return 
			lhs.boundingBox == rhs.boundingBox && 
			lhs.leaf_info == rhs.leaf_info &&
			lhs.leftOrFirst == rhs.leftOrFirst && 
			lhs.rightOrLast == rhs.rightOrLast;
	}


	inline bool operator==(const BVH& lhs, const BVH& rhs) {
		return lhs.nodes == rhs.nodes && rhs.triangleIndices== lhs.triangleIndices;
	}

	void describeBVH(std::ostream& os, const BVH &bvh);

	struct BVHMeshInfo {
		glm::mat4 worldXform;   // the world transform for the mesh
		uint32_t vtcsBase;   // the position of the mesh's first vertex in the vertex table
		uint32_t indsBase;   // the position of the mesh's first index in the index table
		uint32_t triIndsBase;  // the position of the mesh's first triangle index in the triangleIndex table
		BVHFlags flags;		// flags
		glm::vec4 payload;	// space available for extra data about the mesh
	};

	static_assert(sizeof(BVHMeshInfo) == (sizeof(glm::mat4) + 8 * 4));

	struct MultiBVH {
		std::vector<glm::vec4> vertexTable; // all the meshes' vertices, back to back. Each in its object space
		std::vector<uint32_t> indexTable; // all the meshes's indices, back to back. As appear in the adat2
		std::vector<uint32_t> triangleIndexTable; // all the triangle indices, back to back, each set starting from 0
		std::vector<BVHNode> nodeTable; // all the nodes organized in a single structure. The first is the root. Each
									// leaf node has the index of its original mesh in meshInfo
		std::vector<BVHMeshInfo> meshInfo; // information about each mesh

		//! return the first, second and third vertices of the given triangle of the given mesh, in world coordinates
		const glm::vec3 getWV0(size_t meshIndex, size_t localTriangle) const;
		const glm::vec3 getWV1(size_t meshIndex, size_t localTriangle) const;
		const glm::vec3 getWV2(size_t meshIndex, size_t localTriangle) const;

		/**
		 * @brief Returns the index in the vertex table of the given vertex
		 * @param meshIndex the vertex's mesh 
		 * @param localTriangle the vertex's triangle index (0-based)
		 * @param index 0, 1, 2 for the first, second or third vertex of the triangle
		 * @return the vertex's index
		 */
		const uint32_t getVIndex(size_t meshIndex, size_t localTriangle, size_t index) const;
	};

	/*
		Updates the bounding boxes of the nodes of the given mesh with the new world transform matrix provided
		
		@warning This method does *not* change the hierarchy. If the meshes are transformed independently, 
		the hierarchy may not make sense any more. For example, if two initially separated meshes approach. 
		In that case, a full recompute of the MultiBVH should be done. If the transform is similar to all 
		the meshes, then this should suffice
	*/
	void updateWorldMatrix(MultiBVH& scene, size_t meshIndex, const glm::mat4& xform);


	class BVHBuilder {
	public:
		/**
		 * @brief Builds the BVH of the provided mesh.
		 * @param mesh 
		 * @param worldMatrix The initial transform to bring the mesh to the world space
		 * @param maxTrisPerNode Maximum number of triangles in a leaf node
		 * @param infoVal an optional number to store in the nodes. If the bvh is later combined with others, this 
		 * info is overwritten in the leaf nodes to store the mesh index
		 * @param intersectOnlyFrontFace compute intersection only in front faces
		 * @return 
		 */
		static BVH buildBVH(const MeshData& mesh, const glm::mat4& worldMatrix = glm::mat4{ 1.0f }, const BVHFlags flags = {}, size_t maxTrisPerNode = 4, uint32_t infoVal = 0);
		/**
		 * @brief Builds one BVH that contains all the provided bvhs. Each individual bvhs provided probably 
		   references one mesh.
		 * The leaf nodes in the resulting BVH will contain the index of the original bvh where it initially was, 
		   to be able to distinguish the original bvh/mesh.
		 * @return
		 */
		static MultiBVH buildMultiHierarchy(const std::vector<BVH>& bvhs);
	protected:
		BVHBuilder();
		TriangleOrNodeIndex createLeafNode(const std::vector<BoundingBoxGLM<glm::vec3>>& triBBs, const std::vector<uint32_t> &triangleIndices, TriangleOrNodeIndex firstTri, TriangleOrNodeIndex lastTri, uint32_t infoVal);
		TriangleOrNodeIndex createInternalNode(TriangleOrNodeIndex leftChild, TriangleOrNodeIndex rightChild, uint32_t splitAxis, uint32_t infoVal);
		TriangleOrNodeIndex buildBVH(std::vector<BoundingBoxGLM<glm::vec3>>&triBBs , std::vector<uint32_t>& triangleIndices, TriangleOrNodeIndex start, TriangleOrNodeIndex end, size_t maxTrisPerNode, uint32_t infoVal);
		std::vector<BVHNode> nodes;
	};
};
