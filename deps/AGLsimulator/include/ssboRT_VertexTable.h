#pragma once

#include "ssboDataContainer.h"
#include <glm/fwd.hpp>

namespace autis {
	class SSBO_RT_VertexTable : public SSBODataContainer<glm::vec4> {
	public:
		const std::string& getBlockName() const override;
		const std::vector<std::string>& getDefinition() const override;
		static std::shared_ptr<SSBO_RT_VertexTable> build(const std::vector<glm::vec4>& m, GLenum usage);
	private:
		SSBO_RT_VertexTable(uint32_t size, GLenum usage) : SSBODataContainer<glm::vec4>(size, usage) {};
	};
};
