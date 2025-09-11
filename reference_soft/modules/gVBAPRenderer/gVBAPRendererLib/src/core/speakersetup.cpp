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
 
 IOSONO GmbH and Fraunhofer IIS retain full right to modify and use the code for its own
 purpose, assign or donate the code to a third party and to inhibit third parties from
 using the code for products that do not conform to MPEG-related ITU Recommendations
 and/or ISO/IEC International Standards.
 
 This copyright notice must be included in all copies or derivative works. 
 
 Copyright (c) ISO/IEC 2013.
 
 ***********************************************************************************/

//-------------------------------------------------------------------------------------------------

// Local headers:
#include "speakersetup.hpp"
#include "functions.hpp"
#include "exception.hpp"

// quick hull
#include "quickHull.h"

// STL headers:
#include <cstdlib>

//-------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace referencerenderer
{

//-------------------------------------------------------------------------------------------------

using namespace std;

//-------------------------------------------------------------------------------------------------
namespace
{

const float DefaultRadius(1.f);

}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

const int Speakers::s_heightSpeakers[] =
{ CH_U_L030, 
  CH_U_R030, 
  CH_U_000, 
  CH_U_L135, 
  CH_U_R135, 
  CH_U_180,
  CH_U_L090,
  CH_U_R090, 
  CH_T_000, 
  CH_U_L110, 
  CH_U_R110, 
  CH_U_L045, 
  CH_U_R045, 
    };
const int Speakers::s_numHeightSpeakers = 13; 

const int Speakers::s_index[]    = {   0,   1,   2,   3,    4,    5,   6,   7,    8,    9,   10,  11,  12,    13,   14,   15,   16,   17,   18,   19,   20,   21,   22,   23,   24,   25,   26,   27,   28,   29,   30,   31,   32,   33,   34,   35,   36 };
const int Speakers::s_azi[]      = { +30, -30,   0,   0, +110, -110, +22, -22, +135, -135,  180, 999, 999,   +90,  -90,  +60,  -60,  +30,  -30,    0, +135, -135,  180,  +90,  -90,    0,  +45,  +45,  -45,    0, +110, -110,  +45,  -45,  +45,  -45,  -45 };
const int Speakers::s_ele[]      = {   0,   0,   0, -15,    0,    0,   0,   0,    0,    0,    0, 999, 999,     0,    0,    0,    0,  +35,  +35,  +35,  +35,  +35,  +35,  +35,  +35,  +90,  -15,  -15,  -15,  -15,  +35,  +35,  +35,  +35,    0,    0,  -15 };
const int Speakers::s_aziStart[] = { +23, -37,  -7, -22, +101, -124,  +8, -22,  125, -142,  158, 999, 999,   +76, -100,  +53,  -75,  +11,  -37,  -10, +125, -157, +158,  +67, -100, -180,  +23,  +11,  -75,  -10, +101, -124,  +38,  -66,  +38,  -52, -180 };
const int Speakers::s_aziEnd[]   = { +37, -23,  +7, +22, +124, -101, +22,  -8,  142, -125, -158, 999, 999,  +100,  -76,  +75,  -53,  +37,  -11,  +10, +157, -125, -158, +100,  -67, +180, +180,  +75,  -11,  +10, +124, -101,  +66,  -38,  +52,  -38,  -23 };
const int Speakers::s_eleStart[] = {  -9,  -9,  -9, -90,  -45,  -45,  -9,  -9,  -45,  -45,  -45, 999, 999,   -45,  -45,   -9,   -9,  +21,  +21,  +21,  +21,  +21,  +21,  +21,  +21,  +61,  -90,  -45,  -45,  -45,  +21,  +21,  +21,  +21,   -9,   -9,  -90 };
const int Speakers::s_eleEnd[]   = { +20, +20, +20, +90,  +20,  +20, +20, +20,  +20,  +20,  +20, 999, 999,   +20,  +20,  +20,  +20,  +60,  +60,  +60,  +60,  +60,  +60,  +60,  +60,  +90,  +90,  -10,  -10,  -10,  +60,  +60,  +60,  +60,  +20,  +20,  +90 };
const int Speakers::s_isLfe[]    = {   0,   0,   0,   1,    0,    0,   0,   0,    0,    0,    0,   0,   0,     0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1,    0,    0,    0,    0,    0,    0,    0,    0,    0,    1 };
const int Speakers::s_isValid[]  = {   1,   1,   1,   1,    1,    1,   1,   1,    1,    1,    1,   0,   0,     1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1,    1 };

//--------------------------------------------------------------------------------------------------

Speakers::Value Speakers::classifySpeaker(int azi, int ele, int isLfe)
{
  Value result = CH_EMPTY;
  for (int i = 0; i < NUM_SPKR_TYPES; ++i)
  {
    if (!s_isValid[i])
    {
      continue;
    }
    if (s_isLfe[i] == isLfe)
    {
      bool aziMatches = false;
      bool eleMatches = false;
      if (s_aziEnd[i] >= s_aziStart[i])
      {
        aziMatches = azi <= s_aziEnd[i] && azi >= s_aziStart[i];
      }
      else
      {
        aziMatches = azi <= s_aziEnd[i] || azi >= s_aziStart[i];        
      }
      
      if (s_eleEnd[i] >= s_eleStart[i])
      {
        eleMatches = ele <= s_eleEnd[i] && ele >= s_eleStart[i];
      }
      else
      {
        eleMatches = ele <= s_eleEnd[i] || ele >= s_eleStart[i];        
      }
      
      if (aziMatches && eleMatches)
      {
        result = static_cast<Value>(i);
      }
    }
  }
  
  return result;
}

//--------------------------------------------------------------------------------------------------

bool Speakers::getAttributes(int type, int& azi, int& ele, int& isLfe)
{
  if (type >= NUM_SPKR_TYPES || type <=CH_EMPTY)
  {
    return false;
  }
  if (s_isValid[type] < 1)
  {
    return false;
  }
  azi = s_azi[type];
  ele = s_ele[type];
  isLfe = s_isLfe[type];
  return true;
}

//--------------------------------------------------------------------------------------------------

std::string Speakers::toString(Speakers::Value val)
{
  switch(val)
  {
    case CH_M_000 : return "CH_M_000";
    case CH_M_L022 : return "CH_M_L022";
    case CH_M_R022 : return "CH_M_R022";
    case CH_M_L030 : return "CH_M_L030";
    case CH_M_R030 : return "CH_M_R030";
    case CH_M_L045 : return "CH_M_L045";
    case CH_M_R045 : return "CH_M_R045";
    case CH_M_L060 : return "CH_M_L060";
    case CH_M_R060 : return "CH_M_R060";
    case CH_M_L090 : return "CH_M_L090";
    case CH_M_R090 : return "CH_M_R090";
    case CH_M_L110 : return "CH_M_L110";
    case CH_M_R110 : return "CH_M_R110";
    case CH_M_L135 : return "CH_M_L135";
    case CH_M_R135 : return "CH_M_R135";
    case CH_M_180 : return "CH_M_180";
    
    case CH_U_000 : return "CH_U_000";
    case CH_U_L030 : return "CH_U_L030";
    case CH_U_R030 : return "CH_U_R030";
    case CH_U_L045 : return "CH_U_L045";
    case CH_U_R045 : return "CH_U_R045";
    case CH_U_L090 : return "CH_U_L090";
    case CH_U_R090 : return "CH_U_R090";
    case CH_U_L110 : return "CH_U_L110";
    case CH_U_R110 : return "CH_U_R110";
    case CH_U_L135 : return "CH_U_L135";
    case CH_U_R135 : return "CH_U_R135";
    case CH_U_180 : return "CH_U_180";
    
    case CH_T_000 : return "CH_T_000";
    
    case CH_L_000 : return "CH_L_000";
    case CH_L_L045 : return "CH_L_L045";
    case CH_L_R045 : return "CH_L_R045";
    case CH_LFE1 : return "CH_LFE1";
    case CH_LFE2 : return "CH_LFE2";
    case CH_LFE3 : return "CH_LFE3";
    
    case CH_DUMMY_11 : return "CH_DUMMY_11";
    case CH_DUMMY_12 : return "CH_DUMMY_12";
    case CH_EMPTY : return "CH_EMPTY";
    case NUM_SPKR_TYPES :
      THROW_MPEGH_EXCEPTION(Exception, "Speakertype is set to NUM_SPKR_TYPES')");
      break;
    case USER_DEFINED : return "User Defined";

  }

  THROW_MPEGH_EXCEPTION(Exception, "Invalid Speakertype')");
  return "";
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

Speaker::Speaker(int az,
        int el,
        bool islfe,
        bool isGhost):
  lfe(islfe), ghost(isGhost)
{
  azimuth = static_cast<float>(az);
  elevation = static_cast<float>(el);
  int isLFE = (islfe) ? 1 : 0;
  id = Speakers::classifySpeaker(az, el,isLFE);
}

//--------------------------------------------------------------------------------------------------

geo::Vector3d Speaker::toVector() const
{
  geo::Vector3d vec;
  vec.setSpherical(azimuth, elevation);
  return vec;
}

//--------------------------------------------------------------------------------------------------

geo::SphericalPosition Speaker::toSphrPosRad() const
{
  geo::SphericalPosition ret(azimuth, elevation, 1.f);
  ret.deg2rad();
  return ret;
}

//--------------------------------------------------------------------------------------------------

std::ostream& Speaker::toStream(std::ostream& strm) const
{
  strm << Speakers::toString(id) << "(" << azimuth << "°," << elevation << "°)";
  return strm;
}

//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------

Speakersetup::Speakersetup() :
    m_coversUpperHalfSphere(false), m_coversLowerHalfSphere(false)
{
}

//--------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------

void Speakersetup::init(CICP2GEOMETRY_CHANNEL_GEOMETRY* geometry, int nSpeakers)
{
  using namespace geo;

  m_speakers.clear();
  m_outputMapping.clear();
  m_speakerVertices.clear();

  for ( int i = 0; i < nSpeakers; ++i )
  { /* non-LFE channels */
    if(geometry[i].LFE <= 0){
        m_speakers.push_back( Speaker(geometry[i].Az, geometry[i].El, false));
        m_outputMapping.push_back(i);
    }
  }

  for ( int i = 0; i < nSpeakers; ++i )
  { /* LFEs */
    if(geometry[i].LFE > 0){
        m_speakers.push_back( Speaker( geometry[i].Az, geometry[i].El, true));
        m_outputMapping.push_back(i);
    }
  }
  
  postInit();
}

//-------------------------------------------------------------------------------------------------

void Speakersetup::postInit()
{
  // Check if setup has height speakers
  
  // triangulation / ghost speakers / downmix matrix is generated  
  matrix* downmix_mat = qh_matrix_new(0,0);

  {
    const int cone(0);

    vertexList* vL = qh_vertexList_new(0);
    triangleList* tL = qh_triangleList_new(0);


    std::vector<float> az,el;
    generateAngles(az,el);

    /* Initialise vertex list  */
    qh_gen_VertexList(az.size(), &az[0], &el[0], cone, vL);
    /* Triangulation and Downmix matrix */
    qh_sphere_triangulation(vL, tL, downmix_mat);

    // Convert triangle list
    for (int i(0); i < tL->size; ++i)
    {
      triangle tr = tL->get(tL, i);
      m_speakerTriangles.push_back(geo::VertexTriangle(tr.index[0], tr.index[1], tr.index[2]));
    }

    // create speaker vertices for rendering
    for (int i(0); i < vL->size; ++i)
    {
      vertex spk = vL->get(vL, i);

      float azimuth(spk.azi);

      geo::normalizeAngleDegPositiv(azimuth);

      geo::SphericalPosition input(azimuth, spk.ele, DefaultRadius);

      input.deg2rad();
      geo::Vector3d output(input);
      m_speakerVertices.push_back(output);
    }

    /* Destroy lists and free all memory */
    qh_vertexList_destroy(vL);
    qh_triangleList_destroy(tL);

  }


  // copy downmix matrix
  m_downmixMatrix.resize(downmix_mat->row, downmix_mat->col);
  for (int x(0); x < downmix_mat->row; ++x)
  {
    for (int y(0); y < downmix_mat->col; ++y)
    {
      m_downmixMatrix(x,y) = downmix_mat->get(downmix_mat, x, y);
    }
  }

  qh_matrix_destroy(downmix_mat);

}

//--------------------------------------------------------------------------------------------------

std::vector<geo::Vector3d> Speakersetup::getRealSpeakerPositions() const
{
  std::vector<geo::Vector3d> spkrPos;
  for (std::size_t i(0); i < m_speakers.size(); ++i)
  {
    if (!m_speakers[i].lfe && !m_speakers[i].ghost)
    {
      spkrPos.push_back(m_speakers[i].toVector());
    }  
  }
  return spkrPos;
}

//-------------------------------------------------------------------------------------------------

std::size_t Speakersetup::getNumSpeakers() const
{
  std::size_t ret(0);
  for (std::size_t i(0); i < m_speakers.size(); ++i)
  {
    if (!(m_speakers[i].lfe) && !(m_speakers[i].ghost))
    {
      ++ret;
    }
  }
  return ret;
}

//-------------------------------------------------------------------------------------------------

std::size_t Speakersetup::getNumSubwoofers() const
{
  std::size_t ret(0);
  for (std::size_t i(0); i < m_speakers.size(); ++i)
  {
    if (m_speakers[i].lfe)
    {
      ++ret;
    }
  }
  return ret;
}

//--------------------------------------------------------------------------------------------------

void Speakersetup::getInfo(bool& coversUpperHalfsphere, bool& coversLowerHalfsphere) const
{
  coversUpperHalfsphere = m_coversUpperHalfSphere;
  coversLowerHalfsphere = m_coversLowerHalfSphere;
}

//-------------------------------------------------------------------------------------------------

const std::vector<int>& Speakersetup::getOutputMapping() const
{
  return m_outputMapping;
}

//-------------------------------------------------------------------------------------------------

bool Speakersetup::hasOutputMapping() const
{
  return m_outputMapping.size() > 0 ? 1 : 0;
}

//-------------------------------------------------------------------------------------------------

void Speakersetup::generateAngles(std::vector<float>& az, std::vector<float>& el)
{
  az.clear();
  el.clear();
  m_coversUpperHalfSphere = false;
  m_coversLowerHalfSphere = false;
  
  for (std::size_t i(0); i < m_speakers.size(); ++i)
  {
    if(!m_speakers[i].lfe)
    {
      az.push_back(m_speakers[i].azimuth);

      const float ele = m_speakers[i].elevation;
      el.push_back(ele);

      if (ele >= 5.f)
      {
        m_coversUpperHalfSphere = true;
      }
      else if (ele <= -5.f)
      {
        m_coversLowerHalfSphere = true;
      }
    }
  }
}

//-------------------------------------------------------------------------------------------------

std::size_t Speakersetup::getNumGhosts() const
{
  std::size_t ret(0);
  for (std::size_t i(0); i < m_speakers.size(); ++i)
  {
    if (m_speakers[i].ghost)
    {
      ++ret;
    }
  }
  return ret;
}

//-------------------------------------------------------------------------------------------------

std::ostream& Speakersetup::toStream(std::ostream& strm) const
{
  strm << "\nSpeakersetup: ";
  for (std::size_t i = 0; i < m_speakers.size(); ++i)
  {
    m_speakers[i].toStream(strm) << ", ";
  }
  
  std::vector<geo::Vector3d> pos = getRealSpeakerPositions();
  std::vector<geo::Vector3d> posAll = getAllSpeakerPositions();
  geo::sequenceToOstream(std::cout << "\n\nspkrVec", pos);
  geo::sequenceToOstream(std::cout << "\n\nspkrVecAll", posAll);
  strm << "\n";
  
  strm << "\nNumber of speaker vertices: " << getNumSpeakers();
  strm << ". Number of speakers + ghosts vertices: " << m_speakerVertices.size();
  bool coversLower, coversUpper;
  getInfo(coversUpper, coversLower);
  strm << ". Sphere Coverage: upper = " << coversUpper << ", lower = " << coversLower << "\n";
  strm << "\nDownmixMatrix: ";
  getDowmixMatrix().infoToStream(strm) << ".\n";
  strm << "\ntriangleList "<< getSpeakerTriangles();
  strm << "\n";
  return strm;
}
//-------------------------------------------------------------------------------------------------

bool Speakersetup::hasHeightSpeakers()
{
  for (std::size_t i = 0; i < m_speakers.size(); ++i)
  {
    // All speakers > 21° elevation are whether CH_U or CH_T speakers --> height speakers, so we use this simple check instead of speaker classification.
    if (m_speakers[i].elevation > 21)
    {
      return true;
    }
  }
  return false;
}

//-------------------------------------------------------------------------------------------------

} // namespace referencerenderer
} // namespace mpeg
} // namespace iosono
