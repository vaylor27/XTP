cmake_minimum_required(VERSION 3.28)
project(XTPTest)
include(ShaderCompiler.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_SKIP_RPATH TRUE)
set(ENABLE_OPT FALSE)

add_subdirectory(../src/core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory(../src/logger ${CMAKE_CURRENT_BINARY_DIR}/logger)
add_subdirectory(../src/event ${CMAKE_CURRENT_BINARY_DIR}/event)
add_subdirectory(../src/backends/glfw-vulkan ${CMAKE_CURRENT_BINARY_DIR}/glfw-vk-windowing)
add_subdirectory(../lib/glslang ${CMAKE_CURRENT_BINARY_DIR}/glslang)

add_executable(XTPTest
        XTPTest.cpp
        TestWindowData.h
        TestShaderObject.cpp
        TestShaderObject.h)


target_link_libraries(XTPTest XTPCore XTPEvents XTPLogger XTPGlfwVkWindowing)

add_dependencies(XTPTest glslang-standalone)

if (APPLE)
    set(RPATH "@loader_path")
elseif (UNIX) #MacOS also registers as unix, so this has to come after
    set(RPATH "$ORIGIN")
endif ()

function(setRpath target)
    set_target_properties(${target}
            PROPERTIES
            LINK_FLAGS "-Wl,-rpath,${RPATH}"
    )
endfunction()

# Though some of the subprojects link against libraries as well, the rpath only needs to be set for the root executable, since all the libraries will use its rpath.
setRpath(XTPTest)

# Rpath needs to be correctly set for this as well, since it has no relation to XTPTest.
setRpath(glslang-standalone)

compileShaders(XTPTest shaders)