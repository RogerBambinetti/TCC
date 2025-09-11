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

#ifndef __VBAP_CORE_HPP__
#define __VBAP_CORE_HPP__


//--------------------------------------------------------------------------------------------------

// Local headers:
#include "vbap_interface.hpp"
#include "types.hpp"

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
* @brief represents a set of three loudspeakers (triangle)
*
* This struct contains all loudspeaker related data (for one loudspeaker triplet)
* that is required for the calculation of VBAP based gain values.
*/
struct LoudspeakerTriplet : mpeg::geo::VertexTriangle
{
  LoudspeakerTriplet() {};
  LoudspeakerTriplet(std::size_t i1, std::size_t i2, std::size_t i3)
      : mpeg::geo::VertexTriangle(i1, i2, i3) {}

  /// matrix containing the coordinates of the three loudspeakers
  mpeg::geo::PointCartesian matrix[3];

  /// inverse of the loudspeaker coordinates matrix
  mpeg::geo::PointCartesian matrixInverse[3];
};

//--------------------------------------------------------------------------------------------------

inline std::ostream& operator<<(std::ostream& strm, const LoudspeakerTriplet& obj)
{
  strm << obj.index[0] << " " << obj.index[1] << " " << obj.index[2];
  return strm;
}

//--------------------------------------------------------------------------------------------------
/**
 * @brief VBAP - Core Calculation Class
 * Implements the 2D/3D vbap algorithm.
 */
class VbapCore : public VbapInterface
{
public:

  VbapCore();

  void init(const std::vector<geo::PointCartesian>& speakerArray,
      const std::vector<geo::VertexTriangle>& speakerTriangles,
      bool is3dSetup,
      bool hasHeightSpeakers);

  void infoToStream(std::ostream& strm) const;

  /**
   * 
   * @param source Source direction given in iosono (mathematical) coordinate space (azimuth = 0 --> X-Axis --> means right)
   * @param final_gs
   * @param gainFactor
   */
  void calculcateVbap(geo::PointCartesian source, float *final_gs, float gainFactor, float spread[4], int hasUniformSpread) const;

  /**
   * @return Number of speaker gains which calculateVbap parameter final_gs must contain. 
   */
  /*virtual*/ std::size_t getNumGains() const;


private:

  /**
   * The first part of spread functionality. Calculates the 18 spread source positions.
   * @param p0 Source position
   * @param spreadAngle in radiant
   */
  void calcSpreadVectors(const geo::Vector3d& source, float spread[4], int hasUniformSpread) const;
  
  /**
   * The second part of spread functionality. Cross-fading towards power-normlized unit gain
   * @param spreadAngle in radiant
   */
  void calcSpreadGains(float spreadAngle) const;


  void calculcateOneSourcePosition(geo::PointCartesian source) const;

  /**
   * @brief Selects loudspeaker pairs and calculates the inversion matrixes for each selected pair.
   *
   * Loudspeakers a sorted according to their azimuth angle. Adjacent loudspeakers are the loudspeaker pairs
   * to be used. Thereby is has to be checked whether the loudspeaker setup is an open (e.g. a line or a
   * closed (e.g. a circle) setup.
   */
  void calculatePairs(std::vector<LoudspeakerTriplet> &lsTriplets);


  /// auxiliary method to calculate the inverse matrix for three points p0, p1, p2
  void calculateInverseMatrix(const geo::PointCartesian &p0,
                              const geo::PointCartesian &p1,
                              const geo::PointCartesian &p2,
                              geo::PointCartesian inverseMatrix[3]);

  /// fills the matrix of a loudspeaker triplet and calculates the inverse matrix
  template <typename StlContainer>
  void fillMatrixes(StlContainer &lsTriplets);


  /// number of loudspeakers
  unsigned m_nSpeaker;

  /// indicates whether the loudspeaker setup is a 2D or a 3D setup
  bool m_is3dSetup;
  
  /// Indicates if the setup allows 3D spreading.
  bool m_hasHeightSpeakers;

  /**
   * @brief relative loudspeaker positions
   *
   * This vector contains all loudspeaker positions <b>relative</b> to the current center
   * of the array. The VBAP algorithm assumes that the center of the array is the
   * origin (0/0/0), so having a center position different from the origin or moving
   * the center position means in fact moving the loudspeaker positions.
   */
  std::vector<geo::PointCartesian> m_speakerPosition;

  std::vector<LoudspeakerTriplet> m_lsTriplets;
  
  /// caching member for intermediate gain values 
  mutable std::vector<float> m_gains;
  /// caching member for spread source vectors 
  mutable std::vector<geo::Vector3d> m_baseVectors3D;

};

//--------------------------------------------------------------------------------------------------


} // namespace referencerenderer
} // namespace mpeg
} // namespace iosono

#endif
