cmake_minimum_required(VERSION 4.0)
project(BluEngine)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(COMPILED_SHADER_DIR ${CMAKE_BINARY_DIR}/bin/shaders)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

if (TARGET Vulkan::glslc)
    set(GLSLC_EXECUTABLE Vulkan::glslc)
else()
    # fallback if no CMake target, point directly to executable
    find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "${Vulkan_GLSLC_EXECUTABLE}")
endif()

if (NOT GLSLC_EXECUTABLE)
    message(FATAL_ERROR "Could not find glslc executable")
endif()

add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)
add_subdirectory(vendored/assimp EXCLUDE_FROM_ALL)
add_subdirectory(vendored/glm EXCLUDE_FROM_ALL)

add_executable(BluEngine main.cpp
        core/application.cpp
        core/application.h
        core/config.cpp
        core/config.h
        core/etime.cpp
        core/etime.h
        renderer/renderer.cpp
        renderer/renderer.h
        renderer/shadermanager.cpp
        renderer/shadermanager.h
        renderer/pipelinemanager.cpp
        renderer/pipelinemanager.h
        renderer/vertex.h
        renderer/vertexbuffer.cpp
        renderer/vertexbuffer.h
        renderer/indexbuffer.cpp
        renderer/indexbuffer.h
        renderer/uniformbuffer.cpp
        renderer/uniformbuffer.h)

target_link_libraries(BluEngine PRIVATE SDL3::SDL3)
target_link_libraries(BluEngine PRIVATE assimp::assimp)
target_link_libraries(BluEngine PRIVATE glm::glm-header-only)

set(SHADER_DIR ${CMAKE_SOURCE_DIR}/content/shaders)
set(COMPILED_SHADER_DIR ${CMAKE_BINARY_DIR}/shaders)

file(GLOB SHADERS ${SHADER_DIR}/*.vert ${SHADER_DIR}/*.frag)

foreach(SHADER ${SHADERS})
    get_filename_component(FILE_NAME ${SHADER} NAME)
    set(SPV_FILE ${COMPILED_SHADER_DIR}/${FILE_NAME}.spv)

    add_custom_command(
            OUTPUT ${SPV_FILE}
            COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER} -o ${SPV_FILE}
            DEPENDS ${SHADER}
            COMMENT "Compiling ${FILE_NAME}"
    )
    list(APPEND SPV_OUTPUTS ${SPV_FILE})
endforeach()

add_custom_target(compile_shaders DEPENDS ${SPV_OUTPUTS})

add_dependencies(BluEngine compile_shaders)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${COMPILED_SHADER_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)