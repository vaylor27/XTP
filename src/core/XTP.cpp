#include "XTP.h"

#include "TimeManager.h"

#include "XTPRendering.h"
#include "XTPWindowing.h"


SimpleLogger* XTP::logger = new SimpleLogger("XTP Logger", DEBUG ? LogLevel::DEBUG: LogLevel::INFORMATION);

void XTP::init() {
    if (XTPWindowing::windowBackend == nullptr) {
        logger->logCritical("Windowing Backend Is Not Set!");
    }
    if (XTPRendering::renderBackend == nullptr) {
        logger->logCritical("Windowing Backend Is Not Set!");
    }
    XTPWindowing::windowBackend->createWindow();
    XTPRendering::renderBackend->initializeBackend();
    TimeManager::startup();
}

void XTP::start() {
    init();
    while (!XTPWindowing::windowBackend->shouldClose()) {
        XTPWindowing::windowBackend->beginFrame();
        TimeManager::endFrame();
    }
    cleanUp();
}

SimpleLogger* XTP::getLogger() {
    return logger;
}

void XTP::cleanUp() {
    XTPRendering::renderBackend->cleanUp();
    XTPWindowing::windowBackend->destroyWindow();
    delete logger;
}
