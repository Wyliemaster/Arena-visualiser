#ifndef VISUALISER_HPP
#define VISUALISER_HPP

#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <cmath>

#include "Arena.hpp"

class Visualiser
{
public:
    Visualiser() : width(800), height(800) {}
    Visualiser(int width, int height)
    {
        this->width = width;
        this->height = height;
    }

    bool init();
    bool deinit();
    void draw();
    void process_arena_memory_state(const Arena& arena);

    enum class MemoryState
    {
        UNUSED_MEMORY,
        ALLOCATED_MEMORY,
        FREE_MEMORY
    };

private:
    void draw_square(int w, int h);
    std::string get_shader(const char* name);

private:
    int width, height;
    GLFWwindow *window;
    std::vector<MemoryState> memory_state;
};

#endif