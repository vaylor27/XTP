
#ifndef SHADERREGISTEREVENT_H
#define SHADERREGISTEREVENT_H
#include "Event.h"


class ShaderRegisterEvent: public Event {
public:
    virtual void onRegisterShaders() = 0;
};



#endif //SHADERREGISTEREVENT_H