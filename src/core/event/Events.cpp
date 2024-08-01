//
// Created by Viktor Aylor on 27/07/2024.
//
#include "Events.h"

std::vector<std::unique_ptr<Event>> Events::registeredEvents;
SimpleLogger* Events::logger = new SimpleLogger("XTP Events", INFORMATION);