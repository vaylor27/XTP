
#ifndef EVENT_H
#define EVENT_H
#include <string>


class Event {

public:
    ~Event() = default;

    virtual std::string getEventType() = 0;

    virtual void onError() = 0;
};

#endif //EVENT_H
