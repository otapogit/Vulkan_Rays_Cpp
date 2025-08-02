#pragma once

#include "core/core.h"
#include "glm/ext.hpp"
#include <vector>

namespace core{
	struct SimpleMesh {

		std::vector<glm::vec3> verts;
		std::vector<glm::vec3> norms;


		BufferMemory m_vb;
		BufferMemory m_indexbuffer;
		BufferMemory m_normalbuffer;
		BufferMemory m_uvbuffer;
		
		size_t m_vertexBufferSize = 0;
		size_t m_indexBufferSize = 0;
		size_t m_normalBufferSize = 0;
		size_t m_uvBufferSize = 0;

		uint32_t id = 0;
		glm::mat4 m_transMat = glm::mat4(1.0f);

		//colorbuffer?
		glm::vec4 color;

		VkIndexType m_indexType = VK_INDEX_TYPE_UINT32;
		int vertexcount = 0;
		VulkanTexture* m_pTex = NULL;
		int texIndex = -1;

		void Destroy(VkDevice device) {
			m_vb.Destroy(device);
			if (m_pTex) {
				m_pTex->Destroy(device);
				delete m_pTex;
			}
			if (m_indexbuffer.m_buffer)
				m_indexbuffer.Destroy(device);
			if (m_normalbuffer.m_buffer)
				m_normalbuffer.Destroy(device);
			if (m_uvbuffer.m_buffer)
				m_uvbuffer.Destroy(device);

		}
	};
}