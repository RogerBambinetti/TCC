#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>

// Application class that manages the main 3D application
class Application
{
public:
    Application();
    ~Application();

    // Initialize the application
    bool initialize();

    // Run the main application loop
    void run();

    // Cleanup resources
    void cleanup();

    // Input handling methods (called by InputHandler)
    void handleMouseButton(int button, int action, int mods, double xpos, double ypos);
    void handleCursorPos(double xpos, double ypos);
    void handleFramebufferSize(int width, int height);

    // Button callbacks
    void onGenerateLayoutClick();
    void onConvertClick();

private:
    // Window and OpenGL context
    GLFWwindow *window;
    int windowWidth;
    int windowHeight;

    // Shader programs
    GLuint shaderProgram;
    GLuint guiShaderProgram;
    GLuint textShaderProgram;

    // OpenGL objects for different geometries
    GLuint sphereVAO, sphereVBO, sphereEBO;
    GLuint cubeVAO, cubeVBO, cubeEBO;
    GLuint gridVAO, gridVBO, gridEBO;

    // Geometry data
    std::vector<float> sphereVertices, cubeVertices, gridVertices;
    std::vector<unsigned int> sphereIndices, cubeIndices, gridIndices;

    // View matrices
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    // Cube positions and interaction state
    glm::vec3 cubePositions[6];
    bool isDragging;
    int selectedCube;
    glm::vec2 lastMousePos;
    float dragPlaneY;

    // Private helper methods
    bool initializeWindow();
    bool initializeOpenGL();
    void initializeGeometry();
    void render();
    void renderScene();
    void renderGUI();
};