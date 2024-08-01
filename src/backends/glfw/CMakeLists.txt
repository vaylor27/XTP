cmake_minimum_required(VERSION 3.28)
project(XTPGlfwWindowing)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPGlfwWindowing)
    add_subdirectory(../../../lib/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)

    add_library(XTPGlfwWindowing
            GLFWWindowBackend.cpp
            GLFWWindowBackend.h
    )

    target_include_directories(XTPGlfwWindowing PUBLIC
            ./
    )

    target_link_libraries(XTPGlfwWindowing glfw Vulkan::Loader Vulkan::Headers XTPCore)
endif ()