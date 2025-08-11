#pragma once

#include "ssboDataContainer.h"
#include <bvh.h>


namespace autis {
	class SSBO_RT_MeshInfoTable : public SSBODataContainer<BVHMeshInfo> {
	public:
		const std::string& getBlockName() const override;
		const std::vector<std::string>& getDefinition() const override;
		static std::shared_ptr<SSBO_RT_MeshInfoTable> build(const std::vector<BVHMeshInfo>& m, GLenum usage);
	private:
		SSBO_RT_MeshInfoTable(uint32_t size, GLenum usage) : SSBODataContainer<BVHMeshInfo>(size, usage) {};
	};
};
