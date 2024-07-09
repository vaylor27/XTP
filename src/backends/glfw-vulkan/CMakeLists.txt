cmake_minimum_required(VERSION 3.28)
project(XTPGlfwVkWindowing)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPGlfwVkWindowing)
    add_subdirectory(../../../lib/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)
    add_subdirectory(../../vulkan ${CMAKE_CURRENT_BINARY_DIR}/vulkan-backend)

    add_library(XTPGlfwVkWindowing
            GLFWVulkanWindowing.cpp
            GLFWVulkanWindowing.h
    )

    target_include_directories(XTPGlfwVkWindowing PUBLIC
            ./
    )

    target_link_libraries(XTPGlfwVkWindowing glfw Vulkan::Loader Vulkan::Headers XTPVulkan)
endif ()