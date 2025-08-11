#pragma once

#include "idCommon.h"

namespace autis
{

	struct MeshIdTag {};
	using MeshId = UnsignedIntIdentifier<MeshIdTag>;
	const MeshId FIRST_MESH_ID = MeshId(0);
	const MeshId INVALID_MESH_ID = MeshId(INVALID_UNSIGNED_INT_ID_VALUE);

}
