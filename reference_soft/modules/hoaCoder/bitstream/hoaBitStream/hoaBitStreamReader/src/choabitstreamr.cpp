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
 $Rev: 203 $
 $Author: technicolor-ks $
 $Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
 $Id: choabitstreamr.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#include "choabitstreamr.h"
#include  <iostream>

/** Default constructor
  *
  **********************************************************************/ 
CHoaBitStreamR::CHoaBitStreamR(void)
{
  m_bitReader = std::shared_ptr<CBitStreamReader>( new CBitStreamReader); // has to be initialized as shared pointer 
  m_fileReader = std::shared_ptr<CStreamReader>( new CStreamReader); // has to be initialized as  shared pointer
  m_bitReader->createBufferR(2048);
  m_bitReader->setStreamReader(m_fileReader);
  m_configRead.setBitStreamReader(m_bitReader);
  m_frameRead.setBitStreamReader(m_bitReader);
  m_bValidSpatFrame = false;
  m_bIsEof = true;
  return;
};

/** Destructor
  *
  **********************************************************************/ 
CHoaBitStreamR::~CHoaBitStreamR()
{
}

/** function that initializes the compressed HOA file reader 
  * 
  * - sName: path and file name of the compressed HOA file
  *
  * - returns true on errors
  *
  * - comments: Reads the file header and the first access frame.
  *             Use getFileHeader() and getHoaConfig() for
  *             access to the fields of both frames.
  *
  **********************************************************************/ 
bool CHoaBitStreamR::initHoaFile(const char * sName, unsigned int unNumTransportChannels, unsigned int unCoreCoderFrameLength)
{
  // close stream
  if(!closeStream())
    return true;

  // open new stream
  if(!openStream(sName))
    return true;

  // read access frame
  if(m_configRead.read(unNumTransportChannels, unCoreCoderFrameLength)<=0)
    return true;

  // byte align frame buffer
  unsigned int tmp = 0;
  m_bitReader->byteAlignBuffer(tmp);

  // get access frame reference
  const HoaConfig & hoaConfig = m_configRead.getFields();

  // reset valid spat. info frame
  m_bValidSpatFrame = false; // set true if first spat. frame has been read
  
  // init frame from access frame
  m_frameRead.initArrays((hoaConfig.m_unHoaOrder+1)*(hoaConfig.m_unHoaOrder+1),
                          hoaConfig.m_unMaxNoOfDirSigsForPrediction,
                          hoaConfig.m_unNoOfBitsPerScaleFactor,
                          hoaConfig.m_nMinAmbHoaOrder,
                          hoaConfig.m_unTotalNumCoders,
                          hoaConfig.m_unNumOfAdditionalCoders,
                          hoaConfig.m_unCodedVVecLength,
                          hoaConfig.m_unMaxGainCorrAmpExp,
                          hoaConfig.m_unMaxVVecDirections,
                          hoaConfig.m_unDirGridTableIdx,
                          hoaConfig.m_unMaxNumOfPredDirs,
                          hoaConfig.m_unMaxNumOfPredDirsPerBand, 
                          hoaConfig.m_unNumOfPredSubbands,
                          hoaConfig.m_unFirstSBRSubbandIdx,
                          hoaConfig.m_nMaxHoaOrderToBeTransmitted,
                          hoaConfig.m_vunParUpmixHoaOrderPerParSubBandIdx,
                          hoaConfig.m_vbUseRealCoeffsPerParSubband);

  m_bIsEof = false;
  return false;
}


/** TODO: for multiple access frames return true if a new access frame has been found */
bool CHoaBitStreamR::newHoaConfigFound()
{
  // multiple access frames are not implemented by now
  return false;
}


/** function that reads a new HOA frame from the compressed HOA file 
  * 
  * - bool failed - indicates an error
  *
  * - returns a reference to the current frame. 
  *         the reference is valid until the next call of the function
  *
  * - comment: call initHoaFile() before using this function 
  *
  **********************************************************************/ 
const HoaFrame & CHoaBitStreamR::readHoaFrame(bool &failed)
{
  failed = true;
  // if end of file has been reached, return the delay element
  if(m_bitReader->isEof() )
  {
    // there was no error
    failed = false;
    m_bIsEof = true; // end of file has been reached
    m_bValidSpatFrame = false; // spat info is invalid (has been sent before)
    // create empty dummy frame
    m_frameRead.m_bHoaIndependencyFlag = false;
    for(std::vector<std::shared_ptr<CChannelSideInfoData> >::iterator it = m_frameRead.m_vChannelSideInfo.begin();
      it != m_frameRead.m_vChannelSideInfo.end(); ++it)
    {
      // set all channel types to empty
      if((*it)->m_unChannelType != EMPTY_CHANNEL)
      {
        *it = std::shared_ptr<CChannelSideInfoData>(new CEmptyInfoChannel());
      }
    }
    // reset gain correction to zero 
    for(std::vector<CHoaGainCorrection>::iterator it = m_frameRead.m_vGainCorrectionData.begin();
      it != m_frameRead.m_vGainCorrectionData.end(); ++it)
    {
      it->m_bCodedGainCorrectionExp.assign(1,true);
      it->m_bGainCorrectionException = false;
      it->m_nGainCorrAbsAmpExp = 0;
    }
    // disable all coding tools
    m_frameRead.m_spatPredictionData.m_bPerformPrediction = false;
    m_frameRead.m_hoaDirectionalPredictionInfo.m_bUseDirPred = false;
    m_frameRead.m_hoaParInfo.m_bPerformPar = false;

    return m_frameRead;
  }
    
  // read new frame
  // read spatial data
  if(m_frameRead.read()<=0)
  {
    // check for end of file
    if(!m_bitReader->isEof())
    {
      // on errors and not end of file
      failed = true; // reading failed
      m_bValidSpatFrame = false; // spat data invalid
      return m_frameRead; // return invalid first queue element
    }
  }

  // valid spatial info read
  m_bValidSpatFrame = true;

  // return current frame
  failed = false;
  return m_frameRead;
}

/** function for byte alignment of the bit buffer
 * 
 * 
 ************************************************************************/
int CHoaBitStreamR::byteAlignFrameBuffer()
{
   unsigned int tmp = 0;
   return m_bitReader->byteAlignBuffer(tmp);
}

/** function that indicates that the spat. 
  * info of the current frame is valid
  *
  **********************************************************************/ 
bool CHoaBitStreamR::spatialDataValid()
{
  return m_bValidSpatFrame;
}

/** function that opens the compressed HOA file
  *
  **********************************************************************/ 
// TODO: move to private
bool CHoaBitStreamR::openStream(const char * sName)
{
  m_bIsEof = false;
  return m_fileReader->openStream(sName);
}

/** function that closes the HOA stream 
  *
  **********************************************************************/ 
bool CHoaBitStreamR::closeStream()
{
  m_bIsEof = true;
  m_bValidSpatFrame = false;
  return m_fileReader->closeStream();
}

/** function that returns a reference of the currently valid access frame
  *
  * - comment: the access frame is valid after calling initHoaFile()
  *            a change of the access frame data is indicated by 
  *            newHoaConfigFound().
  *
  **********************************************************************/ 
const HoaConfig& CHoaBitStreamR::getHoaConfig()
{
  return m_configRead.getFields();
}

/** function that indicates that the end of the file has been reached
  *
  **********************************************************************/ 
bool CHoaBitStreamR::isEof()
{
    return m_bIsEof;
}

