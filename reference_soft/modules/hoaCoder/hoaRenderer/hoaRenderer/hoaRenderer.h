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
/* hoaRenderer.h 
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:  
*                   - initialize and build rendering matrix according to loudspeaker setup
*                   - process HOA data to channels
* uses:             modules:  HOArenderMtrx to calculate ender matrices ,   spacePositions to handle the speaker positions,*                
*                   optional libsndfile  if  ENABLE_FILE_RENDERING is enabled (http://www.mega-nerd.com/libsndfile/)
* authors:          Johannes Boehm (jb), 
* log:              created 02.09.2013, 
*/
/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: hoaRenderer.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#ifndef __HOARENDERER_H__
#define __HOARENDERER_H__


class HOArenderMtrx; // used forward declaration to reduce number of required include file for using this lib
class spacePositions;
#include "../hoaNfc/hoaNfcFiltering.h"
//class HoaNfcFilter;

#include <string>
#include <stdexcept>
#include <vector>
#include <memory>

#ifndef FLOAT
 #define FLOAT double
#endif


// enables file operations: read and render from a hoa-wav file to channel-wav file
// uses  libsndfile 
//#define ENABLE_FILE_RENDERING 

typedef enum{ satellite = 0,
              subwoofer = 1 } LS_TYPE;


class hoaRenderer
{
public:
   // construct the renderer
   hoaRenderer(const std::string &fileNameWithLoudSpeakerSetup,  // text file with speaker positions, see examples in directory Positions
               int *hoaOrders, int numOfHoaOrders,               // 1D C array that includes as numbers the HOA orders to be supported 
               bool bEnableLFEs //=true,						 // enable or suppress LFEs in output, enable means, zero PCM output in channels at the end of the channel sequence
	            );                  


   // content initialization ( selection of rendering matrix), should be called if parameters of related content change 
   // call before blockProcess anytime the input ( content ) changes
   // returns true on success and false if no matrix matching the parameters could be found (if a rendering ID can not be found, the default matrix matching the HOA order will be used)
   bool inputInit(int hoaOrder,                                    // HOA order of content
	            std::vector<std::string> sHoaMtxFile,              // filnames with transmitted matrices
				const std::string &sScreenSizeFile,
                float fNfcRadius=0.0,                              // NFC radius from content info, a value of zero disables NFC processing
                unsigned long samplingRate=48000,				   // sampling rate for NFC filters, may be used in future versions for the delay handling
				bool isScreenRelative=0);						   // isScreenRelative flag for screen-relative adaptation
				                   	   
			          	



   // returns max. loudspeaker distance
   float GetLdspkDist();
   
   // returns the  m_bUseNfc - if true NFC processing is activated
   bool isNFCpreProcessingActive(){return m_bUseNfc;};
  

   // block based rendering
   // returns: 0 : everything processed as expected
   //          >0: number of clipped samples
   //          <0: nothing processed, this may be ok for values -2 or -4 if there was a processing block to skip and inBuffer had a wrong size (example for file processing- get rid of start up delay)
   //          values of -1 (not initialized, and -3 (out number of channels miss match) should be treated as errors     
   int blockProcess(      std::vector<std::vector<FLOAT> > &inBuffer,   // 2D vector matrix, number of coefficients X time samples
                          std::vector<std::vector<FLOAT> > &outBuffer,  // 2D vector matrix, number of output channels X time samples
                    const std::vector<std::vector<FLOAT> > &objBuffer,  // pre-rendered objects, can be dummy, only if size matches outBuffer it will be mixed to outBuffer
                    bool bEnableClippingProtect=true,                   // turns clipping protection on/off, only simple saturation is implemented as a hook for future improved solutions
                    bool bEnableSpeakerGainAndDelay=false );            // should be set to false, otherwise aborting; a hook for later improvement for sample delay and gain compensation to readjust speaker distances


    // optional method for file handling, needs libsndfile to be linked and included
    // demonstrates use of inputInit() and blockProcess()  and setup of Buffers
#ifdef ENABLE_FILE_RENDERING
   unsigned renderFile(const std::string &hoaInputWavFileName,          // name of HOA hoaOrder multichannel wav file. The HOA coefficients are arranged in frames in sequence order: A00, A1-1, A10, A11, A2-2, A2-1, A2-1, A20, A21, .....  
                   const std::string &channelOutputWavFileName,         // name of multichannel wav file, m_numOutChannels in frame (sequence order from fileNameWithLoudSpeakerSetup), 24 bit signed, little endian
                   std::vector<std::string> sHoaMtxFile);               // filnames with transmitted matrices
#endif

  // returns the number of speakers read from the speaker text file
  unsigned int getNumberOfSpeakers();

  // returns loudspeaker positions as strings in vector 
   const std::vector<std::string > & getSpeakerPositionStrings(){ return m_vsSpeakerPositions;};

   // returns rendering matrix
   std::vector<std::vector<FLOAT> > getRenderingMatrix(); 

private:     
   int m_mtrxSelectedIndx;                      // index to matrix in vector object after content initialization
   int m_preliminaryMtrxSelectedIndx;			// Preliminary Matrix Selection Index 
   int m_numOutChannels;                        // number of output channels 
   std::shared_ptr<std::vector<HOArenderMtrx> > m_pvRenderMtrx; // shared pointer to a vector object that stores rendering matrices 
   std::vector<HOArenderMtrx> &m_vRenderMtrx;   // reference to the shared pointer vector object that stores rendering matrices 
   std::vector<std::string > m_vsSpeakerPositions; // speaker positions (azimuth, elevation) as a string
   float m_LdspkDist;
   bool m_bUseNfc;
   bool m_isScreenRelative;
   std::vector<HoaNfcFilter> m_vHoaNfcFilter;
   std::string m_sFileNameWithLoudSpeakerSetup;
  const hoaRenderer& operator=(const hoaRenderer&);
  void readPositionStrings(const std::string &fileName, int numPositions);
   // for future delay and gain compensation handling
  /* std::vector< unsigned> m_vChDelays;
   std::vector<double> m_vChGains;
   std::vector<std::vector<FLOAT> > delayBuffer;*/
    // TODO description
   bool NfcInit(unsigned int unHoaOrder,
                unsigned int unNumHoaCoefficients,
                float fNfcRadius,
                unsigned int unSampleRate);

   bool permuteLfeChannel(spacePositions *speakerPos,  const std::vector<LS_TYPE> &lsTypes, 
                          const int numOfHoaOrders, const int MtrxSzOffset);
   std::vector<LS_TYPE> getLoudspeakerTypes(const std::string &fileNameWithLoudSpeakerSetup);
   // TODO description
   int preprocessNfc(std::vector<std::vector<FLOAT> > &inBuffer);


   bool permuteSignaledRenderingMatrix(spacePositions *speakerPos, spacePositions *lfePos, const std::vector<LS_TYPE> &lsTypes, const int n);
   bool validateSignaledRenderingMatrix(spacePositions *speakerPos, spacePositions *lfePos, const std::vector<LS_TYPE> &lsTypes, const int n);



   double angularDistance(double inclA, double aziA, double inclB, double aziB);

};


#endif // __HOARENDERER_H__


