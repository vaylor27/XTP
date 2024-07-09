
#ifndef EVENTS_H
#define EVENTS_H
#include <functional>
#include <vector>
#include <unordered_map>
#include "Event.h"
#include "SimpleLogger.h"


class Events {
public:
    static std::vector<std::unique_ptr<Event>> registeredEvents;

    static SimpleLogger* logger;

    static void registerEvent(std::unique_ptr<Event> event);

    template <class EventClass> static void callFunctionOnAllEventsOfType(const std::function<void(EventClass*)>& func) {
        for (const std::unique_ptr<Event>& event : registeredEvents) {
            if (auto* ev = dynamic_cast<EventClass*>(event.get()); ev != nullptr) {
                func(ev);
            }
        }
    }
};



#endif //EVENTS_H
