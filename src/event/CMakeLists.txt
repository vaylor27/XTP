cmake_minimum_required(VERSION 3.28)
project(XTPEvents)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPEvents)
    add_subdirectory(../logger ${CMAKE_CURRENT_BINARY_DIR}/logger)
    add_library(XTPEvents
            Event.h
            Events.h
            Events.cpp
    )

    target_include_directories(XTPEvents PUBLIC
            ./
    )

    target_link_libraries(XTPEvents XTPLogger)
endif ()