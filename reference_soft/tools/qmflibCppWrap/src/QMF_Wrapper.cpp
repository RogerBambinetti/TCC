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
created: 2/2014, J.B. 

updated by A.K: 7/2014

*/


#include "QMF_Wrapper.h"
#include <stdio.h>
#include <stdlib.h>


CqmfWrapper::CqmfWrapper()
:   m_ciLdMode(0),
    m_ciAlignDelay(1),
    m_uiNumFreqBands(QMF_BANDS),
    m_eHybridMode(QMFLIB_HYBRID_OFF),
    m_uiNumChannels(0),
    m_uiFrameSize(0),
    m_uiNoOfSubFrames(0),
    m_bIsInit(false)
 {
     m_vHybFilterStates.clear();
     m_vvfInputBufferFreq_real.clear();
     m_vvfInputBufferFreq_imag.clear();
     m_vvfOutputBufferFreq_real.clear();
     m_vvfOutputBufferFreq_imag.clear();
 }

bool CqmfWrapper::initParams(   unsigned int p_uiNumChannels, 
                                unsigned int p_uiFrameSize,
                                QMFLIB_HYBRID_FILTER_MODE p_eHybridMode)
{
    bool bError = false;

    if (p_uiNumChannels > 0)
    {
        m_uiNumChannels = p_uiNumChannels;
    }
    else
    {
         bError = true;
    }

    // set frame size 
    // Note that it must be a multiple of the number of qmf bands !!!
    unsigned int uiTmpModulus = p_uiFrameSize % QMF_BANDS;
    
    if (uiTmpModulus == 0)
    {
        m_uiFrameSize     = p_uiFrameSize;
        m_uiNoOfSubFrames = p_uiFrameSize / static_cast<unsigned int>(QMF_BANDS);
    }
    else
    {
        bError = true;
    }
    
    bError = (bError | setHybMode(p_eHybridMode));

    return bError;
}

bool CqmfWrapper::setHybMode(QMFLIB_HYBRID_FILTER_MODE p_eHybridMode)
{
    switch (p_eHybridMode) 
    {
        case QMFLIB_HYBRID_THREE_TO_TEN:
            m_uiNumFreqBands = 71;
            break;
        case QMFLIB_HYBRID_THREE_TO_SIXTEEN:
            m_uiNumFreqBands = 77;
            break;
        case QMFLIB_HYBRID_OFF:
        default:
            m_uiNumFreqBands = 64;
            break;
    }

    return false;
}



 unsigned int CqmfWrapper::getAnalSynthFilterDelay()
 {
    unsigned int uiQmfDelay = 577;
    
    switch (m_eHybridMode) {
            
        case QMFLIB_HYBRID_THREE_TO_TEN:
        case QMFLIB_HYBRID_THREE_TO_SIXTEEN:
            uiQmfDelay = 961;
            break;
        case QMFLIB_HYBRID_OFF:
        default:
            uiQmfDelay = 577;
            break;
    }
     
#ifndef PHASE2_HOA_DECORRELATOR
    return 0;
#else
    return uiQmfDelay;
#endif
 }

//----------------------------------------------------------------------------------------------------------



