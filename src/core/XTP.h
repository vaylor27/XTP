#ifndef XTP_VQT_H
#define XTP_VQT_H

#include <iostream>

#include "SimpleLogger.h"

class XTP {


public:
#ifdef NDEBUG
    static constexpr bool DEBUG = false;
#else
    static constexpr bool DEBUG = true;
#endif


#define MAKE_VERSION(major, minor, patch) \
((((uint32_t)(major)) << 22U) | (((uint32_t)(minor)) << 12U) | ((uint32_t)(patch)))

    static constexpr std::string_view engineName = "VQT Engine";
    static constexpr uint32_t engineVersion = MAKE_VERSION(0, 0, 1);

    static void init();

    static void start();

    static SimpleLogger* getLogger();

    static void cleanUp();

    static SimpleLogger* logger;
};


#endif //XTP_VQT_H
