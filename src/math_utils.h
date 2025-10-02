#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

// CICP Geometry structure for audio coordinate conversion
struct CICPGeometry {
    float azimuth;
    float elevation;
    int isLFE;
    int screenRef;
};

// Math utility functions
class MathUtils {
public:
    // Convert screen coordinates to world coordinates
    static glm::vec3 screenToWorld(double screenX, double screenY, float worldZ, 
                                  const glm::mat4& viewMatrix, const glm::mat4& projectionMatrix,
                                  int windowWidth, int windowHeight);
    
    // Test ray-cube intersection
    static bool rayCubeIntersection(const glm::vec3& rayOrigin, const glm::vec3& rayDir, 
                                   const glm::vec3& cubePos, float cubeSize, float& distance);
    
    // Convert cartesian coordinates to CICP geometry
    static CICPGeometry cartesianToCICP(const glm::vec3& pos, bool isLFE = false);
    
    // Convert cube positions to CICP format and output to console
    static void convertToCICP(const glm::vec3 cubePositions[6]);
};