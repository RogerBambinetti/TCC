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
/* hoaRenderer.cpp 
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:  
*                   - initialize and build rendering matrix according to loudspeaker setup
*                   - process HOA data to channels
* uses:             modules:  HOArenderMtrx to calculate ender matrices ,   spacePositions to handle the speaker positions,          
*                   optional/ disabled for MPEG libsndfile  if  ENABLE_FILE_RENDERING is enabled (http://www.mega-nerd.com/libsndfile/)
* authors:          Johannes Boehm (jb), J.DANIEL for NFC integration
* log:              created 02.09.2013, 
*/
/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: hoaRenderer.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/


#include "hoaRenderer.h"
#include "HOArenderMtrx.h" 
#include "hoaNfcFiltering.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <algorithm> 
#include "scrnAdapt.h"


#ifdef ENABLE_FILE_RENDERING 
#include "sndfile.hh"   //  libsndfile:  http://www.mega-nerd.com/libsndfile/
#endif



//-----------------------------------------------------------------------------------------------------------------------------------------
// construct renderer
hoaRenderer::hoaRenderer(const std::string &fileNameWithLoudSpeakerSetup, int *hoaOrders, int numOfHoaOrders, bool bEnableLFEs)
: m_mtrxSelectedIndx(-1), m_numOutChannels(0), m_pvRenderMtrx(std::shared_ptr<std::vector<HOArenderMtrx> >(new std::vector<HOArenderMtrx>)), m_vRenderMtrx(*m_pvRenderMtrx)
{
    m_sFileNameWithLoudSpeakerSetup=fileNameWithLoudSpeakerSetup;
    spacePositions speakerPos( m_sFileNameWithLoudSpeakerSetup, 1);
    spacePositions lfePos( m_sFileNameWithLoudSpeakerSetup, 1, true);
    std::vector<LS_TYPE> lsTypes = getLoudspeakerTypes(m_sFileNameWithLoudSpeakerSetup);
    int nLfe=0;
    if  (bEnableLFEs)
        nLfe=lfePos.getNumPositions();
    int MtrxSzOffset=0;

    m_LdspkDist = (float)speakerPos.getMaxDistance();

    m_bUseNfc = false;

    m_vRenderMtrx.resize(numOfHoaOrders+1);  // the last slot is intended to hold a transmitted matrix
 
   for (int n=0; n<numOfHoaOrders; n++)
      {
        m_vRenderMtrx[n+MtrxSzOffset]  = HOArenderMtrx(fileNameWithLoudSpeakerSetup, 1, hoaOrders[n], 0, bEnableLFEs);   	  
      }
    if  (bEnableLFEs)
        if( permuteLfeChannel(&speakerPos, lsTypes, numOfHoaOrders, MtrxSzOffset) )
        {
          throw(std::runtime_error("Reference setup - miss-match of number of channels"));
        }
    
  m_numOutChannels= nLfe+ speakerPos.getNumPositions();
  readPositionStrings(fileNameWithLoudSpeakerSetup, m_numOutChannels);  // fill the vector with strings of positions
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// get max loudspeaker distance
float hoaRenderer::GetLdspkDist()
{
    return m_LdspkDist;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// content related initialization
bool hoaRenderer::inputInit(int hoaOrder, std::vector<std::string> sHoaMtxFile, const std::string &sScreenSizeFile, float fNfcRadius,  unsigned long samplingRate, bool isScreenRelative)
{
     m_preliminaryMtrxSelectedIndx=-1;
 if (fNfcRadius > m_LdspkDist)
 {
    m_bUseNfc = true;
    NfcInit(hoaOrder, (hoaOrder+1)*(hoaOrder+1),  fNfcRadius, samplingRate);
 }
 else  m_bUseNfc = false;

 if (sHoaMtxFile.size()) 
 {	 
     bool bMatchingMtx = false;
     spacePositions speakerPos( m_sFileNameWithLoudSpeakerSetup, 1);
     spacePositions lfePos( m_sFileNameWithLoudSpeakerSetup, 1, true);
     std::vector<LS_TYPE> lsTypes = getLoudspeakerTypes(m_sFileNameWithLoudSpeakerSetup);
     int numOfSignaledMatrices = sHoaMtxFile.size();
     int idx=m_vRenderMtrx.size()-1;
     for (int n=0; n<numOfSignaledMatrices; ++n)
        {   			
            m_vRenderMtrx[idx] = HOArenderMtrx(sHoaMtxFile[n]);
            bMatchingMtx = permuteSignaledRenderingMatrix(&speakerPos, &lfePos, lsTypes, idx);
            if (bMatchingMtx)
            {
                bMatchingMtx = validateSignaledRenderingMatrix(&speakerPos, &lfePos, lsTypes, idx);
            }
            if (bMatchingMtx)
            {
                m_preliminaryMtrxSelectedIndx=idx;
                break;
            }					  
        }	
  }

  if( m_preliminaryMtrxSelectedIndx==-1)  
      for(int t=0;t<(int) m_vRenderMtrx.size()-1; t++)
          if ( m_vRenderMtrx[t].getRendererID()==0 && m_vRenderMtrx[t].getHOAorder()==hoaOrder)
          {
              m_preliminaryMtrxSelectedIndx=t;
              break;
          }

  if( m_preliminaryMtrxSelectedIndx==-1)      
    return false;  
  
  if (isScreenRelative && sScreenSizeFile.length()){	  
	  m_mtrxSelectedIndx = m_vRenderMtrx.size();
	  m_vRenderMtrx.resize(m_mtrxSelectedIndx+1); 	
	  unsigned int numGridPoints = 900;
	  scrnAdapt(sScreenSizeFile, numGridPoints, m_vRenderMtrx[m_preliminaryMtrxSelectedIndx], m_vRenderMtrx[m_mtrxSelectedIndx]); 
  }
  else{
	  m_mtrxSelectedIndx = m_preliminaryMtrxSelectedIndx;
  }

 return (true); 
  
  

}

//-----------------------------------------------------------------------------------------------------------------------------------------
// init Nfc
bool hoaRenderer::NfcInit(unsigned int unHoaOrder, unsigned int unNumHoaCoefficients, float fNfcRadius, unsigned int unSampleRate)
{
    unsigned int unCurrentComponent=0;
    float tNFC;
    float tNFS;
    
    tNFC = m_LdspkDist/343.f*unSampleRate;
    tNFS = fNfcRadius/343.f*unSampleRate;

    m_vHoaNfcFilter.resize(unNumHoaCoefficients);

    for(unsigned int unCurrentOrder=0 ; unCurrentOrder<=unHoaOrder ; unCurrentOrder++)
    {
        unsigned int unMaxComponent = 2*unCurrentOrder + 1;
        for(unsigned int unComponent=0 ; unComponent<unMaxComponent ; unComponent++)
        {
            m_vHoaNfcFilter[unCurrentComponent] = HoaNfcFilter((int)unCurrentOrder, (FLOAT)tNFC, (FLOAT)tNFS);
            unCurrentComponent++;
        }
    }

    return (true);
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// render block of data
int hoaRenderer::preprocessNfc(std::vector<std::vector<FLOAT> > &inBuffer)
{
    FLOAT *fInBuffer;

    unsigned hoaChannels=(m_vRenderMtrx[m_mtrxSelectedIndx].getHOAorder()+1)*(m_vRenderMtrx[m_mtrxSelectedIndx].getHOAorder()+1);
    if (hoaChannels !=  inBuffer.size() )
        return -2;
    if (hoaChannels < m_vHoaNfcFilter.size() )
        return -3;

    if(m_bUseNfc)
    {
        // Filtering
        for(unsigned int mN=0 ; mN<hoaChannels ; mN++)
        {
            fInBuffer = inBuffer[mN].data();
            m_vHoaNfcFilter[mN].DoFiltering(fInBuffer, fInBuffer, inBuffer[mN].size());
        }
    }

    return m_bUseNfc;
}



//-----------------------------------------------------------------------------------------------------------------------------------------
// render block of data
int hoaRenderer::blockProcess( std::vector<std::vector<FLOAT> > &inBuffer, std::vector<std::vector<FLOAT> > &outBuffer, 
                              const std::vector<std::vector<FLOAT> > &objBuffer,  
                              bool bEnableClippingProtect,  bool bEnableSpeakerGainAndDelay)
{


    int nValidSamples= inBuffer[1].size();
    // resize output buffer
    outBuffer.assign(m_numOutChannels, std::vector<FLOAT>(nValidSamples, 0.0));

    if (m_mtrxSelectedIndx==-1) // not initialized
        return -1;
    // check consistency 
    unsigned hoaChannels=(m_vRenderMtrx[m_mtrxSelectedIndx].getHOAorder()+1)*(m_vRenderMtrx[m_mtrxSelectedIndx].getHOAorder()+1);
    if (hoaChannels !=  inBuffer.size() )
        return -2;
    if(nValidSamples==0)
        return 0; // nothing to process

    if(m_bUseNfc)
        preprocessNfc(inBuffer);
    


    // render- matrix multiplication selected render matrix with HOA time data   
    for(int och=0; och<m_numOutChannels; och++)
    {
        for (int nt=0; nt<(int)nValidSamples; nt++)
        {
            FLOAT Fval=0;
            for (int mN=0; mN<(int)hoaChannels; mN++)
            {
                Fval += (FLOAT) (m_vRenderMtrx[m_mtrxSelectedIndx])[och][mN] * inBuffer[mN][nt];   
                // m_vRenderMtrx[m_mtrxSelectedIndx] gives the HOArenderMtrx object, which can be addressed like an 2D array
                //std::cout<<" "<< (m_vRenderMtrx[m_mtrxSelectedIndx])[och][mN];
            }
            outBuffer[och][nt]= Fval;
        }

    }
    int numOfClippedSamples=0;
    // mix in pre-rendered objects
    if(objBuffer.size()== outBuffer.size() && objBuffer[1].size()>0)  
    {
        for(int och=0; och<m_numOutChannels; och++)
            for (int nt=0; nt<(int)nValidSamples; nt++)
                outBuffer[och][nt]+=objBuffer[och][nt];

    }


    // hook for delay and gain compensation option
    if (bEnableSpeakerGainAndDelay)
        throw(std::runtime_error("Error - renderer process: Gain & Delay speaker compensation is not implemented - disable in call to blockProcess"));

    // simple clipping detection and saturation is done, a hook for later improvements; also the right place to apply dynamic range compression
    if(bEnableClippingProtect)
    {
        for(int och=0; och<m_numOutChannels; och++)
        {
            //double lsb=1.0/(1L<<23);
            for (int nt=0; nt<(int)nValidSamples; nt++)
            {
                if (outBuffer[och][nt]>=1.0)
                {
                    outBuffer[och][nt]=1.0;//-lsb;
                    numOfClippedSamples++;
                }
                if (outBuffer[och][nt]<-1.0)
                {
                    outBuffer[och][nt]=-1.0;
                    numOfClippedSamples++;
                }
            }
        }
    }

    return numOfClippedSamples;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// return rendering matrix
std::vector<std::vector<FLOAT> > hoaRenderer::getRenderingMatrix()
{
  std::vector<std::vector<FLOAT> > vMatrix;
  if (m_mtrxSelectedIndx==-1) // not initialized
    return vMatrix;
  unsigned hoaChannels=(m_vRenderMtrx[m_mtrxSelectedIndx].getHOAorder()+1)*(m_vRenderMtrx[m_mtrxSelectedIndx].getHOAorder()+1);
  vMatrix.assign(m_numOutChannels, std::vector<FLOAT>(hoaChannels, 0.0));
  for(int och=0; och<m_numOutChannels; och++)
  {
    for (int mN=0; mN<(int)hoaChannels; mN++)
    {
        vMatrix[och][mN] = (FLOAT)(m_vRenderMtrx[m_mtrxSelectedIndx])[och][mN];   
    }
  }
  return vMatrix;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// optional file handling method or code example for usage
#ifdef ENABLE_FILE_RENDERING
unsigned hoaRenderer::renderFile(const std::string &hoaInputWavFileName, const std::string &channelOutputWavFileName,  std::vector<std::string> sHoaMtxFile )
{
 SF_INFO sWavInInfo, sWavOutInfo;
 SNDFILE * pWavIn = sf_open(hoaInputWavFileName.c_str(), SFM_READ, &sWavInInfo);
 if (pWavIn==NULL)
    throw(std::runtime_error("Error - unable to open input HOA wave file"));
 int hoaOrder = int(sqrt(double(sWavInInfo.channels)))-1;
 sWavOutInfo.samplerate=sWavInInfo.samplerate;
 sWavOutInfo.channels=m_numOutChannels;
 sWavOutInfo.format=SF_FORMAT_WAV | SF_FORMAT_PCM_24; // set output format to 24 bits, wav, little endian
 SNDFILE * pWavOut = sf_open(channelOutputWavFileName.c_str(), SFM_WRITE, &sWavOutInfo);
 if (pWavOut==NULL)
     throw(std::runtime_error("Error - unable to open output wave file"));
 
 if( m_mtrxSelectedIndx ==-1)
    if (!inputInit( hoaOrder, sHoaMtxFile))
          throw(std::runtime_error("Error - HOA order not supported"));     
 
 int blkSz = 4096;
 std::vector<FLOAT> vInInterleavedSamples, vOutInterleavedSamples;
 vInInterleavedSamples.resize(blkSz*sWavInInfo.channels);
 vOutInterleavedSamples.resize(blkSz*m_numOutChannels);
 std::vector<std::vector<FLOAT> >  vDataIn, vDataOut, dummy;
 vDataIn.resize(sWavInInfo.channels);
 vDataOut.resize(m_numOutChannels);
 for (int i=0;i<sWavInInfo.channels; i++) vDataIn[i].resize(blkSz);
 for (int i=0; i<m_numOutChannels; i++) vDataOut[i].resize(blkSz);
 sf_count_t framesRead = sf_readf_double(pWavIn,vInInterleavedSamples.data(), blkSz );
 unsigned numClippedSamples=0;
 while (framesRead>0)
 {
   // de-interleave
   int scount=0;
   for(int f=0; f< framesRead; f++)
        for(int c=0; c<sWavInInfo.channels; c++)
            vDataIn[c][f]=vInInterleavedSamples[scount++];
   if (framesRead<blkSz)
       for(int c=0; c<sWavInInfo.channels; c++)
           vDataIn[c].resize(int(framesRead));
   // render
   int retV = blockProcess(vDataIn, vDataOut, dummy);
   if (retV >-1)
   {
      numClippedSamples+=retV; 
      // interleave
      int cnt=0;
      for(int f=0; f< framesRead; f++)
          for(int c=0; c<m_numOutChannels; c++)   
                  vOutInterleavedSamples[cnt++] = vDataOut[c][f];
      // write 
      sf_writef_double(pWavOut, vOutInterleavedSamples.data(),framesRead);
   }
  // read next
  framesRead = sf_readf_double(pWavIn,vInInterleavedSamples.data(), blkSz );
  std::cout<<".";
 }
 sf_close(pWavIn) ;
 sf_close(pWavOut) ;
 return numClippedSamples;
}
#endif

//-----------------------------------------------------------------------------------------------------------------------------------------
unsigned int hoaRenderer::getNumberOfSpeakers()
{
  return m_numOutChannels;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
void hoaRenderer::readPositionStrings (const std::string &fileName, int numPositions)
{  // there is no error checking for this function, because it is called when file consistency is already prooved
  using namespace std;
  string line;
  ifstream myfile(fileName);
    // get number of lines in file
    m_vsSpeakerPositions.resize(numPositions);
    if (myfile.is_open())
    {
      for (int ii=0; ii<numPositions; ii++)
            {
             getline (myfile,line);
              if (line.compare(0,4,"LFE_") != 0) 
                    m_vsSpeakerPositions[ii]=line.substr(6,10);
              else
                    m_vsSpeakerPositions[ii]=string("LFE_")+line.substr(10,14);
            }
    }    
    myfile.close();

} 

/*
//-----------------------------------------------------------------------------------------------------------------------------------------
// permute reference matrix to other order of positions
bool hoaRenderer::permuteReferenceMatrices(spacePositions *speakerPos, const std::vector<LS_TYPE> &lsTypes)
{
  // check for spherical coordinates
  if(speakerPos->getType() != 1)
    return true;
  // create an empty permutation matrix
  Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> permuteMatrix(lsTypes.size(), lsTypes.size());

  int nSubIdx = speakerPos->getNumPositions(); 
  int nHoaIdx = 0;

  // create permutation matrix
  for(unsigned int n=0; n<lsTypes.size(); ++n)
  {
    int idx = 0;
    if(lsTypes[n] == subwoofer)
    {
      idx = nSubIdx;
      nSubIdx++;
    }
    else
    {
      while(  (idx < speakerPos->getNumPositions()) 
            & (   (static_cast<double>(nVendorMatrixPositions[idx][0]) 
                   != std::floor((*speakerPos)[nHoaIdx][2]*180.0/PI+.5))
                | (static_cast<double>(nVendorMatrixPositions[idx][1]) 
                   != -std::floor((*speakerPos)[nHoaIdx][1]*180.0/PI+.5)+90.0))
           )
      {
        idx++;
      }
      if(idx== (int) lsTypes.size()) 
        return true;
      nHoaIdx++;
    }
    for(unsigned int m=0; m<lsTypes.size(); ++m)
    {
      if((int) m==idx)
        permuteMatrix(n,m) = 1.0;
      else
        permuteMatrix(n,m) = 0.0;
    }
  }

  // test permutation matrix
  //Eigen::Matrix<double,Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> testVector(lsTypes.size(), 1);
  //Eigen::Matrix<double,Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> testResultVector(lsTypes.size(), 1);
  //for(unsigned int m=0; m<lsTypes.size(); ++m)
  //  testVector(m,0) = m+1;
  //testResultVector = permuteMatrix*testVector;
  //std::cout << testResultVector.transpose() << std::endl;


  // multiply all reference matrices with the permutation matrix
  for (int n= 0; n<NumRenderIds;n++)
  {
    *(m_vRenderMtrx[n].getMtrxPtr()) = permuteMatrix * (*m_vRenderMtrx[n].getMtrxPtr());
  }

  return false;
}
*/
//-----------------------------------------------------------------------------------------------------------------------------------------
// get loudspeaker type for each speaker file position
std::vector<LS_TYPE> hoaRenderer::getLoudspeakerTypes(const std::string &fileNameWithLoudSpeakerSetup)
{
  std::vector<LS_TYPE> lsTypes;
  std::ifstream file(fileNameWithLoudSpeakerSetup);
  while(file.good())
  {
    std::string line;
    std::getline(file, line);
    if(line.size() > 3)
    {
      if(line.compare(0, 4, "LFE_")==0)
        lsTypes.push_back(subwoofer);
      else
        lsTypes.push_back(satellite); 
    }
  }
  return lsTypes;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
// permutes the LFE channels, which are located at the end of the renderer matrix, to the position indicated in the speaker file
bool hoaRenderer::permuteLfeChannel(spacePositions *speakerPos,  const std::vector<LS_TYPE> &lsTypes,
                                    const int numOfHoaOrders, const int MtrxSzOffset)
{
  // check for spherical coordinates
  if(speakerPos->getType() != 1)
    return true;
  // create an empty permutation matrix
  simpleMtrx permuteMatrix(lsTypes.size(), lsTypes.size());

  unsigned int nSubIdx = speakerPos->getNumPositions(); 
  unsigned int nHoaIdx = 0;

  // create permutation matrix
  for(unsigned int n=0; n<lsTypes.size(); ++n)
  {
    unsigned int idx = 0;
    if(lsTypes[n] == subwoofer)
    {
      idx = nSubIdx;
      nSubIdx++;
    }
    else
    {
      idx = nHoaIdx;
      nHoaIdx++;
    }
    for(unsigned int m=0; m<lsTypes.size(); ++m)
    {
      if(m==idx)
        permuteMatrix[n][m] = 1.0;
      else
        permuteMatrix[n][m] = 0.0;
    }
  }

  // test permutation matrix
  //Eigen::Matrix<double,Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> testVector(lsTypes.size(), 1);
  //Eigen::Matrix<double,Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor> testResultVector(lsTypes.size(), 1);
  //for(unsigned int m=0; m<lsTypes.size(); ++m)
  //  testVector(m,0) = m+1;
  //testResultVector = permuteMatrix*testVector;
  //std::cout << testResultVector.transpose() << std::endl;


  // multiply all reference matrices with the permutation matrix
  for (int n= 0; n<numOfHoaOrders;n++)
  {
    *(m_vRenderMtrx[n+MtrxSzOffset].getMtrxPtr()) = permuteMatrix * (*m_vRenderMtrx[n+MtrxSzOffset].getMtrxPtr());
  }

  return false;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
// New stuff from Nils to handle transmitted matrices

double hoaRenderer::angularDistance(double inclA, double aziA, double inclB, double aziB)
{
    return ( acos( std::max(-1.0,(std::min(1.0, (cos(inclA)*cos(inclB)) + (sin(inclA)*sin(inclB)*cos(aziA-aziB)))))));

}



// function to re-arrange a signaled rendering matrix to the best fit to the local loudspeaker setup
bool hoaRenderer::permuteSignaledRenderingMatrix(spacePositions *speakerPos, spacePositions *lfePos, const std::vector<LS_TYPE> &lsTypes, const int n)
{ 

    bool bMatchingMtx = (m_vRenderMtrx[n].getNumberOfSpeakers() == m_numOutChannels); 
    if (m_vRenderMtrx[n].getNumberOfSubwoofer() != lfePos->getNumPositions()) {bMatchingMtx = false;}
    if (m_vRenderMtrx[n].getNumberOfSatellites() != speakerPos->getNumPositions()) {bMatchingMtx = false;}

    if (!bMatchingMtx)
    {
        return bMatchingMtx;
    } 
    else 
    {   		
        int uiLocalLfeIdx = 0;
        int uiLocalLsIdx = 0;
        double inclLocal, aziLocal, inclSignaled, aziSignaled;

        // 1. compute distance matrix across speakers, take care that LFEs and satellites are not mixed up	
        double fVeryBigValue = 1111111;
        std::vector<std::vector<double >> distMatrix;
        distMatrix.resize(m_numOutChannels);
        for (int iSignaledLs=0; iSignaledLs<m_numOutChannels; ++iSignaledLs) 
        {
            distMatrix[iSignaledLs].resize(m_numOutChannels);
        }

        for (int iLocalLs=0; iLocalLs<m_numOutChannels; ++iLocalLs)
        {		
            if (lsTypes[iLocalLs]){
                inclLocal = lfePos->getPositionsArrayRaw()[uiLocalLfeIdx*3+1];
                aziLocal  = lfePos->getPositionsArrayRaw()[uiLocalLfeIdx*3+2];
                uiLocalLfeIdx++;				
            } else {
                inclLocal = speakerPos->getPositionsArrayRaw()[uiLocalLsIdx*3+1];
                aziLocal  = speakerPos->getPositionsArrayRaw()[uiLocalLsIdx*3+2];
                uiLocalLsIdx++;				
            }			  

            for (int iSignaledLs=0; iSignaledLs<m_numOutChannels; ++iSignaledLs)
            {   
                if (lsTypes[iLocalLs] != m_vRenderMtrx[n].getSignaledSpeakerType()[iSignaledLs])	
                {   
                    // The speaker types must match for resorting
                    distMatrix[iSignaledLs][iLocalLs] = fVeryBigValue; 
                }
                else
                {
                    inclSignaled = m_vRenderMtrx[n].getSignaledSpeakerPositionRad()[iSignaledLs][1];
                    aziSignaled  = m_vRenderMtrx[n].getSignaledSpeakerPositionRad()[iSignaledLs][2];
                    distMatrix[iSignaledLs][iLocalLs] = angularDistance(inclLocal, aziLocal, inclSignaled, aziSignaled);
                }
            }
        }
        // 2. solve assignment problem, a better solution may be found via munkres algorithm
        std::vector<unsigned int> vuiNewOrder;
        vuiNewOrder.resize(m_numOutChannels);
        for (int iSignaledLs=0; iSignaledLs<m_numOutChannels; ++iSignaledLs)
        {
            std::vector<double>::iterator result = std::min_element(distMatrix[iSignaledLs].begin(), distMatrix[iSignaledLs].end() );
            unsigned int index = result - distMatrix[iSignaledLs].begin();
            vuiNewOrder[iSignaledLs] = index; 
            for (int m=0; m<m_numOutChannels; ++m)			
                distMatrix[m][index] = fVeryBigValue; // erasing selected index from future selection			
        }

        // 3. resort Rendering matrix		
        simpleMtrx permuteMatrix(lsTypes.size(), lsTypes.size());
        for (int iSignaledLs=0; iSignaledLs<m_numOutChannels; ++iSignaledLs)
        {
            for (int iLocalLs=0; iLocalLs<m_numOutChannels; ++iLocalLs)
            {	  
                if(iLocalLs==vuiNewOrder[iSignaledLs])
                    permuteMatrix[iLocalLs][iSignaledLs] = 1.0;
                else
                    permuteMatrix[iLocalLs][iSignaledLs] = 0.0;
            }
        }

        *(m_vRenderMtrx[n].getMtrxPtr()) = permuteMatrix * (*m_vRenderMtrx[n].getMtrxPtr());

        // 4. resort speaker position matrix and LsType vector
        m_vRenderMtrx[n].resortSpeakerPositions(vuiNewOrder);
        return bMatchingMtx;
    }
}

// dummy function that checks if a signaled rendering matrix is matching a local loudspeaker setup within the specified limits.
// IIS's matching algorithm has to be added instead
bool hoaRenderer::validateSignaledRenderingMatrix(spacePositions *speakerPos, spacePositions *lfePos, const std::vector<LS_TYPE> &lsTypes, const int n)
{
    bool bMatchingMtx = true;
    double fTolerance = 5.01*PI/180.0; // ± 5 degree tolerance
    unsigned int uiSpkIdx = 0;
    unsigned int uiLfeIdx = 0;
    double inclLocal, aziLocal, inclSignaled, aziSignaled;

    for (unsigned int i=0; i< lsTypes.size(); ++i)
    {	
        if (m_vRenderMtrx[n].getSignaledSpeakerType()[i] != lsTypes[i]) {bMatchingMtx = false;}

        if (!lsTypes[i])
        {			
            inclLocal = speakerPos->getPositionsArrayRaw()[uiSpkIdx*3+1];
            aziLocal  = speakerPos->getPositionsArrayRaw()[uiSpkIdx*3+2];
            uiSpkIdx++;
            
            inclSignaled = m_vRenderMtrx[n].getSignaledSpeakerPositionRad()[i][1];
            aziSignaled  = m_vRenderMtrx[n].getSignaledSpeakerPositionRad()[i][2];

            if ( angularDistance(inclLocal, aziLocal, inclSignaled, aziSignaled) > fTolerance)
            {
                bMatchingMtx = false;
            }
        }
    }
    return bMatchingMtx;
}
