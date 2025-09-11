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
#include "vbap_ghost_wrapper.hpp"
#include "functions.hpp"
#include "speakersetup.hpp"
#include "exception.hpp"

// std headers:
#include <iostream>
#include <sstream>


//-------------------------------------------------------------------------------------------------

namespace iosono
{
namespace mpeg
{
namespace referencerenderer
{

using namespace geo;

//-------------------------------------------------------------------------------------------------

VbapGhostWrapper::VbapGhostWrapper(const geo::Matrix& dowmixMatrix, std::auto_ptr<VbapInterface> vbapInterface):
  m_downmix(dowmixMatrix),
  m_interface(vbapInterface),
  m_gains(m_downmix.columns())
{
}

//-------------------------------------------------------------------------------------------------

void VbapGhostWrapper::init(const std::vector<geo::PointCartesian>& speakerArray,
                            const std::vector<geo::VertexTriangle>& speakerTriangles,
                            bool is3dSetup,
                            bool hasHeightSpeakers)
{
  if (m_downmix.rows() > 0 && speakerArray.size() != m_downmix.columns())
  {
    THROW_MPEGH_EXCEPTION(iosono::mpeg::Exception, "Different number of speaker vertices expected!")
  }
  m_interface->init(speakerArray, speakerTriangles, is3dSetup, hasHeightSpeakers);

}

//-------------------------------------------------------------------------------------------------

void VbapGhostWrapper::infoToStream(std::ostream& strm) const
{
  m_interface->infoToStream(strm);
}

//-------------------------------------------------------------------------------------------------

void VbapGhostWrapper::calculcateVbap(geo::PointCartesian source, float *final_gs, float gainFactor, float spread[4], int hasUniformSpread) const
{
  if (m_downmix.rows() > 0)
  {
    m_interface->calculcateVbap(source, &m_gains[0], 1.f, spread, hasUniformSpread);


    // downmix
    for (std::size_t row(0); row < m_downmix.rows(); ++row)
    {
      final_gs[row] = 0.f;
      for(std::size_t column(0); column < m_downmix.columns(); ++column)
      {
        final_gs[row] +=  m_downmix(row,column) * m_gains[column];
      }
    }

    // gain correction is necessary because adding power normalized gains through downmix 
    // introduces sum loudness errors. 
    float gainSum = 0.f;
    for (std::size_t row(0); row < m_downmix.rows(); ++row)
    {
      gainSum += final_gs[row] * final_gs[row];
    }    
    gainSum = std::sqrt(gainSum);
    for (std::size_t row(0); row < m_downmix.rows(); ++row)
    {
      final_gs[row] *= gainFactor / gainSum;
    }        
    
  }
  else
  {
    m_interface->calculcateVbap(source, final_gs, gainFactor, spread, hasUniformSpread);
  }
}

//-------------------------------------------------------------------------------------------------

std::size_t VbapGhostWrapper::getNumGains() const
{
  if (m_downmix.rows() > 0)
  {
    return m_downmix.rows();
  }
  else
  {
    return m_interface->getNumGains();
  }
}

//-------------------------------------------------------------------------------------------------


//-------------------------------------------------------------------------------------------------

} // namespace referencerenderer
} // namespace mpeg
} // namespace iosono
