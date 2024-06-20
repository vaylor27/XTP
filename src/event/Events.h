
#ifndef EVENTS_H
#define EVENTS_H
#include <vector>
#include "Event.h"


class Events {
public:
    static std::unordered_map<std::string, std::vector<Event>> registeredEvents;

    static void registerEvent(Event& event);

    static std::vector<Event> getEventsForName(const std::string& name);
};



#endif //EVENTS_H
