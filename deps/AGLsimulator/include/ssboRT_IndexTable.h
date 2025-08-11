#pragma once

#include "ssboDataContainer.h"


namespace autis {
	class SSBO_RT_IndexTable : public SSBODataContainer<uint32_t> {
	public:
		const std::string& getBlockName() const override;
		const std::vector<std::string>& getDefinition() const override;
		static std::shared_ptr<SSBO_RT_IndexTable> build(const std::vector<uint32_t>& m, GLenum usage);
	private:
		SSBO_RT_IndexTable(uint32_t size, GLenum usage) : SSBODataContainer<uint32_t>(size, usage) {};
	};
};
