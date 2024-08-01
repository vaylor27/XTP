
#ifndef KEYPRESSEVENT_H
#define KEYPRESSEVENT_H
#include "Event.h"


class KeyPressEvent: public Event {
public:
    virtual void onKeyPressed(int key, int action, int mods) = 0;
};



#endif //KEYPRESSEVENT_H
