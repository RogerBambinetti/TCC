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
$Id: HoaEncoder.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include <algorithm>
#include <iostream>
#include "SpatialEncoder.h"
#include "choabitstreamw.h"
#include "HoaEncoder.h"
#include "wavWriter.h"
#include "TabulatedValuesHoaBitstream.h"


const std::map<unsigned int, unsigned int> HoaEncoder::createCoreCoderDelayTable()
{
  std::map<unsigned int, unsigned int> tmp;
  tmp[1200000] = 2625; 
  tmp[512000] = 2625;
  tmp[256000] = 5159;
  tmp[128000] = 5159;
  tmp[96000] = 5159;
  tmp[64000] = 5159;
  tmp[48000] = 5159;
  return tmp;
}


/************ HoaEncoder functions *******************************************/
//---------------------------------------------------------------------------------------
HoaEncoder::HoaEncoder() 
  : m_bitStreamWriter(),  
  m_spatialEncoder(),
  m_fHoaSamples(),
  m_fTransportChannels(), 
  m_unHoaFrameLength(0),
  m_bHoaFileIsOpen(false),
  m_bEndOfFileSet(false),
  m_bEndOfFileReached(false),
  m_unHoaSiPerFrame(1),
  m_unHoaSiCnt(0),
  m_unHoaSiBits(0),
  m_nSamplesRead(0),
  m_nSamplesWritten(0),
  m_nEncoderDelaySamples(0), 
  m_unAddDelayInSamples(0), 
  m_unNumOfDummyFrames(0),
  m_CoreCoderDelayTable(createCoreCoderDelayTable())
{

}

//---------------------------------------------------------------------------------------
HoaEncoder::~HoaEncoder()
{
   closeHoaFile();
}

