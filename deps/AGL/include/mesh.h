
#ifndef _MESH_H
#define _MESH_H 2011

#include <memory>
#include <vector>
#include <mutex>

#include <GL/glew.h>
#include "boInCPU.h"

#include "glContext.h"
#include "baseMesh.h"
#include "vertexArrayObject.h"
#include "meshData.h"

namespace autis {

	/**

	\class Mesh

	Clase que contiene una malla de un modelo. Un modelo (definido por la clase
	Model) puede estar compuesto por varias mallas. Una malla está compuesta normalmente
	por varios arrays (posiciones de vértices, normales, colores,
	coordenadas de textura, etc). Como se están definiendo los atributos de una
	serie de vértices, todos los arrays deberían tener el mismo tamaño, excepto
	el array de índices, que puede tener un número de elementos distinto.
	Cada array se almacenará en un VBO y las conexiones con los puntos de
	vinculación de atributos se almacena en un VAO por cada Mesh.
	Para dibujar primitivas con los vértices y sus atributos anteriores, se pueden
	incorporar llamadas de dibujado con el método addDrawCommand. De esta forma, con
	un mismo array de vértices se pueden ejecutar varias órdenes de dibujado.
	Por ejemplo, para dibujar un triángulo, se podría crear la siguiente Malla:

	GLfloat vertices[] = {-0.5, -0.5, 0.0,
		0.5, -0.5, 0.0,
		0.0, 0.5, 0.0};
	GLfloat colores[] = {1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0};

	auto m = std::make_shared<Mesh>();

	m->addColors(colores, 3, 3);
	m->addVertices(vertices, 3, 3);
	m->addDrawCommand(new PGUPV::DrawArrays(GL_TRIANGLES, 0, 3));

	Hasta que no se dibuja una malla por primera vez, toda la información está almacenada
	en RAM, por lo que no hace falta que haya un contexto OpenGL activo. En la primera
	llamada a Mesh::render es cuando se crean los VBO en GPU (y, por defecto, se libera
	la memoria de los buffers en CPU). Para cargar explícitamente los VBO a la GPU, se puede
	usar Mesh::buildMesh.

	*/

	class Texture;
	class BufferObject;

	class Mesh : public BaseMesh {
	public:
#define NUM_TEX_COORD 4
		// Constructor de una malla
		Mesh();
		~Mesh();

		/**
		 * @brief Keep data in cpu memory after creating the BufferObjects (useful if we need to access
		 * the data in CPU (by default, it releases the CPU memory)
		 * @param keepInCPU true for keeping the data in RAM
		*/
		void setKeepDataInCPU(bool keepInCPU = true);

		/**
		 * @brief Returns true if the data is still in CPU memory
		*/
		bool hasDataInCPU() const;

		/**
		 * @brief Builds a mesh with the previously provided data (vertices, indices, etc.)
		 * @param ctx the current context where to store the VAO for this mesh.
		 * @warning This method should be called within the context where the VAO is going to be used
		*/
		void buildMesh(GLContext& ctx);

		/**
		 * @brief Define la posición de los vértices que componen la malla
		 * @param v  puntero a un array con las posiciones
		 * @param ncomponents número de componentes por vértice (1, 2, 3 o 4)
		 * @param nVertices cantidad de vértices
		 * @param usage Uso que se le dará al buffer (por defecto, GL_STATIC_DRAW)
		*/
		void addVertices(const float* v, unsigned int ncomponents, size_t nVertices,
			GLenum usage = GL_STATIC_DRAW) override;

		using BaseMesh::addVertices;

		bool updateVertices(const float* v, uint32_t ncomponents, uint32_t nVertices, uint32_t firstVertexIndex = 0);

		template <typename T>
		bool updateVertices(const std::vector<T>& v) {
			return updateVertices(&v[0].x, T::length(), static_cast<uint32_t>(v.size()));
		}

		// updates the position of a range of vertices
		template <typename T>
		bool updateVertices(uint32_t indexFirstVertex, const std::vector<T>& v) {
			return updateVertices(&v[0].x, T::length(), static_cast<uint32_t>(v.size()), indexFirstVertex);
		}

		// Define los índices para dibujar la malla. Cada índice es de tipo unsigned
		// byte. Recibe un vector de n índices.
		void addIndices(const GLubyte* i, size_t n, GLenum usage = GL_STATIC_DRAW) override;

		// Define los índices para dibujar la malla. Cada índice es de tipo unsigned
		// short. Recibe un vector de n índices.
		void addIndices(const GLushort* i, size_t n, GLenum usage = GL_STATIC_DRAW) override;

		// Define los índices para dibujar la malla. Cada índice es de tipo unsigned
		// int. Recibe un vector de n índices.
		void addIndices(const GLuint* i, size_t n, GLenum usage = GL_STATIC_DRAW) override;


