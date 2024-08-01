cmake_minimum_required(VERSION 3.28)
project(XTPCore)

set(CMAKE_CXX_STANDARD 17)


if (NOT TARGET XTPCore)
    if(APPLE)
        enable_language(OBJC)
    endif()

    set(XTP_LINK_LIBS Vulkan::Headers Vulkan::Loader glm::glm GPUOpen::VulkanMemoryAllocator)

    if (${XTP_USE_GLTF_LOADING})
        add_subdirectory(../../lib/tinygltf ${CMAKE_CURRENT_BINARY_DIR}/tinygltf)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXTP_USE_GLTF_LOADING")
        set(XTP_LINK_LIBS ${XTP_LINK_LIBS} tinygltf)
    endif ()

    if (${XTP_USE_TRACY})
        add_subdirectory(../../lib/tracy ${CMAKE_CURRENT_BINARY_DIR}/tracy)
        set(XTP_LINK_LIBS ${XTP_LINK_LIBS} Tracy::TracyClient)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DTRACY_ENABLE")
    endif ()

    if (${XTP_USE_IMGUI_UI})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXTP_USE_IMGUI_UI")
        set(XTP_LINK_LIBS ${XTP_LINK_LIBS} XTP::imgui)
    endif ()

    if (${XTP_USE_ADVANCED_TIMING})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DXTP_USE_ADVANCED_TIMING")
    endif ()

    add_subdirectory(../../lib/Vulkan-Headers ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-Headers)
    add_subdirectory(../../lib/Vulkan-Loader ${CMAKE_CURRENT_BINARY_DIR}/Vulkan-Loader)
    add_subdirectory(../../lib/glm ${CMAKE_CURRENT_BINARY_DIR}/glm)
    add_subdirectory(../../lib/VulkanMemoryAllocator ${CMAKE_CURRENT_BINARY_DIR}/vma)
    add_subdirectory(../../lib/stb_image ${CMAKE_CURRENT_BINARY_DIR}/stb_image)

    add_library(VKE
            ../../lib/Vulkan-Descriptor-Allocator/descriptor_allocator.cpp
            ../../lib/Vulkan-Descriptor-Allocator/descriptor_allocator.h
    )

    add_library(XTPCore
            XTP.cpp
            XTP.h
            events/ShaderRegisterEvent.h
            events/FrameEvent.h
            events/RenderDebugUIEvent.h
            events/InitEvent.h
            event/Event.h
            event/Events.h
            time/TimeManager.cpp
            time/TimeManager.h
            time/Ticker.cpp
            time/Ticker.h
            renderer/XTPVulkan.cpp
            renderer/XTPVulkan.h
            renderer/VkFormatParser.h
            renderer/VulkanRenderInfo.h
            renderer/renderable/Renderable.h
            renderer/renderable/SimpleRenderable.h
            renderer/shader/ShaderObject.h
            renderer/shader/SimpleShaderObject.h
            renderer/renderable/SimpleIndexBufferedRenderable.h
            renderer/ray/ScreenPositionRay.h
            renderer/Camera.h
            renderer/buffer/AllocatedBuffer.h
            renderer/Material.h
            renderer/renderable/MergedMeshRenderable.h
            logging/SimpleLogger.h
            logging/LogLevel.h
            util/FileUtil.cpp
            util/FileUtil.h
            renderer/renderable/Mesh.h
            renderer/buffer/BufferManager.h
            renderer/renderable/SimpleMesh.h
            event/Events.cpp
            renderer/VkFormatParser.cpp
            renderer/VulkanRenderInfo.cpp
            renderer/Camera.cpp
            renderer/AllocatedImage.h
            windowing/XTPWindowBackend.h
            windowing/XTPWindowing.cpp
            windowing/XTPWindowData.h
            windowing/XTPWindowing.h
            events/KeyPressEvent.h
            renderer/Light.h
    )

    target_include_directories(XTPCore PUBLIC
            .
            ./event
            ./events
            ./logging
            ./renderer
            ./time
            ./util
            ./windowing
    )

    target_link_libraries(VKE Vulkan::Headers Vulkan::Loader)
    target_include_directories(VKE PUBLIC ../../lib/Vulkan-Descriptor-Allocator)

    target_link_libraries(XTPCore ${XTP_LINK_LIBS} VKE XTP::stb)

    if (APPLE AND CMAKE_SYSTEM_NAME STREQUAL "Darwin")

        add_custom_command(
                TARGET XTPCore POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/lib/MoltenVK/MoltenVK_icd.json
                ${CMAKE_BINARY_DIR}/bin/vulkan/icd.d/MoltenVK_icd.json)

        add_custom_command(
                TARGET XTPCore POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_SOURCE_DIR}/lib/MoltenVK/libMoltenVK.dylib
                ${CMAKE_BINARY_DIR}/bin/libMoltenVK.dylib)
    endif ()
endif ()