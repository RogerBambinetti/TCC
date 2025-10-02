#include "gui.h"
#include <algorithm>
#include <iostream>

// Static member definitions
GLuint GUI::buttonVAO = 0;
GLuint GUI::buttonVBO = 0;
GLuint GUI::buttonEBO = 0;
GLuint GUI::textVAO = 0;
GLuint GUI::textVBO = 0;
std::map<char, Character> GUI::characters;

#if USE_FREETYPE
FT_Library GUI::ft;
FT_Face GUI::face;
#endif

Button GUI::generateButton(20.0f, 20.0f, 140.0f, 40.0f, "GENERATE CICP");
Button GUI::convertButton(170.0f, 20.0f, 100.0f, 40.0f, "CONVERT");

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

    // Create VAO and VBO for text rendering
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

    // Unbind VAO
    glBindVertexArray(0);

    // Initialize simple font
    initializeFont();
}

void GUI::cleanup()
{
    glDeleteVertexArrays(1, &buttonVAO);
    glDeleteBuffers(1, &buttonVBO);
    glDeleteBuffers(1, &buttonEBO);

    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);

    // Clean up character textures
    for (auto &pair : characters)
    {
        glDeleteTextures(1, &pair.second.textureID);
    }
    characters.clear();

#if USE_FREETYPE
    // Clean up FreeType
    if (face)
    {
        FT_Done_Face(face);
        face = nullptr;
    }
    if (ft)
    {
        FT_Done_FreeType(ft);
        ft = nullptr;
    }
#endif
}

bool GUI::isPointInButton(float x, float y, const Button &button)
{
    return (x >= button.x && x <= button.x + button.width &&
            y >= button.y && y <= button.y + button.height);
}