bool CqmfAnalysis::init(    unsigned int p_uiNumChannels, 
                            unsigned int p_uiFrameSize,
                            QMFLIB_HYBRID_FILTER_MODE p_eHybridMode)    
{
    bool bError = initParams(    p_uiNumChannels, 
                                 p_uiFrameSize,
                                 p_eHybridMode);

    // Warning: initialization of static structs with different parameters for different instances might cause HEAVY TROUBLE!!!!
    QMFlib_InitAnaFilterbank(QMF_BANDS, m_ciLdMode);

    m_vshptrAnalysisQmf.resize(0);

    for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNumChannels; uiChanIdx++)
	{
        QMFLIB_POLYPHASE_ANA_FILTERBANK *pTmpAnaFB;

        QMFlib_OpenAnaFilterbank( &pTmpAnaFB );

		std::shared_ptr<QMFLIB_POLYPHASE_ANA_FILTERBANK> tmpshrptrAnaFB( pTmpAnaFB, QMFlib_CloseAnaFilterbank  );

		m_vshptrAnalysisQmf.push_back(tmpshrptrAnaFB);
	}

    // handle buffers
    std::vector<float> vfTmpBuffer( getNumIoSamples(), 0.0f );
    m_vvfInputBuffer.assign( m_uiNumChannels, vfTmpBuffer );

    std::vector<float> vfTmpChannelFreqBuffer( QMF_BANDS, 0.0f );

    if(p_eHybridMode == QMFLIB_HYBRID_OFF)
    {
        m_vvfOutputBufferFreq_real.assign( m_uiNumChannels, vfTmpChannelFreqBuffer);
        m_vvfOutputBufferFreq_imag.assign( m_uiNumChannels, vfTmpChannelFreqBuffer);
    }
    else  // hybrid fb and buffers
    {
        QMFLIB_HYBRID_FILTER_STATE TmpHybFilterState;
        m_vHybFilterStates.assign(m_uiNumChannels, TmpHybFilterState);

        for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNumChannels; uiChanIdx++)
	    {
            QMFlib_InitAnaHybFilterbank( &(m_vHybFilterStates[uiChanIdx]) );
        }  

        m_vvfInputBufferFreq_real.assign(  m_uiNumChannels, vfTmpChannelFreqBuffer);
        m_vvfInputBufferFreq_imag.assign(  m_uiNumChannels, vfTmpChannelFreqBuffer);

        std::vector<float> vfTmpChannelFreqHybBuffer( getNumOfFreqBands(), 0.0f );
        
        m_vvfOutputBufferFreq_real.assign( m_uiNumChannels, vfTmpChannelFreqHybBuffer);
        m_vvfOutputBufferFreq_imag.assign( m_uiNumChannels, vfTmpChannelFreqHybBuffer);
    }

    m_bIsInit = true;

    return bError;
}


void CqmfAnalysis::process( const std::vector<std::vector<FLOAT> > &p_rvvFTimeDomInputBuffer,
                                  std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomOutputBuffer)
{
    unsigned int uiTmpTimeDomainSampleOffset = 0;
    unsigned int uiTmpFreqDomainSampleOffset = 0;

    p_rvvvcFFreqDomOutputBuffer.resize(m_uiNumChannels);

    if( m_eHybridMode == QMFLIB_HYBRID_OFF)
    {
        for (unsigned int uiSubFrameIdx=0; uiSubFrameIdx < m_uiNoOfSubFrames; uiSubFrameIdx++)
        {
            // copy from global time domain input buffer to local time domain buffer
            copyFromFLOATTofloat( p_rvvFTimeDomInputBuffer, 
                                  uiTmpTimeDomainSampleOffset,
                                  m_vvfInputBuffer);                           

            // apply QMF analysis for each channel with samples of local buffer
            for  (unsigned int uiChanIdx=0; uiChanIdx< m_uiNumChannels; uiChanIdx++)
            {
                QMFlib_CalculateAnaFilterbank(  m_vshptrAnalysisQmf[uiChanIdx].get() , 
                                                m_vvfInputBuffer[uiChanIdx].data(), 
                                                m_vvfOutputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfOutputBufferFreq_imag[uiChanIdx].data(),
                                                m_ciLdMode ); 
            }

            // copy local frequency domain buffer to global buffer
            copyFromFloatReImagToComplexFLOAT( m_vvfOutputBufferFreq_real,
                                               m_vvfOutputBufferFreq_imag, 
                                               uiTmpFreqDomainSampleOffset,
                                               p_rvvvcFFreqDomOutputBuffer );

            uiTmpTimeDomainSampleOffset += getNumIoSamples();
            uiTmpFreqDomainSampleOffset += 1;
        } 
    }
    else
    {
        for (unsigned int uiSubFrameIdx=0; uiSubFrameIdx < m_uiNoOfSubFrames; uiSubFrameIdx++)
        {
            // copy from global time domain input buffer to local time domain buffer
            copyFromFLOATTofloat( p_rvvFTimeDomInputBuffer, 
                                  uiTmpTimeDomainSampleOffset,
                                  m_vvfInputBuffer);                           

            
            // apply QMF analysis for each channel with samples of local buffer
            for  (unsigned int uiChanIdx=0; uiChanIdx< m_uiNumChannels; uiChanIdx++)
            {
                QMFlib_CalculateAnaFilterbank(  m_vshptrAnalysisQmf[uiChanIdx].get(),
                                                m_vvfInputBuffer[uiChanIdx].data(),
                                                m_vvfInputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfInputBufferFreq_imag[uiChanIdx].data(),
                                                m_ciLdMode );

                QMFlib_ApplyAnaHybFilterbank(   &(m_vHybFilterStates[uiChanIdx]), 
                                                m_eHybridMode,
                                                QMF_BANDS,
                                                m_ciAlignDelay,
                                                m_vvfInputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfInputBufferFreq_imag[uiChanIdx].data(),
                                                 m_vvfOutputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfOutputBufferFreq_imag[uiChanIdx].data());
            }

            // copy local frequency domain buffer to global buffer
            copyFromFloatReImagToComplexFLOAT( m_vvfOutputBufferFreq_real,
                                               m_vvfOutputBufferFreq_imag, 
                                               uiTmpFreqDomainSampleOffset,
                                               p_rvvvcFFreqDomOutputBuffer );

            uiTmpTimeDomainSampleOffset += getNumIoSamples();
            uiTmpFreqDomainSampleOffset += 1;
        }
    }
}

