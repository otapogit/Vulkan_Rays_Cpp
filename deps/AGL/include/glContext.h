#pragma once

#include <bindingPoint.h>
#include <indexedBindingPoint.h>
#include <string>
#include <functional>
#include <stockPrograms.h>
#include <threadSafeObservable.h>
#include <mutex>

namespace autis {
	class Program;
	class FBO;
	class Mesh;
	class VertexArrayObject;
	class Texture;
	class Sampler;

	class ScopedDebugGroupMessage {
	public:
		ScopedDebugGroupMessage(const std::string& msg, GLuint id = 0x10000);
		~ScopedDebugGroupMessage();
	protected:
		static uint32_t level;
	};

	struct WndContextInfo;

	using VAOCache = std::map<Mesh *, std::unique_ptr<VertexArrayObject>>;

	/**
	 * @brief Contains the state owned by a GL context. Use the class 
			autis::GLContextCreator to create objects of this class.
	*/
	class GLContext {
	public:
		BindingPoint gl_array_buffer,
			gl_copy_read_buffer,
			gl_copy_write_buffer,
			gl_element_array_buffer,
			gl_pixel_pack_buffer,
			gl_pixel_unpack_buffer,
			gl_query_buffer,
			gl_texture_buffer,
			gl_draw_indirect_buffer, // GL 4.0
			gl_dispatch_indirect_buffer; // GL 4.3

		IndexedBindingPoint
			gl_transform_feedback_buffer,
			gl_uniform_buffer,
			gl_atomic_counter_buffer, // GL 4.2
			gl_shader_storage_buffer; // GL 4.3

		/**
		 * @brief Instala el shader en la GPU.
		 * @return el último programa que llamó a este método
		 * @warning si el programa indicado no se puede compilar, mantiene el
		 * programa actual (y lo devuelve en la variable). Para comprobar que
		 * un programa compila bien, llamar a Program::prepareForUse
		*/
		std::shared_ptr<Program> useProgram(std::shared_ptr<Program> program);

		/**
		 * @brief Desinstala el programa de la GPU, dejando como programa actual el 0
		 * @return el último programa que llamó a GLContext::useProgram
			\warning Sólo en raras ocasiones necesitas llamar a esta función. Puedes dejar
			un programa instalado hasta que necesites el siguiente. Al instalar un programa
			(con Program::use), se deinstala el anterior
		*/
		std::shared_ptr<Program> unUseProgram();

		std::shared_ptr<Program> getCurrentProgram() const;

		/*
		
		FBO Management.

		An FBO is not shareable between contexts (textures and RBO are).
		Therefore, an FBO is directly connected with the context which created it, it can
		be bound only to that context, and it should be destroyed by that context.
		
		*/

		/**
		 * @brief 
		 * @return a new FBO created in the context
		*/
		std::shared_ptr<FBO> createFBO();

		/**
		 * @brief Vincula el FBO para lectura (GL_READ_FRAMEBUFFER), escritura
		 *   (GL_DRAW_FRAMEBUFFER), o ambos (GL_FRAMEBUFFER)
		 * @param fbo
		 * @param target el punto de vinculación
		 * @return el último FBO que llamó a esta función
		*/
		std::shared_ptr<FBO> bindFBO(std::shared_ptr<FBO> fbo, GLenum target = GL_DRAW_FRAMEBUFFER);

		/**
		 * @brief Desvincula el FBO del punto de vinculación indicado, vinculando el
		 * framebuffer por defecto
		 * @param target GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER o GL_FRAMEBUFFER
		 * @return @return el último FBO que llamó GLContext::bindFBO
		*/
		std::shared_ptr<FBO> unBindFBO(GLenum target = GL_DRAW_FRAMEBUFFER);

		/**
		 * @brief Devuelve el último FBO vinculado al target indicado
		 * @param target GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER o GL_FRAMEBUFFER
		 * @return
		*/
		std::shared_ptr<FBO> getBoundFBO(GLenum target = GL_DRAW_FRAMEBUFFER);

