cmake_minimum_required(VERSION 3.28)
project(XTPTime)

set(CMAKE_CXX_STANDARD 17)

if (NOT TARGET XTPTime)
    add_library(XTPTime
            TimeManager.cpp
            TimeManager.h
            Ticker.cpp
            Ticker.h
    )

    target_include_directories(XTPTime PUBLIC
            ./
    )
endif ()