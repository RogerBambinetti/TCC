#include "gui.h"
#include <algorithm>

// Static member definitions
GLuint GUI::buttonVAO = 0;
GLuint GUI::buttonVBO = 0;
GLuint GUI::buttonEBO = 0;
Button GUI::generateButton(20.0f, 20.0f, 140.0f, 40.0f, "Generate Layout");

void GUI::initialize()
{
    // Create VAO and VBO for GUI buttons
    glGenVertexArrays(1, &buttonVAO);
    glGenBuffers(1, &buttonVBO);
    glGenBuffers(1, &buttonEBO);

    glBindVertexArray(buttonVAO);

    // Initial button vertices (will be updated per button)
    float buttonVertices[] = {
        0.0f, 0.0f, // Top-left
        0.0f, 0.0f, // Top-right
        0.0f, 0.0f, // Bottom-right
        0.0f, 0.0f  // Bottom-left
    };

    unsigned int buttonIndices[] = {
        0, 1, 2, // First triangle
        2, 3, 0  // Second triangle
    };

    glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buttonEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(buttonIndices), buttonIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // Unbind VAO
    glBindVertexArray(0);
}

void GUI::cleanup()
{
    glDeleteVertexArrays(1, &buttonVAO);
    glDeleteBuffers(1, &buttonVBO);
    glDeleteBuffers(1, &buttonEBO);
}

bool GUI::isPointInButton(float x, float y, const Button &button)
{
    return (x >= button.x && x <= button.x + button.width &&
            y >= button.y && y <= button.y + button.height);
}

void GUI::renderButton(GLuint guiShader, const Button &button, int windowWidth, int windowHeight)
{
    glUseProgram(guiShader);

    // Create orthographic projection matrix for 2D GUI
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(guiShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Set color based on button state and button type
    glm::vec3 color;

    // Different base colors for different buttons
    glm::vec3 baseColor;
    if (button.label == "Generate Layout")
    {
        baseColor = glm::vec3(0.3f, 0.7f, 0.3f); // Green for Generate Layout
    }
    else if (button.label == "Cancel")
    {
        baseColor = glm::vec3(0.7f, 0.3f, 0.3f); // Red for Cancel
    }
    else
    {
        baseColor = glm::vec3(0.5f, 0.5f, 0.5f); // Default gray
    }

    // Modify color based on button state
    if (button.isPressed)
    {
        color = baseColor * 0.6f; // Darker when pressed
    }
    else if (button.isHovered)
    {
        color = baseColor * 1.2f;                 // Brighter when hovered
        color = glm::min(color, glm::vec3(1.0f)); // Clamp to prevent overflow
    }
    else
    {
        color = baseColor; // Default color
    }

    glUniform3fv(glGetUniformLocation(guiShader, "color"), 1, glm::value_ptr(color));

    // Update button vertices
    float vertices[] = {
        button.x, button.y,                                // Top-left
        button.x + button.width, button.y,                 // Top-right
        button.x + button.width, button.y + button.height, // Bottom-right
        button.x, button.y + button.height                 // Bottom-left
    };

    // Update VBO with new button position and size
    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Disable depth testing for GUI overlay
    glDisable(GL_DEPTH_TEST);

    // Draw button
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);
}

Button &GUI::getGenerateButton()
{
    return generateButton;
}