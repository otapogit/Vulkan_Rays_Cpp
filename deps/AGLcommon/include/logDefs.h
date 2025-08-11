#pragma once

// STL
#include <cstdint>

namespace autis {

	/**
	Error severity levels. Used for logging messages.
	*/
	enum Severity : uint32_t
	{
		UiMsg = 0,
		Debug = (1 << 0),
		Info = (1 << 1),
		Warning = (1 << 2),
		Error = (1 << 3),
	};

	/**
	Type of logging callbacks. You have to define one function with the following
	signature to receive logging messages.
	\param msg Log message
	\param severity the severity of the message (information, warning or error)
	\param file file where the logging message occurred
	\param line line in the file where the message occurred
	\param pClient a pointer included in the registration of the callback that will be returned in each log notification
	*/
	typedef void (*LoggerCallback)(const char* msg, Severity severity, const char* file, int32_t line, void* pClient);

}
