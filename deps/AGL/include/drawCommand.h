#ifndef _DRAW_COMMAND_H
#define _DRAW_COMMAND_H

#include <GL/glew.h>
#include <vector>

/** \file drawCommand.h

\author Paco Abad

*/

namespace autis {
	/**
	\class DrawCommand

	Clase virtual que representa una orden de dibujado de OpenGL (glDrawArrays, glDrawElements, etc)
	Los comandos de verdad serán subclases que implementarán la función render.
	¡Cuidado! Si dibujas GL_PATCHES, recuerda establecer el número de vértices por patch con una llamada a
	setVerticesPerPatch.
	*/
	class DrawCommand {
	public:
		explicit DrawCommand(GLenum mode) : mode(mode) {};
		virtual ~DrawCommand() = default;
		virtual void render() const = 0;
		virtual DrawCommand* clone() const = 0;
	protected:
		GLenum mode;
	};

	class DrawCommandWithRestart : public DrawCommand {
	public:
		explicit DrawCommandWithRestart(GLenum mode, uint32_t restartIndex = UINT32_MAX) : 
			DrawCommand(mode), theRestartIndex(restartIndex) {};
		~DrawCommandWithRestart() override = default;
		void render() const override;
		virtual void renderFunc() const = 0;
	protected:
		GLuint theRestartIndex;
	};

	/**
	\class DrawArrays

	Clase envoltorio de glDrawArrays
	*/
	class DrawArrays : public DrawCommand {
	public:
		/**
		  Necesita la información para invocar a glDrawArrays
		  \param mode: tipo de primitivas (GL_TRIANGLE_STRIP, GL_POINTS...)
		  \param first: índice del primer vértice a dibujar
		  \param count: cuántos vértices dibujar
		  */
		DrawArrays(GLenum mode, GLint first, GLsizei count) :
			DrawCommand(mode), first(first), count(count) {};
		void render() const override {
			glDrawArrays(mode, first, count);
		}
		DrawCommand* clone() const override;
	private:
		GLint first; GLsizei count;
	};

	/**
	\class DrawElements

	Clase envoltorio de glDrawElements
	*/
	class DrawElements : public DrawCommand {
	public:
		/**
		  Necesita la información para invocar a glDrawElements
		  \param mode: tipo de primitivas (GL_TRIANGLE_STRIP, GL_POINTS...)
		  \param count: número de índices a dibujar
		  \param type: tipo de los índices (únicamente se permiten GL_UNSIGNED_BYTE,
		  GL_UNSIGNED_SHORT y GL_UNSIGNED_INT)
		  \param offset: posición (en bytes desde el comienzo del buffer vinculado
		  a GL_ELEMENT_ARRAY_BUFFER) del primer índice a dibujar
		  */
		DrawElements(GLenum mode, GLsizei count, GLenum type, const void *offset) :
			DrawCommand(mode), count(count), type(type), offset(offset) {};
		void render() const override {
			glDrawElements(mode, count, type, offset);
		}
		// Mainly for testing
		GLsizei getIndexCount() const { return count; }
		GLenum getIndexType() const { return type; }
		DrawCommand* clone() const override;
	private:
		GLsizei count; GLenum type; const void *offset;
	};


	/**
	\class DrawElements

	Clase envoltorio de glDrawElements
	*/
	class DrawElementsWithRestart : public DrawCommandWithRestart {
	public:
		/**
		  Necesita la información para invocar a glDrawElements
		  \param mode: tipo de primitivas (GL_TRIANGLE_STRIP, GL_POINTS...)
		  \param count: número de índices a dibujar
		  \param type: tipo de los índices (únicamente se permiten GL_UNSIGNED_BYTE,
		  GL_UNSIGNED_SHORT y GL_UNSIGNED_INT)
		  \param offset: posición (en bytes desde el comienzo del buffer vinculado
		  a GL_ELEMENT_ARRAY_BUFFER) del primer índice a dibujar
		  \param restartIndex: índice que indica el punto donde empezar una nueva primitiva
		  */
		DrawElementsWithRestart(GLenum mode, GLsizei count, GLenum type, const void *offset, uint32_t restartIndex) :
			DrawCommandWithRestart(mode, restartIndex), count(count), type(type), offset(offset) {};
		void renderFunc() const override {
			glDrawElements(mode, count, type, offset);
		}
		// Mainly for testing
		GLsizei getIndexCount() const { return count; }
		GLenum getIndexType() const { return type; }
		DrawCommand* clone() const override;
	private:
		GLsizei count; GLenum type; const void *offset;
	};

	/**
	\class MultiDrawArrays

	Clase envoltorio de glMultiDrawArrays

	*/
	class MultiDrawArrays : public DrawCommand {
	public:
		/**
		  Necesita la información para invocar a glMultiDrawArrays
		  \param mode: tipo de primitivas (GL_TRIANGLE_STRIP, GL_POINTS...)
		  \param first: array con primcount elementos, donde cada uno indica el primer vértice
		  a dibujar
		  \param count: array con primcount elementos, donde cada uno indica cuántos vértices utilizar
		  \param primcount: número de primitivas a dibujar

		  \warning Este objeto hace una copia de los arrays pasados. Puedes liberar los originales
		  */
		MultiDrawArrays::MultiDrawArrays(GLenum mode, const GLint* first, const GLint* count, GLsizei primcount) :
			DrawCommand(mode), primcount(primcount), first(primcount), count(primcount) {
			memcpy(&this->first[0], first, sizeof(GLint) * primcount);
			memcpy(&this->count[0], count, sizeof(GLint) * primcount);
		}
		void render() const override {
			glMultiDrawArrays(mode, &first[0], &count[0], primcount);
		}
		DrawCommand* clone() const override;
	private:
		GLsizei primcount;
		std::vector<GLint> first;
		std::vector<GLsizei> count;
	};
};
#endif
