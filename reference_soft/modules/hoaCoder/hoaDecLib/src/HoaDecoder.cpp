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
$Id: HoaDecoder.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include <algorithm>
#include "SpatialDecoder.h"
#include "choabitstreamr.h"
#include "HoaDecoder.h"
#include "hoaRenderer.h"
#include "wavReader.h"
#include "DRC1hoa.h"
#include "hoaRotation.h"

/** init HoaInfo frame */
HoaInfo::HoaInfo() : 
   m_unHoaOrder(0),
   m_unNumHoaCoefficients(0),
   m_unFileSizeInSample(0), 
   m_bNFC(false), 
   m_fNfcRadius(0.0), 
   m_unSampleRate(0),
   m_unNumberOfSpeakers(0)
{

}


/************ HoaDecoder functions *******************************************/

HoaDecoder::HoaDecoder() : 
   m_unSpatDelayFrames(0),
   m_unSpatEofFrames(1),
   m_unNumSamplesDecoded(0),
   m_unFrameLength(1024),
   m_bHoaFileIsOpen(false),
   m_unHoaFramesPerCoreFrame(1),
   m_unHoaFrameCnt(0)
{

}


HoaDecoder::~HoaDecoder()
{

}


bool HoaDecoder::initHoaDecoder(const std::string &transportChanFileName, 
                                const std::string &hoaSiFileName,
                                const std::string & speakerFileName,
                                const std::string & sScreenSizeFileName,
                                const std::vector<std::string> &sHoaMtxFile,
                                int rotationFlag,
                                const std::string & rotationFileName,
                                int drcFlag, 
                                const std::string & drc1FileName,
                                unsigned int unCoreCoderFrameLength,
                                bool disableRendering
                                )
{
   m_disableRendering=disableRendering;
   // check if file is already open
   if(m_bHoaFileIsOpen)
   {
      std::cout << "File is already open" << std::endl;
      return true;
   }

   // open WAV file with transport channels
   m_WavReader = std::shared_ptr<WavReader>(new WavReader);
   if(m_WavReader->openFile(transportChanFileName))
   {
      std::cout << "Cannot open transport channel WAV file " << hoaSiFileName << std::endl;
      return true;
   }

   // init bit stream reader
   m_bitStreamReader = std::shared_ptr<CHoaBitStreamR>(new CHoaBitStreamR());
   if( m_bitStreamReader->initHoaFile(hoaSiFileName.c_str(), 
      static_cast<unsigned int>(m_WavReader->m_iNumChannels), unCoreCoderFrameLength) )
   {
      std::cout << "Cannot open HOA SI input file " << hoaSiFileName << std::endl;
      return true;
   }

   // get first Access Frame Info
   const HoaConfig & hoaConfig = m_bitStreamReader->getHoaConfig();

   // init decoding state variables
   m_bHoaFileIsOpen = true; 
   m_unNumSamplesDecoded = 0;
   m_unSpatDelayFrames = 0;
   /* WavReader signals directly after reading the last sample EOF, without having to read any further bits.
   This is different to std::fstream or the old C FILE, which have to read once again and fail to signal EOF.
   As a result m_unSpatEofFrames has to be increased by one for this WavReader implementation.  */
   m_unSpatEofFrames = 1;

   // get file length 
   unsigned int unFileLength =   m_WavReader->m_iSizeInSamples/m_WavReader->m_iNumChannels;

   // set double frame length if required 
   m_unFrameLength = hoaConfig.m_unHoaFrameLength;

   // set number of HOAFrames per core coder frame
   m_unHoaFramesPerCoreFrame = unCoreCoderFrameLength/m_unFrameLength;

   // reset HOAFrame count (index)
   m_unHoaFrameCnt = 0;

   // additional frame at end of file for QMF delay
   m_unSpatEofFrames++; 

   // create new spatial decoder
   m_spatialDecoder = std::shared_ptr<SpatialDecoder> (new SpatialDecoder());
   // init spatial decoder
   if(m_spatialDecoder->init(	hoaConfig, 
      m_unFrameLength))
   {
      std::cout << "Cannot init spatial decoder " << std::endl;
      return true;    
   }

   // set Hoa File Info parameters
   m_hoaInfo.m_bNFC = hoaConfig.m_bUsesNfc;
   m_hoaInfo.m_fNfcRadius = hoaConfig.m_ufAmbNfcReferenceDistance;
   m_hoaInfo.m_bIsScreenRelative = hoaConfig.m_bIsScreenRelative;
   m_hoaInfo.m_unFileSizeInSample = unFileLength;
   m_hoaInfo.m_unHoaOrder = static_cast<unsigned int>
      (std::sqrt(static_cast<double>
      (m_spatialDecoder->getNumberOfHoaCoefficients())
      ) - 1);
   m_hoaInfo.m_unSampleRate = static_cast<unsigned int>(m_WavReader->m_fSampleRate);
   m_hoaInfo.m_unNumHoaCoefficients = m_spatialDecoder->getNumberOfHoaCoefficients();

   // resize time domain sample buffer
   m_fTimeDomainSamples.clear();
   m_fTimeDomainSamples.resize(hoaConfig.m_unTotalNumCoders, 
      std::vector<FLOAT>(m_unFrameLength, 0.0));

   // init DRC1
   m_drc1mode=drcFlag;
   if (m_drc1mode>0)
   {
      m_DRCwavReader = std::shared_ptr<WavReader>(new WavReader);
      if(m_DRCwavReader->openFile(drc1FileName))
      {
         std::cout << "Cannot open transport channel WAV file " << drc1FileName << std::endl;
         return true;
      }
      try
      { 
         //std::shared_ptr<DRC1hoa> m_drc1;
         if (m_drc1mode==2)
            m_drc1 = std::shared_ptr<DRC1hoa>(new DRC1hoa(m_hoaInfo.m_unHoaOrder, 0 ));
         else
            m_drc1 = std::shared_ptr<DRC1hoa>(new DRC1hoa( m_hoaInfo.m_unHoaOrder, 1 ));
      }
      catch(std::exception const& e)          // catch and print exceptions                       
      {
         std::cout << "HOA-DRC1 initialization exception: " << e.what() << std::endl;  
         return true;
      } 

   }
   m_rotationFlag = rotationFlag;
   if (m_rotationFlag)
   {
      m_hoaRotationWavReader = std::shared_ptr<WavReader>(new WavReader);
      if(m_hoaRotationWavReader->openFile(rotationFileName))
      {
         std::cout << "Cannot open rotation data file " << rotationFileName << std::endl;
         return true;
      }
      m_hoaRotation = std::shared_ptr<hoaRotation>(new hoaRotation(m_hoaInfo.m_unHoaOrder));
   }

   if( m_disableRendering)
   {

   }
   else
   {
      // initialize the HOA renderer
      // create an new HOA renderer for the given speaker setup
      try
      {
         m_hoaRenderer = std::shared_ptr<hoaRenderer>(new hoaRenderer
            (speakerFileName, (int *)&m_hoaInfo.m_unHoaOrder, 1, true));
      }
      catch(std::exception const& e)          // catch and print exceptions                       
      {
         std::cout << "Renderer initialization exception: " << e.what() << std::endl;  
         return true;
      } 

      // init renderer using content information
      if(!m_hoaInfo.m_bNFC)  m_hoaInfo.m_fNfcRadius=0.0; // set radius t zero if content has no NFC information  
      if(!m_hoaRenderer->inputInit(m_hoaInfo.m_unHoaOrder,sHoaMtxFile, sScreenSizeFileName,
         m_hoaInfo.m_fNfcRadius,  m_hoaInfo.m_unSampleRate, m_hoaInfo.m_bIsScreenRelative))
      {
         std::cout << "Renderer matrix initialization failed" << std::endl;  
         return true;
      }
      if (m_hoaRenderer->isNFCpreProcessingActive()) std::cout << "NFC processing active" << std::endl;

      // set number of output channels to the HOA info class
      m_hoaInfo.m_unNumberOfSpeakers = m_hoaRenderer->getNumberOfSpeakers();
      // set loudspeaker positions strings to HOA info class
      m_hoaInfo.m_vsSpeakerPositions = m_hoaRenderer->getSpeakerPositionStrings();
      // set rendering matrix to the HOA info class
      m_hoaInfo.m_vvfRenderingMatrix = m_hoaRenderer->getRenderingMatrix();
   }
   return false;
}

