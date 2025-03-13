#include <iostream>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cfloat>

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
uniform bool isSelected;
void main()
{
    if (isSelected) {
        FragColor = vec4(1.0, 0.5, 0.5, 1.0); // Light red when selected
    } else {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0); // White color
    }
}
)";

// Cube positions
glm::vec3 cubePositions[6] = {
    glm::vec3(2.0f, 0.0f, -2.0f),
    glm::vec3(-2.0f, 0.0f, -2.0f),
    glm::vec3(2.0f, 0.0f, 2.0f),
    glm::vec3(-2.0f, 0.0f, 2.0f),
    glm::vec3(0.0f, 0.0f, 2.0f),
    glm::vec3(0.0f, 0.0f, -2.0f)};

// Global variables for viewing
glm::mat4 viewMatrix;
glm::mat4 projectionMatrix;
int windowWidth = 800;
int windowHeight = 600;

// Global variables for mouse interaction
bool isDragging = false;
int selectedCube = -1;
glm::vec2 lastMousePos;
float dragPlaneY = 0.0f; // Y position of the drag plane

// Function to compile shaders and link them into a program
GLuint compileShaders(const char *vertexSource, const char *fragmentSource)
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);

    // Check for vertex shader compilation errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);

    // Check for fragment shader compilation errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // Check for program linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

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

// Function to generate a grid
void generateGrid(std::vector<float> &vertices, std::vector<unsigned int> &indices, float size, int divisions)
{
    float step = size / divisions;
    for (int i = -divisions; i <= divisions; ++i)
    {
        vertices.push_back(i * step);
        vertices.push_back(0.0f);
        vertices.push_back(-size);
        vertices.push_back(i * step);
        vertices.push_back(0.0f);
        vertices.push_back(size);

        vertices.push_back(-size);
        vertices.push_back(0.0f);
        vertices.push_back(i * step);
        vertices.push_back(size);
        vertices.push_back(0.0f);
        vertices.push_back(i * step);
    }

    for (unsigned int i = 0; i < vertices.size() / 3; ++i)
    {
        indices.push_back(i);
    }
}

// Function to convert screen coordinates to world coordinates
glm::vec3 screenToWorld(double screenX, double screenY, float worldZ)
{
    // Viewport transform
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);

    // Inverse project from screen to world
    glm::vec3 screenPos(screenX, windowHeight - screenY, 0.0f); // Invert Y for OpenGL coordinate system

    // Get near and far points of a ray from camera through cursor
    glm::vec3 nearPoint = glm::unProject(
        glm::vec3(screenPos.x, screenPos.y, 0.0f),
        viewMatrix,
        projectionMatrix,
        viewport);

    glm::vec3 farPoint = glm::unProject(
        glm::vec3(screenPos.x, screenPos.y, 1.0f),
        viewMatrix,
        projectionMatrix,
        viewport);

    // Calculate the ray direction
    glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

    // Calculate t where the ray intersects the Y plane
    float t = (worldZ - nearPoint.y) / rayDir.y;

    // Calculate the intersection point
    glm::vec3 intersectionPoint = nearPoint + rayDir * t;

    return intersectionPoint;
}

// Ray-Cube Intersection test
bool rayCubeIntersection(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir, const glm::vec3 &cubePos, float cubeSize, float &distance)
{
    glm::vec3 min = cubePos - glm::vec3(cubeSize);
    glm::vec3 max = cubePos + glm::vec3(cubeSize);

    // Check for intersection with each axis-aligned plane
    float tmin = (min.x - rayOrigin.x) / rayDir.x;
    float tmax = (max.x - rayOrigin.x) / rayDir.x;

    if (tmin > tmax)
        std::swap(tmin, tmax);

    float tymin = (min.y - rayOrigin.y) / rayDir.y;
    float tymax = (max.y - rayOrigin.y) / rayDir.y;

    if (tymin > tymax)
        std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (min.z - rayOrigin.z) / rayDir.z;
    float tzmax = (max.z - rayOrigin.z) / rayDir.z;

    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    // Set the distance to the nearest intersection
    distance = tmin;

    return true;
}

// Mouse button callback function
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            lastMousePos = glm::vec2(xpos, ypos);

            // Generate ray from camera position through mouse point
            glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);
            glm::vec3 nearPoint = glm::unProject(
                glm::vec3(xpos, windowHeight - ypos, 0.0f),
                viewMatrix,
                projectionMatrix,
                viewport);

            glm::vec3 farPoint = glm::unProject(
                glm::vec3(xpos, windowHeight - ypos, 1.0f),
                viewMatrix,
                projectionMatrix,
                viewport);

            glm::vec3 rayOrigin = nearPoint;
            glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

            // Find closest cube intersection
            float minDistance = FLT_MAX;
            selectedCube = -1;

            for (int i = 0; i < 6; ++i)
            {
                float distance;
                if (rayCubeIntersection(rayOrigin, rayDir, cubePositions[i], 0.5f, distance))
                {
                    if (distance < minDistance)
                    {
                        minDistance = distance;
                        selectedCube = i;
                    }
                }
            }

            if (selectedCube != -1)
            {
                isDragging = true;

                // Calculate the Y plane for dragging (use the cube's current Y position)
                dragPlaneY = cubePositions[selectedCube].y;
            }
        }
        else if (action == GLFW_RELEASE)
        {
            isDragging = false;
        }
    }
}