		/**
		 * @brief Devuelve el último FBO vinculado al target indicado, preguntando a GL (en vez de usar 
		 * los valores cacheados). Útil cuando hay un elemento externo modificando los FBO (Qt?)
		 * @param target GL_READ_FRAMEBUFFER, GL_DRAW_FRAMEBUFFER o GL_FRAMEBUFFER
		 * @return
		*/
		std::shared_ptr<FBO> getBoundFBOFromGL(GLenum target = GL_DRAW_FRAMEBUFFER);


		static std::shared_ptr<FBO> createNonOwningFBOFromId(GLint id);

		void pushCurrentFBO();

		void popCurrentFBO();


		void captureState();

		struct Viewport {
			int32_t x, y, width, height;
		};

		void setViewport(const Viewport& vp);
		Viewport getViewport() const;

		/**
		Returns a summary of the current GL context and thread.
		\return a string with information about the current window DC, GL context and thread,
		   and the ones when the context was created
		*/
		std::string getGLInfo() const;
		/**
		\return the current HDC
		*/
		static uint64_t getCurrentDC();
		/**
		\return the current GL context
		*/
		static uint64_t getCurrentContext();
		/**
		\return the current thread's id
		*/
		static std::string getCurrentThreadId();

		/**
		* @brief Start OpenGL debug logging
		* @return true, if the current GL context was created with the DEBUG bit set
		*/
		bool startLoggingOpenGLDebug();

		/**
		 * @brief Stops OpenGL debug logging
		 * @return true, if the current GL context was created with the DEBUG bit set
		*/
		bool stopLoggingOpenGLDebug();

		void setCurrent();
		bool amICurrent() const;
		void unSetCurrent();
		/**
		\return the HDC (or Window in Linux) created for the GL context
		*/
		uint64_t getDC() const;
		/**
		\return the HGLRC of the GL context
		*/
		uint64_t getGLContext() const;
		/**
		\return the thread if of the thread that created the GL context
		*/
		std::string getThreadId() const;

		std::unique_ptr<GLContext> createSharedContext() const;

		int32_t getMajorVersion() const;
		int32_t getMinorVersion() const;

		/**
		\return el fabricante de la tarjeta gráfica
		\warning Sólo se puede invocar esta función si existe un contexto OpenGL
		*/
		enum class Manufacturer { UNKNOWN, NVIDIA, AMD, INTEL, MICROSOFT};
		static Manufacturer getManufacturer();

		/*
		
		Texture management

		*/
		/**
		 Vincula un objeto textura a la unidad de textura indicada.
		 \param unit Unidad de textura (GL_TEXTURE0, GL_TEXTURE1...)
		 \param tex la textura a vincular
		 \return la texture que estaba vinculada
		 */
		std::shared_ptr<Texture> bindTexture(uint32_t unit, std::shared_ptr<Texture> tex);
		std::shared_ptr<Texture> unBindTexture(uint32_t unit, GLenum textureType);

		std::shared_ptr<Texture> getBoundTexture(uint32_t unit, GLenum textureType);

		static std::shared_ptr<Texture> createNonOwningTextureFromId(GLint id);


		/**
		 * @brief Vincula un objeto sampler a la unidad de textura indicada. A partir de ese momento, se usará 
		 * en vez del sampler definido en los objetos textura.
		 * @param unit Índice de la unidad de textura donde vincular el sampler (0, 1, ...)
		 * @param samp el sampler
		 * @return el sampler que estaba instalado
		 */
		std::shared_ptr<Sampler> bindSampler(uint32_t unit, std::shared_ptr<Sampler> samp);

		/**
		 * @brief Desvincula el sampler de la unidad indicada. A partir de ese momento, el sampler que se usará será
		 * el de los objetos textura
		 * @param unit Índice de la unidad de textura donde vincular el sampler (0, 1, ...)
		 * @return el sampler que estaba instalado
		 */
		std::shared_ptr<Sampler> unbindSampler(uint32_t unit);

