#pragma once

#include <GL/glew.h>
#include <memory>
#include <string>
#include <vector>
namespace autis {
	class BufferObject;

	/**
	 * @brief BufferObject in CPU. Holds the memory and required data to build a BufferObject in GPU
	*/
	class BOInCPU {
	public:
		/**
		 * @brief Creates a BufferObject in CPU RAM
		 * @param type Type of the components in the BO (GL_FLOAT, GL_UNSIGNED_BYTE...)
		 * @param usage Intended use of the buffer (GL_STATIC_DRAW, GL_DYNAMIC_DRAW...)
		 * @param nComponents Number of components per element (1, 2, 3, 4)
		 * @param nElems Number of elements in the buffer
		 * @param normalize If type is integer, normalize == true means interpret the data
		 * as a float, normalizing to [0,1] or [-1,1]. If normalize == false, interpret the
		 * data as a float with its original value
		 * @param data Pointer to the data
		*/
		BOInCPU(GLenum type, GLenum usage, uint32_t nComponents, size_t nElems, bool normalize,
			const void* data);
		~BOInCPU();
		/**
		 * @brief Creates a BufferObject in GPU with the provided data
		 * @param debugName [Optional] A label for debugging
		 * @return A handler to the GL BufferObject
		*/
		std::shared_ptr<BufferObject> createBufferObject(const std::string& debugName = "");
		/**
		 * @brief Creates a BufferObject in GPU with the provided data, or reuses the provided one
		 * @param current [Optional] The current BufferObject. If it is not null, it will be reused if 
		 * there is enough space. If it is null or the data does not fit in the current BO, 
		 * a new BufferObject will be created
		 * @param debugName [Optional] A label for debugging
		 * @return A handler to the GL BufferObject
		*/
		std::shared_ptr<BufferObject> createOrReuseBufferObject(std::shared_ptr<BufferObject> current, const std::string& debugName = "");
		/**
		 * @brief
		 * @return The base type of the data in the BO (GL_FLOAT, GL_UNSIGNED_BYTE...)
		*/
		GLenum type() const { return mGLType; }
		/**
		 * @brief
		 * @return The total size of the buffer, in bytes
		*/
		size_t size() const { return totalSize; }
		/**
		 * @brief
		 * @return Number of components per element
		*/
		uint32_t nComponents() const { return mNComponents; }
		/**
		 * @brief
		 * @return Intended use of the BufferObject (GL_STATIC_DRAW...)
		*/
		GLenum usage() const { return mUsage; }

		/**
		 * @brief
		 * @return The number of elements in the array
		*/
		size_t count() const { return mNElems; }
		/**
		 * @brief
		 * @return Pointer to the data
		*/
		const void* data() const { return mData; }

		/**
		 * @brief
		 * @return If the original data was integer, it will be normalized to [0..1] or [-1,1]
		*/
		bool normalize() const { return mNormalize; }

		/**
		 * @brief Writes data to the buffer
		 * @param data
		 * @param offset
		 * @param size
		 * @return
		*/
		bool write(const void* data, size_t size, size_t offset);
	private:
		GLenum mGLType, mUsage;
		uint32_t mNComponents;
		size_t mNElems;
		uint8_t* mData;
		size_t totalSize;
		bool mNormalize;
	};
};
