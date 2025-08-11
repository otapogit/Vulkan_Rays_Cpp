#pragma once

#include <stdint.h>
#include <string>
// AGLcommon
#include "idCommon.h"

namespace autis
{
	struct NodeIdTag {};
	using NodeId = UnsignedIntIdentifier<NodeIdTag>;
	const NodeId INVALID_NODE_ID = NodeId(INVALID_UNSIGNED_INT_ID_VALUE);
	const NodeId ROOT_ID = NodeId(1000);
}

namespace autis
{

	enum class NodeType
	{
		Unknown = 0,
		Group = (1 << 1),
		Transform = (1 << 2),
		Geode = (1 << 3),
		NodeAuxiliarGeometryPart = (1 << 4),
		NodeInspectionCamera = (1 << 5),
		NodeLight = (1 << 6),
		NodeVehicle = (1 << 7),
		NodeVehiclePart = (1 << 8),
		NodeVirtualView = (1 << 9),
		Node3DMaxMatrix = (1 << 10),
		NodeReferenceGroup = (1 << 11),
		NodeRobotTrajectory = (1 << 12),
		NodeTrajectoryLine = (1 << 13),
		NodePositionOrientationWidget = (1 << 14),
		NodeAnimationTransform = (1 << 15),
		NodeAuxiliarGeometry = (1 << 16),
		NodeReferenceTransform = (1 << 17),
		NodeTransformationWidget = (1 << 18),
		NodeEndEffector = (1 << 19),
		NodeCursor3D = (1 << 20),
		WidgetGeode = (1 << 21),
		NodeDirect3DSMaxMatrix = (1 << 22)
	};

	using NodeMask = uint32_t;
	constexpr NodeMask ALL_NODES_MASK = 0xffffffff;
	constexpr NodeMask NO_NODES_MASK = 0x00000000;
	inline constexpr NodeMask operator&(NodeMask x, NodeType y) { return static_cast<uint32_t>(x) & static_cast<uint32_t>(y); }
	inline constexpr NodeMask operator|(NodeMask x, NodeType y) { return static_cast<uint32_t>(x) | static_cast<uint32_t>(y); }
	inline constexpr NodeMask operator|(NodeType x, NodeType y) { return static_cast<uint32_t>(x) | static_cast<uint32_t>(y); }
	inline constexpr NodeMask operator~(NodeType x) { return ~static_cast<uint32_t>(x); }

	struct FlatNode
	{
		NodeId nodeId = INVALID_NODE_ID;
		std::string name;
		NodeType nodeType;
		NodeId parentId = INVALID_NODE_ID;
		std::string comment;

		inline bool operator==(const FlatNode& o) const {
			return	nodeId == o.nodeId &&
				name == o.name &&
				nodeType == o.nodeType &&
				parentId == o.parentId;
		}
		inline bool operator!=(const FlatNode& o) const {
			return !(*this == o);
		}
	};

}

namespace autis::suffix
{
	enum MutableNames
	{
		ANIMATION_NAMES = 1,
		ANIMATION_NODES = 2,
		VIRTUAL_VIEWS_NODES = 4,
		INSPECTION_CAMERAS_NODES = 8,
		VEHICLE_NODES = 16,
		TRANSFORMS = 32,
		GROUPS = 64,
		LIGHTS_NODES = 128,
		REFERENCE_GROUP = 256
	};
	using NamesToHandle = uint64_t;

	constexpr const char* const SEPARATOR = "::";

	// view name field separator, splits name from suffix
	static const std::string VIEWNAME_SEPARATOR = autis::suffix::SEPARATOR;
	// camera name field separator, splits name from suffix
	static const std::string INSPCAMNAME_SEPARATOR = autis::suffix::SEPARATOR;

}
