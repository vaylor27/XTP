cmake_minimum_required(VERSION 3.28)
project(XTPCore)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPCoreEvents)
    add_library(XTPCoreEvents
            ShaderRegisterEvent.h
            ShaderRegisterEvent.cpp
            FrameEvent.h
    )

    target_include_directories(XTPCoreEvents PUBLIC
            .
    )

    target_link_libraries(XTPCoreEvents XTPEvents)
endif ()