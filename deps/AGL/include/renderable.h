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
		// Funci�n encargada de dibujar. 
		virtual void render(GLContext& ctx) const = 0;
		// Devuelve la caja de inclusi�n alineada a los ejes
		virtual BoundingBox getBB() = 0;
		// Devuelve la dimensi�n m�xima en X, Y, Z
		float maxDimension();
		// Devuelve la posici�n del centro de la caja de inclusi�n
		glm::vec3 center();
	};
}