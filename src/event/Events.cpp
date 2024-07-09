
#include "Events.h"
#include <string>

#include "SimpleLogger.h"

std::vector<std::unique_ptr<Event>> Events::registeredEvents;
SimpleLogger* Events::logger = new SimpleLogger("XTP Events", INFORMATION);

void Events::registerEvent(std::unique_ptr<Event> event) {
    registeredEvents.emplace_back(std::move(event));
}