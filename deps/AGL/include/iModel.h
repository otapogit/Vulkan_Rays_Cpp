#ifndef _I_MODEL_H
#define _I_MODEL_H

#include <functional>
#include <map>
#include <memory>
#include <ostream>
#include <vector>

#include "renderable.h"
#include <meshCommon.h>

namespace autis
{
	class GLContext;
	class Mesh;
	struct MeshesData;

	/* Un objeto Model almacena uno o varios punteros a objetos de tipo Mesh.
	Normalmente contendrá un modelo sencillo, que puede estar compuesto por
	diferentes partes (cada parte en un Mesh), pero que comparten el mismo
	sistema de coordenadas */
	class IModel : public Renderable {
	public:
		// Dibuja el modelo
		virtual void render(GLContext& ctx) const override = 0;
		// Dibuja los cables del modelo
		virtual void renderEdges(GLContext& ctx) const = 0;
		// Dibuja los vértices de los cables que pueden seleccionar
		virtual void renderEdgeVertices(GLContext& ctx) const = 0;
		// Devuelve la caja de inclusión del modelo
		virtual BoundingBox getBB() override = 0;
		// Recalcula la caja de inclusión
		virtual void recomputeBB() = 0;
		// Añade una malla al modelo
		virtual void addMesh(std::shared_ptr<Mesh> mesh) = 0;
		// Devuelve el número de mallas 
		virtual uint32_t getNMeshes() const = 0;
		// Devuelve true si todas las mallas del modelo mantienen sus datos en CPU. Si no hay mallas devuelve false
		virtual bool keepsAllDataInCPU() const = 0;
		// Libera la memoria de las mallas del modelo
		virtual void clearMeshes() = 0;
		// Devuelve una referencia a la malla i-ésima
		virtual Mesh& getMesh(const autis::MeshId& i) const = 0;
		// Devuelve un puntero a la malla i-ésima
		virtual std::shared_ptr<Mesh> getMeshPtr(size_t i) const = 0;
		/**
		Función de conveniencia para procesar todas las mallas del modelo (en modo de sólo lectura)
		\param op se invocará a la función op en cada una de las mallas del modelo
		*/
		virtual void accept(std::function<void(const Mesh&)> op) const = 0;
		/**
		Función de conveniencia para procesar todas las mallas del modelo (con posibilidad de modificar las mallas)
		\param op se invocará a la función op en cada una de las mallas del modelo
		*/
		virtual void accept(std::function<void(Mesh&)> op) = 0;
		/**
		\return una cadena con información sobre el modelo
		*/
		virtual const std::string to_string() const = 0;
		/**
		* Función que transforma todos los vértices y normales de las mallas dl modelo conforme a la matriz proporcionada
		*/
		virtual void transform(const glm::mat4& transform) = 0;
		/**
		* Returns a shared_ptr pointing to a deep copy of the model and its meshes
		*/
		virtual std::shared_ptr<IModel> clone() const = 0;
		/**
		Las especializaciones de esta (pseudo)interfaz pueden ser proxies de otros modelos
		*/
		virtual bool isProxy() const { return false; }
	};

};

#endif
