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
/*   DRC1hoa.cpp
*                   Copyright (c) 2014, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    DRC1 for HOA signals,  (MPEG-H-RM3)
* authors:          Johannes Boehm (jb), 
* log:              created 29.07.2014, 
*/
/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: DRC1hoa.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#include "DRC1hoa.h"
#include "dshtTs2cMtrx.h"


//-----------------------------------------------------------------------------------------------------------------------------------------
// construct an empty object 
DRC1hoa::DRC1hoa()
 : bOutputAreChannels(false), numCoeffs(0), numOutputs(0),tdMode(1)
 {}

//-----------------------------------------------------------------------------------------------------------------------------------------
// construct using HOA order of content
DRC1hoa::DRC1hoa(int HOAorder, int mode)
 : bOutputAreChannels(false), numCoeffs( (HOAorder+1)*(HOAorder+1)), numOutputs((HOAorder+1)*(HOAorder+1)), tdMode(mode)
 {
   T1 = std::shared_ptr<dshtTc2sMtrx>(new dshtTc2sMtrx(HOAorder));
   T2D= std::shared_ptr<dshtTs2cMtrx>(new dshtTs2cMtrx(HOAorder, *T1));
   if (tdMode==0)
#if !defined    ENABLE_HOA_DRC_MULTIBAND
     throw(std::runtime_error("DRC1hoa: FD mode not implemented yet"));
#else
    {// init QMFs and related I/O buffers
    m_pQmfAna = std::shared_ptr<CqmfAnalysis>(new CqmfAnalysis());
    m_pQmfSyn = std::shared_ptr<CqmfSynthesis>(new CqmfSynthesis());
     QMFLIB_HYBRID_FILTER_MODE eHybridMode = QMFLIB_HYBRID_OFF; 
    m_pQmfAna->init((HOAorder+1)*(HOAorder+1), 64,eHybridMode);
    m_pQmfSyn->init((HOAorder+1)*(HOAorder+1), 64,eHybridMode);
     m_vFdBuffer.resize(numCoeffs);  
    for (int c=0; c<numCoeffs; c++)
    { m_vFdBuffer[c].resize(64); for(int b=0; b<64; b++) m_vFdBuffer[c][b].resize(1);}                   
    m_vTdBufer.resize(numCoeffs);  for (int c=0; c<numCoeffs; c++)   m_vTdBufer[c].resize(64);
    }
#endif
 }
