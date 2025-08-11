
#ifndef _VERTEXARRAYOBJECT_H
#define _VERTEXARRAYOBJECT_H 2013

/** \file bufferObject.h

\author Paco Abad

*/

#include <GL/glew.h>

namespace autis {

/**
\class VertexArrayObject 

Clase que representa un vertex array object (VAO) en una implementación OpenGL.

Un VAO almacena la configuración de los atributos de un modelo (qué atributo está conectado
a qué buffer object, qué atributos tienen valores estáticos y qué valores, etc).
Normalmente habrá un VAO por malla.

*/

class VertexArrayObject {
public:
	VertexArrayObject();
	~VertexArrayObject();
	VertexArrayObject(VertexArrayObject&& other) noexcept;
	VertexArrayObject& operator=(VertexArrayObject&& other) noexcept;
	void bind() const;
	void unbind() const;
	GLuint getId() const { return vao; }
	void destroy();
private:
	// Prohibir la copia
	VertexArrayObject(const VertexArrayObject &) = delete;
	VertexArrayObject& operator=(VertexArrayObject) = delete;
	GLuint vao;
};
};

#endif