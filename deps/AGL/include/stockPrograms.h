#pragma once
// 2016

#include <memory>
#include <logging.h>
#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>
#include <palettes.h>

#include "program.h"
//#include "lifetimeManager.h"

namespace autis {
    class Texture1D;
    
    void buildConstantShadingMVP(Program& program);
	void buildConstantColorUniformMVP(Program& program);
	void buildTouchDepthMVP(Program& program);
	void buildNormalViewerMVP(Program& program);
    void buildGrayToColor(Program& program);
	void buildGrayToColorMVP(Program& program);
	void buildPlaneGrazeHighlightingProgramMVP(Program& program);
	void buildShadowMapMVP(Program& program);
	void buildConstantLightMVP(Program& program);

	template <void (*builder)(Program&)>
	class LocalStockProgram {
	public:
		LocalStockProgram() = default;
		~LocalStockProgram() = default;
		/**
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext &ctx) {
			init();
			return ctx.useProgram(program);
		}
		/**
		\return una referencia al programa.
		*/
		std::shared_ptr<Program> getProgram() {
			init();
			return program;
		}
	protected:
		void init() {
			if (!program) {
				program = std::make_shared<Program>();
				builder(*program);
			}
		}
		std::shared_ptr<Program> program;
	};

	/**
  * \class ConstantIllumProgramMVP
  * Igual que \sa ConstantIllumProgram, pero sin usar GLMatrices (se usa el uniform mvp, que tiene que escribir
  * el usuario.
  */
	class ConstantIllumProgramMVP {
	public:
		/**
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext &ctx);
		/**
		Establece la matriz modelviewprojection.
		*/
		void setMVP(const glm::mat4& mvp);
	private:
		GLint mvpLoc = -1;
		LocalStockProgram<buildConstantShadingMVP> program;
	};


	/**
	* \class ConstantUniformColorProgramMVP
	* Igual que \sa ConstantUniformColorProgram, pero sin usar GLMatrices (se usa el uniform mvp, que tiene que escribir
	* el usuario.
	*/
	class ConstantUniformColorProgramMVP {
	public:
		/**
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);
		/**
		Establece el color que se usará para dibujar las primitivas
		\param color el color
		\warning El programa debe estar instalado en la GPU (con 'use' *antes* de llamar a este
		método)
		*/
		void setColor(const glm::vec4& color);

		/**
		Establece la matriz modelviewprojection.
		*/
		void setMVP(const glm::mat4& mvp);
	private:
		GLint colorLoc = -1, mvpLoc = -1;
		LocalStockProgram<buildConstantColorUniformMVP> program;

	};

	/**
	* \class PlaneGrazeHighlightingProgramMVP
	* Programa que resalta las partes de la malla que se encuentran a una distancia de un plano menor que un umbral
	* el usuario.
	*/
	class PlaneGrazeHighlightingProgramMVP {
	public:
		/**
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);
		/**
		Establece el color que se usará para realtar el roce
		\param color el color
		\warning El programa debe estar instalado en la GPU (con 'use' *antes* de llamar a este
		método)
		*/
		void setColor(const glm::vec4& color);

		/**
		Establece los parámetros del plano de roce
		\param color el color
		\warning El programa debe estar instalado en la GPU (con 'use' *antes* de llamar a este
		método)
		*/
		void setPlane(const glm::vec3& position, const glm::vec3& normal, float threshold);

		/**
		Establece la matriz modelviewprojection.
		\warning El programa debe estar instalado en la GPU (con 'use' *antes* de llamar a este
		método)
		*/
		void setMVP(const glm::mat4& mvp);

		void setM(const glm::mat4& m);
	private:
		GLint colorLoc = -1, mvpLoc = -1, mLoc = -1, planeLoc = -1, thresholdLoc = -1;
		LocalStockProgram<buildPlaneGrazeHighlightingProgramMVP> program;

	};

	/**
	* \class TouchDepthProgramMVP
	* Programa que actualiza únicamente el z-buffer. Se puede hacer glDrawBuffer(GL_NONE) para no modificar el
	* buffer de color.
	*/
	class TouchDepthProgramMVP {
	public:
		/**
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);
		/**
		Establece la matriz modelviewprojection.
		*/
		void setMVP(const glm::mat4& mvp);
	private:
		GLint mvpLoc = -1;
		LocalStockProgram<buildTouchDepthMVP> program;
	};

	/**
	\class NormalViewerProgram
	Programa que muestra las normales del modelo, codificadas en color (rojo: eje X, verde: eje Y y azul: eje Z)
	*/
	class NormalViewerProgramMVP {
	public:
		/*
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);
		/**
		Establece la matriz modelviewprojection.
		*/
		void setMVP(const glm::mat4& mvp);
	private:
		GLint mvpLoc = -1;
		LocalStockProgram<buildNormalViewerMVP> program;
	};


    /**
    \class GrayToColorProgram
    Programa que aplica una paleta de color a una textura en grises
    */
    class GrayToColorProgram {
    public:
        /*
        Instala el programa en la GPU
        */
        std::shared_ptr<Program> use(GLContext& ctx);
        /**
        Establece la paleta
        */
        void setPalette(const Palette palette);
    private:
        GLint mvpLoc = -1;
        LocalStockProgram<buildGrayToColor> program;
        std::shared_ptr<Texture1D> paletteTex;
        Palette currentPaletteType;
    };

	/**
	\class GrayToColorProgramMVP
	Programa que aplica una paleta de color a una textura en grises
	*/
	class GrayToColorProgramMVP {
	public:
		/*
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);
		/**
		Establece la paleta
		*/
		void setPalette(const Palette palette);
		/**
		Establece la matriz modelviewprojection.
		*/
		void setMVP(const glm::mat4& mvp);
	private:
		GLint mvpLoc = -1;
		LocalStockProgram<buildGrayToColorMVP> program;
		std::shared_ptr<Texture1D> paletteTex;
		Palette currentPaletteType;
	};

	/**
	\class ShadowMapProgramMVP
	Programa que construye el shadow map de una escena
	*/
	class ShadowMapProgramMVP {
	public:
		/*
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);
		/**
		Establece la matriz modelviewprojection.
		*/
		void setMVP(const glm::mat4& mvp);
	private:
		GLint mvpLoc = -1;
		LocalStockProgram<buildShadowMapMVP> program;
	};

	/**
	* Permite visualizar la geometria sin necesitar fuentes de luz
	* Similar al modo de renderizado "Solid" Rendering" de blender
	*/
	class ConstantLightMVP {
	public:
		/*
		Instala el programa en la GPU
		*/
		std::shared_ptr<Program> use(GLContext& ctx);

		void setModelViewMatrix(const glm::mat4& modelView);
		void setProjectionMatrix(const glm::mat4& proj);
		void setColor(const glm::vec4& color);
		void setBackFaceColor(const glm::vec4& color);
	private:
		GLint modelViewLoc = -1;
		GLint projMatLoc = -1;
		GLint colorLoc = -1, backfaceColLoc = -1;
		LocalStockProgram<buildConstantLightMVP> program;
	};
};