//----------------------------------------------------------------------------------------------------------

bool CqmfSynthesis::init(   unsigned int p_uiNumChannels, 
                            unsigned int p_uiFrameSize,
                            QMFLIB_HYBRID_FILTER_MODE p_eHybridMode)
{
    bool bError = initParams(    p_uiNumChannels, 
                                 p_uiFrameSize,
                                 p_eHybridMode);

    // open qmf

        // Warning: initialization of static structs with different parameters for different instances might cause HEAVY TROUBLE!!!!
        QMFlib_InitSynFilterbank(QMF_BANDS, m_ciLdMode);

        m_vshptrSynthesisQmf.resize(0);

        for (unsigned int uiChanIdx=0; uiChanIdx < m_uiNumChannels; uiChanIdx++)
	    {
            QMFLIB_POLYPHASE_SYN_FILTERBANK *pTmpSynFB;

            QMFlib_OpenSynFilterbank( &pTmpSynFB );

		    std::shared_ptr<QMFLIB_POLYPHASE_SYN_FILTERBANK> tmpshrptrSynFB( pTmpSynFB, QMFlib_CloseSynFilterbank );

		    m_vshptrSynthesisQmf.push_back(tmpshrptrSynFB);
	    } 

    // handle buffers

        std::vector<float> vfTmpBuffer( getNumIoSamples(), 0.0f );
        m_vvfOutputBuffer.assign( m_uiNumChannels, vfTmpBuffer );

        std::vector<float> vfTmpChannelFreqBuffer( QMF_BANDS, 0.0f );

        if(p_eHybridMode==QMFLIB_HYBRID_OFF)
        {
            m_vvfInputBufferFreq_real.assign(  m_uiNumChannels, vfTmpChannelFreqBuffer);
            m_vvfInputBufferFreq_imag.assign(  m_uiNumChannels, vfTmpChannelFreqBuffer);

        }
        else  // hybrid fb and buffers
        {
            std::vector<float> vfTmpChannelFreqHybBuffer( getNumOfFreqBands(), 0.0f );

            m_vvfInputBufferFreq_real.assign( m_uiNumChannels, vfTmpChannelFreqHybBuffer);
            m_vvfInputBufferFreq_imag.assign( m_uiNumChannels, vfTmpChannelFreqHybBuffer);

            m_vvfOutputBufferFreq_real.assign( m_uiNumChannels, vfTmpChannelFreqBuffer);
            m_vvfOutputBufferFreq_imag.assign( m_uiNumChannels, vfTmpChannelFreqBuffer);
        }

        m_bIsInit = true;

        return bError;
}


 void CqmfSynthesis::process(const std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomInputBuffer,
                                   std::vector<std::vector<FLOAT> > &p_rvvFTimeDomOutputBuffer)
{
    unsigned int uiTmpTimeDomainSampleOffset = 0;
    unsigned int uiTmpFreqDomainSampleOffset = 0;

    p_rvvFTimeDomOutputBuffer.resize(m_uiNumChannels);

    for (unsigned int uiChanIdx=0; uiChanIdx< m_uiNumChannels; uiChanIdx++)
    {
            p_rvvFTimeDomOutputBuffer[uiChanIdx].resize(m_uiFrameSize);
    }

    if( m_eHybridMode == QMFLIB_HYBRID_OFF)
    {
        for (unsigned int uiSubFrameIdx=0; uiSubFrameIdx < m_uiNoOfSubFrames; uiSubFrameIdx++)
        {
            // copy from global frequency domain buffer to local one
            copyFromComplexFLOATToFloatReImag(  p_rvvvcFFreqDomInputBuffer,
                                                uiTmpFreqDomainSampleOffset,
                                                m_vvfInputBufferFreq_real,
                                                m_vvfInputBufferFreq_imag);

            // apply QMF synthesis for each channel with samples of local buffer
            for  (unsigned int uiChanIdx=0; uiChanIdx< m_uiNumChannels; uiChanIdx++)
            {
                QMFlib_CalculateSynFilterbank(  m_vshptrSynthesisQmf[uiChanIdx].get(),
                                                m_vvfInputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfInputBufferFreq_imag[uiChanIdx].data(),
                                                m_vvfOutputBuffer[uiChanIdx].data(),
                                                m_ciLdMode );
            }

            // copy from local time domain output buffer to global one 
            copyFromfloatToFLOAT(   m_vvfOutputBuffer, 
                                    uiTmpTimeDomainSampleOffset,
                                    p_rvvFTimeDomOutputBuffer );

            uiTmpTimeDomainSampleOffset += getNumIoSamples();
            uiTmpFreqDomainSampleOffset += 1;

        }
    }
    else
    {
        for (unsigned int uiSubFrameIdx=0; uiSubFrameIdx < m_uiNoOfSubFrames; uiSubFrameIdx++)
        {
            // copy from global frequency domain buffer to local one
            copyFromComplexFLOATToFloatReImag(  p_rvvvcFFreqDomInputBuffer,
                                                uiTmpFreqDomainSampleOffset,
                                                m_vvfInputBufferFreq_real,
                                                m_vvfInputBufferFreq_imag);

            // apply QMF synthesis for each channel with samples of local buffer
            for  (unsigned int uiChanIdx=0; uiChanIdx< m_uiNumChannels; uiChanIdx++)
            {
                QMFlib_ApplySynHybFilterbank(   QMF_BANDS,
                                                m_eHybridMode,
                                                m_vvfInputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfInputBufferFreq_imag[uiChanIdx].data(),
                                                m_vvfOutputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfOutputBufferFreq_imag[uiChanIdx].data());

                QMFlib_CalculateSynFilterbank(  m_vshptrSynthesisQmf[uiChanIdx].get(),
                                                m_vvfOutputBufferFreq_real[uiChanIdx].data(),
                                                m_vvfOutputBufferFreq_imag[uiChanIdx].data(),
                                                m_vvfOutputBuffer[uiChanIdx].data(),
                                                m_ciLdMode );
            }

            // copy from local time domain output buffer to global one
            copyFromfloatToFLOAT(   m_vvfOutputBuffer, 
                                    uiTmpTimeDomainSampleOffset,
                                    p_rvvFTimeDomOutputBuffer );

            uiTmpTimeDomainSampleOffset += getNumIoSamples();
            uiTmpFreqDomainSampleOffset += 1;
        }
    }
}