void GUI::renderButton(GLuint guiShader, GLuint textShader, const Button &button, int windowWidth, int windowHeight)
{
    glUseProgram(guiShader);

    // Create orthographic projection matrix for 2D GUI
    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(guiShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // Set color based on button state and button type
    glm::vec3 color;

    // Different base colors for different buttons
    glm::vec3 baseColor;
    if (button.label == "GENERATE CICP" || button.label == "Generate Layout")
    {
        baseColor = glm::vec3(0.2f, 0.5f, 0.8f); // Blue for Generate CICP
    }
    else if (button.label == "CONVERT")
    {
        baseColor = glm::vec3(0.2f, 0.7f, 0.3f); // Green for Convert
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
        color = baseColor * 0.7f; // Darker when pressed
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

    // Add some padding and rounded corners effect by drawing multiple rectangles
    float padding = 2.0f;

    // Draw button background
    float vertices[] = {
        button.x + padding, button.y + padding,                                // Top-left
        button.x + button.width - padding, button.y + padding,                 // Top-right
        button.x + button.width - padding, button.y + button.height - padding, // Bottom-right
        button.x + padding, button.y + button.height - padding                 // Bottom-left
    };

    // Update VBO with new button position and size
    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

    // Disable depth testing for GUI overlay
    glDisable(GL_DEPTH_TEST);

    // Draw button background
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    // Draw button border (darker color)
    glm::vec3 borderColor = color * 0.6f;
    glUniform3fv(glGetUniformLocation(guiShader, "color"), 1, glm::value_ptr(borderColor));

    // Border vertices (slightly larger)
    float borderVertices[] = {
        button.x, button.y,                                // Top-left
        button.x + button.width, button.y,                 // Top-right
        button.x + button.width, button.y + button.height, // Bottom-right
        button.x, button.y + button.height                 // Bottom-left
    };

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(borderVertices), borderVertices);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glLineWidth(2.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // Re-enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Render button text
    glm::vec3 textColor = glm::vec3(1.0f, 1.0f, 1.0f); // White text
    if (button.isPressed)
    {
        textColor = glm::vec3(0.9f, 0.9f, 0.9f); // Slightly darker when pressed
    }

    // Calculate text position (centered)
    float textScale = 0.7f;
    float textWidth = calculateTextWidth(button.label, textScale);
    float textHeight = 24.0f * textScale; // Use font size * scale for height
    float textX = button.x + (button.width - textWidth) / 2.0f;
    float textY = button.y + (button.height / 2.0f) - (textHeight / 3.0f); // Center vertically - move up significantly

    renderText(textShader, button.label, textX, textY, textScale, textColor, windowWidth, windowHeight);
}

Button &GUI::getGenerateButton()
{
    return generateButton;
}

Button &GUI::getConvertButton()
{
    return convertButton;
}

float GUI::calculateTextWidth(const std::string &text, float scale)
{
    float totalWidth = 0.0f;

    for (const char &c : text)
    {
        Character ch = characters[c];
        totalWidth += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    return totalWidth;
}

void GUI::renderText(GLuint textShader, const std::string &text, float x, float y, float scale, glm::vec3 color, int windowWidth, int windowHeight)
{
    // Activate corresponding render state
    glUseProgram(textShader);

    glm::mat4 projection = glm::ortho(0.0f, (float)windowWidth, (float)windowHeight, 0.0f, -1.0f, 1.0f);
    glUniformMatrix4fv(glGetUniformLocation(textShader, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3fv(glGetUniformLocation(textShader, "textColor"), 1, glm::value_ptr(color));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Disable depth testing for text overlay
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++)
    {
        Character ch = characters[*c];

        float xpos = x + ch.bearing.x * scale;
        float ypos = y - (ch.size.y - ch.bearing.y) * scale;

        float w = ch.size.x * scale;
        float h = ch.size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            {xpos, ypos + h, 0.0f, 1.0f},
            {xpos, ypos, 0.0f, 0.0f},
            {xpos + w, ypos, 1.0f, 0.0f},

            {xpos, ypos + h, 0.0f, 1.0f},
            {xpos + w, ypos, 1.0f, 0.0f},
            {xpos + w, ypos + h, 1.0f, 1.0f}};

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.textureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
}

void GUI::initializeFont()
{
#if USE_FREETYPE
    // Initialize FreeType
    if (FT_Init_FreeType(&ft))
    {
        std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        std::cerr << "Falling back to simple font rendering" << std::endl;

        // Fall back to simple font
        initializeSimpleFont();
        return;
    }

    // Try to load a system font
    std::string fontPath = "C:/Windows/Fonts/arial.ttf";

    if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
    {
        // Try alternative font paths
        fontPath = "C:/Windows/Fonts/calibri.ttf";
        if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
        {
            fontPath = "C:/Windows/Fonts/segoeui.ttf";
            if (FT_New_Face(ft, fontPath.c_str(), 0, &face))
            {
                std::cerr << "ERROR::FREETYPE: Failed to load any font, falling back to simple font" << std::endl;
                initializeSimpleFont();
                return;
            }
        }
    }

    // Set size to load glyphs as (width, height in pixels)
    FT_Set_Pixel_Sizes(face, 0, 24);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++)
    {
        // Load character glyph
        if (FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "ERROR::FREETYPE: Failed to load Glyph for character: " << c << std::endl;
            continue;
        }

        // Generate texture
        GLuint texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer);

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)};
        characters.insert(std::pair<char, Character>(c, character));
    }
    glBindTexture(GL_TEXTURE_2D, 0);
    std::cout << "FreeType font loading successful!" << std::endl;
#else
    // Use simple bitmap font
    initializeSimpleFont();
#endif
}

void GUI::initializeSimpleFont()
{
    std::cout << "Using simple bitmap font rendering" << std::endl;

    // Create simple bitmap font for basic ASCII characters
    std::string charset = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789 .,-_";

    for (char c : charset)
    {
        GLuint texture;
        createCharacterTexture(c, texture);

        Character character = {
            texture,
            glm::ivec2(12, 16), // Slightly larger size for better readability
            glm::ivec2(0, 16),
            12 << 6 // Advance
        };
        characters[c] = character;
    }
}

