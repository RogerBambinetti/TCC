#include "shader.h"
#include <iostream>

namespace Shaders
{
    // 3D Object Vertex Shader Source Code
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

    // 3D Object Fragment Shader Source Code
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

    // GUI Vertex Shader Source Code
    const char *guiVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec2 aPos;
uniform mat4 projection;
void main()
{
    gl_Position = projection * vec4(aPos.x, aPos.y, 0.0, 1.0);
}
)";

    // GUI Fragment Shader Source Code
    const char *guiFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec3 color;
void main()
{
    FragColor = vec4(color, 1.0);
}
)";
}

GLuint ShaderManager::compileShaders(const char *vertexSource, const char *fragmentSource)
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

GLuint ShaderManager::create3DShaderProgram()
{
    return compileShaders(Shaders::vertexShaderSource, Shaders::fragmentShaderSource);
}

GLuint ShaderManager::createGUIShaderProgram()
{
    return compileShaders(Shaders::guiVertexShaderSource, Shaders::guiFragmentShaderSource);
}