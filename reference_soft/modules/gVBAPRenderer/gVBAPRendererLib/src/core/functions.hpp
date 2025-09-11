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

#ifndef __DEFAULT_RENDERER_FUNCTIONS_HPP__
#define __DEFAULT_RENDERER_FUNCTIONS_HPP__



//--------------------------------------------------------------------------------------------------

// Local headers:
#include "types.hpp"

// STL headers:
#include <ostream>
#include <vector>
#include <cmath>


//--------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace geo
{

const float rad2deg = static_cast<float>(180.0 / M_PI);
const float deg2rad = static_cast<float>( M_PI / 180.0);

//--------------------------------------------------------------------------------------------------

/**
 * Angle ops
 * @{
 */

inline float rad(float a) {return a * deg2rad;}
inline float deg(float a) {return a * rad2deg;}
void normalizeAngleRad(float & ang);
void normalizeAngleDeg(float & ang);
void normalizeAngleDegPositiv(float & ang);

/**
 * @}
 */

/**
 * Vector ops
 * @{
 */

float unsigned_angle(const Vector3d& lhs, const Vector3d& rhs);
Vector3d operator*(float lhs, const Vector3d& rhs);

/**
 * @}
 */




//--------------------------------------------------------------------------------------------------

inline void limit(float& valInOut, float min, float max)
{
  if (valInOut > max)
  {
    valInOut = max;
  }
  else if (valInOut < min)
  {
    valInOut = min;
  }
}


/**
 * Container ops
 * @{
 */

inline float norm1(const float * values, std::size_t num)
{
  float norm = 0.f;
  for (std::size_t i = 0; i < num; ++i)
  {
    norm += std::fabs(values[i]);
  }
  return norm;
}

inline float norm2(const float * values, std::size_t num)
{
  float norm = 0.f;
  for (std::size_t i = 0; i < num; ++i)
  {
    norm += values[i] * values[i];
  }
  norm = std::sqrt(norm);
  return norm;
}

inline void normalizeAmplitude(float * values, std::size_t num)
{
  float norm = norm1(values, num);
  if (norm > 0.f)
  {
    for (std::size_t i = 0; i < num; ++i)
    {
      values[i] /= norm;
    }
  }
}

inline void normalizePower(float * values, std::size_t num)
{
  float norm = norm2(values, num);

  if (norm > 0.f)
  {
    for (std::size_t i = 0; i < num; ++i)
    {
      values[i] /= norm;
    }
  }
}

template<class container>
std::ostream& sequenceToOstream(std::ostream& ostrm,
                                container array)
{
  std::streamsize oldPrec = ostrm.precision();
  std::ios::fmtflags oldSettings = ostrm.flags();
  int newPrec = 3;
  int width = newPrec + 3;
  ostrm.precision(newPrec);
  ostrm << std::left << std::fixed;
  std::size_t size = array.size();
  ostrm << " = [ ";
  if (size > 0)
  {
    ostrm.width(width);
    ostrm << array[0];
  }
  for (std::size_t i = 1; i < size; ++i)
  {
    ostrm << "; ";
    ostrm.width(width);
    ostrm << array[i];
  }
  ostrm << " ]; ";
  ostrm.precision(oldPrec);
  ostrm.flags(oldSettings);
  return ostrm;
}

/**
 * @}
 */


//--------------------------------------------------------------------------------------------------

} // namespace geo
} // namespace mpeg
} // namespace iosono


#endif
