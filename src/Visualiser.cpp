#include "Visualiser.hpp"

bool Visualiser::init()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialise GLFW\n";
        return false;
    }

    glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);

    this->window = glfwCreateWindow(this->width, this->height, "Arena", nullptr, nullptr);

    if (!this->window)
    {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(this->window);

    if (!gladLoadGL(glfwGetProcAddress))
    {
        std::cerr << "Failed to load OpenGL via GLAD\n";
        glfwDestroyWindow(this->window);
        glfwTerminate();
        return false;
    }

    return true;
}

bool Visualiser::deinit()
{
    glfwDestroyWindow(window);
    glfwTerminate();

    return true;
}

void Visualiser::draw()
{
    while (!glfwWindowShouldClose(this->window))
    {
        int w, h;
        glfwGetFramebufferSize(this->window, &w, &h);
        glViewport(0, 0, w, h);

        glClearColor(0.f, 0.f, 0.f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        {
            int root = std::sqrt(this->memory_state.size());
            this->draw_square(root, root);
        }

        glfwSwapBuffers(this->window);
        glfwPollEvents();
    }
}

void Visualiser::draw_square(int columns, int rows)
{
    if (columns <= 0 || rows <= 0)
    {
        std::cerr << "Invalid grid dimensions\n";
        return;
    }

    struct Vtx
    {
        float x, y, z;
        int state;
    };

    const float gap = 0.2f;

    std::vector<Vtx> vertices;

    const float cellWidth = 2.0f / static_cast<float>(columns);
    const float cellHeight = 2.0f / static_cast<float>(rows);

    for (int y = 0; y < rows; ++y)
    {
        for (int x = 0; x < columns; ++x)
        {
            int state = static_cast<int>(this->memory_state[(rows - 1 - y) * columns + x]);

            // Calculate the cell boundaries
            float cellLeft =
                -1.0f + x * cellWidth;

            float cellRight =
                cellLeft + cellWidth;

            float cellBottom =
                -1.0f + y * cellHeight;

            float cellTop =
                cellBottom + cellHeight;

            // Shrink the square inside the cell.
            // gap is relative to the cell dimensions.
            float left =
                cellLeft + cellWidth * gap / 2.0f;

            float right =
                cellRight - cellWidth * gap / 2.0f;

            float bottom =
                cellBottom + cellHeight * gap / 2.0f;

            float top =
                cellTop - cellHeight * gap / 2.0f;

            // First triangle
            vertices.insert(vertices.end(), {{left, top, 0.0f, state},
                                             {left, bottom, 0.0f, state},
                                             {right, bottom, 0.0f, state}});

            // Second triangle
            vertices.insert(vertices.end(), {{left, top, 0.0f, state},
                                             {right, bottom, 0.0f, state},
                                             {right, top, 0.0f, state}});
        }
    }

    // VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // VBO
    unsigned int VBO;
    glGenBuffers(1, &VBO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        vertices.size() * sizeof(Vtx),
        vertices.data(),
        GL_STATIC_DRAW);

    // Vertex attributes
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vtx),
        (void *)offsetof(Vtx, x));

    glEnableVertexAttribArray(0);

    glVertexAttribIPointer(
        1, 1, GL_INT,
        sizeof(Vtx),
        (void *)offsetof(Vtx, state));
    glEnableVertexAttribArray(1);

    // Vertex shader
    std::string vertexSource =
        this->get_shader("square.vert");

    if (vertexSource.empty())
    {
        std::cerr << "Unable to read vertex shader\n";

        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        return;
    }

    const char *vertexShaderSource =
        vertexSource.c_str();

    unsigned int vertexShader =
        glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(
        vertexShader,
        1,
        &vertexShaderSource,
        nullptr);

    glCompileShader(vertexShader);

    int success;
    char infoLog[512];

    glGetShaderiv(
        vertexShader,
        GL_COMPILE_STATUS,
        &success);

    if (!success)
    {
        glGetShaderInfoLog(
            vertexShader,
            sizeof(infoLog),
            nullptr,
            infoLog);

        std::cerr
            << "Vertex shader compilation failed:\n"
            << infoLog
            << '\n';

        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        return;
    }

    // Fragment shader
    std::string fragmentSource =
        this->get_shader("square.frag");

    if (fragmentSource.empty())
    {
        std::cerr << "Unable to read fragment shader\n";

        glDeleteShader(vertexShader);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        return;
    }

    const char *fragmentShaderSource =
        fragmentSource.c_str();

    unsigned int fragmentShader =
        glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(
        fragmentShader,
        1,
        &fragmentShaderSource,
        nullptr);

    glCompileShader(fragmentShader);

    glGetShaderiv(
        fragmentShader,
        GL_COMPILE_STATUS,
        &success);

    if (!success)
    {
        glGetShaderInfoLog(
            fragmentShader,
            sizeof(infoLog),
            nullptr,
            infoLog);

        std::cerr
            << "Fragment shader compilation failed:\n"
            << infoLog
            << '\n';

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        return;
    }

    // Shader program
    unsigned int shaderProgram =
        glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glLinkProgram(shaderProgram);

    glGetProgramiv(
        shaderProgram,
        GL_LINK_STATUS,
        &success);

    if (!success)
    {
        glGetProgramInfoLog(
            shaderProgram,
            sizeof(infoLog),
            nullptr,
            infoLog);

        std::cerr
            << "Shader program linking failed:\n"
            << infoLog
            << '\n';

        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        glDeleteProgram(shaderProgram);
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);

        return;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Draw
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);

    glDrawArrays(
        GL_TRIANGLES,
        0,
        static_cast<GLsizei>(vertices.size()));

    // Cleanup
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

std::string Visualiser::get_shader(const char *name)
{
    std::filesystem::path shader_path =
        std::filesystem::path("shaders") / name;

    std::ifstream file(shader_path);

    if (!file)
    {
        return {};
    }

    std::stringstream buf;
    buf << file.rdbuf();

    return buf.str();
}

static constexpr std::optional<std::size_t> safe_add(std::size_t a, std::size_t b)
{
    if (a > std::numeric_limits<std::size_t>::max() - b)
        return std::nullopt;
    return a + b;
}

void Visualiser::process_arena_memory_state(const Arena &arena)
{
    this->memory_state.clear();

    auto state = arena.state_table.begin();

    for (int block = 0; block < arena.size; block += arena.alignment)
    {
        while (state != arena.state_table.end())
        {
            auto state_end = safe_add(state->region, state->size);

            if (!state_end)
            {
                ++state;
                continue;
            }

            if (block >= *state_end)
            {
                ++state;
                continue;
            }

            break;
        }

        if (state == arena.state_table.end())
        {
            this->memory_state.push_back(
                Visualiser::MemoryState::UNUSED_MEMORY);
            continue;
        }

        auto state_end = safe_add(state->region, state->size);

        if (state->region <= block && block < *state_end)
        {
            switch (state->state)
            {
            case Arena::EventState::ALLOC:
                this->memory_state.push_back(
                    Visualiser::MemoryState::ALLOCATED_MEMORY);
                break;

            case Arena::EventState::FREE:
                this->memory_state.push_back(
                    Visualiser::MemoryState::FREE_MEMORY);
                break;
            }
        }
        else
        {
            this->memory_state.push_back(
                Visualiser::MemoryState::UNUSED_MEMORY);
        }
    }
}