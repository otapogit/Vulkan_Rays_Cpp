#pragma once

/** \file stockModels.h

\author Paco Abad

*/

#include "model.h"
#include "group.h"
#include <glm/vec4.hpp>

namespace autis {

	// Color por defecto para todos los modelos a los que se les puede asignar un
	// color
	#define DEFAULT_COLOR glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)

	/**
	  \class Axes
	  Ejes coordenados, con los siguientes colores: X -> rojo, Y -> verde, Z -> azul
	  */
	class Axes : public Model {
	public:
		/**
		Constructor
		\param length longitud de los ejes
		*/
		explicit Axes(float length = 1.0f);
	};

	/**
	\class WireBox
	Caja dibujada en alámbrico (sólo se dibujan sus aristas)
	*/
	class WireBox : public Model {
	public:
		/**
		Constructor
		\param size Longitud del lado del cubo
		\param color Color del cubo
		*/
		explicit WireBox(float size = 1.0f, const glm::vec4 &color = DEFAULT_COLOR);
		/**
		Constructor
		\param xsize Ancho del cubo
		\param ysize Alto del cubo
		\param zsize Profundidad del cubo
		\param color Color
		*/
		WireBox(float xsize, float ysize, float zsize,
			const glm::vec4 &color = DEFAULT_COLOR);
		/**
		Constructor
		\param bbox Caja de inclusión
		\param color color de la caja (opcional)
		*/
		WireBox(const BoundingBox &bbox, const glm::vec4 &color = DEFAULT_COLOR);
	};

	/**
	   \class Cylinder

	   Cilindro centrado en el origen y paralelo al eje Y. NO define las 'tapas'.
	   Define posiciones, normales, coordenadas de textura y un color
	   Se puede especificar el número de rodajas a lo largo del eje Y (stacks)
	   y el número de sectores en el plano XZ

	   \warning Para definir un cono, usa un radio muy pequeño (en vez de 0.0), para
	   que no se generen polígonos degenerados
	   */

	class Cylinder : public Model {
	public:
		Cylinder(float r_bottom = 1.0f / 6.2832f, float r_top = 1.0f / 6.2832f,
			float height = 1.0f, uint32_t stacks = 10, uint32_t slices = 20,
			const glm::vec4 &color = DEFAULT_COLOR);
	};

	/**
	   \class WireCylinder

	   Cilindro centrado en el origen y paralelo al eje Y. NO define las 'tapas'.
	   Define posiciones, normales, coordenadas de textura y un color
	   Se puede especificar el número de rodajas a lo largo del eje Y (stacks)
	   y el número de sectores en el plano XZ.
	   Muestra tan solo los cables.

	   \warning Para definir un cono, usa un radio muy pequeño (en vez de 0.0), para
	   que no se generen polígonos degenerados
	*/

	class WireCylinder : public Model {
	public:
		WireCylinder(float r_bottom = 1.0f / 6.2832f, float r_top = 1.0f / 6.2832f,
			float height = 1.0f, uint32_t stacks = 10, uint32_t slices = 20,
			const glm::vec4& color = DEFAULT_COLOR);
	};

	/**
	 \class Disk
	Disco definido en el plano XY centrado en el origen. El radio interior define
	el tamaño del agujero (si es cero, entonces el objeto es un círculo)
	*/
	class Disk : public Model {
	public:
		/**
		 Construye un disco con los parámetros indicados
		 \param r_in Radio interior
		 \param r_out Radio exterior
		 \param slices Sectores que componen el disco
		 \param rings Anillos a lo largo del anillo
		 \param color Color
		 */
		Disk(float r_in = 0.0f, float r_out = 1.0f, uint32_t slices = 20, uint32_t rings = 4,
			const glm::vec4 &color = DEFAULT_COLOR);
	};

	class MultipleDisk : public Model {
	public:
		/**
		 Construye una esfera de discos
		 \param radius Radio del disco
		 \param slices Sectores que componen el disco
		 \param colorX Color en el eje X
		 \param colorY Color en el eje Y
		 \param colorZ Color en el eje Z
		 */
		MultipleDisk(float radius, const GLushort slices, const glm::vec4& colorX = DEFAULT_COLOR,
			const glm::vec4& colorY = DEFAULT_COLOR, const glm::vec4& colorZ = DEFAULT_COLOR);
	};


	/**
	\class CanonicalScreenPolygon

	Quad en el plano Z=0 con los vértices en el rango -1, 1 (ocupa toda la pantalla con la matriz de proyección identidad)
	*/
	class CanonicalScreenPolygon : public Model {
	public:
		CanonicalScreenPolygon();
	};

};