bool CqmfWrapper::copyFromFLOATTofloat(  const std::vector<std::vector<FLOAT> > &p_rvvFInputBuffer, 
                                                 unsigned int p_uiSampleOffsetForInput,
                                                 std::vector<std::vector<float> > &p_rvvfOutputBuffer )
{
    for (unsigned int uiChanIdx=0; uiChanIdx < p_rvvfOutputBuffer.size(); uiChanIdx++)
    {
        for (unsigned int uiSampleIdx=0;uiSampleIdx < p_rvvfOutputBuffer[0].size(); uiSampleIdx++)
        {
            p_rvvfOutputBuffer[uiChanIdx][uiSampleIdx] = static_cast<float>( p_rvvFInputBuffer[uiChanIdx][uiSampleIdx + p_uiSampleOffsetForInput]);
        }
    }

    return false;
}

bool CqmfWrapper::copyFromfloatToFLOAT(   const std::vector<std::vector<float> > &p_rvvfInputBuffer, 
                                                unsigned int p_uiSampleOffsetForOutput,
                                                std::vector<std::vector<FLOAT> > &p_rvvFOutputBuffer )
{
    for (unsigned int uiChanIdx=0; uiChanIdx < p_rvvfInputBuffer.size(); uiChanIdx++)
    {
        for (unsigned int uiSampleIdx=0;uiSampleIdx < p_rvvfInputBuffer[0].size(); uiSampleIdx++)
        {
            p_rvvFOutputBuffer[uiChanIdx][uiSampleIdx+p_uiSampleOffsetForOutput] = static_cast<FLOAT>(p_rvvfInputBuffer[uiChanIdx][uiSampleIdx]);
        }
    }

    return false;
}


