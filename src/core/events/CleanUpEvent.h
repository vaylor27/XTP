//
// Created by Viktor Aylor on 14/07/2024.
//

#ifndef CLEANUPEVENT_H
#define CLEANUPEVENT_H
#include "Event.h"

class CleanUpEvent: public Event {
public:
    virtual void cleanUp() = 0;
};

#endif //CLEANUPEVENT_H
