
#ifndef XTPWINDOWDATA_H
#define XTPWINDOWDATA_H


class XTPWindowData {
public:
    ~XTPWindowData() = default;

    virtual const char* getWindowTitle() = 0;

    virtual int getWidth() = 0;

    virtual int getHeight() = 0;
};



#endif //XTPWINDOWDATA_H
