#include "XTP.h"

#include "Events.h"
#include "TimeManager.h"

#include "XTPWindowing.h"
#include "XTPVulkan.h"


SimpleLogger* XTP::logger = new SimpleLogger("XTP Logger", DEBUG ? LogLevel::DEBUG: LogLevel::INFORMATION);
Ticker* XTP::ticker;
std::thread XTP::updateThread;
bool XTP::doneExecuting;

void XTP::init(const std::chrono::nanoseconds tickInterval) {
    if (XTPWindowing::windowBackend == nullptr) {
        logger->logCritical("Windowing Backend Is Not Set!");
    }
    XTPWindowing::windowBackend->createWindow();
    XTPVulkan::init();

    TimeManager::startup();
    ticker = new Ticker(tickInterval, tick);

    updateThread = std::thread(runTicker);
}

void XTP::start(const std::chrono::nanoseconds tickInterval) {
    init(tickInterval);
    while (!XTPWindowing::windowBackend->shouldClose()) {
        XTPWindowing::windowBackend->pollEvents();
        XTPVulkan::render();
    }
    //Ensure that the thread exits gracefully
    updateThread.join();
    cleanUp();
    std::cout << TimeManager::getAverageFPS() << std::endl;
}

SimpleLogger* XTP::getLogger() {
    return logger;
}

void XTP::cleanUp() {
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
        for (const std::shared_ptr<Renderable>& renderable : renderables) {
            renderable->tick();
        }
    }
}
