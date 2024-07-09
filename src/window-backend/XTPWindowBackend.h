
#ifndef XTPWINDOW_H
#define XTPWINDOW_H

#include <iostream>

class XTPWindowBackend {
public:
    bool framebufferResized;

    XTPWindowBackend() = default;

    virtual ~XTPWindowBackend() = default;

    virtual void createWindow() {
    }

    virtual std::pair<int, int> getWindowSize() {
        return {0, 0};
    }

    virtual void setWindowSize(int width, int height) = 0;

    virtual bool shouldClose() {
        return true;
    }

    virtual void beginFrame() = 0;

    virtual void destroyWindow() = 0;

    virtual bool supportsRenderer(const char* str) {
        return false;
    }

    virtual void getFramebufferSize(int * width, int * height) = 0;

    virtual void waitForEvents() = 0;

    virtual void pollEvents() {}
};



#endif //XTPWINDOW_H
