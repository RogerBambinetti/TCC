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
 $Rev: 196 $
 $Author: technicolor-ks $
 $Date: 2015-10-12 13:45:11 +0200 (Mo, 12 Okt 2015) $
 $Id: main.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/

#include <iostream>
#include <sstream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <memory>
#include <cstring>
#include "DataTypes.h"
#include "HoaEncoder.h"
#include "wavReader.h"

/*
extract numeric value after searchString from hamString
*/
int getValueFromString(std::string hamString, std::string searchString, float &value)
{
  std::size_t pos1 = hamString.find(searchString);
  if (pos1 ==  std::string::npos)
  {
    std::cout << "ERROR: search string '" << searchString << "' not found" << std::endl;
    return(-1);
  }
  std::size_t pos2 = hamString.find(";", pos1);
  if (pos2 ==  std::string::npos)
  {
    std::cout << "ERROR: search string '" << ";" << "' not found" << std::endl;
    return(-1);
  }
  std::string sNumber = hamString.substr(pos1 + searchString.size(), pos2 - pos1 - searchString.size());
  std::stringstream st(sNumber);
  std::cout << "HOA metadata: " << searchString << sNumber << std::endl;
  st >> value;
  return(0);
}

/*
read .ham file and extract HOA metadata
*/
int parseHamFile(std::string hamFilename,
                  bool &bNfcUsed, /*< [out] NFC flag */
                  float &fNfcDistance  /*< [out] NFC radius */
                  )
{
  // read full txt file into string
  std::ifstream inFile(hamFilename);
  std::string hamString((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());
  if (hamString.size() == 0)
  {
    std::cout << "ERROR: nothing read from .ham file" << std::endl;
    return(-1);
  }

  // remove all spaces from string
  hamString.erase(std::remove_if(hamString.begin(), hamString.end(), isspace), hamString.end());
  
  // read HOA metadata
  float value;
  if (getValueFromString(hamString, "NFCflag=", value))
  {
    return(-1);
  }
  bNfcUsed = (value == 1);
  if(bNfcUsed)
  {
    if (getValueFromString(hamString, "NFCrefDist=", fNfcDistance))
    {
      return(-1);
    }
  }

  return(0);
}

/**
  * @brief function that parses the command line parameter
  *
  */
static bool parseCmdParameter(int argc, /*< cmd. parameter list size */
                              char* argv[], /*< cmd. parameter list */
                              std::string &sTransportChannelFile, /*< [out] name of the transport channel wav file */
                              std::string &sSideInfoFile, /*< [out] name of the HOA side info file */
                              std::string &sSideInfoSizeFile, /*< [out] name of the HOA side info size file */
                              std::string &sFirstHoaInputFile,  /*< [out] name of the input file */
                              unsigned int &unBitRate, /*< [out] total bit rate */
                              bool &bNfcUsed, /*< [out] NFC flag */
                              float &fNfcDistance,  /*< [out] NFC radius */
							         unsigned int &unCoreCoderFrameLength  /*< [out] frame length of MPEG-H core coder */
                              )
{
  // show copyright header
  std::cout << "*******************************************************************************" << std::endl;
  std::cout << "*                                                                             *" << std::endl;
  std::cout << "*  MPEG-H HOA Encoder Edition 4.0                                      *" << std::endl; 
  std::cout << "*  Copyright (c) 2013-2015 Deutsche Thomson OHG (DTO).                        *" << std::endl;
  std::cout << "*  Vector based predominant sound processing:                                 *" << std::endl; 
  std::cout << "*      Copyright (c) 2013-2015 Qualcomm Technologies, Inc. (QTI).        *" << std::endl; 
  std::cout << "*  All Rights Reserved.                                                       *" << std::endl;
  std::cout << "*                                                                             *" << std::endl;
  std::cout << "*  This software is provided \"as is\" and without warranties as to             *" << std::endl;
  std::cout << "*  performance, merchantability, non-infringement or any other                *" << std::endl;
  std::cout << "*  warranties whether expressed or implied. Because of the various            *" << std::endl;
  std::cout << "*  hardware and software environments into which this software may be         *" << std::endl;
  std::cout << "*  put, no warranty of fitness for a particular purpose is offered.           *" << std::endl;
  std::cout << "*  In no event shall DTO/QTI be liable for any damages, including             *" << std::endl;
  std::cout << "*  without limitation, direct, special, indirect, or consequential damages    *" << std::endl;
  std::cout << "*  arising out of, or relating to, use of this software by any customer       *" << std::endl;
  std::cout << "*  or any third party. DTO and QTI are under no obligation to provide         *" << std::endl;
  std::cout << "*  support to customer.                                                       *" << std::endl;
  std::cout << "*                                                                             *" << std::endl;
  std::cout << "*******************************************************************************" << std::endl;
  unsigned int nArgs = 0;
  std::string hamFilename;
  unCoreCoderFrameLength = 1024;
  // loop over all parameters
  for ( int i = 1; i < argc; ++i )
	{
		if (!strcmp(argv[i],"-if"))      /* Required */
		{
			if (++i < argc )
			{
				sFirstHoaInputFile.assign(argv[i]);
        if(sFirstHoaInputFile.find("_00+.wav") == std::string::npos)
        {
          std::cout << "ERROR: first HOA file name has to end with '_00+.wav'" << std::endl;
          return true;
        }
				nArgs++;
				continue;
			}
			else
				break;
		}
		if (!strcmp(argv[i],"-ofpcm"))      /* Required */
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
		else if (!strcmp(argv[i],"-ofside"))      /* Required */
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
		else if (!strcmp(argv[i],"-ofsize"))      /* Required */
		{
			if (++i < argc )
			{
				sSideInfoSizeFile.assign(argv[i]);
				nArgs++;
				continue;
			}
			else
				break;
		}
		else if (!strcmp(argv[i],"-br"))      /* Required */
		{
			if (++i < argc )
			{
        std::stringstream st(argv[i]);
        st >> unBitRate;
				nArgs++;
				continue;
			}
			else
				break;
		}
    else if (!strcmp(argv[i],"-coreFrameLength"))      /* Required */
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
    else if (!strcmp(argv[i],"-ham"))      /* optional */
		{
			if (++i < argc )
			{
				hamFilename.assign(argv[i]);
        if (parseHamFile(hamFilename, bNfcUsed, fNfcDistance))
        {
          return true;
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
	if(nArgs< 6)  
	{   
		std::cout << "ERROR: Wrong number of parameters" << std::endl;
		std::cout << "Please call: " << std::endl << "   " << argv[0] 
         << " -if [firstMonoHoaFile.wav] -ofpcm [transportChan.wav] -ofside [hoaSi.bin] -ofsize [hoaSiSize.txt]"
         << " -br [bit rate in bit/s] [-ham [hoaMetadata.ham]] -coreFrameLength [1024]" 
         << std::endl;
    return true;
  }
  else
  {
    return false;
  }
};

unsigned int buildInputFileNames(std::string sFirstHoaInputFile, 
                          std::vector<std::string> &hoaInputFilenames)
{
  // extract file name prefix
  std::basic_string <char>::size_type index = sFirstHoaInputFile.find("_00+.wav");
  std::string sPrefix = sFirstHoaInputFile.substr(0, index);
  // extract HOA order
  std::stringstream sHoaOrder(sFirstHoaInputFile.substr(index-1, index));
  int hoaOrder;
  sHoaOrder >> hoaOrder;

  // build vector with all HOA file names
  hoaInputFilenames.push_back(sFirstHoaInputFile);
  std::ostringstream sFilename;
  for(int n = 1; n <= hoaOrder; n++)
  {
    for(int m = -n; m < 0; m++)
    {
      sFilename.str(""); // clear buffer
      sFilename << sPrefix << "_" << n << -m << "-.wav"; 
      hoaInputFilenames.push_back(sFilename.str());
    }
    for(int m = 0; m <= n; m++)
    {
      sFilename.str(""); // clear buffer
      sFilename << sPrefix << "_" << n << m << "+.wav"; 
      hoaInputFilenames.push_back(sFilename.str());
    }
  }

  return static_cast<unsigned int>(hoaOrder);
}


/**
  * @brief example executable for the RM0 MPEG-H HOA encoder
  *
  */
int main(int argc, char* argv[])
{
  // HOA encoding parameter
  unsigned int unTotalBitRate = 1200000;
  bool bUseNfc = false;
  float fNfcDistance = 1.0;
  std::string sTransportChannelFile; /*< [out] name of the transport channel wav file */
  std::string sSideInfoFile; /*< [out] name of the HOA side info file */
  std::string sSideInfoSizeFile; /*< [out] name of the HOA side info size file */
  std::string sHoaInputFile;
  std::string sFirstHoaInputFile;
  bool bCoreCoderUsesSbr = false;
  unsigned int unCoreFrameLength = 1024;

  // parse cmd parameter
  if(parseCmdParameter(argc, argv,
                       sTransportChannelFile,
                       sSideInfoFile,
                       sSideInfoSizeFile,
                       sFirstHoaInputFile, 
                       unTotalBitRate,
                       bUseNfc,
                       fNfcDistance,
					        unCoreFrameLength))
                              return -1;
  
  sHoaInputFile = sFirstHoaInputFile;
  // build input file names and get HOA order
  std::vector<std::string> hoaInputFilenames;
  unsigned int unHoaOrder = buildInputFileNames(sFirstHoaInputFile, hoaInputFilenames);
  unsigned int unNumHoaCoeffs = hoaInputFilenames.size();

  // open all mono HOA wav files
  std::vector<WavReader> hoaInputFiles(hoaInputFilenames.size()); // vector of wavio objects
  std::vector<std::string>::iterator fname; // file name iterator

  unsigned int ctr = 0;
  for(fname = hoaInputFilenames.begin(); fname != hoaInputFilenames.end(); fname++, ctr++)
  {
    if( hoaInputFiles[ctr].openFile(fname->c_str()) ) 
    {
      std::cout << "ERROR: cannot open valid input file" << *fname << std::endl;
      return -1;
    }
    if(hoaInputFiles[ctr].m_iNumChannels != 1)
    {
      std::cout << "ERROR: wrong number of channels: " << *fname << std::endl;
      return -1;
    }
  }

  // get fs and length (samples), check for same values in all files
  fname = hoaInputFilenames.begin();
  ctr = 0;
  unsigned int unFs           = static_cast<unsigned int>(hoaInputFiles[ctr].m_fSampleRate);
  unsigned int unNumSamples   = static_cast<unsigned int>(hoaInputFiles[ctr].m_iSizeInSamples);
  //for(ctr = 1; ctr < unNumHoaCoeffs; ctr++)
  for(ctr = 1, fname++; fname != hoaInputFilenames.end(); fname++, ctr++)
  {
    if(unFs != static_cast<unsigned int>(hoaInputFiles[ctr].m_fSampleRate))
    {
      std::cout << "ERROR: wrong sample rate: " << *fname << std::endl;
      return -1;
    }
    if(unNumSamples != static_cast<unsigned int>(hoaInputFiles[ctr].m_iSizeInSamples))
    {
      std::cout << "ERROR: wrong file length: " << *fname << std::endl;
      return -1;
    }
  }

   // set HOA frame length in samples equal to core coder frame length
   unsigned int unFrameLength = unCoreFrameLength;

   // for 256 kbps use half core frame length for HOA coding (only if SBR is enabled)
   if ( (unTotalBitRate == 256000) & (unCoreFrameLength == 2048) )
   {
      unFrameLength = unCoreFrameLength/2;
   }

   // init HOA encoder
   HoaEncoder hoaEncoder;
   if(  hoaEncoder.initHoaEncoder(  sTransportChannelFile,
                                    sSideInfoFile,
                                    sSideInfoSizeFile, 
                                    unFs, 
                                    unTotalBitRate, 
                                    unHoaOrder, 
                                    bUseNfc, 
                                    fNfcDistance, 
                                    bCoreCoderUsesSbr,
                                    unCoreFrameLength,
                                    unFrameLength)
   )
   return -1;

   // create de-interleaved sample buffer
   std::vector<std::vector<FLOAT> > fDeInterleavedSampleBuffer(unNumHoaCoeffs, 
                                             std::vector<FLOAT>(unFrameLength, (FLOAT)0.0));

  //start decoding loop
  unsigned int unSamplesInBuffer = 0;
  unsigned int unTotalSamplesProcessed = 0;
  while(!hoaEncoder.isEndOfFile() )
  {
    // read wav files
    if(unSamplesInBuffer == 0)
    {
      // init read buffer
      fDeInterleavedSampleBuffer.assign(fDeInterleavedSampleBuffer.size(), 
                                std::vector<FLOAT>(unFrameLength, (FLOAT)0.0));
      for(ctr = 0; ctr < unNumHoaCoeffs; ctr++)
      {

        hoaInputFiles[ctr].readSamples(fDeInterleavedSampleBuffer[ctr]);
      }
     
      unSamplesInBuffer = fDeInterleavedSampleBuffer[0].size();
    }

    // do time domain encoding
    if(hoaEncoder.encode(fDeInterleavedSampleBuffer))
    {
      std::cout << "HOA encoding failed" << std::endl;
      return -1;     
    }

    // check for end of file
    if(hoaInputFiles[0].isEof())
      hoaEncoder.setEndOfFile();   

    //handle input buffer
    unTotalSamplesProcessed += unSamplesInBuffer-fDeInterleavedSampleBuffer[0].size();
    unSamplesInBuffer = fDeInterleavedSampleBuffer[0].size();
    std::cout << "\rProgress: " << (unsigned int)(unTotalSamplesProcessed/(double)unNumSamples*100) <<  "%";
    
  }

  // close all input wav files
  for(ctr = 0; ctr < unNumHoaCoeffs; ctr++)
  {
    hoaInputFiles[ctr].closeFile();
  }

  std::cout << std::endl << "Encoding okay" << std::endl;
  return 0;
}
