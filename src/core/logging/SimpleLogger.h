
#ifndef XTP_SIMPLELOGGER_H
#define XTP_SIMPLELOGGER_H

#include <string>
#include <iostream>
#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#else
#define ZoneScopedN(e)
#endif

#include "LogLevel.h"

class SimpleLogger {
public:
    SimpleLogger(std::string name, LogLevel level) {
        ZoneScopedN("new");
        loggerName = std::move(name);
        logLevel = level;
    }

    void logTrace(const std::string &s) const {
        ZoneScopedN("logTrace");
        log(s, TRACE);
    }

    void logDebug(const std::string &s) const {
        ZoneScopedN("logDebug");
        log(s, DEBUG);
    }

    void logInformation(const std::string &s) const {
        ZoneScopedN("logInformation");
        log(s, INFORMATION);
    }

    void logWarning(const std::string &s) const {
        ZoneScopedN("logWarning");
        log(s, WARNING);
    }

    void logError(const std::string &s, const bool thr = true) const {
        ZoneScopedN("logError");
        if (thr) {
            throw std::runtime_error(s);
        }
        log(s, ERROR);
    }

    void logCritical(const std::string &s, const bool thr = true) const {
        ZoneScopedN("logCritical");
        if (thr) {
            throw std::runtime_error(s);
        }
        log(s, CRITICAL);
    }

    [[nodiscard]] LogLevel getLogLevel() const {
        ZoneScopedN("getLogLevel");
        return logLevel;
    }

private:
    std::string loggerName;
    LogLevel logLevel;
    void log(const std::string& message, LogLevel severity) const {
        ZoneScopedN("log");
        if (severity >= logLevel) {
            if (severity == CRITICAL || severity == ERROR) {
                std::cerr << "[" << loggerName << "] - " << getLogLevelName(severity) << ": " << message << "\n";
            } else {
                std::cout << "[" << loggerName << "] - " << getLogLevelName(severity) << ": " << message << "\n";
            }
        }
    }

    // ReSharper disable once CppDFAConstantFunctionResult
    static std::string getLogLevelName(const LogLevel level) {
        ZoneScopedN("getLevelName");
        std::string toReturn = "invalid";
        switch (level) {
            case TRACE: toReturn = "TRACE";
            break;
            case DEBUG: toReturn = "DEBUG";
            break;
            case INFORMATION: toReturn = "INFORMATION";
            break;
            case WARNING: toReturn = "WARNING";
            break;
            case ERROR: toReturn = "ERROR";
            break;
            case CRITICAL: toReturn = "CRITICAL";
            break;
            case NONE:
                break;
        }
        return toReturn;
    }
};

#endif //XTP_SIMPLELOGGER_H
