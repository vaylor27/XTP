cmake_minimum_required(VERSION 3.28)
project(XTP)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_SKIP_RPATH TRUE)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Links incorrectly when used dynamically, so I am building it statically
option(GLM_BUILD_LIBRARY OFF)

add_subdirectory(lib/glfw-3.4)
add_subdirectory(lib/Vulkan-Headers)
add_subdirectory(lib/Vulkan-Loader)
add_subdirectory(lib/glm)


add_library(XTPLogger
        src/logger/SimpleLogger.h
        src/logger/SimpleLogger.cpp
        src/logger/LogLevel.h
)

add_library(XTPEvents
        src/event/Event.h
        src/event/Events.h
        src/event/Events.cpp
)

add_library(XTPTime
        src/time/TimeManager.cpp
        src/time/TimeManager.h
)

add_library(XTPCore
        src/core/XTP.cpp
        src/core/XTP.h
        src/core/events/ShaderRegisterEvent.cpp
        src/core/events/ShaderRegisterEvent.h
)

add_library(XTPVulkan
        src/backends/vulkan/XTPVulkan.cpp
        src/backends/vulkan/XTPVulkan.h
        src/backends/vulkan/VkFormatParser.cpp
        src/backends/vulkan/VkFormatParser.h
        src/backends/vulkan/VulkanRenderInfo.cpp
        src/backends/vulkan/VulkanRenderInfo.h
        src/backends/vulkan/VulkanWindow.h
)

add_library(XTPGlfwVkWindowing
        src/backends/glfw-vulkan/GLFWVulkanWindowing.cpp
        src/backends/glfw-vulkan/GLFWVulkanWindowing.h
)

add_library(XTPRenderBackend
        src/render-backend/XTPRenderBackend.h
        src/render-backend/XTPRendering.cpp
        src/render-backend/XTPRendering.h
)

add_library(XTPWindowBackend
        src/window-backend/XTPWindowBackend.h
        src/window-backend/XTPWindowData.h
        src/window-backend/XTPWindowing.cpp
        src/window-backend/XTPWindowing.h
)


add_executable(XTPTest
        test/xtp_test.cpp
        test/TestWindowData.h)

target_include_directories(XTPCore PUBLIC
        src/core/
        src/core/events/
)
target_include_directories(XTPVulkan PUBLIC
        src/backends/vulkan/
)
target_include_directories(XTPTime PUBLIC
        src/time/
)
target_include_directories(XTPEvents PUBLIC
        src/event/
)
target_include_directories(XTPLogger PUBLIC
        src/logger/
)
target_include_directories(XTPRenderBackend PUBLIC
        src/render-backend/
)
target_include_directories(XTPWindowBackend PUBLIC
        src/window-backend/
)

target_include_directories(XTPGlfwVkWindowing PUBLIC
        src/backends/glfw-vulkan
)

target_link_libraries(XTPCore ${GLFW_LIBRARIES} glm XTPLogger XTPTime XTPRenderBackend XTPWindowBackend)
target_link_libraries(XTPVulkan XTPCore ${GLFW_LIBRARIES} Vulkan::Loader Vulkan::Headers glm XTPLogger XTPRenderBackend XTPWindowBackend)
target_link_libraries(XTPTest XTPCore XTPLogger XTPGlfwVkWindowing)
target_link_libraries(XTPGlfwVkWindowing glfw Vulkan::Loader Vulkan::Headers XTPVulkan)
target_link_libraries(XTPWindowBackend XTPLogger)
target_link_libraries(XTPRenderBackend XTPLogger)

if (APPLE)
    add_custom_command(
            TARGET XTPCore POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/lib/MoltenVK/libMoltenVK.dylib
            ${CMAKE_CURRENT_BINARY_DIR}/libMoltenVK.dylib)
    add_custom_command(
            TARGET XTPCore POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_SOURCE_DIR}/lib/MoltenVK/vulkan/icd.d/MoltenVK_icd.json
            ${CMAKE_CURRENT_BINARY_DIR}/vulkan/icd.d/MoltenVK_icd.json)

    set(RPATH "@loader_path")
elseif (UNIX)
    set(RPATH "$ORIGIN")
endif ()


function(setRpath target)
    set_target_properties(${target}
            PROPERTIES
            LINK_FLAGS "-Wl,-rpath,${RPATH}"
    )
endfunction()


setRpath(XTPTest)
setRpath(XTPCore)