bool CqmfWrapper::copyFromFloatReImagToComplexFLOAT( const  std::vector< std::vector<float > > &p_rvvfInputBufferFreq_real,
                                                     const  std::vector< std::vector<float > > &p_rvvfInputBufferFreq_imag, 
                                                            unsigned int p_uiSampleOffsetForOutput,
                                                            std::vector<std::vector<std::vector<std::complex<FLOAT> > > > &p_rvvvcFOutputBuffer )
{
    for (unsigned int uiChanIdx=0; uiChanIdx < p_rvvfInputBufferFreq_real.size(); uiChanIdx++)
    {
        for (unsigned int uiQMFBandIdx=0; uiQMFBandIdx < p_rvvfInputBufferFreq_real[0].size(); uiQMFBandIdx++)
        {
            p_rvvvcFOutputBuffer[uiChanIdx][uiQMFBandIdx][p_uiSampleOffsetForOutput] =
                std::complex<FLOAT> (   static_cast<FLOAT>(p_rvvfInputBufferFreq_real[uiChanIdx][uiQMFBandIdx] ) , 
                                        static_cast<FLOAT>(p_rvvfInputBufferFreq_imag[uiChanIdx][uiQMFBandIdx] ) );
        }
    }

    return false;
}

bool CqmfWrapper::copyFromComplexFLOATToFloatReImag(  const std::vector<std::vector<std::vector<std::complex<FLOAT> > > > &p_rvvvcFInputBuffer,
                                                                            unsigned int p_uiSampleOffsetForInput,
                                                                           std::vector< std::vector<float > > &p_rvvfOutputBufferFreq_real,
                                                                           std::vector< std::vector<float > > &p_rvvfOutputBufferFreq_imag)
{
    for (unsigned int uiChanIdx=0; uiChanIdx < p_rvvvcFInputBuffer.size(); uiChanIdx++)
    {
        for (unsigned int uiQMFBandIdx=0; uiQMFBandIdx < p_rvvvcFInputBuffer[0].size(); uiQMFBandIdx++)
        {
            p_rvvfOutputBufferFreq_real[uiChanIdx][uiQMFBandIdx] = static_cast<float>( p_rvvvcFInputBuffer[uiChanIdx][uiQMFBandIdx][p_uiSampleOffsetForInput].real() );
            p_rvvfOutputBufferFreq_imag[uiChanIdx][uiQMFBandIdx] = static_cast<float>( p_rvvvcFInputBuffer[uiChanIdx][uiQMFBandIdx][p_uiSampleOffsetForInput].imag() );
        }
    }

    return false;
}