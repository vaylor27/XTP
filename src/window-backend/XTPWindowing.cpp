#include "XTPWindowing.h"

std::unique_ptr<XTPWindowBackend> XTPWindowing::windowBackend = nullptr;
std::unique_ptr<XTPWindowData> XTPWindowing::data = nullptr;

void XTPWindowing::setWindowBackend(std::unique_ptr<XTPWindowBackend> backend) {
    windowBackend = std::move(backend);
}

void XTPWindowing::setWindowData(std::unique_ptr<XTPWindowData> backend) {
    data = std::move(backend);
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

void XTPWindowing::getFramebufferSize(int *width, int *height) {
    windowBackend->getFramebufferSize(width, height);
}
