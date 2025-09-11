/***********************************************************************************
 
 This software module was originally developed by 
 
 IOSONO GmbH, Fraunhofer IIS
 
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
 
 IOSONO GmbH and Fraunhofer IIS retain full right to modify and use the code for its 
 own purpose, assign or donate the code to a third party and to inhibit third parties 
 from using the code for products that do not conform to MPEG-related ITU Recommen-
 dations and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

//-------------------------------------------------------------------------------------------------

// Local headers:
#include "vbap_core.hpp"
#include "functions.hpp"
#include "speakersetup.hpp"
#include "exception.hpp"

// std headers:
#include <iostream>
#include <sstream>
#include <algorithm>

//-------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace referencerenderer
{
using namespace geo;

//-------------------------------------------------------------------------------------------------

namespace
{

/// Minimum internal gain level; levels below this value indicate a source position outside of the corresponding loudspeaker triplet.
const float MIN_GAIN = -0.01f;


struct SpeakerSortData{
  SpeakerSortData()
    :index(0)
  {}
  unsigned index;
  PointSpherical position;
};

//-------------------------------------------------------------------------------------------------

bool azimuthGreater(const SpeakerSortData &s0, const SpeakerSortData &s1)
{
  return s0.position.azi < s1.position.azi;
}
} // nameless namespace


//-------------------------------------------------------------------------------------------------

VbapCore::VbapCore() :
    m_nSpeaker(0), m_is3dSetup(false), m_hasHeightSpeakers(false),  m_baseVectors3D(19)
{
}

//-------------------------------------------------------------------------------------------------

void VbapCore::init(
    const std::vector<geo::PointCartesian>& speakerArray,
    const std::vector<geo::VertexTriangle>& speakerTriangles,
    bool is3dSetup,
    bool hasHeightSpeakers)
{
  m_nSpeaker = speakerArray.size();
  m_gains.resize(m_nSpeaker);
  m_is3dSetup = is3dSetup;
  m_hasHeightSpeakers = hasHeightSpeakers;

  if (m_is3dSetup)
  {
    if (m_nSpeaker < 3)
    {
      THROW_MPEGH_EXCEPTION(iosono::mpeg::Exception, "3D setups must contain at least three speakers. ")
    }
  }
  else
  {
    if (m_nSpeaker < 2)
    {
      THROW_MPEGH_EXCEPTION(iosono::mpeg::Exception, "2D setups must contain at least two speakers. ")
    }
  }


  m_speakerPosition = speakerArray;
  if (! m_is3dSetup)
  {
    for (unsigned i = 0; i<m_nSpeaker; i++)
    {
      m_speakerPosition[i][2] = 0.f;
    }
  }

  if (m_is3dSetup )
  {
    // copy triangles
    m_lsTriplets.resize(speakerTriangles.size());
    for (std::size_t i = 0; i < speakerTriangles.size(); ++i)
    {
      const VertexTriangle& in(speakerTriangles[i]);
      LoudspeakerTriplet& out(m_lsTriplets[i]);
      out.index[0] = in.index[0];
      out.index[1] = in.index[1];
      out.index[2] = in.index[2];
    }
    fillMatrixes(m_lsTriplets);
  }
  else
  {
    // calculate new loudspeaker triplets
    calculatePairs(m_lsTriplets);
  }

  if (m_lsTriplets.empty())
  {
    THROW_MPEGH_EXCEPTION(iosono::mpeg::Exception, "Calculation of loudspeaker triplets failed.");
  }
}


//-------------------------------------------------------------------------------------------------


void VbapCore::calculcateVbap(PointCartesian source, float *final_gs, float gainFactor, float spread[4], int hasUniformSpread) const
{
  
  for (unsigned i=0; i<m_nSpeaker; i++)
  {
    m_gains[i] = 0.f;
  }
  
  if ( ((hasUniformSpread) && (spread[0] > 0.0f)) || ((hasUniformSpread == 0) && ((spread[1] > 0.0f) || (spread[2] > 0.0f))) )
  {
    // ** First part spreading (0°-90°) **
    calcSpreadVectors(source, spread, hasUniformSpread);

    // Iterate over all spread vectors
    for (std::size_t i = 0; i < m_baseVectors3D.size(); ++i)
    {
      calculcateOneSourcePosition(m_baseVectors3D[i]);
    } 
    
    // ** Second part spreading (90°-180°), includes gain normalization **
    if ( hasUniformSpread )
    {
      calcSpreadGains(spread[0]);
    }
    else
    {
      calcSpreadGains(spread[1]);
    }
  }
  else
  {
    calculcateOneSourcePosition(source);
  }
  
  // Final power normalization  
  normalizePower(&m_gains[0], m_gains.size());

  // ** Incorporate gainFactor **
  for (unsigned i=0; i<m_nSpeaker; i++)
  {
    final_gs[i] = m_gains[i] * gainFactor;
  }
}

//--------------------------------------------------------------------------------------------------

void VbapCore::calculcateOneSourcePosition(geo::PointCartesian source) const
{
  // check if we are in the middle of the speaker array
  float srcLength = source.length();
  const float c_minCenterDist = 0.01f;
  if(srcLength <= c_minCenterDist)
  {
    source[1] = c_minCenterDist;
  }

  if (m_lsTriplets.empty())
  {
    THROW_MPEGH_EXCEPTION(iosono::mpeg::Exception,
          "Coefficients cannot be calculated since there is no loudspeaker triplet available.");
  }

  unsigned winner_set = 0;
  float big_sm_g = -100000.f;   // initial value for largest minimum gain value
  unsigned best_neg_g_am = 3;   // how many negative values in this set
  float g[3] = {0.f, 0.f, 0.f};

  for (unsigned i=0; i<m_lsTriplets.size(); i++)
  {
    float small_g = 10000000.f;
    unsigned neg_g_am = 3;
    float gtmp[3];
    for (unsigned j=0; j<3; j++)
    {
      gtmp[j] = source.getVectorProduct(m_lsTriplets[i].matrixInverse[j]);
      if (gtmp[j] < small_g)
      {
        small_g = gtmp[j];
      }
      if (gtmp[j] >= MIN_GAIN)
      {
        neg_g_am--;
      }
    }

    if ((small_g > big_sm_g && neg_g_am <= best_neg_g_am) || (neg_g_am < best_neg_g_am))
    {
      big_sm_g = small_g;
      best_neg_g_am = neg_g_am;
      winner_set = i;
      g[0] = gtmp[0];
      g[1] = gtmp[1];
      g[2] = gtmp[2];
    }
  }

  // If chosen set produced a negative value, make it zero and
  // calculate direction that corresponds  to these new
  // gain values. This happens when the virtual source is outside of
  // all loudspeaker sets.
  bool gains_modified = false;

  for (unsigned i=0; i<3; i++)
  {
    if (g[i] < MIN_GAIN)
    {
      g[i]=0.f;
      gains_modified = true;
    }
  }

  if (gains_modified == true)
  {
    source.x =  m_lsTriplets[winner_set].matrix[0].x * g[0] +
                m_lsTriplets[winner_set].matrix[1].x * g[1] +
                m_lsTriplets[winner_set].matrix[2].x * g[2];
    source.y =  m_lsTriplets[winner_set].matrix[0].y * g[0] +
                m_lsTriplets[winner_set].matrix[1].y * g[1] +
                m_lsTriplets[winner_set].matrix[2].y * g[2];
    source.z =  m_lsTriplets[winner_set].matrix[0].z * g[0] +
                m_lsTriplets[winner_set].matrix[1].z * g[1] +
                m_lsTriplets[winner_set].matrix[2].z * g[2];
  }

  if (g[0] < 0.f) {g[0] = 0.f;} // small negative values might occur - set them to 0
  if (g[1] < 0.f) {g[1] = 0.f;}
  if (g[2] < 0.f) {g[2] = 0.f;}

  float power = std::sqrt(g[0]*g[0] + g[1]*g[1] + g[2]*g[2]);
  if ( power != 0.f)
  {
    g[0] /= power;
    g[1] /= power;
    g[2] /= power;
  }

  m_gains[m_lsTriplets[winner_set].index[0]] += g[0];
  m_gains[m_lsTriplets[winner_set].index[1]] += g[1];
  m_gains[m_lsTriplets[winner_set].index[2]] += g[2];

}

//-------------------------------------------------------------------------------------------------

std::size_t VbapCore::getNumGains() const
{
  return static_cast<std::size_t> (m_nSpeaker);
}

//--------------------------------------------------------------------------------------------------

void VbapCore::calcSpreadVectors(const geo::Vector3d& source, float spread[4], int hasUniformSpread) const
{
  PointSpherical dir;
  float az_el_spread_ratio = 1.0f;
  float temp_spread[4];

  temp_spread[0] = spread[0];
  temp_spread[1] = spread[1];
  temp_spread[2] = spread[2];
  temp_spread[3] = spread[3];

  source.toSpherical(dir);
  
  // First setting up base vectors
  Vector3d p0;
  Vector3d v;
  Vector3d u;
  // 3D Setup
  p0.setSpherical(dir.azi,dir.ele);
  if (dir.ele < 0.f)
  {
    dir.ele += 90.f;
  }
  else if (dir.ele >= 0.f)
  {
    dir.ele -= 90.f;
  }
  v.setSpherical(dir.azi, dir.ele);
  u.setCrossProduct(v, p0);    

  limit(temp_spread[0], rad(0.001f), rad(90.f));
  limit(temp_spread[1], rad(0.001f), rad(90.f));
  limit(temp_spread[2], rad(0.001f), rad(90.f));
  limit(temp_spread[3], 0.0f, 16.0f);

  if ( !hasUniformSpread )
  {
    az_el_spread_ratio = temp_spread[2] / temp_spread[1];
    v.set(v.x*az_el_spread_ratio, v.y*az_el_spread_ratio, v.z*az_el_spread_ratio);  
  }
    
  if (!m_hasHeightSpeakers)
  {
    // 2D Setup
    v.set(0.f, 0.f, 0.f);
  }
  //std::cout << "\n p0=" << p0 << ", v=" << v << ", U=" << u << ". ";
  
  // now creating all directions
  const Vector3d p7  =  0.500f*u + 0.866f*v + 0.333f*p0;
  const Vector3d p10 = -0.500f*u + 0.866f*v + 0.333f*p0;
  const Vector3d p13 = -0.500f*u - 0.866f*v + 0.333f*p0;
  const Vector3d p16 =  0.500f*u - 0.866f*v + 0.333f*p0;

  m_baseVectors3D[0]  =  1.000f*p0;

  m_baseVectors3D[1]  =  1.000f*u;
  m_baseVectors3D[4]  = -1.000f*u;
  
  m_baseVectors3D[2]  =  0.750f*u + 0.250f*p0; 
  m_baseVectors3D[5]  = -0.750f*u + 0.250f*p0;
  
  m_baseVectors3D[3]  =  0.375f*u + 0.625f*p0;
  m_baseVectors3D[6]  = -0.375f*u + 0.625f*p0;
  
  m_baseVectors3D[7]  = p7;
  m_baseVectors3D[10] = p10;
  m_baseVectors3D[13] = p13;
  m_baseVectors3D[16] = p16;
  
  m_baseVectors3D[8]  =  0.500f*p7  + 0.500f*p0;
  m_baseVectors3D[11] =  0.500f*p10 + 0.500f*p0;
  m_baseVectors3D[14] =  0.500f*p13 + 0.500f*p0;
  m_baseVectors3D[17] =  0.500f*p16 + 0.500f*p0;
  
  m_baseVectors3D[9]  =  0.250f*p7  + 0.750f*p0;
  m_baseVectors3D[12] =  0.250f*p10 + 0.750f*p0;
  m_baseVectors3D[15] =  0.250f*p13 + 0.750f*p0;
  m_baseVectors3D[18] =  0.250f*p16 + 0.750f*p0;
  
  for (std::size_t i = 0; i < m_baseVectors3D.size(); ++i)
  {
    if ( hasUniformSpread )
    {
      m_baseVectors3D[i] += p0 / std::tan(temp_spread[0]);
    }
    else
    {
      m_baseVectors3D[i] += p0 / std::tan(temp_spread[1]);
    }
  }
  
  // normalization is only done for usefull debug output
  for (std::size_t i = 0; i < m_baseVectors3D.size(); ++i)
  {
    m_baseVectors3D[i].normalize();
  }
}

//-------------------------------------------------------------------------------------------------

void VbapCore::calcSpreadGains(float spreadAngle) const
{
  float w = spreadAngle / rad(90.f) - 1.f;
  limit(w, 0.f, 1.f);

  float numSpeakers = static_cast<float>(m_gains.size());
  float norm = norm2(&m_gains[0], m_gains.size());
  float unitGain = 1.f / std::sqrt(numSpeakers);
  
  for (std::size_t i = 0; i < m_gains.size(); ++i)
  {
    m_gains[i] = (1.f - w) * m_gains[i] / norm   +   w * unitGain;
  }
}

//--------------------------------------------------------------------------------------------------

void VbapCore::calculatePairs(std::vector<LoudspeakerTriplet> &lsTriplets)
{
  assert(m_is3dSetup == false);

  lsTriplets.clear();

  // sort loudspeakers according to their azimuth angle
  std::vector<SpeakerSortData> sortedLs(m_nSpeaker);
  for (unsigned i = 0; i<m_nSpeaker; i++)
  {
    m_speakerPosition[i].toSpherical(sortedLs[i].position);
    sortedLs[i].index = i;
  }
  std::sort(sortedLs.begin(), sortedLs.end(), azimuthGreater);

  //adjacent loudspeakers are the loudspeaker pairs to be used
  for (unsigned i = 0; i < m_nSpeaker; i++)
  {
    float angleDifference = sortedLs[(i + 1) % m_nSpeaker].position.azi - sortedLs[i].position.azi;
    normalizeAngleDegPositiv(angleDifference);

    LoudspeakerTriplet triplet;
    triplet.index[0] = sortedLs[i].index;
    triplet.index[1] = sortedLs[(i + 1) % m_nSpeaker].index;
    triplet.index[2] = 0; // to avoid index errors - according gain values are always zero and do not affect the gain value of channel index 0
    triplet.matrix[0] = m_speakerPosition[sortedLs[i].index];
    triplet.matrix[1] = m_speakerPosition[sortedLs[(i + 1) % m_nSpeaker].index];
    lsTriplets.push_back(triplet);
  }

  // calculate inverse 2x2 matrix
  for (unsigned i = 0; i<lsTriplets.size(); i++)
  {
    PointSpherical p0, p1;
    lsTriplets[i].matrix[0].toSpherical(p0);
    lsTriplets[i].matrix[1].toSpherical(p1);
    float rad2ang = 360.f / ( 2.f * (float)M_PI );

    float a = std::cos(p0.azi / rad2ang);
    float b = std::sin(p0.azi / rad2ang);
    float c = std::cos(p1.azi / rad2ang);
    float d = std::sin(p1.azi / rad2ang);
    float det = (a * d) - (c * b);
    if(fabs(det) > 0.001)
    {
      lsTriplets[i].matrixInverse[0].x = (d / det);
      lsTriplets[i].matrixInverse[0].y = (-c / det);
      lsTriplets[i].matrixInverse[1].x = (-b / det);
      lsTriplets[i].matrixInverse[1].y = (a / det);
    }
  }
}

//--------------------------------------------------------------------------------------------------

void VbapCore::calculateInverseMatrix(const PointCartesian &p0,
                                                      const PointCartesian &p1,
                                                      const PointCartesian &p2,
                                                      PointCartesian inverseMatrix[3])
{
  float invdet = 1.0f / (  p0.x * ((p1.y * p2.z) - (p1.z * p2.y))
    - p0.y * ((p1.x * p2.z) - (p1.z * p2.x))
    + p0.z * ((p1.x * p2.y) - (p1.y * p2.x)));

  inverseMatrix[0].x = ((p1.y * p2.z) - (p1.z * p2.y)) * invdet;
  inverseMatrix[1].x = ((p0.y * p2.z) - (p0.z * p2.y)) * -invdet;
  inverseMatrix[2].x = ((p0.y * p1.z) - (p0.z * p1.y)) * invdet;
  inverseMatrix[0].y = ((p1.x * p2.z) - (p1.z * p2.x)) * -invdet;
  inverseMatrix[1].y = ((p0.x * p2.z) - (p0.z * p2.x)) * invdet;
  inverseMatrix[2].y = ((p0.x * p1.z) - (p0.z * p1.x)) * -invdet;
  inverseMatrix[0].z = ((p1.x * p2.y) - (p1.y * p2.x)) * invdet;
  inverseMatrix[1].z = ((p0.x * p2.y) - (p0.y * p2.x)) * -invdet;
  inverseMatrix[2].z = ((p0.x * p1.y) - (p0.y * p1.x)) * invdet;
}

//--------------------------------------------------------------------------------------------------

template <typename StlContainer>
void VbapCore::fillMatrixes(StlContainer &lsTriplets)
{
  for (typename StlContainer::iterator triplet = lsTriplets.begin(); triplet != lsTriplets.end(); triplet++)
  {
    const PointCartesian &ls0 = m_speakerPosition[triplet->index[0]];
    const PointCartesian &ls1 = m_speakerPosition[triplet->index[1]];
    const PointCartesian &ls2 = m_speakerPosition[triplet->index[2]];

    // fill matrix
    triplet->matrix[0] = ls0;
    triplet->matrix[1] = ls1;
    triplet->matrix[2] = ls2;

    calculateInverseMatrix(ls0, ls1, ls2, triplet->matrixInverse);
  }
}

//--------------------------------------------------------------------------------------------------

void VbapCore::infoToStream(std::ostream& strm) const
{
  strm << "%VbapSpeakerConfiguration:";
  strm << " m_nSpeaker = " << m_nSpeaker;
  strm << ", m_is3dSetup = " << m_is3dSetup;
  strm << ", m_hasHeightSpeakers = " << m_hasHeightSpeakers;
  
  sequenceToOstream(strm << "\nspkrVec", m_speakerPosition);
  sequenceToOstream(strm << "\nspreadVec", m_baseVectors3D);
  strm << "; ";
}

//-------------------------------------------------------------------------------------------------

} // namespace referencerenderer
} // namespace mpeg
} // namespace iosono