//---------------------------------------------------------------------------------------
bool HoaEncoder::initHoaEncoder(const std::string &hoaTransportChanFile,
                                const std::string &hoaSideInfoFile, 
                                const std::string &hoaSideInfoSizeFile,
                                const unsigned int unSampleRate,
                                const unsigned int unTotalBitRate,
                                const unsigned int unHoaOrder,
                                const bool bUseNfc,
                                float fNfcDistance,
                                bool bCoreCoderUsesSbr,
                                unsigned int unCoreCoderFrameLength,
                                unsigned int unHoaFrameLength)
{

  m_unHoaFrameLength = unHoaFrameLength;

  // get delay handling parameters from selected bit rate
  try
  {
    unsigned int unCoreCoderDelay = m_CoreCoderDelayTable.at(unTotalBitRate);
    m_unNumOfDummyFrames = unCoreCoderDelay/m_unHoaFrameLength;
    if(unCoreCoderDelay%m_unHoaFrameLength)
      m_unNumOfDummyFrames++;
    m_unAddDelayInSamples = m_unNumOfDummyFrames*m_unHoaFrameLength - unCoreCoderDelay;
  }catch(std::exception e)
  {
    std::cout << "The selected bit rate of " << unTotalBitRate <<  " bit/s is not supported" << std::endl;
    return(true);
  }

  // close file if it is open
  if(m_bHoaFileIsOpen)
    if(closeHoaFile())
      return(true);

  // create new bit stream writer
  m_bitStreamWriter = std::shared_ptr<CHoaBitStreamW>(new CHoaBitStreamW());
  // open file for writing
  if(!m_bitStreamWriter->openStream(hoaSideInfoFile.c_str()))
  {
    std::cout << "Cannot open HOA side info file " << hoaSideInfoFile << std::endl;
    return true;
  }
  m_bHoaFileIsOpen =true;
  // create new spatial encoder
  m_spatialEncoder = std::shared_ptr<SpatialEncoder>(new SpatialEncoder());

  // Open side info size stream
  m_sideInfoSizeStream.open(hoaSideInfoSizeFile.c_str(), std::ios_base::out);
  if(m_sideInfoSizeStream.bad())
  {
    std::cout << "Cannot open HOA side info size file " << hoaSideInfoSizeFile << std::endl;
    return true;    
  }

  // set encoder parameters

  // these parameters only affect the encoder and are not included in the config
  unsigned int    uiMaxNoOfDirSigs		= 4;

  // the folllowing parameters are included in the config
  unsigned int    uiHOAOrder				= unHoaOrder;
  int             iMinOrderForAmbHOA	    = 1;
  unsigned int    uiMaxNofOfDirSigsForPred = 2;
  unsigned int    uiNoOfBitsPerScaleFactor = 8;
  unsigned int    uiMaxAmplifyExponent	 = 4;
  unsigned int    uiInterpolationSamples   = 256;
  unsigned int    uiInterpMethod		     = 0; // linear
  unsigned int    uiNoBitsForVecElemQuant  = 8;
  unsigned int    uiCodedVVecLength        = 0;

  // phase 2 settings 
  unsigned int uiDirGridTableIdx = 2;
  unsigned int uiMaxNumOfPredDirsPerBand = 4;
  unsigned int uiMaxNumOfPredDirsForSubbandPred = 16;
  unsigned int uiFirstSBRSubbandIdx = 7;
  std::vector<unsigned int> vuiUpmixHoaOrderPerSubBand;
  int iMaxHoaOrderToBeTransmitted = 1;
  unsigned int uiSubBandConfigForDirPredTableIdx = 1; 
  if (unTotalBitRate > 256000)
  {
    uiSubBandConfigForDirPredTableIdx = 0; // no subband prediction for high bitrates
  }

  // compute dependent variables
  unsigned int uiNoOfHOACoeffs = (uiHOAOrder+1)*(uiHOAOrder+1);

  // copy encoder parameter to access frame
  HoaConfig & hoaConfig = m_bitStreamWriter->getHoaConfig(unCoreCoderFrameLength);

  hoaConfig.m_unHoaOrder	= uiHOAOrder;
  hoaConfig.m_bUsesNfc	= bUseNfc;
  if (bUseNfc)
  {
    hoaConfig.m_ufAmbNfcReferenceDistance = fNfcDistance;
  }

  if(unTotalBitRate >= 256000)
    iMaxHoaOrderToBeTransmitted = static_cast<int>(uiHOAOrder);
  else
    iMaxHoaOrderToBeTransmitted = 1;

  if((unTotalBitRate >= 1200000))
  {
    // high bit rate setting of phase 1
    hoaConfig.m_unTotalNumCoders = 17;
  }
  else if((unTotalBitRate < 1200000) & (unTotalBitRate >= 512000))
  {
    // mid bit rate setting
    hoaConfig.m_unTotalNumCoders = 8;
  }
  else if((unTotalBitRate < 512000) & (unTotalBitRate >= 256000))
  {
    // low bit rate setting 
    hoaConfig.m_unTotalNumCoders = 8;
  }
  else if((unTotalBitRate < 256000))
  {
    hoaConfig.m_unTotalNumCoders = std::max( static_cast<unsigned int>(1),  
      static_cast<unsigned int>(std::floor( 
      static_cast<float>(unTotalBitRate)/static_cast<float>(32000) )));
    uiMaxNoOfDirSigs = 0;
  }

  // get subband widths
   std::vector<std::vector<unsigned int>> vvunPredSubbandTable;
   TabulatedValuesHoaBitstream TabVal;
   TabVal.getPredSubbandTable(vvunPredSubbandTable);
   std::vector<unsigned int> vuiSubBandWidths = vvunPredSubbandTable[uiSubBandConfigForDirPredTableIdx];
   unsigned int uiNoOfPredSubbands = vuiSubBandWidths.size(); // 10

  // PAR settings
   unsigned int uiSubBandConfigForParTableIdx = 2;   
   // disable PAR for high bitrates
   if(unTotalBitRate > 256000)
   {
      uiSubBandConfigForParTableIdx = 0;
   }
   std::vector<std::vector<unsigned int>> vvuiPARSubbandTable;
   TabVal.getParSubbandTable(vvuiPARSubbandTable);
   std::vector<unsigned int> vuiPARSubBandWidths = vvuiPARSubbandTable[uiSubBandConfigForParTableIdx];
    
   unsigned int uiNoOfPARSubbands = vuiPARSubBandWidths.size();
   unsigned int uiLastFirstOrderSubBandIdx = 0;
   std::vector<unsigned int> vuiParUpmixHoaOrderPerParSubBandIdx(uiNoOfPARSubbands, 2);
   if(vuiParUpmixHoaOrderPerParSubBandIdx.size() > 0)
      vuiParUpmixHoaOrderPerParSubBandIdx[0] = 1;
   std::vector<bool> m_vbUseRealCoeffsPerParSubband(uiNoOfPARSubbands, false);
   if(m_vbUseRealCoeffsPerParSubband.size() > 0)
      m_vbUseRealCoeffsPerParSubband[uiNoOfPARSubbands-1] = true;

  unsigned int uiMaxNoOfTransmittedHOACoeffs = static_cast<int>((iMaxHoaOrderToBeTransmitted+1)*(iMaxHoaOrderToBeTransmitted+1));
  ////////////////////////

  if ( hoaConfig.m_unTotalNumCoders < static_cast<unsigned int>((iMinOrderForAmbHOA+1)*(iMinOrderForAmbHOA+1)))
  {
    iMinOrderForAmbHOA = static_cast<int>( std::floor(sqrt(static_cast<double>(hoaConfig.m_unTotalNumCoders)) - 1.0) );
    std::cout<<"The minimum transmitted ambient HOA order was reset to " << iMinOrderForAmbHOA << " due to the small amount of transport channels!" << std::endl;
  }

  hoaConfig.m_nMinAmbHoaOrder				    = iMinOrderForAmbHOA;
  hoaConfig.m_unMaxNoOfDirSigsForPrediction	= uiMaxNofOfDirSigsForPred;
  hoaConfig.m_unNoOfBitsPerScaleFactor		= uiNoOfBitsPerScaleFactor;
  hoaConfig.m_unSpatialInterpolationTime	    = uiInterpolationSamples;
  hoaConfig.m_unSpatInterpMethod			    = uiInterpMethod;
  hoaConfig.m_unCodedVVecLength               = uiCodedVVecLength;
  hoaConfig.m_unMaxGainCorrAmpExp             = uiMaxAmplifyExponent;

  hoaConfig.m_unNumOfAdditionalCoders = hoaConfig.m_unTotalNumCoders - (iMinOrderForAmbHOA+1)*(iMinOrderForAmbHOA+1);

  hoaConfig.m_unHoaFrameLength                =  m_unHoaFrameLength;
  hoaConfig.m_unMaxNumOfPredDirs              = uiMaxNumOfPredDirsForSubbandPred;
  hoaConfig.m_unMaxNumOfPredDirsPerBand       = uiMaxNumOfPredDirsPerBand;
  hoaConfig.m_nMaxHoaOrderToBeTransmitted     = iMaxHoaOrderToBeTransmitted;
  hoaConfig.m_unDirGridTableIdx               = uiDirGridTableIdx;
  hoaConfig.m_unFirstSBRSubbandIdx            = uiFirstSBRSubbandIdx;
  hoaConfig.m_unSubbandConfigIdx              = uiSubBandConfigForDirPredTableIdx;
  hoaConfig.m_unNumOfPredSubbands             = uiNoOfPredSubbands;

  hoaConfig.m_unParSubBandTableIdx            = uiSubBandConfigForParTableIdx;
  hoaConfig.m_vunParSubbandWidths             = vuiPARSubBandWidths;
  hoaConfig.m_unLastFirstOrderSubBandIdx      = uiLastFirstOrderSubBandIdx;
  hoaConfig.m_vunParUpmixHoaOrderPerParSubBandIdx     = vuiParUpmixHoaOrderPerParSubBandIdx;
  hoaConfig.m_vbUseRealCoeffsPerParSubband            = m_vbUseRealCoeffsPerParSubband;

  // init HOA frame
  m_bitStreamWriter->initFrame();

  const std::vector<unsigned int> vunBitratesPerCoder(hoaConfig.m_unTotalNumCoders, unTotalBitRate/hoaConfig.m_unTotalNumCoders);
  // init Spatial Encoder
  if(m_spatialEncoder->init(m_unHoaFrameLength, 
    hoaConfig.m_unHoaOrder,  
    uiMaxNoOfDirSigs, 
    hoaConfig.m_nMinAmbHoaOrder, 
    hoaConfig.m_unTotalNumCoders,
    vunBitratesPerCoder[0], 
    hoaConfig.m_unMaxNoOfDirSigsForPrediction,
    hoaConfig.m_unNoOfBitsPerScaleFactor, 
    hoaConfig.m_unMaxGainCorrAmpExp, 
    hoaConfig.m_unSpatialInterpolationTime,
    hoaConfig.m_unSpatInterpMethod,
    uiNoBitsForVecElemQuant,
    hoaConfig.m_unCodedVVecLength,
    uiMaxNoOfTransmittedHOACoeffs,
    vuiSubBandWidths,
    uiDirGridTableIdx,
    uiMaxNumOfPredDirsPerBand,
    uiFirstSBRSubbandIdx,
    vuiParUpmixHoaOrderPerParSubBandIdx,
    vuiPARSubBandWidths,
    m_vbUseRealCoeffsPerParSubband, 
    hoaConfig,
    m_bitStreamWriter->getFrame()) )
    return true;


  // write HoaConfig()
  unsigned int nBitsWritten = m_bitStreamWriter->writeHoaConfig();
  if(nBitsWritten<=0)
    return true;

  // just for testing
  if(nBitsWritten !=  m_bitStreamWriter->getHoaConfigSize())
  {
    std::cout << "HOA config size is wrong" << std::endl;
    return true;
  }

  // byte align HoaConfig() data
  nBitsWritten += m_bitStreamWriter->byteAlignFrameBuffer();

  // write HoaConfig size to the side info size file
  m_sideInfoSizeStream << nBitsWritten << std::endl;

  // allocate buffer
  m_nSamplesRead = 0;
  m_nEncoderDelaySamples = 2048 + m_unAddDelayInSamples; // TODO: Adapt for phase 2
  m_nSamplesWritten = -m_nEncoderDelaySamples;
  m_fHoaSamples.assign(uiNoOfHOACoeffs, std::vector<FLOAT>(0, (FLOAT)0.0));
  m_fTransportChannels.assign(hoaConfig.m_unTotalNumCoders, std::vector<FLOAT>(m_unHoaFrameLength, (FLOAT)0.0));
  m_fTransportChannelsDelayBuffer.assign(hoaConfig.m_unTotalNumCoders, std::vector<FLOAT>(m_unAddDelayInSamples, (FLOAT)0.0));
  m_unHoaSiBits = 0;
  m_unHoaSiCnt = 0;
  m_bEndOfFileSet = false;
  m_bEndOfFileReached = false;
  m_unHoaSiPerFrame = unCoreCoderFrameLength / m_unHoaFrameLength;

  // get new frame to create an empty dummy frame
  HoaFrame & frame = m_bitStreamWriter->getFrame();
  frame.m_bHoaIndependencyFlag = true;
  for(std::vector<std::shared_ptr<CChannelSideInfoData> >::iterator it = frame.m_vChannelSideInfo.begin(); 
     it < frame.m_vChannelSideInfo.end(); ++it)
  {
     *it = std::shared_ptr<CChannelSideInfoData>(new CEmptyInfoChannel());
  }
  for(std::vector<CHoaGainCorrection>::iterator it = frame.m_vGainCorrectionData.begin();
     it != frame.m_vGainCorrectionData.end(); ++it)
  {
     it->m_bCodedGainCorrectionExp[0] = 1;
     it->m_bCodedGainCorrectionExp.resize(1);
  }
  frame.m_bHoaIndependencyFlag = true;
  frame.m_spatPredictionData.m_bPerformPrediction = false;
  frame.m_hoaDirectionalPredictionInfo.m_bUseDirPred = false;
  frame.m_hoaParInfo.m_bPerformPar = false;

  /* create a copy of the dummy frame for later usage */
  m_dummyHoaFrame = std::shared_ptr<HoaFrame>(new HoaFrame(frame));

  // write dummy frames for synchronization between HOAFrames and decoded transport channels in the 3D audio decoder
  if(m_unNumOfDummyFrames>0)
  {
    for(unsigned int idx = 0; idx < m_unNumOfDummyFrames; ++idx)
    {
      // write dummy frame to the binary bit stream file
      nBitsWritten = m_bitStreamWriter->writeFrame(false);

      // check frame size 
      if(static_cast<unsigned int>(nBitsWritten) != m_bitStreamWriter->getFrameSize(false))
      {
        std::cout << "HOAFrame size is wrong" << std::endl;
        return true;
      }

      // write HOA dummy frame size to the side info size file
      m_unHoaSiCnt++;
      m_unHoaSiBits += nBitsWritten;
      if(m_unHoaSiCnt == m_unHoaSiPerFrame)
      {
        // byte align frame buffer before flushing to file
        m_unHoaSiBits += m_bitStreamWriter->byteAlignFrameBuffer();
        m_sideInfoSizeStream << m_unHoaSiBits << std::endl;
        m_unHoaSiCnt = 0;
        m_unHoaSiBits = 0;
      }
    }
  }

  // Open file for writing the HOA transport channels
  m_WavWriter = std::shared_ptr<WavWriter>(new WavWriter());
  if(m_WavWriter->openFile(hoaTransportChanFile, hoaConfig.m_unTotalNumCoders, unSampleRate))
  {
    std::cout << "Cannot open HOA transport channel file " << hoaTransportChanFile << std::endl;
    return true;      
  } 

  return false;
}


