#include "geometry.h"
#define _USE_MATH_DEFINES
#include <cmath>

void GeometryGenerator::generateSphere(std::vector<float> &vertices, std::vector<unsigned int> &indices,
                                       float radius, int sectors, int stacks)
{
    float x, y, z, xy;
    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stacks; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sectors; ++j)
        {
            sectorAngle = j * sectorStep;
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    int k1, k2;
    for (int i = 0; i < stacks; ++i)
    {
        k1 = i * (sectors + 1);
        k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            if (i != (stacks - 1))
            {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void GeometryGenerator::generateCube(std::vector<float> &vertices, std::vector<unsigned int> &indices,
                                     float size)
{
    // Vertices with positions and normals (x,y,z,nx,ny,nz)
    float verticesArray[] = {
        // Front face
        -size, -size, -size, 0.0f, 0.0f, -1.0f,
        size, -size, -size, 0.0f, 0.0f, -1.0f,
        size, size, -size, 0.0f, 0.0f, -1.0f,
        -size, size, -size, 0.0f, 0.0f, -1.0f,

        // Back face
        -size, -size, size, 0.0f, 0.0f, 1.0f,
        size, -size, size, 0.0f, 0.0f, 1.0f,
        size, size, size, 0.0f, 0.0f, 1.0f,
        -size, size, size, 0.0f, 0.0f, 1.0f,

        // Top face
        -size, size, -size, 0.0f, 1.0f, 0.0f,
        size, size, -size, 0.0f, 1.0f, 0.0f,
        size, size, size, 0.0f, 1.0f, 0.0f,
        -size, size, size, 0.0f, 1.0f, 0.0f,

        // Bottom face
        -size, -size, -size, 0.0f, -1.0f, 0.0f,
        size, -size, -size, 0.0f, -1.0f, 0.0f,
        size, -size, size, 0.0f, -1.0f, 0.0f,
        -size, -size, size, 0.0f, -1.0f, 0.0f,

        // Right face
        size, -size, -size, 1.0f, 0.0f, 0.0f,
        size, size, -size, 1.0f, 0.0f, 0.0f,
        size, size, size, 1.0f, 0.0f, 0.0f,
        size, -size, size, 1.0f, 0.0f, 0.0f,

        // Left face
        -size, -size, -size, -1.0f, 0.0f, 0.0f,
        -size, size, -size, -1.0f, 0.0f, 0.0f,
        -size, size, size, -1.0f, 0.0f, 0.0f,
        -size, -size, size, -1.0f, 0.0f, 0.0f};

    unsigned int indicesArray[] = {
        0, 1, 2, 2, 3, 0,       // Front
        4, 5, 6, 6, 7, 4,       // Back
        8, 9, 10, 10, 11, 8,    // Top
        12, 13, 14, 14, 15, 12, // Bottom
        16, 17, 18, 18, 19, 16, // Right
        20, 21, 22, 22, 23, 20  // Left
    };

    vertices.assign(verticesArray, verticesArray + 144); // 24 vertices * 6 floats each
    indices.assign(indicesArray, indicesArray + 36);
}

void GeometryGenerator::generateGrid(std::vector<float> &vertices, std::vector<unsigned int> &indices,
                                     float size, int divisions)
{
    float step = size / divisions;
    for (int i = -divisions; i <= divisions; ++i)
    {
        vertices.push_back(i * step);
        vertices.push_back(0.0f);
        vertices.push_back(-size);
        vertices.push_back(i * step);
        vertices.push_back(0.0f);
        vertices.push_back(size);

        vertices.push_back(-size);
        vertices.push_back(0.0f);
        vertices.push_back(i * step);
        vertices.push_back(size);
        vertices.push_back(0.0f);
        vertices.push_back(i * step);
    }

    for (unsigned int i = 0; i < vertices.size() / 3; ++i)
    {
        indices.push_back(i);
    }
}