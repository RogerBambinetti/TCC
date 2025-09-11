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

#ifndef __VBAP_GHOST_WRAPPER_HPP__
#define __VBAP_GHOST_WRAPPER_HPP__


//--------------------------------------------------------------------------------------------------

// Local headers:
#include "vbap_interface.hpp"
#include "types.hpp"

// STL headers:
#include <iostream>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <vector>
#include <memory>

//--------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace referencerenderer
{

//--------------------------------------------------------------------------------------------------
/**
 * @brief VBAP - Ghost Wrapper
 *
 * wraps vbap calculation and performs ghost-downmix
 */
class VbapGhostWrapper : public VbapInterface
{
public:

  /**
   * @brief VbapGhostWrapper
   * @param vbapInterface
   */
  VbapGhostWrapper(const geo::Matrix& dowmixMatrix,
                   std::auto_ptr<VbapInterface> vbapInterface);

  /**
   * @brief init
   * @param speakerArray
   * @param speakerTriangles
   * @param is3dSetup
   */
  void init(const std::vector<geo::PointCartesian>& speakerArray,
                      const std::vector<geo::VertexTriangle>& speakerTriangles,
                      bool is3dSetup,
                      bool hasHeightSpeakers);

  /**
   * @brief infoToStream
   * @param strm
   */
  void infoToStream(std::ostream& strm) const;

  /**
   * @brief calculcateVbap
   * @param source
   * @param final_gs
   * @param gainFactor
   */
  void calculcateVbap(geo::PointCartesian source, float *final_gs, float gainFactor, float spread[4], int hasUniformSpread) const;
  
  /**
   * @return Number of speaker gains which calculateVbap parameter final_gs must contain. 
   */
  /*virtual*/ std::size_t getNumGains() const;
  
  /**
   * For debugging purpose only 
   */
  const std::vector<float>& getAllGains() const {return m_gains;};
  
private:

  /// @brief downmix matrix
  geo::Matrix m_downmix;

  /// @brief vbap calculator
  std::auto_ptr<VbapInterface> m_interface;

  /// @brief temporary internal gains
  mutable std::vector<float> m_gains;
};

//--------------------------------------------------------------------------------------------------


} // namespace referencerenderer
} // namespace mpeg
} // namespace iosono

#endif
