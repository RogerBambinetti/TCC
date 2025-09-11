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
$Id: main.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include "DataTypes.h"
#include "HoaDecoder.h"
#include "wavWriter.h"

  /**
    * @brief command line parameter parser
    * 
    *
    */ 
static bool parseCmdParameter(int argc, char* argv[],
                              std::string &sTransportChannelFile,
                              std::string &sSideInfoFile,
                              std::string &sSpeakerFile,
                              std::string &sScreenSizeFile,
                              std::string &sOutputFile,
                              std::string &sOutputFileHoa,
                              int *rotationFlag,
                              std::string &sRotationDataFile,
                              int *drcFlag,
                              std::string &sDrcFile,
                              std::vector<std::string> &sHoaMtxFile,
                              unsigned int &unCoreCoderFrameLength,
                              int *bytesPerSample)
{
  // show copyright header
  std::cout << "*******************************************************************************" << std::endl;
  std::cout << "*                                                                             *" << std::endl;
  std::cout << "*  MPEG-H HOA Decoder Edition 4.0                                      *" << std::endl; 
  std::cout << "*  Copyright (c) 2013-2015 Deutsche Thomson OHG (DTO).                        *" << std::endl;
  std::cout << "*  NFC: Copyright (c) 2013-2015 Orange.                                       *" << std::endl;
  std::cout << "*  Vector based predominant sound synthesis:                                  *" << std::endl; 
  std::cout << "*      Copyright (c) 2013-2016 Qualcomm Technologies, Inc. (QTI).             *" << std::endl; 
  std::cout << "*  All Rights Reserved.                                                       *" << std::endl;
  std::cout << "*                                                                             *" << std::endl;
  std::cout << "*  This software is provided \"as is\" and without warranties as to             *" << std::endl;
  std::cout << "*  performance, merchantability, non-infringement or any other                *" << std::endl;
  std::cout << "*  warranties whether expressed or implied. Because of the various            *" << std::endl;
  std::cout << "*  hardware and software environments into which this software may be         *" << std::endl;
  std::cout << "*  put, no warranty of fitness for a particular purpose is offered.           *" << std::endl;
  std::cout << "*  In no event shall DTO/Orange/QTI be liable for any damages, including      *" << std::endl;
  std::cout << "*  without limitation, direct, special, indirect, or consequential damages    *" << std::endl;
  std::cout << "*  arising out of, or relating to, use of this software by any customer       *" << std::endl;
  std::cout << "*  or any third party. DTO, Orange and QTI are under no obligation to         *" << std::endl;
  std::cout << "*  provide support to customer.                                               *" << std::endl;
  std::cout << "*                                                                             *" << std::endl;
  std::cout << "*******************************************************************************" << std::endl;
  int nArgs = 0;
  unCoreCoderFrameLength = 1024;
  sHoaMtxFile.clear(); 
  // loop over all parameters
  *drcFlag=0;
  *rotationFlag=0;

   sHoaMtxFile.clear(); 
  for ( int i = 1; i < argc; ++i )
    {
        if (!strcmp(argv[i],"-ifpcm"))      /* Required */
        {
            if (++i < argc )
            {
                sTransportChannelFile.assign(argv[i]);
                nArgs++;
                continue;
            }
            else
                break;
        }
        if (!strcmp(argv[i],"-ifside"))      /* Required */
        {
            if (++i < argc )
            {
                sSideInfoFile.assign(argv[i]);
                nArgs++;
                continue;
            }
            else
                break;
        }
        if (!strcmp(argv[i],"-spk"))      /* Required */
        {
            if (++i < argc )
            {
                sSpeakerFile.assign(argv[i]);
                nArgs++;
                continue;
            }
            else
                break;
        }
        else if (!strcmp(argv[i],"-ofpcm"))      /* Required, optional if -ofhoa */
        {
            if (++i < argc )
            {
                sOutputFile.assign(argv[i]);
                nArgs++;
                continue;
            }
            else
                break;
        }
        else if (!strcmp(argv[i],"-ofhoa"))      /* Required, optional if --ofpcm*/
        {
            if (++i < argc )
            {
                sOutputFileHoa.assign(argv[i]);
                nArgs++;
                continue;
            }
            else
                break;
        }

        if (!strcmp(argv[i],"-coreFrameLength"))      /* Required */
        {
          if (++i < argc )
          {
            unCoreCoderFrameLength = atoi(argv[i]);
            nArgs++;
            continue;
          }
          else
            break;
        }

        else if (!strcmp(argv[i],"-scrnInfo"))      /* Optional */
        {
             if (++i < argc )
            {
                sScreenSizeFile.assign(argv[i]);                
                continue;
            }
            else
                break;
        }
        else if (!strcmp(argv[i],"-rot"))      /* Optional */
        {
             if (++i < argc )
            {
                sRotationDataFile.assign(argv[i]);
                *rotationFlag = 1;
                continue;
            }
            else
                break;
        }
        else if (!strcmp(argv[i],"-drc"))      /* Optional */
        {
            if (++i < argc )
            {
                *drcFlag = atoi(argv[i]);
                continue;
            }
            else
                break;
        }
        else if (!strcmp(argv[i], "-drcf"))      /* Optional, but required in DRC case*/
        {
            if (++i < argc)
            {
                sDrcFile.assign(argv[i]);
                continue;
            }
            else
                break;
        }
        else if (!strcmp(argv[i],"-mtx"))      /* Optional */
        {   
            int mtxCount = 0;			
            i++;
            while (argv[i][0] != '-')
            {   				
                sHoaMtxFile.resize(sHoaMtxFile.size()+1);
                sHoaMtxFile[mtxCount++].assign(argv[i]); 
                if (++i >= argc )
                    break;			
            }
            --i;
            continue;
        }
        else if (!strcmp(argv[i],"-bitDepth"))  /* Optional */
        {
            int bitDepth = 0;
            if (++i < argc )
            {
                bitDepth = atoi(argv[i]);
                switch (bitDepth) {
                  case 32:
                    *bytesPerSample = 4;
                    break;
                  default:
                    *bytesPerSample = 3;
                }
                continue;
            }
            else
                break;
        }
        else 
        {
            printf("Wrong argument!\n");
            //return 1;
        }  
    }
  // check number of parameters found
  if(nArgs< 4)
  {
    std::cout << "ERROR: Wrong number of parameters" << std::endl;
    std::cout << "Please call: " << std::endl << "   " << argv[0] << " -ifpcm [transportChan.wac] -ifside [sideInfo.bin] -ofpcm [speakerOutputFile.wav] -spk [speakerPositionFile.txt] [-ofhoa [hoaOutputFile.wav]] [-drc [1=TD] -drcf [drcFileName.wav]] [-mtx [signaledRenderingMatrix1.txt [signaledRenderingMatrix2.txt]]]" << std::endl;
    std::cout << "-ifpcm [transportChan.wav]       full path-name of multichannel WAV file with core-decoded base pcm Audio"<< std::endl;
    std::cout << "-ifside [sideInfo.bin]           side info for spatial decoding" << std::endl;
    std::cout << "-ofpcm [speakerOutputFile.wav]   name of multichannel WAV file with channel based Audio" << std::endl;
    std::cout << "-spk [speakerPositionFile.txt]   name of text file with speaker positions" << std::endl; 
    std::cout << "-ofhoa [hoaOutputFile.wav]       name of multichannel WAV file with HOA Audio (N3D, ACN)" << std::endl;
    std::cout << "-coreFrameLength [1024]          frame length in samples used inside the MPEG-H core coder"<< std::endl;
    std::cout << "-drc [1=TD]                      enable DRC-1 processing, 1: TD, [2: FD to be implemented]" << std::endl; 
    std::cout << "-drcf [drcFileName.wav]          name of multichannel WAV with (linear) DRC coefficients"<< std::endl;
    std::cout << "-rot [rotFileName.wav]           name of a three-channel WAV with normalized rotation data in the channel order Yaw,Pitch,Roll. range [-1..1] for rotation from [-pi..pi]"<< std::endl;
    std::cout << "-scrnInfo [screenInfo.txt]       name of text file with refernce screen info and reproduction screen info"<< std::endl;
    std::cout << "-mtx [signaledRenderingMatrix1.txt [signaledRenderingMatrix2.txt]]]        names of rendering matrix files"<< std::endl;
    std::cout << "-bitDepth [24]                   wave output file format: 24, 32 (float)"<< std::endl;
    std::cout << "[-ofpcm [speakerOutputFile.wav] -spk [speakerPositionFile.txt] ] and/or [-ofhoa [hoaOutputFile.wav]] is possible"<< std::endl;
    return true;
  }
  else
  {
    return false;
  }
};


