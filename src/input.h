#pragma once

#include <glm/glm.hpp>

// Forward declaration
struct GLFWwindow;
class Application;

// Input handling system
class InputHandler {
public:
    // Initialize input callbacks for the given window
    static void initialize(GLFWwindow* window, Application* app);
    
    // GLFW callback functions
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);

private:
    static Application* appInstance;
};