//---------------------------------------------------------------------------------------
bool HoaEncoder::encode(std::vector<std::vector<FLOAT>> &vvfHoaSignals)
{
  // check if HOA file is open
  if(!m_bHoaFileIsOpen)
    return true;

  // check number of input channels
  if(vvfHoaSignals.size() != m_fHoaSamples.size() )
    return true;

  // copy to input buffer
  if((m_fHoaSamples[0].size() < m_unHoaFrameLength) & !m_bEndOfFileSet)
  {
    unsigned int unSamplesInBuffer = m_fHoaSamples[0].size();
    unsigned int unSamplesMissing = m_unHoaFrameLength-unSamplesInBuffer;
    unsigned int unSamplesAvailable = vvfHoaSignals[0].size();
    unsigned int unSamplesToCopy = std::min(unSamplesMissing, unSamplesAvailable);
    m_nSamplesRead += unSamplesToCopy;

    for(unsigned int nChan=0; nChan < m_fHoaSamples.size(); ++nChan)
    {
      // copy samples to internal buffer
      m_fHoaSamples[nChan].resize(unSamplesToCopy+unSamplesInBuffer, (FLOAT)0.0);
      std::copy(vvfHoaSignals[nChan].begin(), vvfHoaSignals[nChan].begin()+unSamplesToCopy, 
        m_fHoaSamples[nChan].begin()+unSamplesInBuffer);
      // copy remaining samples of input buffer to the beginning of the buffer
      if(unSamplesToCopy < vvfHoaSignals[nChan].size())
      {
        std::copy(vvfHoaSignals[nChan].begin()+unSamplesToCopy, vvfHoaSignals[nChan].end(), vvfHoaSignals[nChan].begin());
      }  
      // resize output buffer to number of remaining samples
      vvfHoaSignals[nChan].resize(vvfHoaSignals[nChan].size() - unSamplesToCopy);  
    }
  }
  // return if buffer is still empty
  if((m_fHoaSamples[0].size() < m_unHoaFrameLength) & !m_bEndOfFileSet)
    return false;
  else if((m_fHoaSamples[0].size() > 0) & (m_fHoaSamples[0].size() < m_unHoaFrameLength) & m_bEndOfFileSet)
    for(unsigned int nChan=0; nChan < m_fHoaSamples.size(); ++nChan)
      m_fHoaSamples[nChan].resize(m_unHoaFrameLength, (FLOAT)0.0); // fill last frame with zeros

  // set end of file
  if( m_bEndOfFileSet & (m_fHoaSamples[0].size()==0) )
    m_spatialEncoder->setEndOfInputFile();

  // get new frame
  HoaFrame & frame = m_bitStreamWriter->getFrame();

  // call spatial encoding
  if(!m_spatialEncoder->isEndOfOutputFile())
  {
    // set independency flag for the first frame
    if(-m_nSamplesWritten == m_nEncoderDelaySamples)
    {
      frame.m_bHoaIndependencyFlag = true;
    }
    else
    {
      frame.m_bHoaIndependencyFlag = false;
    }    

    if(m_spatialEncoder->process(m_fHoaSamples, m_fTransportChannels, frame))
      return true;
    for(unsigned int nChan=0; nChan < m_fHoaSamples.size(); ++nChan)
      m_fHoaSamples[nChan].resize(0);

    // write frame
    int nBitsWritten = m_bitStreamWriter->writeFrame(false);
    if( nBitsWritten < 0)
      return true;

    // just for testing
    if(static_cast<unsigned int>(nBitsWritten) != m_bitStreamWriter->getFrameSize(false))
    {
      std::cout << "HOAFrame size is wrong" << std::endl;
      return true;
    }

    // write side info sizes files
    m_unHoaSiCnt++;
    m_unHoaSiBits += nBitsWritten;
    if(m_unHoaSiCnt == m_unHoaSiPerFrame)
    {
      // byte align frame buffer before flushing to file
      m_unHoaSiBits += m_bitStreamWriter->byteAlignFrameBuffer();
      m_sideInfoSizeStream << m_unHoaSiBits << std::endl;
      m_unHoaSiCnt = 0;
      m_unHoaSiBits = 0;
    }

    // Add additional delay to the transport channels for synchronization 
    // of HOA frames with the decoded transport channels
    std::vector<std::vector<FLOAT>>::iterator chanDelayIt = m_fTransportChannelsDelayBuffer.begin();
    std::vector<FLOAT> tmpBuffer = *chanDelayIt; 
    unsigned int unTakenSamples = m_unHoaFrameLength - m_unAddDelayInSamples;
    for (std::vector<std::vector<FLOAT>>::iterator chanTransIt = m_fTransportChannels.begin();
      chanTransIt != m_fTransportChannels.end(); ++chanTransIt, ++chanDelayIt)
    {
      // keep new delay samples in tmp buffer
      std::copy(chanTransIt->begin()+unTakenSamples, chanTransIt->end(), tmpBuffer.begin());
      // wrap transport channel by number of delay samples
      std::copy_backward(chanTransIt->begin(), chanTransIt->begin()+unTakenSamples, chanTransIt->end());
      // copy delay sample from the prev. frame to the output buffer
      std::copy(chanDelayIt->begin(), chanDelayIt->begin()+m_unAddDelayInSamples, chanTransIt->begin());
      // copy tmp buffer to delay buffer 
      *chanDelayIt = tmpBuffer;
    }

    // handle buffer length of the transport channels 
    // cut length of transport channel to original file length + encoder delay
    // to recover the OFL at the decoder. 
    m_nSamplesWritten += (int64_t)m_fTransportChannels[0].size();
    if(m_nSamplesWritten > m_nSamplesRead)
    {
      int samplesToWrite = (int)std::max((int64_t)0,  m_nSamplesRead 
        - m_nSamplesWritten 
        + (int64_t)m_fTransportChannels[0].size());
      // update number of samples written
      m_nSamplesWritten =   m_nSamplesWritten - (int64_t)m_fTransportChannels[0].size() 
        + samplesToWrite;
      // resize transport channel output buffer      
      for(int n = 0; (unsigned)n < m_fTransportChannels.size(); ++n)
      {
        m_fTransportChannels[n].resize(samplesToWrite, 0.0); 
      }
    }

    // write samples to transport channels
    m_WavWriter->write(m_fTransportChannels);
  }
  else
  {
    // copy samples from delay buffer if required
    if(m_nSamplesWritten < m_nSamplesRead)
    {
      // Add additional delay to the transport channels for synchronization 
      // of HOA frames with the decoded transport channels
      std::vector<std::vector<FLOAT>>::iterator chanDelayIt = m_fTransportChannelsDelayBuffer.begin();      
      unsigned int unMissingSamples = static_cast<unsigned int>(m_nSamplesRead-m_nSamplesWritten);
      m_nSamplesWritten += unMissingSamples;
      if(unMissingSamples > m_unHoaFrameLength)
      {
        std::cout << "ERROR: Not all input samples have been encoded" << std::endl;
        unMissingSamples = m_unHoaFrameLength;
      }
      unsigned int unSamplesToCopyFromDelayBuffer = std::min(unMissingSamples, static_cast<unsigned int>(chanDelayIt->size()));
      for (std::vector<std::vector<FLOAT>>::iterator chanTransIt = m_fTransportChannels.begin();
        chanTransIt != m_fTransportChannels.end(); ++chanTransIt, ++chanDelayIt)
      {
        // reset output buffer
        chanTransIt->assign(unMissingSamples, static_cast<FLOAT>(0.0));
        // copy delay sample from the prev. frame to the output buffer
        std::copy(chanDelayIt->begin(), chanDelayIt->begin()+unSamplesToCopyFromDelayBuffer, chanTransIt->begin());
      }

      // write samples to transport channels
      m_WavWriter->write(m_fTransportChannels);
    }
    else
    {
      // set end of file reached
      m_bEndOfFileReached = true;
    }
  }

  return false;
}

