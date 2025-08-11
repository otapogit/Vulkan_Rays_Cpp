
#ifndef _TEXTURE_H
#define _TEXTURE_H 2011

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>

namespace autis {

	/* Esta clase representa la parte común de todos los tipos de texturas
	en OpenGL. Permite cargar establecer los parámetros para su uso.
	*/
	class Texture {
	public:
		// Constructor
		Texture(GLenum texture_type, GLenum minfilter, GLenum magfilter,
			GLenum wrap_s, GLenum wrap_t = GL_REPEAT, GLenum wrap_r = GL_REPEAT,
			const glm::vec4& bordercolor = glm::vec4(0.0, 0.0, 0.0, 0.0));

		/**
		 * @brief Creates a non-owning texture
		*/
		Texture(GLuint id, GLenum texture_type, GLenum minfilter, GLenum magfilter,
			GLenum wrap_s, GLenum wrap_t, GLenum internalFormat);

		explicit Texture(Texture&& other) = default;

		virtual ~Texture();

		// Devuelve el identificador del objeto textura (devuelto por glGenTextures)
		GLuint getId() const { return _texId; };
		// Devuelve el tipo de textura (GL_TEXTURE_1D, GL_RECTANGLE_TEXTURE, etc)
		GLenum getTextureType() const { return _texture_type; };


		// Establece los filtros de minimización/maximización
		void setMinFilter(GLenum filter);
		void setMagFilter(GLenum filter);
		// Establece los modos de repetición
		void setWrapS(GLenum wrap);
		void setWrapT(GLenum wrap);
		void setWrapR(GLenum wrap);
		/**
		 Establece el modo de comparación de la textura (GL_TEXTURE_COMPARE_MODE)
		 \param compareMode Modo de comparación (GL_NONE o GL_COMPARE_REF_TO_TEXTURE)
		 */
		void setCompareMode(GLenum compareMode);
		/**
		 Establece el operador de comparación de la textura (GL_TEXTURE_COMPARE_FUNC)
		\param compareFunc El operador a utilizar (GL_LEQUAL, GL_GEQUAL, etc.)
		 */
		void setCompareFunc(GLenum compareFunc);
		/**
		Establece el color del borde de la textura que se usa en el modo
		GL_CLAMP_TO_BORDER
		\param color color del borde
		*/
		void setBorderColor(const glm::vec4& color);

		/**
		Pedir a OpenGL que genere los mipmaps de la textura (glGenerateMipmaps)
		*/

		void generateMipmap();

		// Funciones de consulta.
		GLenum getMinFilter() const { return _minfilter; };
		GLenum getMagFilter() const { return _magfilter; };
		GLenum getWrapS() const { return _wrap_s; };
		GLenum getWrapT() const { return _wrap_t; };
		GLenum getWrapR() const { return _wrap_r; };
		glm::vec4 getBorderColor() const { return _bordercolor; };

		/**
		 Devuelve el modo de comparación de la textura (GL_TEXTURE_COMPARE_MODE)
		 \returns GL_NONE o GL_COMPARE_REF_TO_TEXTURE
		 */
		GLenum getCompareMode(void) const { return _compareMode; };
		/**
		 Devuelve el operador de comparación de la textura (GL_TEXTURE_COMPARE_FUNC)
		 \returns GL_LEQUAL, GL_EQUAL...
		 */
		GLenum getCompareFunc(void) const { return _compareFunc; };

		GLenum getInternalFormat() const { return _internalFormat; }

		// Devuelve la unidad de textura utilizada internamente por la librería (es la
	  // última unidad de textura disponible en el sistema. Esta unidad está reservada
	  // y no la deberías utilizar)
	  // \return Número de unidad (desde 0 a GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS-1)
		static GLint getScratchUnitTextureNumber();

        /**
         * @brief Gets the contents of the texture image
         * @param dst Buffer where to write the pixels
         * @param dstSize Size of the buffer, in bytes
         * @param level The level of the texture to obtain
         * @return false, in case of error
        */
        bool read(void* dst, uint32_t dstSize, int32_t level = 0);

		/**
		 * @brief Gets the contents of the texture image, with the format specified
		 * @param dst Buffer where to write the pixels
		 * @param dstSize Size of the buffer, in bytes
		 * @param level The level of the texture to obtain
		 * @param format The pixel format for the returned data (GL_RGBA, GL_STENCIL_INDEX, GL_RED...)
		 * @param type the pixel type (GL_UNSIGNED_BYTE, GL_FLOAT, GL_INT...)
		 * @return false, in case of error
		 * @see https://docs.gl/gl4/glGetTexImage
		 * 
		*/
		bool read(void* dst, uint32_t dstSize, GLenum format, GLenum type, int32_t level = 0);

        /**
        Borra el nivel de la textura al valor indicado (o cero si se pasa NULL).
        */
        void clear(int level = 0, void* data = nullptr);

		/**
		Borra toda la textura al valor indicado.
		\warning Disponible a partir de GL 4.4. Usa glClearTexImage
		\param level Nivel a borrar
		\param format Formato de los datos apuntados por data (GL_RGBA, GL_R...)
		\param type Tipo de los elementos apuntods por data (GL_FLOAT, GL_INT)...
		\param data puntero al elemento que se usará para borrar la textura. Si es NULL, inicializa a cero
		*/
		void clear(int level, GLenum format, GLenum type, const void* data);

		//! Devuelve el nombre de la textura
		const std::string getName() const { return _name; }
		// Establece el nombre de la textura
		void setName(const std::string& name) { _name = name; }

		/**
		Establece una etiqueta que se mostrará en los mensajes de depuración de GL (GL 4.3)
		\param label Etiqueta a mostrar para esta textura
		*/
		void setGlDebugLabel(const std::string& label);
		/**
		\return La etiqueta que muestra OpenGL en los mensajes de depuración de este objeto
		*/
		std::string getGlDebugLabel() const { return debugLabel; };


	protected:
		void setTexParam(GLenum pname, GLint value);
		void setTexParam(GLenum pname, const GLfloat* value);

		GLenum _texture_type;
		GLuint _texId;
		GLenum _minfilter, _magfilter, _wrap_s, _wrap_t, _wrap_r, _compareMode,
			_compareFunc, _internalFormat;
		glm::vec4 _bordercolor;
		bool _ready;
		std::string _name;
		bool _owning{ true };
		std::string debugLabel;

        void findBaseFormatAndBaseTypeFromInternalFormat(GLenum internalformat);
        GLenum baseFormat, baseType;
	};
};

#endif