// Mouse motion callback function
void cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (isDragging && selectedCube != -1)
    {
        // Check if Shift key is pressed
        bool shiftPressed = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS ||
                            glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

        // Calculate world position at the current mouse cursor on the drag plane
        glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);

        glm::vec3 nearPoint = glm::unProject(
            glm::vec3(xpos, windowHeight - ypos, 0.0f),
            viewMatrix,
            projectionMatrix,
            viewport);

        glm::vec3 farPoint = glm::unProject(
            glm::vec3(xpos, windowHeight - ypos, 1.0f),
            viewMatrix,
            projectionMatrix,
            viewport);

        glm::vec3 rayOrigin = nearPoint;
        glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

        if (shiftPressed)
        {
            // Vertical movement: Move along y-axis based on mouse y delta
            float mouseDeltaY = (ypos - lastMousePos.y) * -0.01f; // Negate for intuitive up/down
            cubePositions[selectedCube].y += mouseDeltaY;

            // Update dragPlaneY to follow the cube's Y position
            dragPlaneY = cubePositions[selectedCube].y;
        }
        else
        {
            // Horizontal movement (original behavior)
            // Calculate t where the ray intersects the Y plane
            float t = (dragPlaneY - rayOrigin.y) / rayDir.y;

            // Calculate the intersection point
            glm::vec3 intersectionPoint = rayOrigin + rayDir * t;

            // Update cube position, but maintain its Y coordinate
            cubePositions[selectedCube].x = intersectionPoint.x;
            cubePositions[selectedCube].z = intersectionPoint.z;
        }

        // Update last mouse position
        lastMousePos = glm::vec2(xpos, ypos);
    }
}

// Framebuffer size callback
void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
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
    GLFWwindow *window = glfwCreateWindow(windowWidth, windowHeight, "3D GUI", NULL, NULL);
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
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // Compile shaders
    GLuint shaderProgram = compileShaders(vertexShaderSource, fragmentShaderSource);

    // Generate sphere
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    generateSphere(sphereVertices, sphereIndices, 0.5f, 36, 18);

    // Generate cube
    std::vector<float> cubeVertices;
    std::vector<unsigned int> cubeIndices;
    generateCube(cubeVertices, cubeIndices, 0.5f);

    // Generate grid
    std::vector<float> gridVertices;
    std::vector<unsigned int> gridIndices;
    generateGrid(gridVertices, gridIndices, 5.0f, 10);

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

    // Create VAO, VBO, and EBO for grid
    GLuint gridVAO, gridVBO, gridEBO;
    glGenVertexArrays(1, &gridVAO);
    glGenBuffers(1, &gridVBO);
    glGenBuffers(1, &gridEBO);

    glBindVertexArray(gridVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gridVBO);
    glBufferData(GL_ARRAY_BUFFER, gridVertices.size() * sizeof(float), gridVertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gridEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, gridIndices.size() * sizeof(unsigned int), gridIndices.data(), GL_STATIC_DRAW);

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
        viewMatrix = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f),  // Camera position
                                 glm::vec3(0.0f, 0.0f, 0.0f),  // Looking at the center
                                 glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector

        projectionMatrix = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

        // Pass view and projection matrices to the shader
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(viewMatrix));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projectionMatrix));

        // Set selection uniform to false for non-selected objects
        glUniform1i(glGetUniformLocation(shaderProgram, "isSelected"), 0);

        // Render grid
        glm::mat4 gridModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(gridModel));
        glBindVertexArray(gridVAO);
        glDrawElements(GL_LINES, gridIndices.size(), GL_UNSIGNED_INT, 0);

        // Render sphere
        glm::mat4 sphereModel = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));
        glBindVertexArray(sphereVAO);
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);

        // Render cubes
        for (int i = 0; i < 6; ++i)
        {
            // Set selection uniform
            glUniform1i(glGetUniformLocation(shaderProgram, "isSelected"), (i == selectedCube) ? 1 : 0);

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

    glDeleteVertexArrays(1, &gridVAO);
    glDeleteBuffers(1, &gridVBO);
    glDeleteBuffers(1, &gridEBO);

    glDeleteProgram(shaderProgram);

    // Terminate GLFW
    glfwTerminate();
    return 0;
}
