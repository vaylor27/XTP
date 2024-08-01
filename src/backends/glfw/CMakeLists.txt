cmake_minimum_required(VERSION 3.28)
project(XTPGlfwWindowing)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPGlfwWindowing)
    add_subdirectory(../../../lib/glfw ${CMAKE_CURRENT_BINARY_DIR}/glfw)

    set(XTP_GLFW_LINK_LIBS glfw Vulkan::Loader Vulkan::Headers XTPCore)
    if (${XTP_USE_IMGUI_UI})
        add_library(imgui
                ../../../lib/imgui/imconfig.h
                ../../../lib/imgui/imgui.h
                ../../../lib/imgui/imgui.cpp
                ../../../lib/imgui/imgui_demo.cpp
                ../../../lib/imgui/imgui_draw.cpp
                ../../../lib/imgui/imgui_internal.h
                ../../../lib/imgui/imgui_tables.cpp
                ../../../lib/imgui/imgui_widgets.cpp
                ../../../lib/imgui/imstb_rectpack.h
                ../../../lib/imgui/imstb_textedit.h
                ../../../lib/imgui/imstb_truetype.h
                ../../../lib/imgui/backends/imgui_impl_vulkan.h
                ../../../lib/imgui/backends/imgui_impl_vulkan.cpp
                ../../../lib/imgui/backends/imgui_impl_glfw.h
                ../../../lib/imgui/backends/imgui_impl_glfw.cpp
        )
        target_link_libraries(imgui Vulkan::Headers Vulkan::Loader glfw)

        target_include_directories(imgui PUBLIC
                ../../../lib/imgui)

        add_library(XTP::imgui ALIAS imgui)

        set(XTP_GLFW_LINK_LIBS ${XTP_GLFW_LINK_LIBS} imgui)
    endif ()

    add_library(XTPGlfwWindowing
            GLFWWindowBackend.cpp
            GLFWWindowBackend.h
    )

    target_include_directories(XTPGlfwWindowing PUBLIC
            ./
    )

    target_link_libraries(XTPGlfwWindowing ${XTP_GLFW_LINK_LIBS})
endif ()