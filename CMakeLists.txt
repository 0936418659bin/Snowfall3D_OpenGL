cmake_minimum_required(VERSION 3.10)

# Set vcpkg toolchain file BEFORE project() command
set(CMAKE_TOOLCHAIN_FILE "C:/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

project(Snowfall3D VERSION 1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find packages
find_package(OpenGL REQUIRED)

# Find GLFW
find_package(glfw3 3.3 REQUIRED)

# Find GLM
find_package(glm CONFIG REQUIRED)

# Find Assimp
find_package(assimp CONFIG REQUIRED)

# Check if GLAD source exists
set(GLAD_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/include/external/glad/src/glad.c")
if(NOT EXISTS ${GLAD_SOURCE})
    message(FATAL_ERROR "GLAD source not found at ${GLAD_SOURCE}")
endif()

# GLAD
add_library(glad STATIC ${GLAD_SOURCE})
target_include_directories(glad PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include/external/glad/include"
)

# Define source files
set(SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Shader.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Camera.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/ParticleSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Terrain.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Skybox.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Light.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/CloudSystem.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Snowman.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Vegetation.cpp"
)

# Check if source files exist
foreach(SOURCE_FILE ${SOURCES})
    if(NOT EXISTS ${SOURCE_FILE})
        message(FATAL_ERROR "Source file not found: ${SOURCE_FILE}")
    endif()
endforeach()

# Define header files
set(HEADERS
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Shader.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Camera.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Particle.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/ParticleSystem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Terrain.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Skybox.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Light.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/CloudSystem.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Snowman.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/Vegetation.h"
)

# Check if header files exist
foreach(HEADER_FILE ${HEADERS})
    if(NOT EXISTS ${HEADER_FILE})
        message(FATAL_ERROR "Header file not found: ${HEADER_FILE}")
    endif()
endforeach()

# Executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${OPENGL_INCLUDE_DIR}
)

# Link libraries
target_link_libraries(${PROJECT_NAME}
    glad
    glfw
    ${OPENGL_LIBRARIES}
    glm::glm
    assimp::assimp
)

# Copy shaders to build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/shaders
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders
)

# Copy assets (models/textures) to runtime output so Assimp can find model files
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)

# Set output directories
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic)
endif()

# Debug/Release configurations
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")