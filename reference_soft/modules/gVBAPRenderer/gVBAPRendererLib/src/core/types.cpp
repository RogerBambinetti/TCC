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
#include "types.hpp"

// Stl headers:
#include <cmath>
#include <limits>
#include <iomanip>

//--------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace geo
{

//--------------------------------------------------------------------------------------------------

using namespace std;


//--------------------------------------------------------------------------------------------------

Vector2d::Vector2d()
  :x(0.f)
  ,y(0.f)
{}

//--------------------------------------------------------------------------------------------------

Vector2d::Vector2d(float _x, float _y)
  :x(_x)
  ,y(_y)
{}

//--------------------------------------------------------------------------------------------------

bool Vector2d::operator==(const Vector2d& rhs) const
{
  return (x == rhs.x) &&
         (y == rhs.y) ;
}

//--------------------------------------------------------------------------------------------------

void Vector2d::set(float _x, float _y)
{
  x = _x;
  y = _y;
}

//--------------------------------------------------------------------------------------------------

float Vector2d::operator[](std::size_t i) const
{
  return *(&x + i);
}

float& Vector2d::operator[](std::size_t i)
{
  return *(&x + i);
}

//--------------------------------------------------------------------------------------------------

Vector2d Vector2d::operator-(const Vector2d &subtrahend) const
{
  Vector2d result = *this;
  result.x-=subtrahend.x;
  result.y-=subtrahend.y;
  return result;
}

//--------------------------------------------------------------------------------------------------

Vector2d Vector2d::operator+(const Vector2d &summand) const
{
  Vector2d result = *this;
  result.x+=summand.x;
  result.y+=summand.y;
  return result;
}

//--------------------------------------------------------------------------------------------------

Vector2d Vector2d::operator/(const float divisor) const
{
  Vector2d result = *this;
  result.x /= divisor;
  result.y /= divisor;
  return result;
}

//--------------------------------------------------------------------------------------------------

Vector2d Vector2d::operator*(const float scale) const
{
  Vector2d result = *this;
  result.x *= scale;
  result.y *= scale;
  return result;
}

//--------------------------------------------------------------------------------------------------

float Vector2d::getLength() const
{
  return (std::sqrt(x*x + y*y));
}

//--------------------------------------------------------------------------------------------------

void Vector2d::normalize()
{
  float power = getLength();
  if (power != 0.f)
  {
    x /= power;
    y /= power;
  }
}

//--------------------------------------------------------------------------------------------------

float Vector2d::unsigned_angle_2D(const Vector2d &v2) const
{
  float inner = dot(v2) / (getLength() * v2.getLength());

  if (inner >= 1.f)
  {
    return 0.f;
  }
  if (inner <= -1.f)
  {
    return static_cast<float>(M_PI);
  }
  return std::fabs(std::acos(inner));
}

//--------------------------------------------------------------------------------------------------

float Vector2d::dot(const Vector2d &v2) const
{
  return (x*v2.x + y*v2.y);
}



//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------



PointCartesian::PointCartesian()
  :x(0.f)
  ,y(0.f)
  ,z(0.f)
{}

//--------------------------------------------------------------------------------------------------

PointCartesian::PointCartesian(float _x, float _y, float _z)
  :x(_x)
  ,y(_y)
  ,z(_z)
{}

//--------------------------------------------------------------------------------------------------

PointCartesian::PointCartesian(const SphericalPosition& pos)
{
  *this = pos;
}

//--------------------------------------------------------------------------------------------------

PointCartesian& PointCartesian::operator=(const SphericalPosition& pos)
{

  float radius = pos.radius;
  // min() gives the minimal non denormalized float value. We scale with 2 to keep precise the direction.
  const float minR = std::numeric_limits<float>::min() * 2.f;
  if (radius < minR)
  {
    radius = minR;
  }
  const float zW = std::cos(pos.ele);
  x = radius * std::cos(pos.azi) * zW;
  y = radius * std::sin(pos.azi) * zW;
  z = radius * std::sin(pos.ele);
  return *this;
}

//--------------------------------------------------------------------------------------------------

bool PointCartesian::operator==(const PointCartesian& rhs) const
{
  return (x == rhs.x) &&
         (y == rhs.y) &&
         (z == rhs.z);
}

//--------------------------------------------------------------------------------------------------

void PointCartesian::set(float _x, float _y, float _z)
{
  x = _x;
  y = _y;
  z = _z;
}

//--------------------------------------------------------------------------------------------------

float PointCartesian::operator[](std::size_t i) const
{
  return *(&x + i);
}

float& PointCartesian::operator[](std::size_t i)
{
  return *(&x + i);
}

//--------------------------------------------------------------------------------------------------

PointCartesian PointCartesian::operator-(const PointCartesian &subtrahend) const
{
  PointCartesian result = *this;
  result.x-=subtrahend.x;
  result.y-=subtrahend.y;
  result.z-=subtrahend.z;
  return result;
}

//--------------------------------------------------------------------------------------------------

PointCartesian PointCartesian::operator+(const PointCartesian &summand) const
{
  PointCartesian result = *this;
  result.x+=summand.x;
  result.y+=summand.y;
  result.z+=summand.z;
  return result;
}

//--------------------------------------------------------------------------------------------------

PointCartesian PointCartesian::operator+=(const PointCartesian &summand)
{
  x+=summand.x;
  y+=summand.y;
  z+=summand.z;
  return *this;
}

//--------------------------------------------------------------------------------------------------

PointCartesian PointCartesian::operator/(const float divisor) const
{
  PointCartesian result = *this;
  result.x /= divisor;
  result.y /= divisor;
  result.z /= divisor;
  return result;
}

//--------------------------------------------------------------------------------------------------

PointCartesian PointCartesian::operator*(const float scale) const
{
  PointCartesian result = *this;
  result.x *= scale;
  result.y *= scale;
  result.z *= scale;
  return result;
}

//--------------------------------------------------------------------------------------------------

float PointCartesian::getLength() const
{
  return (std::sqrt(x*x + y*y + z*z));
}

//--------------------------------------------------------------------------------------------------

void PointCartesian::normalize()
{
  float power = getLength();
  if (power != 0.f)
  {
    x /= power;
    y /= power;
    z /= power;
  }
}

//--------------------------------------------------------------------------------------------------

float PointCartesian::getAngleDifference(const PointCartesian &v2) const
{
  float inner = getVectorProduct(v2) / (getLength() * v2.getLength());

  if (inner >= 1.f)
  {
    return 0.f;
  }
  if (inner <= -1.f)
  {
    return static_cast<float>(M_PI);
  }
  return std::fabs(std::acos(inner));
}

//--------------------------------------------------------------------------------------------------

float PointCartesian::dot(const PointCartesian &v2) const
{
  return (x*v2.x + y*v2.y + z*v2.z);
}

//--------------------------------------------------------------------------------------------------

PointCartesian PointCartesian::cross(const PointCartesian &v2) const
{
  Vector3d result((y * v2.z ) - (z * v2.y),
                  (z * v2.x ) - (x * v2.z),
                  (x * v2.y ) - (y * v2.x));
  return result;
}

//--------------------------------------------------------------------------------------------------

void PointCartesian::setCrossProduct(const PointCartesian &v1, const PointCartesian &v2)
{
  x = (v1.y * v2.z ) - (v1.z * v2.y);
  y = (v1.z * v2.x ) - (v1.x * v2.z);
  z = (v1.x * v2.y ) - (v1.y * v2.x);
  normalize();
}

//--------------------------------------------------------------------------------------------------

void PointCartesian::setSpherical(float azimuth, float elevation)
{
  float atorad = 2.f * (float)M_PI / 360.f;
  x = std::cos(azimuth * atorad) * std::cos(elevation * atorad);
  y = std::sin(azimuth * atorad) * std::cos(elevation * atorad);
  z = std::sin(elevation * atorad);
}

//--------------------------------------------------------------------------------------------------

void PointCartesian::toSpherical(PointSpherical &ang) const
{
  float atorad = 2.f * (float)M_PI / 360.f;
  float atan_y_per_x = 0.f;
  float atan_x_pl_y_per_z = 0.f;

  // azimuth
  if (x == 0.f)
  {
    if (y >= 0.f)
    {
      atan_y_per_x = (float)M_PI/2.f;
    }
    else
    {
      atan_y_per_x = (float)M_PI/-2.f;
    }
  }
  else
  {
    atan_y_per_x = std::atan(y / x);
  }
  float azimuth = atan_y_per_x / atorad;
  if (x < 0.f)
  {
    azimuth += 180.f;
  }

  // elevation
  float dist = std::sqrt(x*x + y*y);
  if (z == 0.f)
  {
    atan_x_pl_y_per_z = 0.f;
  }
  else
  {
    atan_x_pl_y_per_z = std::atan(z / dist);
  }
  if (dist == 0.f)
  {
    if (z < 0.f)
    {
      atan_x_pl_y_per_z = (float)M_PI/-2.f;
    }
    else
    {
      atan_x_pl_y_per_z = (float)M_PI/2.f;
    }
  }
  float elevation = atan_x_pl_y_per_z / atorad;

  if (azimuth >= 180.f)
  {
    azimuth -= 360.f;
  }

  ang.azi = azimuth;
  ang.ele = elevation;
}

//--------------------------------------------------------------------------------------------------

void PointCartesian::posToStream(std::ostream& strm) const
{
  strm << x << " " << y << " " << z;
}

//--------------------------------------------------------------------------------------------------

SphericalPosition::SphericalPosition(const PointCartesian& vec)
{
  *this = vec;
}

//--------------------------------------------------------------------------------------------------

void SphericalPosition::set(float a, float e, float r)
{
  azi = a;
  ele = e;
  radius = r;
}

//--------------------------------------------------------------------------------------------------

void SphericalPosition::setDeg(float a, float e, float r)
{
  const float d2r(static_cast<float>(M_PI / 180.0));
  set(a * d2r, e * d2r, r);
}


//--------------------------------------------------------------------------------------------------

SphericalPosition& SphericalPosition::operator=(const PointCartesian& vec)
{
  radius = vec.length();

  float rXY = std::sqrt(vec[0]*vec[0] + vec[1]*vec[1]);
  azi = std::atan2(vec[1], vec[0]);
  ele = std::atan2(vec[2], rXY );

  return *this;
}

//--------------------------------------------------------------------------------------------------

void SphericalPosition::deg2rad()
{
  const float d2r(static_cast<float>(M_PI / 180.0));
  azi *= d2r;
  ele *= d2r;
}

//--------------------------------------------------------------------------------------------------

void SphericalPosition::rad2deg()
{
  const float r2d(static_cast<float>(180.0 / M_PI));
  azi *= r2d;
  ele *= r2d;
}

//--------------------------------------------------------------------------------------------------

Matrix::Matrix()
{

}

//--------------------------------------------------------------------------------------------------
Matrix::Matrix(std::size_t rows, std::size_t columns)
{
  resize(rows,columns);
}

//--------------------------------------------------------------------------------------------------

void Matrix::resize(std::size_t rows, std::size_t columns)
{
  matrix.resize(rows);
  for (std::size_t i(0); i < rows; ++i)
  {
    matrix[i].resize(columns);
  }
}

//--------------------------------------------------------------------------------------------------

float& Matrix::operator()(std::size_t row, std::size_t column)
{
  return matrix[row][column];
}

//--------------------------------------------------------------------------------------------------

const float& Matrix::operator()(std::size_t row, std::size_t column) const
{
  return matrix[row][column];
}

//--------------------------------------------------------------------------------------------------

std::size_t Matrix::rows() const
{
  return matrix.size();
}

//--------------------------------------------------------------------------------------------------

std::size_t Matrix::columns() const
{
  if(matrix.size())
  {
    return matrix[0].size();
  }
  return 0;
}

//--------------------------------------------------------------------------------------------------

std::ostream& Matrix::infoToStream(std::ostream& strm) const
{
  strm << " = [\n";
  for (std::size_t row = 0; row < rows(); ++row)
  {
    for (std::size_t col = 0; col < columns(); ++col)
    {
      float val = this->operator()(row, col);
      strm.width(8);
      strm.precision(3);
      strm << std::left << val;

    }
    strm << ";\n";
  }
  strm << "]; ";
  return strm;
}

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const SphericalPosition& obj)
{
  const float r2d = static_cast<float>(180.0 / M_PI);
  std::streamsize oldPrec = strm.precision();
  std::streamsize angleWidth = 6;
  std::streamsize anglePrec = 1;
  std::streamsize distWidth = 6;
  std::streamsize distPrec = 3;
  strm.precision(anglePrec);
  strm << "a=" << std::setw(angleWidth) << obj.azi * r2d << "° e="<< std::setw(angleWidth) << obj.ele * r2d << "° ";
  strm.precision(distPrec);
  strm << "r=" << std::setw(distWidth) << obj.radius << "m ";
  strm.precision(oldPrec);
  return strm;
}

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const PointSpherical& obj)
{
  std::streamsize oldPrec = strm.precision();
  std::streamsize angleWidth = 6;
  std::streamsize anglePrec = 1;
  strm.precision(anglePrec);
  strm << "A=" << std::setw(angleWidth) << obj.azi << "°, E=" << std::setw(angleWidth) << obj.ele << "°";
  strm.precision(oldPrec);
  return strm;
}

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const VertexTriangle& obj)
{
  strm << " " << obj.index[0] << " " << obj.index[1] << " " << obj.index[2];
  return strm;
}

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const std::vector<VertexTriangle>& obj)
{
  strm << " = [";
  for (std::size_t i = 0; i < obj.size(); ++i)
  {
    strm << obj[i] << "; ";
  }
  strm << " ]; ";
  return strm;
}

//--------------------------------------------------------------------------------------------------

std::ostream& operator<<(std::ostream& strm, const Matrix& obj)
{
  strm << " = [";
  for (std::size_t row(0); row < obj.rows(); ++row)
  {
    for (std::size_t col(0); col < obj.columns(); ++col)
    {
        strm << obj(row,col);
        if ((col + 1) < obj.columns())
        {
          strm << ", ";
        }
    }
    strm << "; ";
  }
  strm << " ]; ";
  return strm;
}


//--------------------------------------------------------------------------------------------------

} // namespace geo
} // namespace mpeg
} // namespace iosono
