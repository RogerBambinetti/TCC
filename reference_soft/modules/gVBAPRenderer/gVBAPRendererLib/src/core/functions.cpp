/***********************************************************************************
 
 This software module was originally developed by 
 
 IOSONO GmbH
 
 in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. Those intending to use this 
 software module in products are advised that its use may infringe existing patents. 
 ISO/IEC have no liability for use of this software module or modifications thereof. 
 Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
 standard.
 
 IOSONO GmbH retains full right to modify and use the code for its own purpose,
 assign or donate the code to a third party and to inhibit third parties from using 
 the code for products that do not conform to MPEG-related ITU Recommendations and/or 
 ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

// Local headers:
#include "functions.hpp"

// util headers:
#include "exception.hpp"

// Stl headers:
#include <cmath>
#include <limits>

//--------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace geo
{

namespace
{
  //usefull constants
  const float pi05(static_cast<float>(M_PI * 0.5));
  const float pi = static_cast<float>(M_PI);
  const float pi2 = static_cast<float>(M_PI * 2.0);
}

//--------------------------------------------------------------------------------------------------

void normalizeAngleRad(float & ang)
{
  ang += pi;
  ang = ang - std::floor(ang / pi2) * pi2;
  ang -=pi;
}

//--------------------------------------------------------------------------------------------------

void normalizeAngleDeg(float & ang)
{
  ang += 180.f;
  ang = ang - std::floor(ang / 360.f) * 360.f;
  ang -=180.f;
}

//--------------------------------------------------------------------------------------------------

void normalizeAngleDegPositiv(float & ang)
{
  ang = ang - std::floor(ang / 360.f) * 360.f;
}

//--------------------------------------------------------------------------------------------------

float unsigned_angle(const Vector3d& lhs, const Vector3d& rhs)
{
  float dot = lhs.dot(rhs);
  dot /= lhs.getLength() * rhs.getLength();
  if (dot > 1.f)
  {
    dot = 1.f;
  }
  else if (dot < -1.f)
  {
    dot = -1.f;
  }
  float angle = std::acos(dot);
  return angle;
}

//--------------------------------------------------------------------------------------------------

Vector3d operator*(float lhs, const Vector3d& rhs)
{
  return rhs * lhs; 
}

//--------------------------------------------------------------------------------------------------

} // namespace geo
} // namespace mpeg
} // namespace iosono
