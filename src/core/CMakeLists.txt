cmake_minimum_required(VERSION 3.28)
project(XTPCore)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPCore)
    add_subdirectory(../time ${CMAKE_CURRENT_BINARY_DIR}/time)
    add_subdirectory(../window-backend ${CMAKE_CURRENT_BINARY_DIR}/window-backend)
    add_subdirectory(../vulkan ${CMAKE_CURRENT_BINARY_DIR}/vulkan)
    add_subdirectory(./events)

    add_library(XTPCore
            XTP.cpp
            XTP.h
    )

    target_include_directories(XTPCore PUBLIC
            .
    )

    target_link_libraries(XTPCore XTPLogger XTPVulkan XTPTime XTPWindowBackend XTPCoreEvents XTPEvents)
endif ()