#include "input.h"
#include "app.h"
#include <GLFW/glfw3.h>

// Static member definition
Application *InputHandler::appInstance = nullptr;

void InputHandler::initialize(GLFWwindow *window, Application *app)
{
    appInstance = app;

    // Set GLFW callbacks
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void InputHandler::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    if (appInstance)
    {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        appInstance->handleMouseButton(button, action, mods, xpos, ypos);
    }
}

void InputHandler::cursorPosCallback(GLFWwindow *window, double xpos, double ypos)
{
    if (appInstance)
    {
        appInstance->handleCursorPos(xpos, ypos);
    }
}

void InputHandler::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    if (appInstance)
    {
        appInstance->handleFramebufferSize(width, height);
    }
}