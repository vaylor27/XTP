#include "XTP.h"

#include "Camera.h"
#include "CleanUpEvent.h"
#include "Events.h"
#include "InitEvent.h"
#include "TimeManager.h"

#include "XTPWindowing.h"
#include "XTPVulkan.h"


SimpleLogger* XTP::logger = new SimpleLogger("XTP Logger", DEBUG ? LogLevel::DEBUG: LogLevel::INFORMATION);
Ticker* XTP::ticker;
std::thread XTP::updateThread;
// std::thread XTP::renderThread;

void XTP::init(const std::chrono::nanoseconds tickInterval) {
    if (XTPWindowing::windowBackend == nullptr) {
        logger->logCritical("Windowing Backend Is Not Set!");
    }
    XTPWindowing::windowBackend->createWindow();
    XTPVulkan::init();
    XTPWindowing::windowBackend->postRendererInit();
    TimeManager::startup();
    Events::callFunctionOnAllEventsOfType<InitEvent>([](auto e) {e->onInit();});

    ticker = new Ticker(tickInterval, tick);

    updateThread = std::thread(runTicker);
    // renderThread = std::thread(render);

}

// void XTP::render() {
    // TimeManager::startup();
    // Events::callFunctionOnAllEventsOfType<InitEvent>([](auto e) {e->onInit();});

    // while (!XTPWindowing::windowBackend->shouldClose()) {
        // XTPVulkan::render();
    // }
// }

void XTP::start(const std::chrono::nanoseconds tickInterval) {
    init(tickInterval);
    while (!XTPWindowing::windowBackend->shouldClose()) {
        XTPWindowing::windowBackend->pollEvents();
        XTPVulkan::render();
    }
    //Ensure that the thread exits gracefully
    updateThread.join();
    // renderThread.join();

    cleanUp();
}

SimpleLogger* XTP::getLogger() {
    return logger;
}

void XTP::cleanUp() {
    Events::callFunctionOnAllEventsOfType<CleanUpEvent>([](auto e) {e->cleanUp();});
    XTPVulkan::cleanUp();
    XTPWindowing::windowBackend->destroyWindow();
    delete logger;
}

void XTP::runTicker() {
    while (!XTPWindowing::windowBackend->shouldClose()) {
        ticker->tryExecute();
    }
}

void XTP::tick() {
    for (auto& [shader, renderables] : XTPVulkan::toRender) {
        for (const auto&[material, renderables] : renderables) {
            for (const auto& renderable : renderables) {
                renderable->tick();
            }
        }
    }
}
