//
// Created by Viktor Aylor on 14/07/2024.
//

#ifndef INITEVENT_H
#define INITEVENT_H
#include "Event.h"

class InitEvent: public Event {
public:
    virtual void onInit() = 0;
};

#endif //INITEVENT_H
