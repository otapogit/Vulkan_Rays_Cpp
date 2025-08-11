
#ifndef _TEXTURE2D_H
#define _TEXTURE2D_H 2014

#include "texture.h"
#include <filesystem>

namespace autis {

  class Image;

  /*
  \class Texture2D
  Esta clase representa un objeto textura 2D. Permite cargar imágenes desde
  fichero y establecer los parámetros para su uso.
  */
  class Texture2D : public Texture {
  public:
    /**
     Constructor por defecto con unos parámetros iniciales (NO corresponden con los
     valores por defecto de OpenGL)
     */
    explicit Texture2D(GLenum minfilter = GL_LINEAR,
      GLenum magfilter = GL_LINEAR, GLenum wrap_s = GL_REPEAT,
      GLenum wrap_t = GL_REPEAT);

    Texture2D(GLenum texture_type, GLenum minfilter, GLenum magfilter, GLenum wrap_s,
      GLenum wrap_t);

    /**
     * @brief Creates a non-owning texture
     * @param id 
     * @param texture_type 
     * @param minfilter 
     * @param magfilter 
     * @param wrap_s 
     * @param wrap_t 
    */
    Texture2D(GLint id, GLenum texture_type, GLenum minfilter, GLenum magfilter, GLenum wrap_s,
      GLenum wrap_t, GLenum internalFormat, uint32_t width, uint32_t height);

    virtual ~Texture2D(){};
    /**
     Función para cargar en el objeto textura una imagen desde fichero:

     \param filename nombre y ruta (absoluta o relativa) del fichero a cargar
	 \param internalFormat (opcional) establece el formato de los píxeles (GL_RGB, GL_RGBA...)
     \return true en caso de haber podido cargar la imagen.
     */
    virtual bool loadImage(const std::filesystem::path &filename, GLenum internalFormat = GL_RGB);

    /**
     Función para cargar el objeto Image al objeto textura.
     \param image Imagen a cargar
     \return true si la carga ha tenido éxito
     */
    virtual bool loadImage(const Image &image);
	void updateImage(const Image &image);

	/**
	Función para cargar el objeto Image al objeto textura.
	\param image Imagen a cargar
	\param internalFormat (opcional) establece el formato de los píxeles (GL_RGB, GL_RGBA...)
	\return true si la carga ha tenido éxito
	*/
	virtual bool loadImage(const Image &image, GLenum internalFormat);

    /**
     Carga al objeto textura desde la zona de memoria indicada una imagen del
     tamaño indicado.

     \param pixels Puntero al buffer que contiene la imagen
     \param width Ancho de la imagen, en píxeles
     \param height Alto de la imagen, en píxeles
     \param pixels_format Contenido del buffer (GL_RGBA, GL_RED,
     GL_DEPTH_STENCIL...) Ver Table 8.3 de la especificación de OpenGL 4.4
     \param pixels_type Tipo de datos de los elementos del buffer
     (GL_UNSIGNED_BYTE, GL_FLOAT...) Ver Table 8.2 de la especificación de OpenGL
     4.4
     \param internalformat Formato interno de la textura (GL_RGBA, GL_DEPTH_COMPONENT,
     GL_DEPTH_STENCIL)
     */
    void loadImageFromMemory(const void *pixels, uint32_t width, uint32_t height,
      GLenum pixels_format,
      GLenum pixels_type,
      GLenum internalformat);

	void updateImageFromMemory(const void *pixels, uint32_t width, uint32_t height, GLenum pixels_format,
		GLenum pixels_type);
    /**
     Reserva memoria asociada a la textura para almacenar una imagen del tamaño y
     formato especificado.
     \param width Ancho de la imagen, en píxeles
     \param height Alto de la imagen, en píxeles
     \param internalformat Formato interno de la image, con tamaño (GL_RGBA8, GL_RG16F
     GL_DEPTH_COMPONENT16, GL_DEPTH24_STENCIL8...)
     */
    void allocate(uint32_t width, uint32_t height, GLenum internalformat);

	/**
	Guarda el contenido de la textura en el fichero indicado. Se puede almacenar en los 
	formatos habituales.
	*/
	void save(const std::filesystem::path &filename, unsigned int bpp = 24, bool upsideDown = false);
    void saveHDR(const std::filesystem::path& filename);
	static void save(GLuint texId, const std::string &filename, unsigned int bpp = 24);

    uint32_t getWidth() const { return _width; };
    uint32_t getHeight() const { return _height; };

	static void save(const std::filesystem::path &filename, GLuint texId, uint32_t width, uint32_t height, unsigned int bpp, bool upsideDown = false);
    static void saveHDR(const std::filesystem::path& filename, GLuint texId);

    /**
    Guarda una porción del contenido de la textura en el buffer en memoria indicado
    \warning Necesita OpenGL 4.5
    \return true si se ha podido recuperar la zona pedida, false en otro caso
    */
    bool getPixels(uint32_t xoffset, uint32_t yoffset, uint32_t width,
        uint32_t height, uint32_t bufferSize, void* buffer) const;

    /**
    Borra una parte de la textura al valor indicado.
    \warning Disponible a partir de GL 4.4.Usa glClearTexSubImage
    \param level Nivel a borrar
    \param format Formato de los datos apuntados por data(GL_RGBA, GL_R...)
    \param type Tipo de los elementos apuntods por data(GL_FLOAT, GL_INT)...
    \param xoffset coordenada x de la esquina inferior izquierda del bloque a borrar
    \param yoffset coordenada y de la esquina inferior izquierda del bloque a borrar
    \param width ancho del bloque a borrar
    \param height alto del bloque a borrar
    \param data puntero al elemento que se usar� para borrar la textura.Si es NULL, inicializa a cero
    */
    void clear(int level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height);

    /**
    Borra una parte de la textura al valor indicado.
    \warning Disponible a partir de GL 4.4.Usa glClearTexSubImage
    \param level Nivel a borrar
    \param format Formato de los datos apuntados por data(GL_RGBA, GL_R...)
    \param type Tipo de los elementos apuntods por data(GL_FLOAT, GL_INT)...
    \param xoffset coordenada x de la esquina inferior izquierda del bloque a borrar
    \param yoffset coordenada y de la esquina inferior izquierda del bloque a borrar
    \param width ancho del bloque a borrar
    \param height alto del bloque a borrar
    \param data puntero al elemento que se usará para borrar la textura.Si es NULL, inicializa a cero
    */
    void clear(int level, GLenum format, GLenum type, GLint xoffset, GLint yoffset,
        GLsizei width, GLsizei height, const void* data = nullptr);

    using Texture::clear;
    using Texture::read;

  protected:
    void setParams();
    uint32_t _width, _height;
  };

};

#endif
