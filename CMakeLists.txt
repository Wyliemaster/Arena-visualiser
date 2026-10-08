cmake_minimum_required(VERSION 3.12)

project(Arena LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# GLFW
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory(vendor/glfw)

# GLAD
add_subdirectory(vendor/glad/cmake EXCLUDE_FROM_ALL)
glad_add_library(glad_gl_core_33 STATIC REPRODUCIBLE LOADER API gl:core=3.3)

add_executable(Arena
    main.cpp
    src/Arena.cpp
    src/Visualiser.cpp
)

target_compile_definitions(Arena PRIVATE GLFW_INCLUDE_NONE)

target_link_libraries(Arena PRIVATE
    glfw
    glad_gl_core_33
)