bool HoaDecoder::closeHoaFile()
{
   if(m_bHoaFileIsOpen)
   {
      m_bitStreamReader.reset();
      m_WavReader.reset();
      m_spatialDecoder.reset();
      m_bHoaFileIsOpen = false;
   }
   return false;
}


bool HoaDecoder::decode(std::vector<std::vector<FLOAT> > &vvfSpeakerPcmSamplesCS)
{
   // check if file is open
   if(!m_bHoaFileIsOpen)
   {
      std::cout << "Hoa file is not open" << std::endl;
      return true;
   }  

   // check for end of file
   if(isEndOfFile())
   {
      m_fHoaSamples.clear();
      for(std::vector<std::vector<FLOAT> >::iterator it = vvfSpeakerPcmSamplesCS.begin();
         it < vvfSpeakerPcmSamplesCS.end(); ++it)
      {
         it->clear();
      }
      return false;
   }

   // check for new access Frame
   if(m_bitStreamReader->newHoaConfigFound())
   {
      std::cout << "Streaming mode is not supported so far!" << std::endl;
      return true;
   }
   // TODO: Handle new Access Frames

   // read new HOA frame
   bool bReadingFrameFailed = false;
   const HoaFrame & frame = m_bitStreamReader->readHoaFrame(bReadingFrameFailed);
   if(bReadingFrameFailed)
   {
      std::cout << "HOA frame could not be read" << std::endl;
      return true;
   }

   // byte alignment of bit buffer after last HOA frame of the Core frame has been read
   m_unHoaFrameCnt++;
   if(m_unHoaFrameCnt == m_unHoaFramesPerCoreFrame)
   {
      m_bitStreamReader->byteAlignFrameBuffer();
      m_unHoaFrameCnt = 0;
   }

   // do time domain decoding
   if(m_WavReader->readSamples(m_fTimeDomainSamples))
   {
      std::cout << "Perceptual decoding failed" << std::endl;
      return true;     
   }

   // resize time domain sample buffer if it is smaller than the frame size
   for(unsigned int n=0; n < m_fTimeDomainSamples.size(); ++n)
   {
      // Reset sample buffer to zero if the HOA frame couldn't be read.
      if(m_bitStreamReader->isEof())
      {
         m_fTimeDomainSamples[n].assign(m_unFrameLength, static_cast<FLOAT>(0.0));
      }
      else
      {
         if(m_fTimeDomainSamples[n].size() < m_unFrameLength)
            m_fTimeDomainSamples[n].resize(m_unFrameLength);
         // convert sample buffer to float accuracy to test bit exactness with call for proposal decoder
#ifdef EXACTNESS_TEST
         for(unsigned int m=0; m < m_fTimeDomainSamples[n].size(); ++m)
         {
            float fTmp = static_cast<float>(m_fTimeDomainSamples[n][m]);
            m_fTimeDomainSamples[n][m] = static_cast<FLOAT>(fTmp);
         }
#endif
      }
   }


   // do spatial decoding
   if(m_spatialDecoder->process(m_fTimeDomainSamples, m_fHoaSamples, frame))
   {
      std::cout << "Spatial decoding failed" << std::endl;
      return true;
   }

   // check for delay
   unsigned int nMaxSamplesOut = 0;
   if(m_unSpatDelayFrames == 0)
   {
      // compute frame size from number of samples left
      nMaxSamplesOut = m_unFrameLength; // quick fix to disable OFL for Phase2 due to QMF delay
   }
   else
   {
      // decrease number of delay frames
      m_unSpatDelayFrames--;
   }      

   // cut samples to match OFL if required
   if( (nMaxSamplesOut != m_unFrameLength) )
   {
      for(std::vector<std::vector<FLOAT> >::iterator it = m_fHoaSamples.begin();
         it < m_fHoaSamples.end(); ++it)
      {
         it->resize(nMaxSamplesOut);
      }
   }      

   // increase number of processed samples
   m_unNumSamplesDecoded += nMaxSamplesOut;

   // call renderer
   if(nMaxSamplesOut)
   {
      //  DRC 1 
      if (m_drc1mode>0)
      {
         m_DCR1Samples.resize(m_fHoaSamples.size());
         for (int i=0;i<(int)m_fHoaSamples.size(); i++)  m_DCR1Samples[i].resize(m_fHoaSamples[i].size());

         if (m_DRCwavReader->readSamples(m_DCR1Samples))
         {
            std::cout << "Reading HOA DRC1 coefficients failed" << std::endl;
            return true;     
         }
         m_drc1->blockProcess(m_fHoaSamples, m_DCR1Samples, m_fHoaSamples, false, true);
         // clipping protect is disabled becauase anti clipping in renderer

      }
      // HOA rotation	 
      if (m_rotationFlag)
      {
         m_hoaRotationSamples.resize(3);
         for (int i=0;i<3; i++)
         {
            m_hoaRotationSamples[i].resize(m_unFrameLength);
         }
         if (m_hoaRotationWavReader->readSamples(m_hoaRotationSamples))
         {
            std::cout << "Reading HOA Rotation parameter failed" << std::endl;
            return true;     
         }
         m_hoaRotation->blockProcess(m_fHoaSamples, m_hoaRotationSamples, m_fHoaSamples);
      }


      if (!m_disableRendering)
      {
         // render HOA to speaker signals
         int err = m_hoaRenderer->blockProcess(m_fHoaSamples, vvfSpeakerPcmSamplesCS, std::vector<std::vector<FLOAT> >());
         if(err > 0)
         {
            std::cout << "Number of clipped samples: " << err << std::endl;
         }
         else if(err < 0)
         {
            std::cout << "Renderer error: " << err << std::endl;
            return true;
         }
      }
   }
   else
   {
      // clear output buffers
      for(std::vector<std::vector<FLOAT> >::iterator it = vvfSpeakerPcmSamplesCS.begin();
         it < vvfSpeakerPcmSamplesCS.end(); ++it)
      {
         it->clear();
      }
   }

   // everything is OKAY
   return false;
}


const std::vector<std::vector<FLOAT> > &HoaDecoder::getHoaSamples()
{
   return m_fHoaSamples;
}

HoaInfo HoaDecoder::getHoaInfo()
{
   return m_hoaInfo;
}


bool HoaDecoder::isEndOfFile()
{
   /* WavReader signals directly after reading the last sample EOF, without having to read any further bits.
   This is different to std::fstream or fopen, which have to read once again and fail to signal EOF.
   As a result m_unSpatEofFrames has to be increased by one for this WavReader implementation.  */
   if(m_WavReader->isEof() & (m_unSpatEofFrames>0))
   { 
      m_unSpatEofFrames--;
      return false;
   }
   return ( m_WavReader->isEof() & (m_unSpatEofFrames == 0));
}
