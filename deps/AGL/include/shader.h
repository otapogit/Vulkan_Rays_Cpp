#ifndef _SHADER_H 
#define _SHADER_H 2014

// STL
#include <map>
#include <memory>
#include <string>
#include <vector>
// dependencies (glmw)
#include <GL/glew.h>

namespace autis {

  /**
  \class Shader

  Esta clase representa un Shader, almacenado normalmente en un fichero. Este shader será de
  alguno de los tipos conocidos (vértice, fragmento, geometría, etc).
  Esta clase trabaja conjuntamente con la clase PGUPV::Program

  */
  class Shader {
  public:
    ~Shader();

    /**
    Tipos de shaders reconocidos por la librería
    Para añadir nuevos tipos de shaders, mira en shader.cpp
    */
    enum class ShaderType {
      CHECK_EXTENSION = -1, VERTEX_SHADER = 0, FRAGMENT_SHADER, GEOMETRY_SHADER,
      TESS_CONTROL_SHADER, TESS_EVALUATION_SHADER
#ifndef __APPLE__
        , COMPUTE_SHADER
#endif
        , NUM_SHADER_TYPES
    };

   /**
    Cargar un shader desde memoria
    Si no se puede cargar, lanza una excepción
    \param ssrc El código fuente del shader
    \param shader_type Tipo del shader que se está cargando
    */
    static std::shared_ptr<Shader> loadFromMemory(const std::vector<std::string> &ssrc, ShaderType shader_type);
    /**
    \return el tipo del shader
    */
    ShaderType getType() const { return type; };

    /**
    Devuelve la extensión de fichero por defecto del tipo de shader indicado.
    \return Una cadena con la extensión (por ejemplo, ".vert")
    */
    static std::string getDefaultShaderExtension(ShaderType type);

    void applyTranslation(const std::map<std::string, std::vector<std::string>>& transTable);

    /**
     Imprime el código fuente del shader. Para imprimir todos los shaders cargados, ver el método \sa{Program::printSrcs}
     \param os Flujo de salida donde escribir el código fuente del shader
     \param printHeader Si true, imprime el nombre del fichero y el tipo de shader
     \param printLineNumber si true, precede cada línea del código fuente con su número de línea
     */
    void printSrc(std::ostream &os, bool printHeader = true, bool printLineNumber = true) const;
    /**
      Compila el código fuente del shader proporcionado (si es necesario). Si se producen errores de compilación
      se mandan al Log.
      \return true, si la compilación ha tenido éxito
      */
    bool compile();
    
	void attach(GLint programId);
	void detach(GLint programId);

    /**
    Si el shader está compilado, elimina su código (el shader se puede volver a recompilar para recrearlo)
    */
    void deleteShaderObject();
    /**
    Traduce desde el tipo ShaderType a la constante correspondiente de GL. Por ejemplo:
    toGLType(Shader::VERTEX_SHADER) -> GL_VERTEX_SHADER
    */
    static GLint toGLType(ShaderType type);

    /**
    Traduce desde el tipo de ShaderType a un nombre entendible. Por ejemplo:
    toFriendlyName(Shader::VERTEX_SHADER) -> "Vertex shader"
    */
    static std::string toFriendlyName(ShaderType type);

    std::vector<std::string> preprocessShader(const std::vector<std::string>& src);
  private:
    Shader();	// Usar las funciones factoría para crear un shader
    void processInclude(const std::string& line, std::vector<std::string>& dst);
	std::vector<std::string> src; // Código fuente
    ShaderType type; // Tipo de shader
    GLuint shaderId;
  };
};
#endif
