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

#ifndef __DEFAULT_RENDERER_TYPES_HPP__
#define __DEFAULT_RENDERER_TYPES_HPP__


//--------------------------------------------------------------------------------------------------


// STL headers:
#include <iostream>
#include <vector>

//--------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace geo
{

//--------------------------------------------------------------------------------------------------


/**
* @brief Point (or vector) in spherical coordinates.
* @note Since VBAP does not use the distance of speaker or source positions, this struct does not contain an
* element to keep the distance of a point (or length of a vector). Conversions from PointSpherical to PointCartesian
* and vice versa are based on a distance/length of 1.
*/
struct PointSpherical
{
  PointSpherical()
    :azi(0.f)
    ,ele(0.f)
  {}
  /// azimuth, horizontal angle (degrees)
  float azi;
  /// elevation, vertical angle (degrees)
  float ele;
};

// Forward decl
struct SphericalPosition;

//--------------------------------------------------------------------------------------------------

/**
 * @brief Point (or vector) in cartesian coordinates.
  */
struct Vector2d
{
  /// x position
  float x;
  /// y position
  float y;

  /**
   * Standard Constructor
   */
  Vector2d();

  /**
   * Init Constructor
   * @param _x
   * @param _y
   * @param _z
   */
  Vector2d(float _x, float _y);


  /**
   * Bit exact comparison of two vectors.
   * @param rhs
   * @return
   */
  bool operator==(const Vector2d& rhs) const;
  inline bool operator!=(const Vector2d& rhs) const{ return ! operator==(rhs);}

  void set(float _x, float _y);
  float operator[](std::size_t) const;
  float& operator[](std::size_t);

  Vector2d operator-(const Vector2d &subtrahend) const;
  Vector2d operator+(const Vector2d &summand) const;
  Vector2d operator/(const float divisor) const;
  Vector2d operator*(const float scale) const;

  /// normalizes the vector
  void normalize();

  /// calculates the length of the vector
  float getLength() const;
  inline float length() const {return getLength();}

  /// calculates the angle between this and another vector
  float unsigned_angle_2D(const Vector2d &v2) const;

  /// calculates the vector product of this and another vectors
  float dot(const Vector2d &v2) const;


  /// outputs the position
  void toStream(std::ostream& strm) const;
};


//--------------------------------------------------------------------------------------------------

/**
 * @brief Point (or vector) in cartesian coordinates.
  */
struct PointCartesian
{
  /// x position
  float x;
  /// y position
  float y;
  /// z position
  float z;

  /**
   * Standard Constructor
   */
  PointCartesian();

  /**
   * Init Constructor
   * @param _x
   * @param _y
   * @param _z
   */
  PointCartesian(float _x, float _y, float _z);

  /**
   * Conversion constructor.
   * @param pos The spherical coordinate to use for initialisation.
   */
  explicit PointCartesian(const SphericalPosition& pos);

  /**
   * Assignment / conversion operator for SphericalPosition
   * @param pos
   * @return
   */
  PointCartesian& operator=(const SphericalPosition& pos);

  /**
   * Bit exact comparison of two vectors.
   * @param rhs
   * @return
   */
  bool operator==(const PointCartesian& rhs) const;
  inline bool operator!=(const PointCartesian& rhs) const{ return ! operator==(rhs);}

  void set(float _x, float _y, float _z);
  float operator[](std::size_t) const;
  float& operator[](std::size_t);

  PointCartesian operator-(const PointCartesian &subtrahend) const;
  PointCartesian operator+(const PointCartesian &summand) const;
  PointCartesian operator+=(const PointCartesian &summand);
  PointCartesian operator/(const float divisor) const;
  PointCartesian operator*(const float scale) const;

  /// normalizes the vector
  void normalize();

  /// calculates the length of the vector
  float getLength() const;
  inline float length() const {return getLength();}

  /// calculates the angle between this and another vector
  float getAngleDifference(const PointCartesian &v2) const;

  /// calculates the vector product of this and another vectors
  float dot(const PointCartesian &v2) const;
  inline float getVectorProduct(const PointCartesian &v2) const {return dot(v2);}

  /// Calculates the cross product with v2
  PointCartesian cross(const PointCartesian &v2) const;

  /// set the normalized cross product of two vectors as new value
  void setCrossProduct(const PointCartesian &v1, const PointCartesian &v2);

  /// converts spherical (in degrees) to cartesian coordinates and set them as new value
  void setSpherical(float azimuth, float elevation);

  /// converts cartesian to spherical coordinates (given in degrees)
  void toSpherical(PointSpherical &ang) const;

  /// outputs the position
  void posToStream(std::ostream& strm) const;
};

typedef PointCartesian Vector3d;

//--------------------------------------------------------------------------------------------------

struct SphericalPosition
{
  /**
   * Standard constructor
   */
  SphericalPosition()
    :azi(0.f)
    ,ele(0.f)
    ,radius(0.f)
  {}

  /**
   * Constructor
   *
   * @param[in] a Azimuth in radiant
   * @param[in] e Elevation in radiant
   * @param[in] r Radius in meters
   */
  SphericalPosition(float a, float e, float r)
    :azi(a)
    ,ele(e)
    ,radius(r)
  {
  }

  /**
   * Constructor. Converts from cartesian to sphere coordinates.
   *
   * @param vec Cartesian position vector
   */
  explicit SphericalPosition(const PointCartesian& vec);

  /**
   * Sets new position
   *
   * @param[in] a Azimuth in radiant
   * @param[in] e Elevation in radiant
   * @param[in] r Radius in meters
   */
  void set(float a, float e, float r);

  /**
   * Sets new position
   *
   * @param[in] a Azimuth in degree
   * @param[in] e Elevation in degree
   * @param[in] r Radius in meters
   */
  void setDeg(float a, float e, float r);

  /**
   * Assignment operator.
   */
  SphericalPosition& operator=(const PointCartesian& vec);

  /**
   * Scales the internal angle values from degree to radiant
   */
  void deg2rad();

  /**
   * Scales the internal angle values from radiant  to degree
   */
  void rad2deg();

  /// azimuth, horizontal angle (radiant)
  float azi;
  /// elevation, vertical angle (radiant)
  float ele;

  /// radius (meter)
  float radius;
};

//--------------------------------------------------------------------------------------------------

struct VertexTriangle
{
  VertexTriangle()
  {
    index[0] = 0;
    index[1] = 0;
    index[2] = 0;
  }

  VertexTriangle(std::size_t i1, std::size_t i2, std::size_t i3)
  {
    index[0] = i1;
    index[1] = i2;
    index[2] = i3;
  }
  /// contains indices of avertex array which form a triangle.
  std::size_t index[3];
};

//--------------------------------------------------------------------------------------------------

class Matrix
{
public:
  Matrix();

  Matrix(std::size_t rows, std::size_t columns);

  void resize(std::size_t rows, std::size_t columns);

  float& operator()(std::size_t row, std::size_t column);
  const float& operator()(std::size_t row, std::size_t column) const;

  std::size_t rows() const;
  std::size_t columns() const;
  
  std::ostream& infoToStream(std::ostream& strm) const;

private:

  std::vector<std::vector<float> > matrix;
};

//--------------------------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& strm, const PointCartesian& obj)
{
  obj.posToStream(strm);
  return strm;
}

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const SphericalPosition& obj);

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const PointSpherical& obj);

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const VertexTriangle& obj);

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const std::vector<VertexTriangle>& obj);

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const Matrix& obj);

//--------------------------------------------------------------------------------------------------

} // namespace geo
} // namespace mpeg
} // namespace iosono

#endif
