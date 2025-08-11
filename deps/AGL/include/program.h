#ifndef _PROGRAM_H
#define _PROGRAM_H 2011

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <GL/glew.h>

#include "shader.h"

namespace autis {

	class UniformBufferObject;
	class ShaderStorageBufferObject;
	class BufferObject;
	class GLContext;

	struct Attribute {
		unsigned int loc;
		std::string name;
	};

	/**
	\class Program

	Esta clase representa a un programa ejecutable en la GPU, que está compuesto por
	varios shaders.

	*/

	class Program {
	public:
		Program();
		~Program();

		/**
		\return true if the program has been compiled and is ready to use
		*/
		bool ready();
		/**
		Carga las cadenas que componen el programa
		\return El número de shaders creados.
		*/
		int loadStrings(
			const std::vector<std::string>& vertexShader,
			const std::vector<std::string>& fragmentShader = std::vector<std::string>(),
			const std::vector<std::string>& geometryShader = std::vector<std::string>(),
			const std::vector<std::string>& tessCtrlShader = std::vector<std::string>(),
			const std::vector<std::string>& tessEvalShader = std::vector<std::string>());

		/**
		Carga desde memoría un shader de computación
		*/
		void loadComputeStrings(const std::vector<std::string>& computeShader);

		/**
		Carga, compila, crea un programa, adjunta los shaders al programa y enlaza el
		programa.
		Antes se han debido declarar las posiciones de los atributos con
		\sa{Program::addAttributeLocation}
		\return true si el programa se ha creado correctamente
		*/
		bool compile();
		/**
		Libera todos los recursos asociados a este shader (memoria, shaders
		compilados, etc)
		*/
		void release();

		/**
		Indica la localización de los atributos de un shader.
		\warning Llamar antes de compilar el shader
		\param loc Posición del atributo (entre 0 y 15)
		\param name Nombre de la variable in del shader de vértice que recibirá el
		atributo
		*/
		void addAttributeLocation(unsigned int loc, const std::string& name);

		/**
	  Devuelve la localización del uniform indicado en el programa. Esta localización
	  sólo cambia al enlazar el programa
	  \param uniform Nombre del uniform cuya localización se desea obtener
	  \return Localización del uniform (para usarla en glUniform*)
	  */
		int getUniformLocation(const std::string& uniform);

		/**
		Devuelve información sobre el programa compilado
		\param pname Una de las constantes admitidas por glGetProgramiv
		(http://www.opengl.org/sdk/docs/man3/xhtml/glGetProgram.xml)
		\return Un entero con el valor pedido
		*/
		int getProgramInfo(GLenum pname) const;

		/**
		\return el identificador del programa
		*/
		GLuint getId() const { return programId; };

		/**
		Devuelve si hay un shader cargado del tipo indicado
		\param type uno de las constantes de tipo ShaderType
		*/
		bool hasShaderType(Shader::ShaderType type) const;

		/**
		Imprime el código de todos los shaders que componen el programa
		\param os el flujo de salida donde imprimir el código fuente
		*/
		void printSrcs(std::ostream& os) const;

		/**
		Añade el shader proporcionado al programa. El shader debe estar listo para su
		compilación
		(no se expandirán cadenas ni se preprocesará el código fuente).
		Si ya existiera un shader con el mismo tipo, lo reemplazaría.
		\param shader Shader a incluir en el programa
		*/
		void addShader(std::shared_ptr<Shader> shader);

		/**
		 * @brief Libera los shaders asociados al programa (el programa no se
		 * destruye)
		*/
		void releaseShaders();
		/**
		Desvincula el shader del tipo indicado del programa. El shader puede seguir
		usándose en otros
		programas. No falla si el programa no tiene ningún shader del tipo indicado.
		*/
		void removeShader(Shader::ShaderType type);

		/**
		Devuelve una referencia al shader del tipo indicado
		\param type Tipo del shader a devolver
		*/
		std::shared_ptr<Shader> getShader(Shader::ShaderType type) const;

		/**
		Devuelve la cantidad de shaders que componen el programa
		*/
		size_t getNumShaders() const { return shaders.size(); };

		/**
		 * @brief Prepara el programa para su uso (si no está compilado, lo compila)
		 * @return true, si el programa está listo para usarse
		*/
		bool prepareForUse();

