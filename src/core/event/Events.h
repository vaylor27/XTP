
#ifndef EVENTS_H
#define EVENTS_H
#include <functional>
#include <vector>
#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#else
#define ZoneScopedN(name)
#endif

#include "Event.h"
#include "SimpleLogger.h"


class Events {
public:
    static std::vector<std::unique_ptr<Event>> registeredEvents;

    static SimpleLogger* logger;

    static void registerEvent(std::unique_ptr<Event> event) {
        ZoneScopedN("Events::registerEvent");
        registeredEvents.emplace_back(std::move(event));
    }

    template <class EventClass> static void callFunctionOnAllEventsOfType(const std::function<void(EventClass*)>& func) {
        ZoneScopedN("Events::callFunctionOnAllEventsOfType");
        for (const std::unique_ptr<Event>& event : registeredEvents) {
            if (auto* ev = dynamic_cast<EventClass*>(event.get()); ev != nullptr) {
                func(ev);
            }
        }
    }
};


#endif //EVENTS_H
