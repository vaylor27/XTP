cmake_minimum_required(VERSION 3.28)
project(XTPVulkan)

set(CMAKE_CXX_STANDARD 17)

# Links incorrectly when used dynamically, so I am building it statically
option(GLM_BUILD_LIBRARY OFF)

if (NOT TARGET XTPVulkan)
    add_subdirectory(../../lib/Vulkan-Headers ${CMAKE_CURRENT_BINARY_DIR}/vulkan-headers)
    add_subdirectory(../../lib/Vulkan-Loader ${CMAKE_CURRENT_BINARY_DIR}/vulkan-loader)
    add_subdirectory(../../lib/glm ${CMAKE_CURRENT_BINARY_DIR}/glm)
    add_subdirectory(../../lib/VulkanMemoryAllocator ${CMAKE_CURRENT_BINARY_DIR}/VulkanMemoryAllocator)
    add_subdirectory(../window-backend ${CMAKE_CURRENT_BINARY_DIR}/window-backend)
    add_subdirectory(../util ${CMAKE_CURRENT_BINARY_DIR}/util)

    add_library(XTPVulkan
            XTPVulkan.cpp
            XTPVulkan.h
            VkFormatParser.cpp
            VkFormatParser.h
            VulkanRenderInfo.cpp
            VulkanRenderInfo.h
            VulkanWindow.h
            renderable/Renderable.cpp
            renderable/Renderable.h
            renderable/SimpleRenderable.cpp
            renderable/SimpleRenderable.h
            shader/ShaderObject.h
            shader/SimpleShaderObject.cpp
            shader/SimpleShaderObject.h
            renderable/SimpleIndexBufferedRenderable.h
            renderable/SimpleIndexBufferedRenderable.cpp
    )

    target_include_directories(XTPVulkan PUBLIC
            .
    )

    target_link_libraries(XTPVulkan XTPUtils Vulkan::Loader Vulkan::Headers XTPTime XTPEvents XTPCoreEvents glm XTPLogger XTPWindowBackend GPUOpen::VulkanMemoryAllocator)

    if (APPLE)
        add_custom_command(
                TARGET XTPVulkan POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/lib/MoltenVK/libMoltenVK.dylib
                ${CMAKE_BINARY_DIR}/bin/libMoltenVK.dylib)
        add_custom_command(
                TARGET XTPVulkan POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/lib/MoltenVK/MoltenVK_icd.json
                ${CMAKE_BINARY_DIR}/bin/vulkan/icd.d/MoltenVK_icd.json)
    endif ()
endif ()