#pragma once

// STL
#include <vector>
// dependencies (glm)
#include <glm/vec3.hpp>
// dependencies (glew)
#include <GL/glew.h>
// AGLcommon
#include <boundingVolumes.h>
// AGL
#include "meshData.h"

namespace autis {
	class DrawCommand;

	class BaseMesh {
	public:
		// Usar siempre estos puntos de vinculación para los atributos
		enum BufferObjectType {
			VERTICES,
			NORMALS,
			COLORS,
			INDICES,
			TEX_COORD0,
			TEX_COORD1,
			TEX_COORD2,
			TEX_COORD3,
			TANGENTS,
			_LAST_
		};
		static constexpr char const* bufferObjectTypeLabels[]{
			"Vertices",
			"Normals",
			"Colors",
			"Indices",
			"Texture coord. (0)",
			"Texture coord. (1)",
			"Texture coord. (2)",
			"Texture coord. (3)",
			"Tangents"
		};

		static_assert(sizeof(bufferObjectTypeLabels) / sizeof(bufferObjectTypeLabels[0]) == _LAST_);
		BaseMesh();
		virtual ~BaseMesh();

		/**
		 * @brief Obtiene la caja de inclusión de la malla
		 * @return La caja de inclusión
		*/
		BoundingBox getBB() const { return bb; };
		/**
		 * @brief Devuelve la posición del centro de la caja de inclusión
		 * @return El centro de la caja de inclusión
		*/
		glm::vec3 center() const;

		/**
		 * @brief Define la posición de los vértices que componen la malla
		 * @param v  puntero a un array con las posiciones
		 * @param ncomponents número de componentes por vértice (1, 2, 3 o 4)
		 * @param nVertices cantidad de vértices
		 * @param usage Uso que se le dará al buffer (por defecto, GL_STATIC_DRAW)
		*/
		virtual void addVertices(const float* v, unsigned int ncomponents, size_t nVertices,
			GLenum usage) = 0;

		/**
		 * @brief Define la posición de los vértices que componen la malla.
		 * @tparam T Tipo de los vértices (glm::vec2, glm::vec3, glm::vec4)
		 * @param v Puntero a los datos
		 * @param n Número de vértices
		 * @param usage Uso que se le dará al buffer (por defecto, GL_STATIC_DRAW)
		*/
		template<typename T>
		void addVertices(const T* v, size_t n, GLenum usage = GL_STATIC_DRAW) {
			addVertices(&v[0].x, T::length(), n, usage);
		}

		/**
		 * @brief Define la posición de los vértices que componen la malla.
		 * @tparam T Tipo de los vértices (glm::vec2, glm::vec3, glm::vec4)
		 * @param v El vector con los datos
		 * @param usage Uso que se le dará al buffer (por defecto, GL_STATIC_DRAW)
		*/
		template<typename T>
		void addVertices(const std::vector<T>& v, GLenum usage = GL_STATIC_DRAW) {
			addVertices(&v[0].x, T::length(), v.size(), usage);
		}

		/**
		 * @brief Define los índices para dibujar la malla.
		 * @param i Puntero a los índices (que son GLubyte)
		 * @param n Número de índices
		 * @param usage Uso que se le dará al buffer
		*/
		virtual void addIndices(const GLubyte* i, size_t n, GLenum usage) = 0;

		/**
		 * @brief Define los índices para dibujar la malla.
		 * @param i Puntero a los índices (que son GLushort)
		 * @param n Número de índices
		 * @param usage Uso que se le dará al buffer
		*/
		virtual void addIndices(const GLushort* i, size_t n, GLenum usage) = 0;

		/**
			* @brief Define los índices para dibujar la malla.
			* @param i Puntero a los índices (que son GLuint)
			* @param n Número de índices
			* @param usage Uso que se le dará al buffer
		*/
		virtual void addIndices(const GLuint* i, size_t n, GLenum usage) = 0;

