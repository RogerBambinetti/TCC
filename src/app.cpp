#include "app.h"
#include "shader.h"
#include "geometry.h"
#include "math_utils.h"
#include "gui.h"
#include "input.h"
#include <iostream>
#include <cfloat>

Application::Application()
    : window(nullptr), windowWidth(800), windowHeight(600),
      shaderProgram(0), guiShaderProgram(0), textShaderProgram(0),
      sphereVAO(0), sphereVBO(0), sphereEBO(0),
      cubeVAO(0), cubeVBO(0), cubeEBO(0),
      gridVAO(0), gridVBO(0), gridEBO(0),
      isDragging(false), selectedCube(-1), dragPlaneY(0.0f)
{

    // Initialize cube positions
    cubePositions[0] = glm::vec3(2.0f, 0.5f, -2.0f);
    cubePositions[1] = glm::vec3(-2.0f, 0.5f, -2.0f);
    cubePositions[2] = glm::vec3(2.0f, 0.5f, 2.0f);
    cubePositions[3] = glm::vec3(-2.0f, 0.5f, 2.0f);
    cubePositions[4] = glm::vec3(0.0f, 0.5f, 2.0f);
    cubePositions[5] = glm::vec3(0.0f, 0.5f, -2.0f);
}

Application::~Application()
{
    cleanup();
}

bool Application::initialize()
{
    if (!initializeWindow())
    {
        return false;
    }

    if (!initializeOpenGL())
    {
        return false;
    }

    initializeGeometry();
    return true;
}

bool Application::initializeWindow()
{
    // Initialize GLFW
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Set OpenGL version to 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a windowed mode window and its OpenGL context
    window = glfwCreateWindow(windowWidth, windowHeight, "3D GUI", NULL, NULL);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);

    // Initialize input handling
    InputHandler::initialize(window, this);

    return true;
}

bool Application::initializeOpenGL()
{
    // Load all OpenGL functions using GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Compile shaders
    shaderProgram = ShaderManager::create3DShaderProgram();
    guiShaderProgram = ShaderManager::createGUIShaderProgram();
    textShaderProgram = ShaderManager::createTextShaderProgram();

    // Initialize GUI system
    GUI::initialize();

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set initial projection matrix
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

    return true;
}

void Application::initializeGeometry()
{
    // Generate geometries
    GeometryGenerator::generateSphere(sphereVertices, sphereIndices, 0.5f, 36, 18);
    GeometryGenerator::generateCube(cubeVertices, cubeIndices, 0.5f);
    GeometryGenerator::generateGrid(gridVertices, gridIndices, 5.0f, 10);

    // Create VAO, VBO, and EBO for sphere
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
}

void Application::run()
{
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        render();

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }
}

void Application::render()
{
    // Clear the color and depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    renderScene();
    renderGUI();
}

void Application::renderScene()
{
    // Use the 3D shader program
    glUseProgram(shaderProgram);

    // Create view matrix
    viewMatrix = glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f),  // Camera position
                             glm::vec3(0.0f, 0.0f, 0.0f),  // Looking at the center
                             glm::vec3(0.0f, 1.0f, 0.0f)); // Up vector

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
}

void Application::renderGUI()
{
    // Render GUI buttons
    GUI::renderButton(guiShaderProgram, textShaderProgram, GUI::getGenerateButton(), windowWidth, windowHeight);
}

void Application::handleMouseButton(int button, int action, int mods, double xpos, double ypos)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            // Check button clicks first (GUI has priority)
            if (GUI::isPointInButton((float)xpos, (float)ypos, GUI::getGenerateButton()))
            {
                GUI::getGenerateButton().isPressed = true;
                onGenerateLayoutClick();
                return;
            }

            // If no button was clicked, proceed with 3D object selection
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
                if (MathUtils::rayCubeIntersection(rayOrigin, rayDir, cubePositions[i], 0.5f, distance))
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
            // Reset button states
            GUI::getGenerateButton().isPressed = false;
            isDragging = false;
        }
    }
}

void Application::handleCursorPos(double xpos, double ypos)
{
    // Update button hover states
    GUI::getGenerateButton().isHovered = GUI::isPointInButton((float)xpos, (float)ypos, GUI::getGenerateButton());

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

void Application::handleFramebufferSize(int width, int height)
{
    glViewport(0, 0, width, height);
    windowWidth = width;
    windowHeight = height;
    projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
}

void Application::onGenerateLayoutClick()
{
    MathUtils::convertToCICP(cubePositions);
}

void Application::cleanup()
{
    // Clean up OpenGL resources
    if (sphereVAO)
    {
        glDeleteVertexArrays(1, &sphereVAO);
        glDeleteBuffers(1, &sphereVBO);
        glDeleteBuffers(1, &sphereEBO);
    }

    if (cubeVAO)
    {
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteBuffers(1, &cubeEBO);
    }

    if (gridVAO)
    {
        glDeleteVertexArrays(1, &gridVAO);
        glDeleteBuffers(1, &gridVBO);
        glDeleteBuffers(1, &gridEBO);
    }

    if (shaderProgram)
    {
        glDeleteProgram(shaderProgram);
    }

    if (guiShaderProgram)
    {
        glDeleteProgram(guiShaderProgram);
    }

    if (textShaderProgram)
    {
        glDeleteProgram(textShaderProgram);
    }

    // Cleanup GUI
    GUI::cleanup();

    // Terminate GLFW
    if (window)
    {
        glfwTerminate();
    }
}