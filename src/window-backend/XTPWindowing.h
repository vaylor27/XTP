
#ifndef XTPDISPLAY_H
#define XTPDISPLAY_H
#include "XTPWindowBackend.h"
#include "XTPWindowData.h"

class XTPWindowing {
public:
    static XTPWindowBackend* windowBackend;
    static XTPWindowData* data;

    static void setWindowBackend(XTPWindowBackend* backend);

    static void createWindow();

    static std::pair<int, int> getWindowSize();

    static void setWindowSize(int width, int height);

    static bool shouldClose();

    static void beginFrame();

    static void destroyWindow();
};



#endif //XTPDISPLAY_H
