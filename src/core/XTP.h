#ifndef XTP_VQT_H
#define XTP_VQT_H

#include "SimpleLogger.h"
#include "Ticker.h"
#include <thread>

class XTP {


public:
#ifdef NDEBUG
    static constexpr bool DEBUG = false;
#else
    static constexpr bool DEBUG = true;
#endif

    static void init(std::chrono::nanoseconds tickInterval);

    static void render();

    static void start(std::chrono::nanoseconds tickInterval);

    static SimpleLogger* getLogger();

    static void cleanUp();

    static void runTicker();

    static void tick();

    static SimpleLogger* logger;

    static Ticker* ticker;

    static std::thread updateThread;
    // static std::thread renderThread;
};


#endif //XTP_VQT_H