		///// <EDGES>
		bool hasEdges() const;
		void addEdge(autis::EdgeId EdgeId, const std::vector<autis::MeshIndex>& indices,
			const std::vector<autis::MeshIndex>& selectableVertices = {});
		size_t getNumEdges() const;
		Edge getEdge(uint32_t index) const;
		bool hasEdge(autis::EdgeId edgeId) const;
		void renderEdge(uint32_t index, GLContext &ctx);
		void renderEdgeById(autis::EdgeId EdgeId, GLContext &ctx);
		void renderEdges(GLContext &ctx);

		// render polylines between the provided indices
		void renderExplicitEdges(std::vector<std::vector<uint32_t>> edges, GLContext& ctx);
		void renderExplicitVertices(std::vector<uint32_t> vtcs, GLContext& ctx);

		void renderSelectableEdgeVertices(uint32_t index, GLContext &ctx);
		void renderSelectableEdgeVerticesByEdgeId(autis::EdgeId edgeId, GLContext &ctx);
		void renderSelectableEdgeVertices(GLContext &ctx);

		void renderSelectableVertex(uint32_t vertexIndex, GLContext& ctx);

		void renderVertices(GLContext& ctx);

		autis::EdgeId getEdgeId(uint32_t index) const;
		void clearEdges();

		/**
		 * @brief Returns the data of a given vertex in a edge
		 * @param edgeId the id of the edge
		 * @param vertexIndex the index of the vertex in the edge
		 * @param pos [out] the position of the vertex
		 * @param normal [out] [optional] the normal of the vertex
		 * @return true, if the edge and vertex exist
		*/
		bool getEdgeVertex(const autis::EdgeId& edgeId, const VertexId& vertexIndex, glm::vec3* pos, glm::vec3* normal = nullptr);

		/**
		 * @brief Returns the data of a given vertex
		 * @param vertexIndex the index of the vertex
		 * @param pos [out] the position of the vertex
		 * @param normal [out] [optional] the normal of the vertex
		 * @return true, if the vertex exists
		 */
		bool getVertex(const autis::VertexId& vertexIndex, glm::vec3* pos, glm::vec3* normal = nullptr);

		///// </EDGES>


		////// <FACES>

		void addFace(autis::FaceId faceId, const std::vector<autis::MeshIndex> &indices);
		size_t getNumFaces() const;
		void clearFaces();
		//! \brief Returns true if there are faces in the mesh
		bool hasFaces();
		Face getFace(uint32_t index) const;
		bool hasFace(autis::FaceId faceId) const;
		void renderFace(uint32_t index, GLContext& ctx);
		void renderFaceById(autis::FaceId faceId, GLContext& ctx);
		////// </FACES>

		using BaseMesh::addIndices;

		bool updateIndices(const GLubyte* inds, uint32_t size, uint32_t offset = 0);
		bool updateIndices(const GLushort* inds, uint32_t size, uint32_t offset = 0);
		bool updateIndices(const GLuint* inds, uint32_t size, uint32_t offset = 0);
		template <typename T>
		bool updateIndices(const std::vector<T>& v, uint32_t offset = 0) {
			static_assert(std::is_integral<T>::value == true, "Only unsigned integral types");
			if (n_indices == 0) {
				addIndices(v);
				return true;
			}

			auto inputSize = static_cast<uint32_t>(v.size());
			if (sizeof(T) == 1 && indices_type == GL_UNSIGNED_BYTE ||
				sizeof(T) == 2 && indices_type == GL_UNSIGNED_SHORT ||
				sizeof(T) == 4 && indices_type == GL_UNSIGNED_INT) {
				return updateIndices(v.data(), inputSize, offset);
			}

			switch (indices_type) {
			case GL_UNSIGNED_BYTE:
			{
				std::vector<GLubyte> is;
				is.reserve(v.size());
				for (const auto i : v) is.push_back(static_cast<uint8_t>(i));
				return updateIndices(is.data(), inputSize, offset);
			}
			case GL_UNSIGNED_SHORT:
			{
				std::vector<GLushort> is;
				is.reserve(v.size());
				for (const auto i : v) is.push_back(static_cast<uint16_t>(i));
				return updateIndices(is.data(), inputSize, offset);
			}
			case GL_UNSIGNED_INT:
			{
				std::vector<GLuint> is;
				is.reserve(v.size());
				for (const auto i : v) is.push_back(static_cast<uint32_t>(i));
				return updateIndices(is.data(), inputSize, offset);
			}
			}
			return false;
		}

		// Define las normales para dibujar la malla. Cada normal está definida por 3
		// floats consecutivos. Recibe un vector de n normales.
		void addNormals(const GLfloat* nr, size_t n, GLenum usage = GL_STATIC_DRAW) override;


