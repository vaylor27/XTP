//
// Created by Viktor Aylor on 14/07/2024.
//

#ifndef RENDERDEBUGUIEVENT_H
#define RENDERDEBUGUIEVENT_H
#include "Event.h"


class RenderDebugUIEvent: public Event {
public:
    virtual void renderDebugUI() = 0;
};



#endif //RENDERDEBUGUIEVENT_H
