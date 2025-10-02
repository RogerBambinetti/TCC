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
    float verticesArray[] = {
        -size, -size, -size,
        size, -size, -size,
        size, size, -size,
        -size, size, -size,
        -size, -size, size,
        size, -size, size,
        size, size, size,
        -size, size, size};

    unsigned int indicesArray[] = {
        0, 1, 2, 2, 3, 0,
        4, 5, 6, 6, 7, 4,
        0, 1, 5, 5, 4, 0,
        2, 3, 7, 7, 6, 2,
        0, 3, 7, 7, 4, 0,
        1, 2, 6, 6, 5, 1};

    vertices.assign(verticesArray, verticesArray + 24);
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