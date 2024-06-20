#include "XTPWindowing.h"

XTPWindowBackend* XTPWindowing::windowBackend = nullptr;
XTPWindowData* XTPWindowing::data = nullptr;

void XTPWindowing::setWindowBackend(XTPWindowBackend *backend) {
    windowBackend = backend;
}

void XTPWindowing::createWindow() {
    windowBackend->createWindow();
}

std::pair<int, int> XTPWindowing::getWindowSize() {
    return windowBackend->getWindowSize();
}

void XTPWindowing::setWindowSize(const int width, const int height) {
    windowBackend->setWindowSize(width, height);
}

bool XTPWindowing::shouldClose() {
    return windowBackend->shouldClose();
}

void XTPWindowing::beginFrame() {
    windowBackend->beginFrame();
}

void XTPWindowing::destroyWindow() {
    windowBackend->destroyWindow();
}