		bool hasNormals() const;

		using BaseMesh::addNormals;

		// Establece la normal que usarán *todos* los vértices
		void setNormal(const glm::vec3& normal);

		bool updateNormals(const float* n, uint32_t nVertices, uint32_t firstNormalIndex = 0);
		bool updateNormals(uint32_t firstNormalIndex, const std::vector<glm::vec3>& v) {
			return updateNormals(&v[0].x, static_cast<uint32_t>(v.size()), firstNormalIndex);
		}
		bool updateNormals(const std::vector<glm::vec3>& v) {
			return updateNormals(&v[0].x, static_cast<uint32_t>(v.size()));
		}

		virtual void addTexCoord(unsigned int tex_unit, const GLfloat* t, unsigned int nComponents,
			size_t n, GLenum usage = GL_STATIC_DRAW) override;

		using BaseMesh::addTexCoord;

		void addColors(const float* c, unsigned int ncomponents, size_t nColors,
			GLenum usage = GL_STATIC_DRAW) override;

		using BaseMesh::addColors;

		// Establece el color que usarán *todos* los vértices
		void setColor(const glm::vec4& color);

		/**
		Inserta un atributo a los vértices.
		\param attribute_index índice del atributo (usar uno mayor o
		igual a _LAST_)
		\param type constante de OpenGL que define el tipo de los
		componentes del atributo (p.e., GL_FLOAT, GL_UNSIGNED_SHORT...)
		\param a puntero a los datos
		\param ncomponents número de componentes de cada atributo
		\param n cantidad de atributos proporcionados
		\param normalize si los datos son de tipo entero y normalize == true, normalizar
		\param usage uso que se le va a dar al buffer (GL_STATIC_DRAW, GL_DYNAMIC_COPY...)
		*/
		void addAttribute(unsigned int attribute_index, GLenum type,
			const void* a, unsigned int ncomponents, size_t n, bool normalize,
			GLenum usage = GL_STATIC_DRAW) override;

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
		void addIntegerAttribute(unsigned int attribute_index, GLenum type,
			const void* a, unsigned int ncomponents, size_t n,
			GLenum usage = GL_STATIC_DRAW) override;

		// Establece un valor estático para el atributo de índice indicado. Todos los
		// vértices compartirán dicho valor para el atributo
		void setAttribute(unsigned int attribute_index, const glm::vec3& a);
		// Establece un valor estático para el atributo de índice indicado. Todos los
		// vértices compartirán dicho valor para el atributo
		void setAttribute(unsigned int attribute_index, const glm::vec4& a);

		/**
		Función que dibuja la malla
		*/
		void render(GLContext& ctx);
		/**
		\return el número de vértices de la malla (cuidado! NO el número de
		 índices)
		*/
		size_t getNVertices() const { return n_vertices; };

		/**
		\return el número de coordenadas por vértice (2, 3 o 4)
		*/
		unsigned int getNVertexCoordinates() const { return n_components_per_vertex; }

		/**
		\return el número de índices de la malla
		*/
		size_t getNIndices()  const { return n_indices; };

		/**
		\return el tipo de vértices (GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT...)
		*/
		GLenum getIndicesType() const { return indices_type; };

		/**
		@brief Da acceso a los buffer objects que contienen la información de la malla
		@param which El buffer object deseado (VERTICES, NORMALS, etc)
		@return una referencia al buffer object
		@warning Si aún no se han creado los buffer objects, devolverá un puntero vacío
		*/
		std::shared_ptr<BufferObject> getBufferObject(BufferObjectType which) const;

		/**
		@brief Da acceso a los buffer objects que contienen la información de la malla
		@param attribute el índice del atributo solicitado
		@return una referencia al buffer object
		@warning Si aún no se han creado los buffer objects, devolverá un puntero vacío
		*/
		std::shared_ptr<BufferObject> getBufferObject(int attribute) const;

		/**
		\return una cadena con una descripción de las características de la malla
		*/
		const std::string to_string() const;
		/**
		Devuelve la posición de los vértices de la malla
		*/


		/**
		 * @brief Devuelve la posición de un rango de vértices de la malla
		 * @param offset [opcional] índice del primer vértice a copiar
		 * @param howmany [opcional] cuántos vértices a copiar
		 * @return
		\warning No abusar de estas funciones, puesto que tienen que traer la información
		desde la GPU, si no se ha activado el guardado en CPU con \sa Mesh::setKeepDataInCPU
		*/

