
#include "XTPRendering.h"

XTPRenderBackend* XTPRendering::renderBackend = nullptr;
XTPRenderInfo* XTPRendering::renderInfo = nullptr;

void XTPRendering::setRenderBackend(XTPRenderBackend *backend) {
    renderBackend = backend;
}