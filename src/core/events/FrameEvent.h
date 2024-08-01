
#ifndef DRAWFRAMEEVENT_H
#define DRAWFRAMEEVENT_H
#include "Event.h"

class FrameEvent: public Event {
public:
    virtual void onFrameBegin() = 0;
    virtual void onFrameDrawBegin() = 0;
    virtual void onFrameDrawEnd() = 0;
    virtual void onFrameEnd() = 0;
};

#endif //DRAWFRAMEEVENT_H