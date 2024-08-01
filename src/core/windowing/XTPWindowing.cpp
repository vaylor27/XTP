#include "XTPWindowing.h"

std::unique_ptr<XTPWindowBackend> XTPWindowing::windowBackend = nullptr;
std::unique_ptr<XTPWindowData> XTPWindowing::data = nullptr;

void XTPWindowing::setWindowBackend(std::unique_ptr<XTPWindowBackend> backend) {
    windowBackend = std::move(backend);
}

void XTPWindowing::setWindowData(std::unique_ptr<XTPWindowData> backend) {
    data = std::move(backend);
}