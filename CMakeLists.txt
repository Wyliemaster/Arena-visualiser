cmake_minimum_required(VERSION 3.12)

project(Arena LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(Arena
    main.cpp
    src/Arena.cpp
)