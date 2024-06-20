
#ifndef XTP_SIMPLELOGGER_H
#define XTP_SIMPLELOGGER_H

#include <string>

#include "LogLevel.h"

class SimpleLogger {
public:
    SimpleLogger(std::string loggerName, LogLevel logLevel);

    void logTrace(const std::string& s) const;

    void logDebug(const std::string& s) const;

    void logInformation(const std::string& s) const;

    void logWarning(const std::string& s) const;

    void logError(const std::string& s, bool thr = true) const;

    void logCritical(const std::string& s, bool thr = true) const;

    [[nodiscard]] LogLevel getLogLevel() const;

private:
    std::string loggerName;
    LogLevel logLevel;
    void log(const std::string& message, LogLevel severity) const;

    static std::string getLevelName(LogLevel level);
};

#endif //XTP_SIMPLELOGGER_H
