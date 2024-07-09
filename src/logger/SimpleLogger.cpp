#include "SimpleLogger.h"

#include <iostream>

void SimpleLogger::log(const std::string &message, const LogLevel severity) const {
    if (severity >= logLevel) {
        if (severity == CRITICAL || severity == ERROR) {
            std::cerr << "[" << loggerName << "] - " << getLevelName(severity) << ": " << message << "\n";
        } else {
            std::cout << "[" << loggerName << "] - " << getLevelName(severity) << ": " << message << "\n";
        }
    }
}

std::string SimpleLogger::getLevelName(const LogLevel level) {
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

SimpleLogger::SimpleLogger(std::string name, LogLevel level) {
    loggerName = std::move(name);
    logLevel = level;
}

void SimpleLogger::logTrace(const std::string &s) const {
    log(s, TRACE);
}

void SimpleLogger::logDebug(const std::string &s) const {
    log(s, DEBUG);
}

void SimpleLogger::logInformation(const std::string &s) const {
    log(s, INFORMATION);
}

void SimpleLogger::logWarning(const std::string &s) const {
    log(s, WARNING);
}

void SimpleLogger::logError(const std::string &s, const bool thr) const {
    if (thr) {
        throw std::runtime_error(s);
    }
    log(s, ERROR);
}

void SimpleLogger::logCritical(const std::string &s, const bool thr) const {
    if (thr) {
        throw std::runtime_error(s);
    }

    log(s, CRITICAL);
}

LogLevel SimpleLogger::getLogLevel() const {
    return logLevel;
}