		/**
		 * @brief Define los índices para dibujar la malla
		 * @tparam T El tipo de índices (GLubyte, GLuint o GLushort)
		 * @param i puntero a los índices
		 * @param usage Uso que se le dará al buffer
		*/
		template<typename T>
		void addIndices(const std::vector<T>& i, GLenum usage = GL_STATIC_DRAW) {
			addIndices(&i[0], i.size(), usage);
		};

		/**
		 * @brief Define the mesh normals
		 * @param nr the normals
		 * @param n number of normals
		 * @param usage Intended use (i.e. GL_STATIC_DRAW)
		*/
		virtual void addNormals(const float* nr, size_t n, GLenum usage) = 0;

		void addNormals(const glm::vec3* nr, size_t n, GLenum usage = GL_STATIC_DRAW) {
			addNormals(&nr[0].x, n, usage);
		}

		void addNormals(const std::vector<glm::vec3>& nr, GLenum usage = GL_STATIC_DRAW) {
			addNormals(&nr[0], nr.size(), usage);
		};

		/** Define las coordenadas de textura para dibujar la malla.
		\param tex_unit unidad de textura a la que hacen referencia las coordenadas
		de textura (0, 1, 2, ...)
		\param t un array con las coordenadas de textura
		\param nComponents número de coordenadas de cada coordenada de textura (1-4)
		\param n el número de coordenadas de textura
		\param usage el tipo de uso que se le dará al buffer object
		 */
		virtual void addTexCoord(unsigned int tex_unit, const GLfloat* t, unsigned int nComponents,
			size_t n, GLenum usage = GL_STATIC_DRAW) = 0;

		void addTexCoord(unsigned int tex_unit, const std::vector<float>& texcoords, GLenum usage = GL_STATIC_DRAW) {
			addTexCoord(tex_unit, &texcoords[0], 1, texcoords.size(), usage);
		}

		/**
		 * @brief Define las coordenadas de textura para dibujar la malla.
		 * @tparam T Puede ser glm::vec2, glm::vec3 o glm::vec4
		 * @param tex_unit unidad de textura a la que hacen referencia las coordenadas
				de textura
		 * @param texcoords las coordenadas de textura
		 * @param usage el tipo de uso que se le dará al buffer object
		*/
		template<typename T>
		void addTexCoord(unsigned int tex_unit, const std::vector<T>& texcoords, GLenum usage = GL_STATIC_DRAW) {
			static_assert(
				std::is_same<T, glm::vec2>::value ||
				std::is_same<T, glm::vec3>::value ||
				std::is_same<T, glm::vec4>::value, "Only 2, 3 and 4D textures");
			addTexCoord(tex_unit, &texcoords[0].x, T::length(), texcoords.size(), usage);
		}

		/**
		 * @brief Define las coordenadas de textura para dibujar la malla.
		 * @tparam T Puede ser glm::vec2, glm::vec3 o glm::vec4
		 * @param tex_unit unidad de textura a la que hacen referencia las coordenadas
				de textura
		 * @param texcoords las coordenadas de textura
		 * @param usage el tipo de uso que se le dará al buffer object
		*/
		template<typename T>
		void addTexCoord(unsigned int tex_unit, const T* texcoords, size_t nTexCoords, GLenum usage = GL_STATIC_DRAW) {
			static_assert(
				std::is_same<T, glm::vec2>::value ||
				std::is_same<T, glm::vec3>::value ||
				std::is_same<T, glm::vec4>::value, "Only 2, 3 and 4D textures");
			addTexCoord(tex_unit, &texcoords[0].x, T::length(), nTexCoords, usage);
		}
		/**
		* @brief Define los colores para dibujar la malla. Cada color está compuesto por
		* 'ncomponents' de floats consecutivos
		* @param c array de colores
		* @param ncomponents número de componentes por color (1-4)
		* @param nColors número de colores en el array
		* @param usage uso que se le dará al buffer object
		*/
		virtual void addColors(const float* c, unsigned int ncomponents, size_t nColors,
			GLenum usage) = 0;


