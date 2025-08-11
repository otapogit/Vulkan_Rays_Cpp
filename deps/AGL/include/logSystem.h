#pragma once

// STL
#include <memory>
// AGLcommon
#include <logDefs.h>

class LogSystemTest_SingleStreamOfMessages_Test;
class LogSystemTest_MultipleStreamOfMessages_Test;


namespace autis {

	/**
	
	LogSystem: singleton containing the logging system.
	It is used by:
	-the back-end, to generate messages. Actually, anyone can log messages invoking to the LogSystem::log method
	-the front-ends, to register one or more callbacks in order to receive log messages.

	The log is multithreaded. The method LogSystem::log can be invoked from any thread, and there is a dedicated thread 
	for pumping messages to registered callbacks.
	Log messages are forgotten once sent. If you want to keep a history of what happened, you have to store the messages
	by yourself.
	Look some examples in logging.h on how to register and unregister callbacks.

	*/

	class LogSystem {
	public:
		/**
		LogSystem is a singleton. Call this method to obtain its reference
		*/
		static LogSystem& instance();
		/**
		Register a callback that will receive all sugsequent messages
		\param cb pointer to a function of type LoggerCallback
		\return an id used for unregister the callback
		*/
		int registerLogCallback(LoggerCallback cb, void* pClient = nullptr);

		/**
		Unregister an existing callback to stop receiving messages
		\param id an id returned by registerLogCallback
		*/
		void unregisterLogCallback(int id);
		/**
		Unregister all callbacks (even those not registed by you). Use with caution.
		*/
		void removeAllCallbacks();
		/**
		This method is used by anyone wanting to log a message to the log. It can be called 
		from any thread. Then, this message will be broadcasted to all the registered callbacks
		\param msg Log message
		\param severity the severity of the message (information, warning or error)
		\param file file where the logging message occurred
		\param line line in the file where the message occurred
		*/
		void log(const char* msg, Severity severity, const char* file, int line);
	private:
		LogSystem();
		~LogSystem();
		class LogSystemImpl;
		std::unique_ptr<LogSystemImpl> pimpl;

		friend class ::LogSystemTest_SingleStreamOfMessages_Test;
		friend class ::LogSystemTest_MultipleStreamOfMessages_Test;
	};
}