		std::vector<glm::vec3> getVertices(size_t offset = 0, size_t howmany = std::numeric_limits<size_t>::max()) const;
		/**
		 * @brief Devuelve la normal de un rango de vértices de la malla
		 * @param offset [opcional] índice del primer vértice a copiar
		 * @param howmany [opcional] cuántos vértices a copiar
		 * @return
		\warning No abusar de estas funciones, puesto que tienen que traer la información
		desde la GPU, si no se ha activado el guardado en CPU con \sa Mesh::setKeepDataInCPU
		*/
		std::vector<glm::vec3> getNormals(size_t offset = 0, size_t howmany = std::numeric_limits<size_t>::max()) const;

		/**
		Devuelve las coordenadas de textura de los vértices de la malla
		\warning No abusar de estas funciones, puesto que tienen que traer la información
		desde la GPU
		*/
		std::vector<glm::vec2> getTexCoords(unsigned int texUnit = 0) const;

		/**
		Devuelve los índices de la malla
		@param skipIndices saltar este número de índices desde el inicio
		@param nIndicesToCopy número de índices a devolver (-1 para devolver hasta el final)
		\warning No abusar de estas funciones, puesto que tienen que traer la información
		desde la GPU
		*/
		std::vector<MeshIndex> getIndices(size_t skipIndices = 0, int64_t nIndicesToCopy = -1) const;

		size_t getVertexMemCapacity() const;
		size_t getIndicesMemCapacity() const;

		bool hasBOInGPU() const {
			return !boInGPU.empty();
		}

		void transform(const glm::mat4& transform);
		std::shared_ptr<Mesh> clone() const;

		static std::shared_ptr<Mesh> fromMeshData(const MeshData& m, bool keepDataInCPU);


		/**
		 * @brief Flips the order of the indices of a triangle
		 * @param firstIndexOffset the index inside of the indices array of the triangle's first vertex
		 * @param fixTheIllumNormals if true, if after flipping the triangle its vertices' normals points away
		 * the new geometric normal, flip them as well
		 */
		void flipTriangle(uint32_t firstIndexOffset, bool fixTheIllumNormals = true);

	protected:
		using BufferObjectCPUIndex = std::pair<int, std::unique_ptr<BOInCPU>>;
		std::vector<BufferObjectCPUIndex> boInCPU;

		using BufferObjectGPUIndex = std::pair<int, std::shared_ptr<BufferObject>>;
		std::vector <BufferObjectGPUIndex> boInGPU;

		GLenum indices_type;
		size_t n_indices, n_vertices;
		unsigned int n_components_per_vertex;
		struct StaticAttribute {
			StaticAttribute(GLint index, const glm::vec4& val) {
				attrIndex = index;
				value = val;
			}
			GLint attrIndex;
			glm::vec4 value;
		};

		float epsilonSquared; // para determinar si dos vértices son iguales
		bool sameVertex(float* vs, unsigned int ncomponents, unsigned int a, unsigned int b);
		std::vector<StaticAttribute> staticAttrValues;

		/**
		  Almacena o actualiza un valor para el atributo indicado.
		  \param index índice del atributo
		  \param val Valor del atributo (valor estático, que se usará siempre que no
		  haya un array de atributos activo)
		  */
		void addStaticAttributeValue(GLint index, const glm::vec4& val);

		void prepareRender(GLContext& ctx);
		bool releaseRAM = true, needsRebuild = false;
		void reconnectVAO(GLContext& ctx);

		struct EdgeInfo {
			autis::EdgeId id;
			uint32_t first, last;
			uint32_t selectableFirst, selectableLast;
			uint32_t numIndices() const { return last - first + 1; }
			uint32_t numSelectableVertices() const { return selectableLast - selectableFirst + 1; }

			void* offsetDrawIndices() const { return reinterpret_cast<void*>(first * sizeof(decltype(allEdgesIndices)::value_type)); }
			void* offsetSelectableIndices() const { return reinterpret_cast<void*>(selectableFirst * sizeof(decltype(allEdgesIndices)::value_type)); }
		};
		std::vector<GLuint> allEdgesIndices;
		std::vector<EdgeInfo> edges;
		std::shared_ptr<BufferObject> edgesIndicesBO;
		bool rebuildEdgeIndices = true;
		void buildEdgeIndicesBO();

		struct FaceInfo {
			autis::FaceId id;
			uint32_t first, last;
			uint32_t numIndices() const { return last - first + 1; }
			void* offsetIndices() const { return reinterpret_cast<void*>(first * sizeof(decltype(allFacesIndices)::value_type)); }
		};
		std::vector<GLuint> allFacesIndices;
		std::vector<FaceInfo> faces;
		std::shared_ptr<BufferObject> facesIndicesBO;
		bool rebuildFaceIndices = true;
		void buildFaceIndicesBO();

		friend class GLContext;

		std::mutex mutex;
	};
};

#endif
