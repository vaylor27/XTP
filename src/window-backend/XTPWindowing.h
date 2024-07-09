
#ifndef XTPDISPLAY_H
#define XTPDISPLAY_H
#include <memory>

#include "XTPWindowBackend.h"
#include "XTPWindowData.h"

class XTPWindowing {
public:
    static std::unique_ptr<XTPWindowBackend> windowBackend;
    static std::unique_ptr<XTPWindowData> data;

    static void setWindowBackend(std::unique_ptr<XTPWindowBackend> backend);

    static void setWindowData(std::unique_ptr<XTPWindowData> backend);

    static void createWindow();

    static std::pair<int, int> getWindowSize();

    static void setWindowSize(int width, int height);

    static bool shouldClose();

    static void beginFrame();

    static void destroyWindow();

    static void getFramebufferSize(int* width, int* height);
};



#endif //XTPDISPLAY_H
