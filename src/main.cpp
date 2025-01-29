#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Vertex Shader Source Code
const char *vertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment Shader Source Code
const char *fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main()
{
    FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
}
)";

// Cube positions
glm::vec3 cubePositions[6] = {
    glm::vec3(2.0f, 0.0f, 0.0f),
    glm::vec3(-2.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 2.0f, 0.0f),
    glm::vec3(0.0f, -2.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 2.0f),
    glm::vec3(0.0f, 0.0f, -2.0f)};

// Function to compile shaders and link them into a program
GLuint compileShaders(const char *vertexSource, const char *fragmentSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

// Function to generate a sphere
void generateSphere(std::vector<float> &vertices, std::vector<unsigned int> &indices, float radius, int sectors, int stacks)
{
    float x, y, z, xy;
    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stacks; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j)
        {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    int k1, k2;
    for (int i = 0; i < stacks; ++i)
    {
        k1 = i * (sectors + 1);
        k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stacks - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

// Function to generate a cube
void generateCube(std::vector<float> &vertices, std::vector<unsigned int> &indices, float size)
{
    float verticesArray[] = {
        -size, -size, -size,
        size, -size, -size,
        size, size, -size,
        -size, size, -size,
        -size, -size, size,
        size, -size, size,
        size, size, size,
        -size, size, size};

    unsigned int indicesArray[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
        0, 3, 7, 7, 4, 0,
        1, 2, 6, 6, 5, 1};

    vertices.assign(verticesArray, verticesArray + 24);
    indices.assign(indicesArray, indicesArray + 36);
}

// Global variables for mouse interaction
bool isDragging = false;
int selectedCube = -1;
glm::vec3 initialMousePos;
glm::vec3 initialCubePos;

// Mouse callback function
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods, const glm::mat4 &view, const glm::mat4 &projection)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            isDragging = true;

            // Get mouse position in screen coordinates
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            initialMousePos = glm::vec3(xpos, ypos, 0.0f);

            // Determine which cube is being clicked (if any)
            // For simplicity, we assume the cubes are aligned along the axes
            // and use a simple distance check to select the closest cube.
            // In a real application, you would use raycasting to determine the clicked object.
            float minDistance = FLT_MAX;
            for (int i = 0; i < 6; ++i)
            {
                glm::vec3 cubePos = cubePositions[i];
                glm::vec3 screenPos = glm::project(cubePos, view, projection, glm::vec4(0, 0, 800, 600));
                float distance = glm::length(glm::vec2(screenPos.x, screenPos.y) - glm::vec2(xpos, ypos));
                if (distance < minDistance)
                {
                    minDistance = distance;
                    selectedCube = i;
                    initialCubePos = cubePos;
                }
            }
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
            selectedCube = -1;
        }
    }
}

// Mouse motion callback function
void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (isDragging && selectedCube != -1)
    {
        // Calculate the change in mouse position
        glm::vec3 currentMousePos = glm::vec3(xpos, ypos, 0.0f);
        glm::vec3 delta = currentMousePos - initialMousePos;

        // Update the cube's position based on the mouse movement
        cubePositions[selectedCube] = initialCubePos + glm::vec3(delta.x * 0.01f, -delta.y * 0.01f, 0.0f);
    }
}

int main()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    GLFWwindow *window = glfwCreateWindow(800, 600, "3D Sphere and Cubes", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Load all OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Set mouse callbacks
    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods)
                               {
        glm::mat4 view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
        mouseButtonCallback(window, button, action, mods, view, projection); });
    glfwSetCursorPosCallback(window, cursorPosCallback);

    // Compile shaders
    GLuint shaderProgram = compileShaders(vertexShaderSource, fragmentShaderSource);

    // Generate sphere
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphere(sphereVertices, sphereIndices, 1.0f, 36, 18);

    // Generate cube
    std::vector<float> cubeVertices;
    std::vector<unsigned int> cubeIndices;
    generateCube(cubeVertices, cubeIndices, 0.5f);

    // Create VAO, VBO, and EBO for sphere
    GLuint sphereVAO, sphereVBO, sphereEBO;
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);

    glBindVertexArray(sphereVAO);

    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Create VAO, VBO, and EBO for cube
    GLuint cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, cubeVertices.size() * sizeof(float), cubeVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, cubeIndices.size() * sizeof(unsigned int), cubeIndices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Clear the color and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use the shader program
        glUseProgram(shaderProgram);

        // Create view and projection matrices
        glm::mat4 view = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

        // Pass view and projection matrices to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // Render sphere
        glm::mat4 sphereModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Render cubes
        for (int i = 0; i < 6; ++i)
        {
            glm::mat4 cubeModel = glm::translate(glm::mat4(1.0f), cubePositions[i]);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(cubeModel));
            glBindVertexArray(cubeVAO);
            glDrawElements(GL_TRIANGLES, cubeIndices.size(), GL_UNSIGNED_INT, 0);
        }

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Clean up
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteBuffers(1, &sphereEBO);

    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteBuffers(1, &cubeVBO);
    glDeleteBuffers(1, &cubeEBO);

    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}