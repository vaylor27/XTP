cmake_minimum_required(VERSION 3.28)
project(XTPWindowBackend)

set(CMAKE_CXX_STANDARD 17)

if (NOT TARGET XTPWindowBackend)
    add_library(XTPWindowBackend
            XTPWindowBackend.h
            XTPWindowData.h
            XTPWindowing.cpp
            XTPWindowing.h
    )

    target_include_directories(XTPWindowBackend PUBLIC
            ./
    )

    target_link_libraries(XTPWindowBackend XTPLogger)
endif ()