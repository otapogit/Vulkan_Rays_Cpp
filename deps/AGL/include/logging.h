#pragma once

#include <logSystem.h>
#include <stdexcept>

#ifdef LOGGING
#include <assert.h>
#include <string>

#define LOG(severity, s) do autis::log((s), severity, __FILE__, __LINE__); while(0)

#define LIBINFO(s) LOG(autis::Severity::Debug, (s))
#define UIMSG(s) LOG(autis::Severity::UiMsg, (s))
#define INFO(s) LOG(autis::Severity::Info, (s))
#define WARN(s) LOG(autis::Severity::Warning, (s))
#define ERR(s) LOG(autis::Severity::Error, (s))
#define ERRT(s) do { LOG(autis::Severity::Error, (s)); throw std::runtime_error(s); } while(0);

namespace autis {
	void log(const std::string &msg, autis::Severity severity, const char *file, int line);

	void startLoggingToFile(const std::string& filename);
	void startLoggingToFile(const std::string& filename, autis::Severity severityThreshold);
	void stopLoggingToFile();
	bool flushLogfile();
	bool flushConsole();
	void startLoggingToConsole();
	void startLoggingToConsole(autis::Severity severityThreshold);
	void stopLoggingToConsole();


#ifdef _WIN32
	void startLoggingToVSConsole();
	void stopLoggingToVSConsole();
#endif
};
#else

#define LIBINFO(s) {}
#define LOG(s) {}
#define ERR(s) {}
#define ERRT(s) {}
#define WARN(s) {}
#define INFO(s) {}

#endif