		void addColors(const float* v, size_t n, GLenum usage = GL_STATIC_DRAW) {
			addColors(v, 1, n, usage);
		}

		/**
		 * @brief Define el color de los vértices que componen la malla.
		 * @tparam T Tipo de los colores (glm::vec2, glm::vec3, glm::vec4)
		 * @param v Puntero a los datos
		 * @param n Número de vértices
		 * @param usage Uso que se le dará al buffer (por defecto, GL_STATIC_DRAW)
		*/
		template<typename T>
		void addColors(const T* v, size_t n, GLenum usage = GL_STATIC_DRAW) {
			addColors(&v.x, T::length(), n, usage);
		}

		template<typename T>
		void addColors(const std::vector<T>& v, GLenum usage = GL_STATIC_DRAW) {
			addColors(&v[0].x, T::length(), v.size(), usage);
		}

		/**
		Inserta un atributo a los vértices.
		\param attribute_index índice del atributo (usar uno menor que 16)
		\param type constante de OpenGL que define el tipo de los componentes del atributo 
		(p.e., GL_FLOAT, GL_UNSIGNED_SHORT...) 
		\param ncomponents número de componentes de cada atributo
		\param n cantidad de atributos proporcionados
		\param normalize si type es de tipo entero y normalize == true, normalizar los datos
		al rango [0..1] o [-1..1].
		\param usage uso que se le va a dar al buffer (GL_STATIC_DRAW, GL_DYNAMIC_COPY...)
		*/
		virtual void addAttribute(unsigned int attribute_index, GLenum type, 
			const void* a, unsigned int ncomponents, size_t n, bool normalize, GLenum usage) = 0;

		/**
		Inserta un atributo de tipo entero (sin normalizar ni convertir a float) a los vértices.
		\param attribute_index índice del atributo (usar uno mayor o igual a _LAST_)
		\param type constante de OpenGL que define el tipo de los
		componentes del atributo (p.e., GL_BYTE, GL_INT, GL_UNSIGNED_SHORT...)
		\param a puntero a los datos
		\param ncomponents número de componentes de cada atributo
		\param n cantidad de atributos proporcionados
		\param usage uso que se le va a dar al buffer (GL_STATIC_DRAW, GL_DYNAMIC_COPY...)
		*/
		virtual void addIntegerAttribute(unsigned int attribute_index, GLenum type, 
			const void* a, unsigned int ncomponents, size_t n, 
			GLenum usage) = 0;

		/**
		Establece el nombre de la malla
		*/
		void setName(const std::string& name);
		/**
		\return el nombre la malla (puede ser la cadena vacía)
		*/
		const std::string& getName() const;


		/**
		  Añade una nueva orden de dibujado sobre los elementos del objeto Mesh
		  \param d El objeto que contiene la orden de dibujo. *No* liberes el
		  puntero
		  */
		void addDrawCommand(DrawCommand* d);
		/**
		  Elimina todas las órdenes de dibujado sobre el Mesh (no se dibujará
		  nada, aunque hayan vértices, colores, etc)
		  */
		void clearDrawCommands();

		/**
		Mainly for testing
		*/
		const std::vector<DrawCommand*> getDrawCommands() const {
			return drawCommands;
		}
		/**
		 * @brief Establece un identificador de la malla. Por defecto, todas las mallas se crean con un identificador
		 * a std::numeric_limits<uint32_t>::max(). Es responsabilidad del usuario gestionar los ids
		 * @param meshId
		*/
		void setId(autis::MeshId meshId) { id = meshId; }

		/**
		 * @brief
		 * @return the id
		*/
		autis::MeshId getId() const { return id; }

	protected:
		autis::MeshId id;
		std::string name;
		BoundingBox bb;
		std::vector<DrawCommand*> drawCommands;

	};


}