//---------------------------------------------------------------------------------------
bool HoaEncoder::closeHoaFile()
{
  if(m_bHoaFileIsOpen)
  {
    // write last side info size line
    if(m_unHoaSiCnt>0)
    {
      // get new frame
      HoaFrame & frame = m_bitStreamWriter->getFrame();
      // copy dummy frame to write frame
      frame = *m_dummyHoaFrame;
      // disable HOA inped flag
      frame.m_bHoaIndependencyFlag = false;
      // write missing number of frames
      while(m_unHoaSiCnt != m_unHoaSiPerFrame)
      {
        // write dummy frame
        m_unHoaSiBits += m_bitStreamWriter->writeFrame(false);
        // increase HOA frame count
        m_unHoaSiCnt++;

      }           
      // byte align frame buffer before flushing to file
      m_unHoaSiBits += m_bitStreamWriter->byteAlignFrameBuffer();
      // write frame size to size file
      m_sideInfoSizeStream << m_unHoaSiBits << std::endl;
    }

    // close bit stream writer
    if(m_bitStreamWriter->closeStream())
      return true;
    // close side info sizes stream
    m_sideInfoSizeStream.close();
    // close transport channel writer
    m_WavWriter->closeFile();
    m_bHoaFileIsOpen = false;
    m_bEndOfFileSet = true;
    m_bEndOfFileReached = true;
    m_unHoaSiPerFrame = 1;
    m_unHoaSiBits = 0;
    m_unHoaSiCnt = 0;
    m_nSamplesRead = 0;
    m_nSamplesWritten = 0;
    m_nEncoderDelaySamples = 0;
  }
  return false;
}

//---------------------------------------------------------------------------------------
void HoaEncoder::setEndOfFile()
{
  if(!m_bEndOfFileSet)
  {
    m_bEndOfFileReached = false;
    m_bEndOfFileSet = true;
  }
}

//---------------------------------------------------------------------------------------
bool HoaEncoder::isEndOfFile()
{
  return m_bEndOfFileReached;
}
