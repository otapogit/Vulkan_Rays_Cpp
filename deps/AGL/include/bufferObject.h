
#ifndef _BUFFER_OBJECT_H
#define _BUFFER_OBJECT_H

/** \file bufferObject.h

\author Paco Abad

*/

#include <memory>
#include <string>
#include <GL/glew.h>

namespace autis {

  /**
  \class BufferObject

  Clase que representa un buffer object en una implementación OpenGL.

  */
  class BufferObject {
  public:
    /**
    Factoría de BufferObjects. Necesita el tamaño en bytes del B.O.
    Se le puede pasar el tipo de uso que se le va a dar al buffer (por defecto
    será GL_DYNAMIC_DRAW).
    \param size Tamaño del buffer a reservar (en bytes)
    \param data un puntero a los datos a copiar (puede ser nullptr)
    \param usage El uso que se le va a dar al buffer (GL_STATIC_DRAW para información estática que se
    usará para dibujar, GL_DYNAMIC_DRAW si dicha información cambiará frecuentemente, etc).
    */
    static std::shared_ptr<BufferObject> build(size_t size, void *data = nullptr, GLenum usage = GL_DYNAMIC_DRAW);

    /**
    Factoría de BufferObjects. Necesita el tamaño en bytes del B.O.
    Se le puede pasar el tipo de uso que se le va a dar al buffer (por defecto
    será GL_DYNAMIC_DRAW). Construye un B.O. inmutable (reservado con glBufferStorage)
    \param size Tamaño del buffer a reservar (en bytes)
    \param usage El uso que se le va a dar al buffer (http://docs.gl/gl4/glBufferStorage).
    \warning Necesita OpenGL 4.4
    */
    static std::shared_ptr<BufferObject> buildImmutable(uint32_t size, void* data = nullptr, GLenum usage = GL_DYNAMIC_STORAGE_BIT);

    /** Devuelve el identificador del B.O. */
    GLuint getId() const  { return id; };
    /** Devuelve el tamaño que se pidió al crear el buffer object */
    size_t getSize() const { return size; };
    /**
    Establece una etiqueta que se mostrará en los mensajes de depuración de GL (GL 4.3)
    \param label Etiqueta a mostrar para este buffer
    */
    void setGlDebugLabel(const std::string &label);
    /**
    \return La etiqueta que muestra OpenGL en los mensajes de depuración de este objeto
    */

    std::string getGlDebugLabel() const { return debugLabel; };

	/**
	\return el tipo de uso que se declaró que se le iba a dar al buffer
	*/
	GLenum getDeclaredUsage() const { return usage; };

    /**
     * @brief Escritura directa al buffer object, sin necesidad de vincularlo a un punto de vinculación
     * @param src puntero a los datos a copiar
     * @param sizeInBytes tamaño en bytes de los datos a copiar
     * @param offset desplazamiento dentro del buffer de destino donde empezar a copiar
     * \warning Necesita OpenGL 4.5
    */
    void write(const void* src, size_t sizeInBytes, uint64_t offset = 0);

    /**
     * @brief Lectura directa desde el buffer object, sin necesidad de vincularlo a un punto de vinculación
     * @param dst puntero al buffer donde copiar los datos
     * @param sizeInBytes tamaño en bytes de los datos a copiar
     * @param offset desplazamiento dentro del buffer de origen donde empezar a copiar
     * \warning Necesita OpenGL 4.5
    */
    void read(void* dst, size_t sizeInBytes, uint64_t offset = 0);
    /**
     * @brief Maps the context of the buffer object to user-land memory, for read-only
     * @tparam C the pointer type (it should match the base type of the contents of the buffer)
     * @return a CPU pointer to the contents of the buffer.
    */
    template <typename C> 
    const C* mapForRead() {
        const C* vb = static_cast<const C*>(glMapNamedBuffer(id, GL_READ_ONLY));
        return vb;
    }

    /**
     * @brief Maps the context of the buffer object to user-land memory, for write-only
     * @tparam C the pointer type (it should match the base type of the contents of the buffer)
     * @return a CPU pointer where to write the contents of the buffer.
    */
    template <typename C>
    C* mapForWrite() {
        C* vb = static_cast<C*>(glMapNamedBuffer(id, GL_WRITE_ONLY));
        return vb;
    }

    /**
     * @brief Maps the context of the buffer object to user-land memory, for read and write
     * @tparam C the pointer type (it should match the base type of the contents of the buffer)
     * @return a CPU pointer where to write to and read from the contents of the buffer.
    */
    template <typename C>
    C* mapForRW() {
        C* vb = static_cast<C*>(glMapNamedBuffer(id, GL_READ_WRITE));
        return vb;
    }

    /**
     * @brief Unmaps the currently mapped pointer. It should be called after one of the mapFor* 
     * methods, when we are done with the pointer
    */
    void unMap();

    /**
     * @brief Establece metadatos para posteriormente configurar el VBO (llamada opcional)
     * @param type tipo base de los elementos del BO
     * @param nComponents número de componentes por elemento (1-4)
     * @param nElems número de elementos
     * @param normalize si el tipo base es entero y normalize == true, normalizar entre 0 y 1
    */
    void setMetadata(GLenum type, uint32_t nComponents, size_t nElems, bool normalize);
    
    /**
     * @brief 
     * @return tipo base de los datos
     * @warning Devuelve lo especificado por BufferObject::setMetadata
    */
    GLenum type() const { return mType; }
    /**
     * @brief
     * @return número de componentes (de tipo type()) por elemento
     * @warning Devuelve lo especificado por BufferObject::setMetadata
    */
    uint32_t numComponents() const { return mNumComponents; }
    /**
     * @brief
     * @return el número de elementos en el buffer
     * @warning Devuelve lo especificado por BufferObject::setMetadata
    */
    size_t count() const { return mCount; }
    /**
     * @brief
     * @return si normalizar los datos o no
     * @warning Devuelve lo especificado por BufferObject::setMetadata
    */
    bool normalize() const { return mNormalize; }

    virtual ~BufferObject();
  protected:
    /** Constructor.
     */
    BufferObject(size_t size, GLenum usage);
    static void allocate(std::shared_ptr<BufferObject> bo, const void *data = nullptr);
    static void allocateImmutable(std::shared_ptr<BufferObject> bo, const void* data = nullptr);
    // No se permite copiar el objeto
    BufferObject(const BufferObject &other) = delete;
    BufferObject& operator=(BufferObject other) = delete;

    std::string getName();
    GLuint id;
    size_t size;
    GLenum usage;
    std::string debugLabel;
    GLenum mType;
	uint32_t mNumComponents;
    size_t mCount;
    bool mNormalize;
  };

};

#endif
