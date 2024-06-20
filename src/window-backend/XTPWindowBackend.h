
#ifndef XTPWINDOW_H
#define XTPWINDOW_H
#include <utility>


class XTPWindowBackend {
public:
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
};



#endif //XTPWINDOW_H