		/**
		 * @brief Devuelve el sampler asociado a la unidad indicada, o nullptr si no hay ninguno
		 * @param unit Índice de la unidad de textura donde vincular el sampler (0, 1, ...)
		 * @return el sampler que está instalado
		 */
		std::shared_ptr<Sampler> getBoundSampler(uint32_t unit);
		
		/*
		
		VAO Management.

		The VAO are not shareable between contexts (the VBO are).
		Therefore, a VAO is directly related to the context it created it, and it must
		be released within the same context.		
		*/

		/**
		 * @brief If it has not been created, create the VAO
		 * @param m the mesh 
		*/
		void createAndBindVAO(Mesh *m);


		/**
		 * @return true if the context has a VAO for the provided mesh
		*/
		bool hasVAO(Mesh *m) const;

		/**
		 * @brief Release the VAO for the given mesh in the current context
		 * @param m 
		*/
		void releaseVAO(Mesh* m);

		/**
		 * @brief 
		 * @param m 
		 * @return The VAO for the given mesh in the current context 
		*/
		VertexArrayObject* getVAO(Mesh* m);


		/**
		 * @brief Insert debug message in the OpenGL debug context
		*/
		void debugMessage(const std::string& msg, GLenum type = GL_DEBUG_TYPE_MARKER, GLuint id = 0xFFFF, GLenum severity = GL_DEBUG_SEVERITY_NOTIFICATION);

		struct ContextCommand {
			virtual ~ContextCommand() {};
			virtual void execute(GLContext& ctx) = 0;
		};

		static void addOnAllContexts(std::shared_ptr<ContextCommand> cmd);
		void addCommand(std::shared_ptr<ContextCommand> cmd);
		void executeCommands();


		bool ownsTheContext() const;

		ConstantIllumProgramMVP constantIllumProgramShader;
		ConstantUniformColorProgramMVP constantUniformColorProgramMVP;
		NormalViewerProgramMVP normalViewerProgram;
		GrayToColorProgram graytoColorProgram;
		GrayToColorProgramMVP graytoColorProgramMVP;
		PlaneGrazeHighlightingProgramMVP planeGrazeHighlightingProgramMVP;
		ConstantLightMVP constantLightMVP;
		ShadowMapProgramMVP shadowMapProgramMVP;

		~GLContext();
	protected:
		GLContext();
		GLContext(uint64_t dc, uint64_t context, const std::string &threadId);
		void releaseAll();
		std::shared_ptr<Program> currentProgram;
		std::shared_ptr<FBO> currentReadFBO, currentDrawFBO;
		uint64_t dc{ 0 }, context{ 0 };
		std::string threadId;
		VAOCache vaoCache;

		// se puede ampliar con más tipos
		struct TextureUnitBindings {
			std::shared_ptr<Texture> texture1D, 
				texture2D;
		};
		std::map<uint32_t, TextureUnitBindings> textureState;
		std::shared_ptr<Texture> setAndGetOldTexture(uint32_t unit, GLenum type, std::shared_ptr<Texture> tex);
		std::map<uint32_t, std::shared_ptr<Sampler> >samplers;
		std::shared_ptr<WndContextInfo> contextInfo;
		struct FrameBufferState {
			std::shared_ptr<FBO> read, draw;
		};
		std::vector<FrameBufferState> fboStack;
		int32_t majorVersion{ 0 }, minorVersion{ 0 };
		bool shouldReleaseContext{ true };

		std::mutex commandQueueMutex;
		std::vector<std::shared_ptr<ContextCommand>> commandQueue;
		static ThreadSafeObservable<std::shared_ptr<ContextCommand>> broadcastCommands;
		ThreadSafeObservable<std::shared_ptr<ContextCommand>>::SubscriptionId broadcastId;

		friend class GLContextCreator;
	};

}