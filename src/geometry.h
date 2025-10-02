#pragma once

#include <vector>

// Geometry generation utilities
class GeometryGenerator
{
public:
    // Generate a sphere with specified radius, sectors, and stacks
    static void generateSphere(std::vector<float> &vertices, std::vector<unsigned int> &indices,
                               float radius, int sectors, int stacks);

    // Generate a cube with specified size
    static void generateCube(std::vector<float> &vertices, std::vector<unsigned int> &indices,
                             float size);

    // Generate a grid with specified size and divisions
    static void generateGrid(std::vector<float> &vertices, std::vector<unsigned int> &indices,
                             float size, int divisions);
};