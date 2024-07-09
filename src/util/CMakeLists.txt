cmake_minimum_required(VERSION 3.28)
project(XTPUtils)

set(CMAKE_CXX_STANDARD 17)

if (NOT TARGET XTPUtils)
    add_library(XTPUtils
            FileUtil.cpp
            FileUtil.h
    )

    target_include_directories(XTPUtils PUBLIC
            ./
    )
endif ()