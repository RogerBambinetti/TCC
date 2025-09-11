/**************************************************************************************
  This software module was originally developed by  
 Deutsche Thomson OHG (DTO)
  in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
 performance may not have been optimized. This software module is an implementation
 of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
 you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
 and make derivative works of this software module or modifications  thereof for use
 in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
 and which satisfy any specified conformance criteria. 
  In no event shall the above provision be qualified, deemed, or construed as granting 
 to use and any third party, either expressly, by implication or by way of estoppel,
 any license or any authorization or other right to license, sell, distribute, under 
 any patent or patent application and any intellectual property rights other than the 
 copyrights owned by Company which are embodied in such software module and expressly 
 licensed hereunder. 
 Those intending to use this software module in products are advised that its use 
 may implement third party intellectual property rights, and in particular existing 
 patents or patent application which licenses to use are not of Company or ISO/IEC 
 responsibility, which hereby fully disclaim any warranty and liability of infringement 
 of free enjoyment with respect to the software module and its use.
 The software modules is provided as is, without warranty of any kind. 
 DTO and ISO/IEC have no liability for use of this software module or modifications
 thereof.
 Copyright hereunder is not released licensed for products that do not conform to the 
 ISO/IEC 23008-3 standard.
 DTO retains full right to modify and use the code for its own  purpose, assign or 
 donate the code to a third party and to inhibit third parties from  using the code 
 for products that do not conform to MPEG-related ITU Recommendations  and/or 
 ISO/IEC International Standards.
 This copyright notice must be included in all copies or derivative works. 
 This copyright license shall be construed according to the laws of Germany. 
  Copyright (c) ISO/IEC 2015.
*****************************************************************************************/
                

/*
 $Wev: 59 $
 $Author: technicolor-ks $
 $Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
 $Id: choabitstreamw.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
 $HeadUWL: https://hanvssvn01/saaswp7/svn-repos/mpeg-h_3daudio/branches/2013-03-21-CompressionPrototype3/src/cpp/bitstream/bitStreamIO/include/basicbitstreamfields.h $
*/

#include "choabitstreamw.h"

CHoaBitStreamW::CHoaBitStreamW()
 :  m_bitWriter(std::shared_ptr<CBitStreamWriter>( new CBitStreamWriter)),
    m_fileWriter(std::shared_ptr<CStreamWriter>( new CStreamWriter)),
    m_hoaConfigWrite(),    
    m_hoaFrameWrite()
{
  m_bitWriter->createBufferW(2048);
  m_bitWriter->setStreamWriter(m_fileWriter);
  m_hoaConfigWrite.setBitStreamWriter(m_bitWriter);
  m_hoaFrameWrite.setBitStreamWriter(m_bitWriter);
}

CHoaBitStreamW::~CHoaBitStreamW()
{
};

bool CHoaBitStreamW::openStream(const char * sName)
{
  return m_fileWriter->openStream(sName);
}

bool CHoaBitStreamW::closeStream()
{
  m_bitWriter->flushBufferToStream();
  return m_fileWriter->closeStream();
}

int CHoaBitStreamW::writeHoaConfig()
{
  // initialize the standard HOA frame from the parameter of the 
  // access frame
  initFrame();
  // write the Access frame to the stream
  return m_hoaConfigWrite.write();
}

void CHoaBitStreamW::initFrame()
{
  m_hoaFrameWrite.initArrays((m_hoaConfigWrite.m_unHoaOrder+1)*(m_hoaConfigWrite.m_unHoaOrder+1),
                      m_hoaConfigWrite.m_unMaxNoOfDirSigsForPrediction,
                      m_hoaConfigWrite.m_unNoOfBitsPerScaleFactor,
                      m_hoaConfigWrite.m_nMinAmbHoaOrder,
                      m_hoaConfigWrite.m_unTotalNumCoders,
                      m_hoaConfigWrite.m_unNumOfAdditionalCoders,
                      m_hoaConfigWrite.m_unCodedVVecLength,
                      m_hoaConfigWrite.m_unMaxGainCorrAmpExp,
                      m_hoaConfigWrite.m_unMaxVVecDirections,
                      m_hoaConfigWrite.m_unDirGridTableIdx,
                      m_hoaConfigWrite.m_unMaxNumOfPredDirs,
                      m_hoaConfigWrite.m_unMaxNumOfPredDirsPerBand, 
                      m_hoaConfigWrite.m_unNumOfPredSubbands,
                      m_hoaConfigWrite.m_unFirstSBRSubbandIdx,
                      m_hoaConfigWrite.m_nMaxHoaOrderToBeTransmitted,
                      m_hoaConfigWrite.m_vunParUpmixHoaOrderPerParSubBandIdx,
                      m_hoaConfigWrite.m_vbUseRealCoeffsPerParSubband);
}

int CHoaBitStreamW::writeFrame(bool  bGlobalIndependencyFlag)
{
  m_hoaFrameWrite.setGlobalIndyFlag(bGlobalIndependencyFlag);
  // write the current frame
  return  m_hoaFrameWrite.write();
}

int CHoaBitStreamW::byteAlignFrameBuffer()
{
   return m_bitWriter->byteAlignBuffer();
};

HoaFrame& CHoaBitStreamW::getFrame()
{
  return m_hoaFrameWrite.getFields();
}

HoaConfig& CHoaBitStreamW::getHoaConfig(unsigned int unCoreCoderFrameLength)
{
  m_hoaConfigWrite.m_unCoreCoderFrameLength = unCoreCoderFrameLength;
  return m_hoaConfigWrite.getFields();
}


unsigned int CHoaBitStreamW::getFrameSize(bool  bGlobalIndependencyFlag)
{
  m_hoaFrameWrite.setGlobalIndyFlag(bGlobalIndependencyFlag);
  return m_hoaFrameWrite.getFrameSize();
};


unsigned int CHoaBitStreamW::getHoaConfigSize()
{
  return m_hoaConfigWrite.getFrameSize();
};

std::shared_ptr<CChannelSideInfoData>  CHoaBitStreamW::getChannelInfo(CHANNEL_TYPE type)
{
  switch(type)
  {
    case DIR_CHANNEL:
      return std::shared_ptr<CChannelSideInfoData>(new CDirectionalInfoChannel());
    case ADD_HOA_CHANNEL:
      return std::shared_ptr<CChannelSideInfoData>(new CAddAmbHoaInfoChannel());
    break;
    case VEC_CHANNEL:
      return std::shared_ptr<CChannelSideInfoData>(new CVectorBasedInfoChannel());
    case EMPTY_CHANNEL:
    default:
      return std::shared_ptr<CChannelSideInfoData>(new CEmptyInfoChannel());
  }
}
