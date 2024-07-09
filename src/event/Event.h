
#ifndef EVENT_H
#define EVENT_H

#include <string>

class Event {

public:

    virtual ~Event() = default;

    virtual std::string getEventType() {
        return "";
    }
};

#endif //EVENT_H