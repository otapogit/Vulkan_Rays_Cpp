#pragma once

// STL
#include <ostream>
#include <string>
#include <vector>
// AGLcommon
#include "idCommon.h"
#include "nodeCommon.h"
#include "partInfo.h"

namespace autis
{
	// PartId is def ined in partInfo.h

	struct FaceIdTag {};
	using FaceId = UnsignedIntIdentifier<FaceIdTag>;
	const FaceId INVALID_FACE_ID = FaceId(INVALID_UNSIGNED_INT_ID_VALUE);
	const FaceId FIRST_FACE_ID = FaceId(0);

	struct EdgeIdTag {};
	using EdgeId = UnsignedIntIdentifier<EdgeIdTag>;
	const EdgeId INVALID_EDGE_ID = EdgeId(INVALID_UNSIGNED_INT_ID_VALUE);
	const EdgeId FIRST_EDGE_ID = EdgeId(0);

	struct VertexIdTag {};
	using VertexId = UnsignedIntIdentifier<VertexIdTag>;
	const VertexId INVALID_VERTEX_ID = VertexId(INVALID_UNSIGNED_INT_ID_VALUE);
	const VertexId FIRST_VERTEXs_ID = VertexId(0);
}

namespace autis
{

	struct StepPart
	{

		enum Type
		{
			UNKNOWN = 0,
			REGULAR,
			HOLE,
			EXTENSION,
			UNION,
			AUX_WIRE,
			SOLID,
			SPLIT,
			__NUM__
		};

		PartId PartId;
		NodeId nodeId;
		std::string name;
		Type piece_type;
	};

	struct StepData
	{
		std::string stepFilename;		// Ends with .stp/.STP/.step/.STEP
		std::string name;				// No extension
		std::vector<StepPart> parts;
	};

	struct CADJobChangesInAssembly
	{
		std::vector<PartId> deletedParts;  // shell ids of the parts that were deleted. The id may have reassigned
		std::vector<PartId> modifiedParts; // shell ids of the parts that were modified
		uint32_t            newPartsAdded; // new parts are added at the end, this is all the info we need

		CADJobChangesInAssembly() : newPartsAdded(0) { }

		void appendChanges(const CADJobChangesInAssembly& other)
		{
			deletedParts.insert(deletedParts.end(), other.deletedParts.begin(), other.deletedParts.end());
			modifiedParts.insert(modifiedParts.end(), other.modifiedParts.begin(), other.modifiedParts.end());
			newPartsAdded += other.newPartsAdded;
		}
	};

	enum MeshingMode
	{
		LOW_POLY = 0,
		COARSE,
		FINE,
		CUSTOM,
		LAST
	};

	enum class CADFormat
	{
		FromExtension,  // Figure out the format based on the file extension
		Step,			// assume it is a STEP file
		Iges			// assume it is a IGES file
	};

	// Build Assembly From File result status
	enum BAFF_RESULT_STATUS
	{
		STATUS_OK,        // Assembly loaded and ready to use
		FILE_NOT_FOUND,   // Assembly file not found
		FILE_NOT_VALID,   // Assembly file is not valid
		PROJECT_MISMATCH, // Assembly file content belongs to a different project
		CONTENT_MISMATCH  // Assembly file content does not match the vehicle parts in the tunnel
	};

	enum class TaskType
	{
		None,
		LoadStepFile,
		LoadAssemblyFile,
		SaveADAT2,
		GenerateOCCTMeshes,
		GenerateNetgenMeshes,
		HealShells,
		SplitShells,
		ExtendEdges,
		JoinEdges,
		RemoveFaces,
	};

	inline bool requiresMeshingAfterFinish(TaskType type) noexcept
	{ 
		return type != TaskType::GenerateOCCTMeshes
			&& type != TaskType::GenerateNetgenMeshes
			&& type != TaskType::RemoveFaces
			&& type != TaskType::None
			&& type != TaskType::SaveADAT2;
	}

	struct MeshingConfig
	{ //https://dev.opencascade.org/doc/refman/html/struct_i_mesh_tools___parameters.html
		enum class Algorithm
		{
			Watson,
			Delabella
		} algo = Algorithm::Watson; // Meshing algorithm

		float edgeAngle, edgeDeflection;
		float intAngle, intDeflection;
		float minSize;
		bool relative;
		bool internalVertices;
		bool controlSurfaceDeflection;
		bool adjustMinSize;
		bool allowQualityDecrease;

		MeshingConfig() : edgeAngle(0.0f), edgeDeflection(0.0f), intAngle(0.0f), intDeflection(0.0f),
			minSize(0.0f), relative(false), internalVertices(false), controlSurfaceDeflection(false),
			adjustMinSize(false), allowQualityDecrease(false)
		{
		}

		MeshingConfig(Algorithm algo, float edgeAngle, float edgeDeflection,
			float intAngle, float intDeflection, float minSize, bool relative,
			bool internalVertices, bool controlSurfaceDeflection, bool adjustMinSize,
			bool allowQualityDecrease)
			: algo(algo), edgeAngle(edgeAngle), edgeDeflection(edgeDeflection), intAngle(intAngle),
			intDeflection(intDeflection), minSize(minSize), relative(relative), internalVertices(internalVertices),
			controlSurfaceDeflection(controlSurfaceDeflection), adjustMinSize(adjustMinSize),
			allowQualityDecrease(allowQualityDecrease)
		{
		}

	};

	struct NetgenMeshingParams
	{
		float minh;
		float maxh;
		float grading;

		NetgenMeshingParams() : minh(0.0f), maxh(0.0f), grading(0.0f) { }

		NetgenMeshingParams(float minh, float maxh, float grading)
			: minh(minh), maxh(maxh), grading(grading) { }

		bool operator==(const NetgenMeshingParams& other) const
		{
			return minh == other.minh && maxh == other.maxh && grading == other.grading;
		}
	};
	
}
