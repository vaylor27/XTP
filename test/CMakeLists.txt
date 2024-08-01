cmake_minimum_required(VERSION 3.28)
project(XTPTest)
include(ShaderCompiler.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MACOSX_RPATH TRUE)
set(CMAKE_SKIP_RPATH TRUE)
set(ENABLE_OPT FALSE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0")
if(APPLE)
    enable_language(OBJC)
endif()

set(XTP_LINK_LIBS XTPCore)

set(XTP_TEST_SOURCES
        XTPTest.cpp
        TestWindowData.h
        TestShaderObject.cpp
        TestShaderObject.h
        TestCamera.cpp
        TestCamera.h
        TestRenderable.cpp
        TestRenderable.h
)

if (CMAKE_BUILD_TYPE MATCHES Debug OR CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(XTP_USE_TRACY TRUE)
    set(XTP_USE_IMGUI_UI TRUE)
    set(XTP_USE_GLTF_LOADING TRUE)
    set(XTP_USE_ADVANCED_TIMING TRUE)

    set(XTP_TEST_SOURCES ${XTP_TEST_SOURCES}
            TestImGuiRenderer.cpp
            TestImGuiRenderer.h
    )

    set(XTP_LINK_LIBS ${XTP_LINK_LIBS} XTP::imgui implot)
endif ()

add_subdirectory(../src/core ${CMAKE_CURRENT_BINARY_DIR}/core)
add_subdirectory(../lib/glslang ${CMAKE_CURRENT_BINARY_DIR}/glslang)
add_subdirectory(../src/backends/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfwWindowing)

if (${XTP_USE_IMGUI_UI})
    add_library(implot
            ../lib/implot/implot.cpp
            ../lib/implot/implot.h
            ../lib/implot/implot_demo.cpp
            ../lib/implot/implot_internal.h
            ../lib/implot/implot_items.cpp
    )

    target_include_directories(implot PUBLIC
            ../lib/implot
    )

    target_link_libraries(implot XTP::imgui)
endif ()

add_executable(XTPTest ${XTP_TEST_SOURCES}
        TestMaterial.cpp
        TestMaterial.h
        Options.h
        Options.cpp)

add_dependencies(XTPTest glslang-standalone)

message("${XTP_LINK_LIBS}")

target_link_libraries(XTPTest ${XTP_LINK_LIBS} XTPGlfwWindowing)

if (APPLE)
    set(RPATH "@loader_path")
    set(CMAKE_MACOSX_BUNDLE TRUE)
    set(MACOS)
elseif (UNIX) #MacOS also registers as unix, so this has to come after
    set(RPATH "$ORIGIN")
endif ()

function(setRpath target)
    set_target_properties(${target}
            PROPERTIES
            LINK_FLAGS "-Wl,-rpath,${RPATH}"
    )
endfunction()

add_custom_command(TARGET XTPTest POST_BUILD
        COMMAND cp -r ${CMAKE_SOURCE_DIR}/test/assets ${CMAKE_BINARY_DIR}/bin
        BYPRODUCTS "${CMAKE_BINARY_DIR}/bin/assets/"
        COMMENT "Creating Shader Directory"
)

# Though some of the subprojects link against libraries as well, the rpath only needs to be set for the root executable, since all the libraries will use its rpath.
setRpath(XTPTest)

# Rpath needs to be correctly set for this as well, since it has no relation to XTPTest.
setRpath(glslang-standalone)

compileShaders(XTPTest assets/shaders)
