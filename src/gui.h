#pragma once

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Button structure
struct Button
{
    float x, y, width, height;
    std::string label;
    bool isHovered;
    bool isPressed;

    Button(float x, float y, float width, float height, const std::string &label)
        : x(x), y(y), width(width), height(height), label(label), isHovered(false), isPressed(false) {}
};

// GUI System for handling buttons and UI rendering
class GUI
{
public:
    // Initialize GUI system (setup VAO, VBO, etc.)
    static void initialize();

    // Cleanup GUI resources
    static void cleanup();

    // Check if a point is inside a button
    static bool isPointInButton(float x, float y, const Button &button);

    // Render a button using OpenGL
    static void renderButton(GLuint guiShader, const Button &button, int windowWidth, int windowHeight);

    // Get the generate layout button instance
    static Button &getGenerateButton();

private:
    static GLuint buttonVAO, buttonVBO, buttonEBO;
    static Button generateButton;
};