		/**
		 * @brief Recompiles the current shaders without destroying the program
		 * @return 
		*/
		bool hotReload();
		/**
		Conecta el buffer object con el programa. Esta función pide al objeto bo el
		nombre del bloque y su definición durante la compilación del shader y realiza
		las conexiones automáticamente. Se usa normalmente, para vincular objetos de
		las clases UBOMaterial, UBOLightSources o GLMatrices.
		\warning Llamar a esta función antes de compilar el programa
		\param bo un puntero inteligente al objeto UniformBufferObject
		\param bindingPoint índice del punto de vinculación GL_UNIFORM_BUFFER donde
		conectar el buffer object.
		*/
		void connectUniformBlock(std::shared_ptr<UniformBufferObject> bo,
			GLuint bindingPoint);
		void connectShaderStorageBlock(std::shared_ptr<ShaderStorageBufferObject> bo,
			GLuint bindingPoint);

		/**
		Conecta el buffer object con el shader. Asume que la definición del bloque
		uniforme ya está en el código fuente del shader (o se incluirá en el momento
		de la compilación con la llamada addReplaceString). Usa esta función para
		vincular un bloque que hayas definido tú.
		\warning Llamar a esta función antes de compilar el programa
		\param blockName Nombre de la variable uniform block en el shader
		\param bo un puntero inteligente al objeto UniformBufferObject
		\param bindingPoint índice del punto de vinculación GL_UNIFORM_BUFFER donde
		conectar el buffer object.
		*/
		void connectUniformBlock(const std::string& blockName,
			std::shared_ptr<UniformBufferObject> bo,
			GLuint bindingPoint);
		void connectShaderStorageBlock(const std::string& blockName,
			std::shared_ptr<ShaderStorageBufferObject> bo,
			GLuint bindingPoint);

		/**
		Establece una cadena que será reemplaza por otra justo antes de
		compilar o mostrar el shader. Se puede usar para firmar shaders,
		añadir macros, etc.

		\param replaceThis: cadena a reemplazar
		\param byThis: nueva cadena
		*/
		void replaceString(const std::string& replaceThis, const std::string& byThis);

		/**
		Establece una cadena que será reemplaza por varias cadenas justo antes de
		compilar o mostrar el
		shader. Se puede usar para firmar shaders, añadir macros, etc.

		\param replaceThis: cadena a reemplazar
		\param byThis: vector de cadenas que sustituirán la aparición del replaceThis
		en el shader
		*/
		void replaceString(const std::string& replaceThis, const std::vector<std::string>& byThis);

		bool bindUBOs(GLContext& ctx);

		/**
		 * @brief Establece la lista de salidas de transform feedback
		 * @param varyings variables que se van a guardar en el buffer de transform feedback
		 * @param interleaved si true, las variables se guardarán de forma intercalada
		 * @warning Llamar a esta función antes de compilar el programa
		*/
		void setTransformFeedbackVaryings(const std::vector<std::string>& varyings, bool interleaved);

		/**
		Establece una etiqueta que se mostrará en los mensajes de depuración de GL (GL 4.3)
		\param label Etiqueta a mostrar para esta textura
		*/
		void setGlDebugLabel(const std::string& label);
		/**
		\return La etiqueta que muestra OpenGL en los mensajes de depuración de este objeto
		*/
		std::string getGlDebugLabel() const { return debugLabel; };

	private:
		// Prohibir la copia
		Program(const Program&);
		Program& operator=(Program) = delete;

		enum class BufferType {
			UniformBufferObject, ShaderStorageBufferObject
		};
		struct PendingConnection {
			std::string blockName;
			std::shared_ptr<BufferObject> bo;
			GLuint bindingPoint;
			BufferType type;
		};
		std::vector<PendingConnection> pendingConnections;
		std::map<std::string, std::vector<std::string>> subStrings;

		void bindAttribs();
		bool linkProgram();
		bool bindBlockToBindingPoint(const std::string& blockName, GLuint bindingPoint);
		std::map<Shader::ShaderType, std::shared_ptr<Shader>> shaders;
		std::vector<struct Attribute> attribs;
		GLuint programId;
		bool reconnectUBOs{ true };
		std::vector<std::string> feedbackVaryings;
		bool feedbackInterleaved{ false };

		std::map<std::string, GLint> uniformLocCache;
		std::string debugLabel;
	};

} // namespace

#endif