void GUI::createCharacterTexture(char c, GLuint &texture)
{
    // Create a better 12x16 bitmap for each character
    const int width = 12;
    const int height = 16;
    unsigned char bitmap[width * height];

    // Fill with transparent background
    std::fill(bitmap, bitmap + width * height, 0);

    // Better character patterns with improved readability
    switch (c)
    {
    case 'G':
        // Better G pattern
        for (int y = 3; y < 13; y++)
        {
            for (int x = 2; x < 9; x++)
            {
                if ((y == 3 || y == 12) && x > 2 && x < 8)
                    bitmap[y * width + x] = 255; // Top/bottom
                if ((x == 2 || (x == 8 && y > 7)) && y > 3 && y < 12)
                    bitmap[y * width + x] = 255; // Sides
                if (y == 8 && x > 5 && x < 9)
                    bitmap[y * width + x] = 255; // Middle bar
            }
        }
        break;
    case 'e':
        for (int y = 6; y < 12; y++)
        {
            for (int x = 2; x < 8; x++)
            {
                if (x == 2)
                    bitmap[y * width + x] = 255; // Left edge
                if ((y == 6 || y == 11 || y == 8) && x > 2 && x < 7)
                    bitmap[y * width + x] = 255;
                if (y == 6 && x == 7)
                    bitmap[y * width + x] = 255; // Top right
            }
        }
        break;
    case 'n':
        for (int y = 6; y < 12; y++)
        {
            bitmap[y * width + 2] = 255; // Left edge
            bitmap[y * width + 7] = 255; // Right edge
            if (y == 6)
            {
                for (int x = 3; x < 7; x++)
                    bitmap[y * width + x] = 255; // Top bar
            }
        }
        break;
    case 'r':
        for (int y = 6; y < 12; y++)
        {
            bitmap[y * width + 2] = 255; // Left edge
            if (y == 6)
            {
                for (int x = 3; x < 6; x++)
                    bitmap[y * width + x] = 255; // Top
            }
        }
        break;
    case 'a':
        for (int y = 7; y < 12; y++)
        {
            if (y == 7 || y == 9)
            {
                for (int x = 2; x < 7; x++)
                    bitmap[y * width + x] = 255; // Top/middle
            }
            if (y > 7)
            {
                bitmap[y * width + 2] = 255; // Left edge
                bitmap[y * width + 6] = 255; // Right edge
            }
        }
        break;
    case 't':
        for (int y = 4; y < 12; y++)
        {
            bitmap[y * width + 4] = 255; // Vertical line
            if (y == 6)
            {
                bitmap[y * width + 3] = 255; // Cross bar left
                bitmap[y * width + 5] = 255; // Cross bar right
            }
            if (y == 11)
            {
                bitmap[y * width + 5] = 255; // Bottom curve
            }
        }
        break;
    case 'C':
        for (int y = 3; y < 13; y++)
        {
            for (int x = 2; x < 8; x++)
            {
                if (x == 2 && y > 3 && y < 12)
                    bitmap[y * width + x] = 255; // Left edge
                if ((y == 3 || y == 12) && x > 2 && x < 7)
                    bitmap[y * width + x] = 255; // Top/bottom
            }
        }
        break;
    case 'I':
        for (int y = 3; y < 13; y++)
        {
            bitmap[y * width + 5] = 255; // Vertical line
            if (y == 3 || y == 12)
            {
                for (int x = 3; x < 8; x++)
                    bitmap[y * width + x] = 255; // Top/bottom
            }
        }
        break;
    case 'P':
        for (int y = 3; y < 13; y++)
        {
            bitmap[y * width + 2] = 255; // Left edge
            if (y == 3 || y == 8)
            {
                for (int x = 2; x < 7; x++)
                    bitmap[y * width + x] = 255; // Top/middle
            }
            if (y > 3 && y < 8)
            {
                bitmap[y * width + 6] = 255; // Right edge (top half)
            }
        }
        break;
    case ' ':
        // Space - keep empty
        break;
    default:
        // Better default pattern
        for (int y = 5; y < 11; y++)
        {
            for (int x = 3; x < 8; x++)
            {
                if (y == 5 || y == 10 || x == 3 || x == 7)
                {
                    bitmap[y * width + x] = 255;
                }
            }
        }
        break;
    }

    // Create texture with better filtering
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, bitmap);

    // Better texture options for improved appearance
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}