/**
* @brief main function of the HOA MPEG-H HOA reference decoder
* 
*
*/ 
int main(int argc, char* argv[]){
  // set input and output file names
  std::string sTransportChanFile;
  std::string sSideInfoFile;
  std::string sSpeakerPcmOutputFile;
  std::string sSpeakerHoaOutputFile;
  std::string sRotationDataFile;
  std::string sSpeakerFile;
  std::string sScreenSizeFile;
  std::vector<std::string> sHoaMtxFile;
  std::string sDrcFile;
  unsigned int unCoreCoderFrameLength = 1024;

  int drcFlag;
  int rotationFlag;
  int bytesPerSample=0;
  bool floatFlag=false; // floatFlag = false: write 24 bit .wav file, floatFlag = true: write 32 float .wav file
  int outmode=0; // 0: error no outputs, 1: pcmSpeaker output, 2: hoa output, 3: pcmSpeaker & hoa output


  // parse cmd parameters
  if(parseCmdParameter(argc, argv, 
                       sTransportChanFile,
                       sSideInfoFile,
                       sSpeakerFile,
                       sScreenSizeFile,
                       sSpeakerPcmOutputFile,
                       sSpeakerHoaOutputFile,
                       &rotationFlag,
                       sRotationDataFile,
                       &drcFlag,
                       sDrcFile, 
                       sHoaMtxFile,
                       unCoreCoderFrameLength,
                       &bytesPerSample) )
  {
    std::cout << "Cannot parse all required input parameters" << std::endl;
    return -1;
  } 

  if(sSpeakerPcmOutputFile.size()>0 && sSpeakerFile.size()>0)
        outmode+=1;
  if(sSpeakerHoaOutputFile.size()>0)
        outmode+=2;
  if (outmode==0)
   {
    std::cout << "No output file" << std::endl;
    return -1;
  }

  // create HOA compression decoder
  HoaDecoder hoaDecoder;
  // open compressed HOA file and initialize decoder
  if(hoaDecoder.initHoaDecoder(sTransportChanFile, sSideInfoFile, sSpeakerFile, sScreenSizeFile, sHoaMtxFile, rotationFlag, sRotationDataFile, drcFlag, sDrcFile, unCoreCoderFrameLength, (outmode==2) ))
  {
    return -1;
  }


    // get HOA file info
    HoaInfo hoaInfo = hoaDecoder.getHoaInfo();

    // init wave file writer for speaker signals
    WavWriter chanWavWrites;
    WavWriter hoaWavWriter;

    // create de-interleaved speaker sample buffer
    std::vector<std::vector<FLOAT> > fDeInterleavedSpeakerSampleBuffer;
    std::vector<std::vector<FLOAT> >fDeInterleavedHoaSampleBuffer; 

    // set floatFlag according to bytesPerSample
    if (4 == bytesPerSample) {
      floatFlag = true;
    }
    
    //std::string outputFileName = sSpeakerPcmOutputFile;
    
    if(outmode & 1)
        if(chanWavWrites.openFile(sSpeakerPcmOutputFile, hoaInfo.m_unNumberOfSpeakers, hoaInfo.m_unSampleRate, floatFlag))
        {
          std::cout << "Cannot open multichannel output file" << std::endl;
          return -1;
        }
    
    if(outmode & 2)
        if(hoaWavWriter.openFile(sSpeakerHoaOutputFile, hoaInfo.m_unNumHoaCoefficients, hoaInfo.m_unSampleRate, true))
        {
          std::cout << "Cannot open multichannel output file" << std::endl;
          return -1;
        }

    //start decoding loop     
    unsigned int unTotalNumSamplesDec = 0;                                          
    while( !hoaDecoder.isEndOfFile() )
    {

        // decode and render HOA frame
        if(hoaDecoder.decode(fDeInterleavedSpeakerSampleBuffer))
        {
            std::cout << "HOA decoding failed" << std::endl;
            return -1;     
        }

        // get HOA signals if required
        fDeInterleavedHoaSampleBuffer = hoaDecoder.getHoaSamples();

        // write samples to file
        if( (outmode & 1) && fDeInterleavedSpeakerSampleBuffer.size() )
        {

            unsigned int numSpeakerSamplesDecoded = fDeInterleavedSpeakerSampleBuffer[0].size();

            
            // write output speaker files
            if(chanWavWrites.write(fDeInterleavedSpeakerSampleBuffer))
                {
                    std::cout << "ERROR: Cannot write to speaker file: " << std::endl;
                    return -1; 
                }            
                       
            // display progress
            unTotalNumSamplesDec += numSpeakerSamplesDecoded;
            std::cout << "\rDecoding progress: " << (unsigned int)((double)unTotalNumSamplesDec/(double)hoaInfo.m_unFileSizeInSample *100.0) << "%";
        }
        // write samples to file
        if( (outmode & 2) && fDeInterleavedHoaSampleBuffer.size() )
        {

            unsigned int numSpeakerSamplesDecoded = fDeInterleavedHoaSampleBuffer[0].size();

            
            // write output speaker files
            if(hoaWavWriter.write(fDeInterleavedHoaSampleBuffer))
                {
                    std::cout << "ERROR: Cannot write to hoa file: " << std::endl;
                    return -1; 
                }            
             if(outmode==2)
             {            
            // display progress
            unTotalNumSamplesDec += numSpeakerSamplesDecoded;
            std::cout << "\rDecoding progress: " << (unsigned int)((double)unTotalNumSamplesDec/(double)hoaInfo.m_unFileSizeInSample *100.0) << "%";
            }
        }


    }



    std::cout << std::endl;
    std::cout << "Decoding okay" << std::endl;
    return 0;
}
