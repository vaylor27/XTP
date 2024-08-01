# This function is used to compile shaders on build. The user must have glslang installed and on the PATH.
# To use, simply include() this file in your CMakeLists.txt and call compileShaders(MyTarget path/to/shader/dir)
function(compileShaders TARGET SHADER_DIR)

    file(GLOB SHADER_SOURCE_FILES "${SHADER_DIR}/*")

    list(LENGTH SHADER_SOURCE_FILES FILE_COUNT)
    if (FILE_COUNT EQUAL 0)
        message(FATAL_ERROR "Cannot create a shaders target without any source files")
    endif ()

    set(SHADER_PRODUCTS)

    add_custom_command(TARGET XTPTest POST_BUILD
            COMMAND mkdir -p ${CMAKE_BINARY_DIR}/bin/assets/shaders
            BYPRODUCTS "${CMAKE_BINARY_DIR}/bin/assets/shaders/${SHADER_NAME}.spv"
            COMMENT "Creating Shader Directory"
    )

    foreach (SHADER_SOURCE IN LISTS SHADER_SOURCE_FILES)
        cmake_path(ABSOLUTE_PATH SHADER_SOURCE NORMALIZE)
        cmake_path(GET SHADER_SOURCE FILENAME SHADER_NAME)

        # Build command
        file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/bin/assets/shaders")


        add_custom_command(TARGET ${TARGET} POST_BUILD
                COMMAND ${CMAKE_BINARY_DIR}/bin/glslang --target-env vulkan1.2 -e main -o ${CMAKE_BINARY_DIR}/bin/assets/shaders/${SHADER_NAME}.spv ${SHADER_SOURCE}
                BYPRODUCTS "${CMAKE_BINARY_DIR}/bin/assets/shaders/${SHADER_NAME}.spv"
                COMMENT "Compiling Shader (${SHADER_SOURCE}). Using Command: glslangValidator -e main -o ${CMAKE_BINARY_DIR}/bin/assets/shaders/${SHADER_NAME} ${SHADER_SOURCE}"
        )
    endforeach ()
endfunction()