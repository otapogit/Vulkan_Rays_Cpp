#ifndef _FBO_H
#define _FBO_H 2013

#include <GL/glew.h>
#include <memory>
#include <vector>
#include <string>
#include <ostream>

namespace autis {
	class Texture;
	class GLContext;
	class RBO;

	class FBO {
	public:
		~FBO();

		/** Asocia la textura al attachment indicado (la textura debe ser compatible y
		 tener memoria asignada)
		 \param attachment Punto de vinculación de la textura (GL_COLOR_ATTACHMENT0,
		 GL_DEPTH_ATTACHMENT, etc.)
		 \param tx La textura a vincular
		 */
		void attach(GLuint attachment, GLuint txid);
		// Asocia el renderbuffer al attachment indicado (descartando el que hubiera
		// previamente)
		void attach(GLuint attachment, std::shared_ptr<RBO> rb);
		/** Asocia la textura al attachment indicado (la textura debe ser compatible y
		 tener memoria asignada)
		 \param attachment Punto de vinculación de la textura (GL_COLOR_ATTACHMENT0,
		 GL_DEPTH_ATTACHMENT, etc.)
		 \param tx La textura a vincular
		 */
		void attach(GLuint attachment, std::shared_ptr<Texture> tx);
		/**
		Crea un renderbuffer con las características indicadas y lo asocia al
		attachment indicado
		\param attachment Punto de vinculación del renderbuffer
		(GL_COLOR_ATTACHMENT0, GL_DEPTH_ATTACHMENT, etc.)
		\param width Ancho del renderbuffer
		\param height Alto del renderbuffer
		\param samples Número de muestras del renderbuffer. Por defecto, cero
		*/
		void createAndAttach(GLuint attachment, uint32_t width, uint32_t height, uint32_t samples = 0);
		/**
		 Devuelve la textura vinculada al attachment indicado, si la hay
		 */
		GLuint getAttachedTextureId(GLuint attachment) const;
		/**
		 Devuelve el RBO que está vinculando al attachment indicado, si lo hay
		 */
		std::shared_ptr<RBO> getAttachedRBO(GLuint attachment) const;
		/**
		 Devuelve la textura vinculada al attachment indicado, si la hay
		 */
		std::shared_ptr<Texture> getAttachedTexture(GLuint attachment) const;
		/**
		 Devuelve todas las texturas adjuntadas al los color attachments del framebuffer
		*/
		std::vector<std::shared_ptr<Texture>> getAllAttachedColorTextures() const;

		/**
		Devuelve si el FBO asociado al punto de anclaje está completo o no, y
		proporciona información en la cadena
		\param message parámetro de salida donde se escribirá la información asociada a
		la comprobación
		\return true si el FBO está completo
		 */
		bool isComplete(std::string *message = nullptr);
		/**
		 Devuelve el ID del FBO
		 */
		GLuint getId() { return _fboId; }

		/**
		Establece una etiqueta que se mostrará en los mensajes de depuración de GL (GL 4.3)
		\param label Etiqueta a mostrar para este FBO
		*/
		void setGlDebugLabel(const std::string& label);
		/**
		\return La etiqueta que muestra OpenGL en los mensajes de depuración de este objeto
		*/
		std::string getGlDebugLabel() const { return debugLabel; };

		/**
		 * @brief Establece los buffers donde se escribirán los distintos índices de color escritos por el
		 * shader de fragmento 
		 * @param drawbuffers un vector con los buffers de color asociados a este FBO donde 
		 * se escribirá cada índice de color.
		 * 
		 * Por ejemplo, si el shader de fragmento tiene:
		 * 
		 * layout(location = 1) out vec3 worldPosOut;
		 * 
		 * y hacemos:
		 * 
		 * fbo->setDrawBuffer({GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT3});
		 * 
		 * entonces lo que se escriba en la variable worldPosOut irá a parar al cuarto buffer de color.
		*/
		void setDrawBuffers(std::vector<uint32_t> drawbuffers);
	private:
		FBO();
		// Builds a non-owning FBO object from the given OpenGL FBO id
		FBO(GLuint id);

		// Prohibir la copia
		FBO(const FBO &other) = delete;
		FBO & operator=(const FBO &) = delete;

		GLuint _fboId;

		/*
		Ideally, only one of the following three structures should be in use per FBO
		*/

		template <typename C>
		struct Data {
			std::vector<C> colorAttachment;
			C depthAttachment{ C{} };
			C stencilAttachment{ C{} };
			C depthStencilAttachment{ C{} };
		};

		Data<GLuint> texIdData;
		Data<std::shared_ptr<RBO>> rboData;
		Data<std::shared_ptr<Texture>> texData;

		static GLint _maxColorAttachments;
		bool owning{ true };
		std::string debugLabel;

		friend class GLContext;
	};


	bool checkFBOCompleteness(GLint fboId, GLenum target, std::string* message);
};

#endif