//-----------------------------------------------------------------------------------------------------------------------------------------
// enable rendering by providing a rendering matrix. As a result the output of blockProcess() will be a channel signal
void DRC1hoa::enableRendering(std::vector<std::vector<FLOAT> >  &renderingMatrix)
{
   bOutputAreChannels=true;
   numOutputs=renderingMatrix.size();
   T2D->overload(renderingMatrix,numCoeffs, numOutputs);   
   vDoverload=renderingMatrix;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// apply DRC to HOA signal and if enabled, render
int DRC1hoa::blockProcess(  std::vector<std::vector<FLOAT> > &inBuffer, std::vector<std::vector<FLOAT> > &drcBuffer,  
                            std::vector<std::vector<FLOAT> > &outBuffer, bool bEnableClippingProtect, bool bInPlaceFlag)
{
    int numOfClippedSamples=0;
    int nValidSamples= inBuffer[0].size();
    // resize output buffer
    if (!bInPlaceFlag)
        outBuffer.assign(numOutputs, std::vector<FLOAT>(nValidSamples, 0.0));

    if (numCoeffs !=  inBuffer.size() )
         throw(std::runtime_error("DRC1hoa in buffer dimension mismatch"));
    if(nValidSamples==0)
        return 0; // nothing to process

    
    int numGainChannels = drcBuffer.size();
    if (numGainChannels !=1 && numGainChannels != numCoeffs || drcBuffer[0].size()!=nValidSamples)
         throw(std::runtime_error("DRC1hoa gain sequence dimension mismatch"));

  if (tdMode==1)    
  {
    if (numGainChannels == numCoeffs)
    {
        std::vector<FLOAT> sC(numCoeffs);
        for (int nt=0; nt<(int)nValidSamples; nt++)    
        {
            // Transform to spatial domain
            for(int och=0; och<numCoeffs; och++) 
            {
                FLOAT Fval=0;
                for (int mN=0; mN<numCoeffs; mN++)
                {
                    Fval += (FLOAT) (*T1)[och][mN] * inBuffer[mN][nt];                                     
                }
               
                sC[och]= Fval;
            }
            // Apply gain sequence
            for(int och=0; och<numCoeffs; och++) 
            {
             sC[och]=sC[och]*drcBuffer[och][nt];
            }

           // Transform to output domain
            for(int och=0; och<numOutputs; och++) 
            {
                FLOAT Fval=0;
                for (int mN=0; mN<numCoeffs; mN++)
                {
                 Fval += (FLOAT) (*T2D)[och][mN] * sC[mN];                   
                }
                outBuffer[och][nt] = Fval;
            }
        }
    } else  // only a common gain sequence is available
    {
        if (bOutputAreChannels)
        {
            for (int nt=0; nt<(int)nValidSamples; nt++)    
            {
              for(int och=0; och<numOutputs; och++) 
              {
                FLOAT Fval=0;
                for (int mN=0; mN<numCoeffs; mN++)
                    {
                    Fval += (FLOAT)  vDoverload[och][mN] * inBuffer[mN][nt];                   
                    }
                outBuffer[och][nt] = Fval* drcBuffer[0][nt];
              }
            }
        }else
        {
            for (int nt=0; nt<(int)nValidSamples; nt++)    
            {
              for(int och=0; och<numOutputs; och++) 
              {
                outBuffer[och][nt]= inBuffer[och][nt] * drcBuffer[0][nt];
              }
            }
        }
    }

  }else 
  {
#ifdef  ENABLE_HOA_DRC_MULTIBAND
    if (nValidSamples%64)
       // throw(std::runtime_error("DRC1hoa block processing error, number of samples mist be x*64 "));
            int i=1;
   

    int numQmfblocks=nValidSamples/64;
    
    for(int sf=0; sf< numQmfblocks; sf++)  
    {
       for (int nt=0; nt<64; nt++)              // transform to spatial domain
        {        
         for(int och=0; och<numCoeffs; och++) 
           {
              FLOAT Fval=0;
              for (int mN=0; mN<numCoeffs; mN++)
                {
                    Fval += (FLOAT) (*T1)[och][mN] * inBuffer[mN][sf*64+nt];                                     
                }
               
              m_vTdBufer[och][nt]= Fval;
            }
        } 
        m_pQmfAna->process(m_vTdBufer, m_vFdBuffer);  // transform to QMF domain
        
                                                  // Apply gain sequence
        if (numGainChannels == numCoeffs)
        {
            for (int c=0; c<numCoeffs; c++)
                for (int fb=0; fb<64; fb++) 
                    m_vFdBuffer[c][fb][0]*= drcBuffer[c][sf*64+fb];
        
        }else
        { 
            for (int c=0; c<numCoeffs; c++)
                for (int fb=0; fb<64; fb++) 
                    m_vFdBuffer[c][fb][0]*= drcBuffer[0][sf*64+fb];
        }
        m_pQmfSyn->process(m_vFdBuffer, m_vTdBufer);  // transform to time domain
         for (int nt=0; nt<64; nt++)              // transform to HOA domain
        {        
         for(int och=0; och<numOutputs; och++) 
           {
              FLOAT Fval=0;
              for (int mN=0; mN<numCoeffs; mN++)
                {
                    Fval += (FLOAT) (*T2D)[och][mN] * m_vTdBufer[mN][nt];                                     
                }
               
              outBuffer[och][sf*64+nt]= Fval;
            }
        } 
    }   
#endif
  }

    if(bEnableClippingProtect)
    {
        for (int nt=0; nt<(int)nValidSamples; nt++)    
        {
           
            for(int och=0; och<numOutputs; och++) 
            {
                if (outBuffer[och][nt]>=1.0)
                {
                    outBuffer[och][nt]=1.0;//-lsb; todo!
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