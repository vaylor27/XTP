cmake_minimum_required(VERSION 3.28)
project(XTPLogger)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPLogger)
    add_library(XTPLogger
            SimpleLogger.h
            SimpleLogger.cpp
            LogLevel.h
    )
    target_include_directories(XTPLogger PUBLIC
            ./
    )
endif ()