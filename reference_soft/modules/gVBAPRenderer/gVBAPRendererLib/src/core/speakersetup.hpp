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

#ifndef __SPEAKERSETUP_HPP__
#define __SPEAKERSETUP_HPP__

//--------------------------------------------------------------------------------------------------

// Local headers:
#include "types.hpp"
#include "cicp2geometry.h"

// STL headers:
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <cassert>
#include <vector>

//--------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace referencerenderer
{

//--------------------------------------------------------------------------------------------------
/**
 * @brief The Speakers enumeration struct
 * Contains all involved speakers.
 * Each speaker layout (5.1, 8.1, ..) uses a subset
 * of these speakers.
 */
struct Speakers
{
  enum Value
  {
    CH_EMPTY = -1,
    CH_M_L030, // 0
    CH_M_R030, // 1
    CH_M_000,  // 2
    CH_LFE1,   // 3
    CH_M_L110, // 4
    CH_M_R110, // 5
    CH_M_L022, // 6
    CH_M_R022, // 7
    CH_M_L135, // 8
    CH_M_R135, // 9
    CH_M_180,  //10
    CH_DUMMY_11,//11
    CH_DUMMY_12,//12
    CH_M_L090, //13
    CH_M_R090, //14
    CH_M_L060, //15
    CH_M_R060, //16
    CH_U_L030, //17
    CH_U_R030, //18
    CH_U_000,  //19
    CH_U_L135, //20
    CH_U_R135, //21
    CH_U_180,  //22
    CH_U_L090, //23
    CH_U_R090, //24
    CH_T_000,  //25
    CH_LFE2,   //26
    CH_L_L045, //27
    CH_L_R045, //28
    CH_L_000,  //29
    CH_U_L110, //30
    CH_U_R110, //31
    CH_U_L045, //32
    CH_U_R045, //33
    CH_M_L045, //34
    CH_M_R045, //35
    CH_LFE3,   //36

    NUM_SPKR_TYPES,
    USER_DEFINED
  };

  /**
   * @brief toString
   * @param val
   * @return
   */
  static std::string toString(Value val);
  
  static Value classifySpeaker(int azimuth, int elevation, int isLfe);
  static bool getAttributes(int type, int& azi, int& ele, int& isLfe);
  
  static const int s_numHeightSpeakers;
  static const int s_heightSpeakers[];
private:
  static const int s_index[];
  static const int s_azi[];
  static const int s_ele[];
  static const int s_aziStart[];
  static const int s_aziEnd[];
  static const int s_eleStart[];
  static const int s_eleEnd[];
  static const int s_isLfe[];
  static const int s_isValid[];
};

//--------------------------------------------------------------------------------------------------

/**
 * @brief The Speaker struct
 */
struct Speaker
{
  Speaker()
  : id(Speakers::CH_EMPTY)
  , azimuth(0.f)
  , elevation(0.f)
  , lfe(false)
  , ghost(false)
  {}

  /**
   * 
   * @param i
   * @param az in degrees (in mpegh coordinate space (azimuth = 0 means front)
   * @param el in degrees 
   * @param islfe
   */
  Speaker(int az,
          int el,
          bool islfe = false,
          bool isGhost = false);

  /// Returns speaker coordinates in cartesian coordinates (Meters) in 
  /// mathematical (iosono) coordinate space.
  geo::Vector3d toVector() const;
  
  /// Returns a SphericalPosition in radiant
  geo::SphericalPosition toSphrPosRad() const;
  
  std::ostream& toStream(std::ostream& strm)const ;

  Speakers::Value id;

  /// Azimuth in degrees
  float azimuth;
  /// Elevation in degrees
  float elevation;

  /// lfe speakers
  bool lfe;

  /// flag indicates ghost (virtual) speaker
  bool ghost;

};

//--------------------------------------------------------------------------------------------------

/**
 * @brief speakersetup class
 *
 * Coordinates are stored in the MPEG-H coordinate system (angular)
 * But: All functions return coordinates in the IOSONO coordinate system.
 * Fixed radius is used (3m)
 */
class Speakersetup
{
public:

  /**
   * @brief Speakersetup
   */
  Speakersetup();

  /**
   * @brief init with user setup coordinates
   *
   * probably ghost speakers are added to the speaker setup
   *
   * @param speakerPositions
   */
  void init(CICP2GEOMETRY_CHANNEL_GEOMETRY* geometry, int nSpeakers);
  //void init(const std::vector<geo::PointSpherical>& speakerPositions);

  /**
   * @brief getSpeakerArray (without subwoofers, including ghost speakers)
   *
   * with ghost speakers
   *
   * @return
   */
  inline std::vector<geo::Vector3d> getAllSpeakerPositions() const { return m_speakerVertices; }
  
  /**
   * Returns speaker coordinates of real speakers (no subs, no ghosts) in iosono coordinate space 
   * @return
   */
  std::vector<geo::Vector3d> getRealSpeakerPositions() const;

  /**
   * @brief getSpeakerTriangles
   *
   * get speaker triangulation (with ghost speakers)
   *
   * @return
   */
  inline const std::vector<geo::VertexTriangle>& getSpeakerTriangles() const {return m_speakerTriangles;}

  /**
   * @brief getNumSpeakers (without subwoofers and without ghost speakers)
   * @return number of speakers
   */
  std::size_t getNumSpeakers() const;

  /**
   * @brief getNumSubwoofers
   * @return number of subwoofers
   */
  std::size_t getNumSubwoofers() const;

  /**
   * @param[out] coversUpperHalfsphere True if at least one speaker exists with elevation >= 5°
   * @param[out] coversLowerHalfsphere True if at least one speaker exists with elevation <= -5°
   */
  void getInfo(bool& coversUpperHalfsphere, bool& coversLowerHalfsphere) const;

  /**
   * @brief get output mapping
   * used for correct the channel ordering
   */
  const std::vector<int>& getOutputMapping() const;

  /**
   * @brief is output mapping available
   */
  bool hasOutputMapping() const;

  /**
   * @brief getDowmixMatrix
   * Used to downmix the internal representation with ghosts (columns)
   * to the plain-coefficients (rows)
   * @return matrix
   */
  inline const geo::Matrix& getDowmixMatrix() const {return m_downmixMatrix;};
  
  std::ostream& toStream(std::ostream& strm) const;
    
  bool hasHeightSpeakers();
  

private:

  void postInit();

  void generateAngles(std::vector<float>& az, std::vector<float>& el);

  /**
   * @brief getNumGhosts (use only if TRIANGULATION_TABLE is used!)
   * @return
   */
  std::size_t getNumGhosts() const;

  /// The currnet setup
  std::vector<Speaker> m_speakers;

  std::vector<geo::Vector3d> m_speakerVertices;
  std::vector<geo::VertexTriangle> m_speakerTriangles;

  geo::Matrix m_downmixMatrix;

  std::vector<int> m_outputMapping;
  
  bool m_coversUpperHalfSphere;
  bool m_coversLowerHalfSphere;
};

//--------------------------------------------------------------------------------------------------

} // namespace referencerenderer
} // namespace mpeg
} // namespace iosono


#endif
