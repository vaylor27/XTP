
#include "Events.h"

std::unordered_map<std::string, std::vector<Event>> Events::registeredEvents = {};

void Events::registerEvent(Event &event) {
    if (registeredEvents.find(event.getEventType()) == registeredEvents.end()) {
        registeredEvents.insert(std::make_pair(event.getEventType(), std::vector<Event> {}));
    }

    registeredEvents[event.getEventType()].emplace_back(event);
}

std::vector<Event> Events::getEventsForName(const std::string& name) {
    return registeredEvents[name];
}
