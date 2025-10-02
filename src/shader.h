#pragma once

#include <glad/glad.h>

// Shader source code definitions
namespace Shaders
{
    // 3D Object Vertex Shader
    extern const char *vertexShaderSource;

    // 3D Object Fragment Shader
    extern const char *fragmentShaderSource;

    // GUI Vertex Shader
    extern const char *guiVertexShaderSource;

    // GUI Fragment Shader
    extern const char *guiFragmentShaderSource;

    // Text Vertex Shader
    extern const char *textVertexShaderSource;

    // Text Fragment Shader
    extern const char *textFragmentShaderSource;
}

// Shader compilation and linking functions
class ShaderManager
{
public:
    // Compile and link shaders into a program
    static GLuint compileShaders(const char *vertexSource, const char *fragmentSource);

    // Create the 3D shader program
    static GLuint create3DShaderProgram();

    // Create the GUI shader program
    static GLuint createGUIShaderProgram();

    // Create the text shader program
    static GLuint createTextShaderProgram();
};