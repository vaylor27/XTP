
#ifndef TESTWINDOWDATA_H
#define TESTWINDOWDATA_H
#include "XTPWindowData.h"

class TestWindowData final: public XTPWindowData {
public:
    const char * getWindowTitle() override {
        return "Test Window";
    }

    int getWidth() override {
        return 1080;
    }

    int getHeight() override {
        return 720;
    }
};

#endif //TESTWINDOWDATA_H
