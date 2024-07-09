
#ifndef DRAWFRAMEEVENT_H
#define DRAWFRAMEEVENT_H
#include "Event.h"

class FrameEvent: public Event {
public:
    std::string getEventType() override {
        return typeid(FrameEvent).name();
    }

    virtual void onFrameBegin() = 0;
    virtual void onFrameDrawBegin() = 0;
    virtual void onFrameDrawEnd() = 0;
    virtual void onFrameEnd() = 0;
};

#endif //DRAWFRAMEEVENT_H