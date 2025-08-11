#pragma once

#include <memory>
#include <string>
#include "glContext.h"

namespace autis {
	/**
	\class GLContextCreator
	This class is in charge of creating the GL context.
	A GL context requires a window, but since we are not sending anything to a screen,
	the window is now shown.
	*/

	struct WndContextInfo;

	class GLContextCreator {
	public:
		/**
		Creates a full GL context in the current thread
		\return a pointer to the context, or nullptr if error
		\warning Before calling this method, no GL functions are available.
		*/
		static std::unique_ptr<GLContext> create();
		/**
		 * @brief Creates a GLContext from the current context (probably because it has been created beforehand
		 * by someone else (Qt?)
		 * @return a GLContext object
		 * @warning Call this method once per existing context, otherwise, there will be two AGL objects working
		 * with the same GL context, and the states will be desynchronized. 
		 * @warning The GL context is not destroyed by this object (it is responsability of the creator)
		*/
		static std::unique_ptr<GLContext> createFromCurrentContext();

		/**
		Creates a GL context in the current thread. This new context will be shared with ctx
		\param ctx the parent context
		\return a pointer to the context, or nullptr if error
		\warning Call this method in the existing context's thread. The new thread
		is not activated. Call GLContext::setCurrent to do so.
		*/
		static std::unique_ptr<GLContext> createSharedContext(GLContext& ctx);

		/**
		Creates a GL context in the current thread. This new context will be shared with the 
		current context.
		\param ctx the parent context
		\return a pointer to the context, or nullptr if error
		\warning Call this method in the existing context's thread. The new thread
		is not activated. Call GLContext::setCurrent to do so.
		*/
		static std::unique_ptr<GLContext> createSharedContextFromCurrent();
	private:
		GLContextCreator() = delete;
		~GLContextCreator() = delete;
		static std::unique_ptr<GLContext> buildContext(std::shared_ptr<WndContextInfo> ctxInfo);
	};
};
