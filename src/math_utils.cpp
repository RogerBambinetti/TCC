#include "math_utils.h"
#include <cmath>

glm::vec3 MathUtils::screenToWorld(double screenX, double screenY, float worldZ,
                                   const glm::mat4 &viewMatrix, const glm::mat4 &projectionMatrix,
                                   int windowWidth, int windowHeight)
{
    // Viewport transform
    glm::vec4 viewport = glm::vec4(0, 0, windowWidth, windowHeight);

    // Inverse project from screen to world
    glm::vec3 screenPos(screenX, windowHeight - screenY, 0.0f); // Invert Y for OpenGL coordinate system

    // Get near and far points of a ray from camera through cursor
    glm::vec3 nearPoint = glm::unProject(
        glm::vec3(screenPos.x, screenPos.y, 0.0f),
        viewMatrix,
        projectionMatrix,
        viewport);

    glm::vec3 farPoint = glm::unProject(
        glm::vec3(screenPos.x, screenPos.y, 1.0f),
        viewMatrix,
        projectionMatrix,
        viewport);

    // Calculate the ray direction
    glm::vec3 rayDir = glm::normalize(farPoint - nearPoint);

    // Calculate t where the ray intersects the Y plane
    float t = (worldZ - nearPoint.y) / rayDir.y;

    // Calculate the intersection point
    glm::vec3 intersectionPoint = nearPoint + rayDir * t;

    return intersectionPoint;
}

bool MathUtils::rayCubeIntersection(const glm::vec3 &rayOrigin, const glm::vec3 &rayDir,
                                    const glm::vec3 &cubePos, float cubeSize, float &distance)
{
    glm::vec3 min = cubePos - glm::vec3(cubeSize);
    glm::vec3 max = cubePos + glm::vec3(cubeSize);

    // Check for intersection with each axis-aligned plane
    float tmin = (min.x - rayOrigin.x) / rayDir.x;
    float tmax = (max.x - rayOrigin.x) / rayDir.x;

    if (tmin > tmax)
        std::swap(tmin, tmax);

    float tymin = (min.y - rayOrigin.y) / rayDir.y;
    float tymax = (max.y - rayOrigin.y) / rayDir.y;

    if (tymin > tymax)
        std::swap(tymin, tymax);

    if ((tmin > tymax) || (tymin > tmax))
        return false;

    if (tymin > tmin)
        tmin = tymin;

    if (tymax < tmax)
        tmax = tymax;

    float tzmin = (min.z - rayOrigin.z) / rayDir.z;
    float tzmax = (max.z - rayOrigin.z) / rayDir.z;

    if (tzmin > tzmax)
        std::swap(tzmin, tzmax);

    if ((tmin > tzmax) || (tzmin > tmax))
        return false;

    if (tzmin > tmin)
        tmin = tzmin;

    if (tzmax < tmax)
        tmax = tzmax;

    // Set the distance to the nearest intersection
    distance = tmin;

    return true;
}

CICPGeometry MathUtils::cartesianToCICP(const glm::vec3 &pos, bool isLFE)
{
    // Calculate distance from origin
    float distance = glm::length(pos);

    // Calculate azimuth (horizontal angle)
    // atan2(x, -z) because in audio: 0° = front (-Z), +angle = left (+X)
    float azimuth = glm::degrees(std::atan2(pos.x, -pos.z));

    // Calculate elevation (vertical angle)
    float horizontalDist = std::sqrt(pos.x * pos.x + pos.z * pos.z);
    float elevation = glm::degrees(std::atan2(pos.y, horizontalDist));

    return {azimuth, elevation, isLFE ? 1 : 0, 0};
}

void MathUtils::convertToCICP(const glm::vec3 cubePositions[6])
{
    std::cout << "6\n";
    for (int i = 0; i < 6; i++)
    {
        CICPGeometry geom = cartesianToCICP(cubePositions[i]);
        std::cout << "g,"
                  << std::round(geom.azimuth) << ","
                  << std::round(geom.elevation) << ","
                  << geom.isLFE << ","
                  << geom.screenRef << "\n";
    }
}