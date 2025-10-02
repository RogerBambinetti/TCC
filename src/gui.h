#pragma once

#include <string>
#include <map>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Try to include FreeType, fallback to simple font if not available
#define USE_FREETYPE 1

#if USE_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif

// Character structure for font rendering
struct Character
{
    GLuint textureID;     // ID handle of the glyph texture
    glm::ivec2 size;      // Size of glyph
    glm::ivec2 bearing;   // Offset from baseline to left/top of glyph
    unsigned int advance; // Offset to advance to next glyph
};

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
    static void renderButton(GLuint guiShader, GLuint textShader, const Button &button, int windowWidth, int windowHeight);

    // Render text at specified position
    static void renderText(GLuint textShader, const std::string &text, float x, float y, float scale, glm::vec3 color, int windowWidth, int windowHeight);

    // Get the generate layout button instance
    static Button &getGenerateButton();

    // Calculate actual text width using character metrics
    static float calculateTextWidth(const std::string &text, float scale);

private:
    static GLuint buttonVAO, buttonVBO, buttonEBO;
    static GLuint textVAO, textVBO;
    static Button generateButton;
    static std::map<char, Character> characters;

#if USE_FREETYPE
    // FreeType library objects
    static FT_Library ft;
    static FT_Face face;
#endif

    // Initialize font system (FreeType or fallback)
    static void initializeFont();
    static void initializeSimpleFont();
    static void createCharacterTexture(char c, GLuint &texture);
};