#pragma once

#include "ssboDataContainer.h"
#include <bvh.h>


namespace autis {
	class SSBO_RT_NodeTable : public SSBODataContainer<BVHNode> {
	public:
		const std::string& getBlockName() const override;
		const std::vector<std::string>& getDefinition() const override;
		static std::shared_ptr<SSBO_RT_NodeTable> build(const std::vector<BVHNode>& m, GLenum usage);
	private:
		SSBO_RT_NodeTable(uint32_t size, GLenum usage) : SSBODataContainer<BVHNode>(size, usage) {};
	};
};
