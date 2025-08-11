#pragma once
// 2016

#include <memory>
#include <glm/vec3.hpp>
#include <boundingVolumes.h>

namespace autis
{
	class GLContext;
	/*
	Clase abstracta que define las funciones que debe implementar cualquier clase
	que se pueda dibujar.
	*/

	class Renderable {
	public:
		virtual ~Renderable();
		// Función encargada de dibujar. 
		virtual void render(GLContext& ctx) const = 0;
		// Devuelve la caja de inclusión alineada a los ejes
		virtual BoundingBox getBB() = 0;
		// Devuelve la dimensión máxima en X, Y, Z
		float maxDimension();
		// Devuelve la posición del centro de la caja de inclusión
		glm::vec3 center();
	};
}