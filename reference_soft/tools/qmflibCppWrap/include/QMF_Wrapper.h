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

#ifndef QMF_WRAPPER_H
#define QMF_WRAPPER_H


#define QMF_BANDS 64
extern "C" {
#include "qmflib.h"
#include "qmflib_hybfilter.h"

}

#include <complex>
#include <vector>
#include <memory>
#include "DataTypes.h"

//  ATTENTION: NO lOW DELAY VERSION implemented

class CqmfWrapper
{
    public:
        CqmfWrapper();  
    
        bool initParams(  unsigned int p_uiNumChannels, 
                          unsigned int p_uiFrameSize,
                          QMFLIB_HYBRID_FILTER_MODE p_eHybridMode);

        bool setHybMode( QMFLIB_HYBRID_FILTER_MODE p_eHybridMode );     // 1: QMFLIB_HYBRID_THREE_TO_TEN,      71 bands
                                                                        // 2: QMFLIB_HYBRID_THREE_TO_SIXTEEN,  77 bands
                                                                        // 0: QMFLIB_HYBRID_OFF  no hybrid,    64 bands

        unsigned int getNumOfFreqBands(){return m_uiNumFreqBands;};
        unsigned int getNumChannels(){return m_uiNumChannels;};
        unsigned int getNumIoSamples(){return QMF_BANDS;};
        unsigned int getNumFreqSamples(){return m_uiNoOfSubFrames;}
        unsigned int getAnalSynthFilterDelay();
  
    protected: 
        
        bool	m_bIsInit;

        const int m_ciLdMode;
        const int m_ciAlignDelay;         // = 1; /* 0: Align hybrid HF part with LF part, 1: no delay alignment for HF part (submission) */
    
        unsigned int m_uiNumFreqBands;
        unsigned int m_uiNumChannels;
        unsigned int m_uiFrameSize;
        unsigned int m_uiNoOfSubFrames;
    
        QMFLIB_HYBRID_FILTER_MODE m_eHybridMode;

        std::vector< QMFLIB_HYBRID_FILTER_STATE > m_vHybFilterStates;
    
        std::vector< std::vector<float > > m_vvfInputBufferFreq_real;
        std::vector< std::vector<float > > m_vvfInputBufferFreq_imag;

        std::vector< std::vector<float > > m_vvfOutputBufferFreq_real;
        std::vector< std::vector<float > > m_vvfOutputBufferFreq_imag;
 
        static bool copyFromFLOATTofloat(   const std::vector<std::vector<FLOAT> > &p_rvvFInputBuffer, 
                                                         unsigned int p_uiSampleOffsetForInput,
                                                         std::vector<std::vector<float> > &p_rvvfOutputBuffer );

        static bool copyFromfloatToFLOAT(   const std::vector<std::vector<float> > &p_rvvfInputBuffer, 
                                                         unsigned int p_uiSampleOffsetForOutput,
                                                         std::vector<std::vector<FLOAT> > &p_rvvFOutputBuffer );

        static bool copyFromFloatReImagToComplexFLOAT(  const  std::vector< std::vector<float > > &p_rvvfInputBufferFreq_real,
                                                                     const  std::vector< std::vector<float > > &p_rvvfInputBufferFreq_imag, 
                                                                            unsigned int p_uiSampleOffsetForOutput,
                                                                            std::vector<std::vector<std::vector<std::complex<FLOAT> > > > &p_rvvvcFOutputBuffer);

        static bool copyFromComplexFLOATToFloatReImag(  const std::vector<std::vector<std::vector<std::complex<FLOAT> > > > &p_rvvvcFInputBuffer,
                                                                            unsigned int p_uiSampleOffsetForInput,
                                                                           std::vector< std::vector<float > > &p_rvvfOutputBufferFreq_real,
                                                                           std::vector< std::vector<float > > &p_rvvfOutputBufferFreq_imag);

    private:

        CqmfWrapper( const CqmfWrapper & );
        const CqmfWrapper &operator=( const CqmfWrapper & );

        
};

class CqmfAnalysis: public CqmfWrapper
{
    public:
    
        CqmfAnalysis(){};

        bool init(  unsigned int p_uiNumChannels, 
                    unsigned int p_uiFrameSize,
                    QMFLIB_HYBRID_FILTER_MODE p_eHybridMode);

        void process( const std::vector<std::vector<FLOAT> > &p_rvvFTimeDomInputBuffer,
                            std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomOutputBuffer );
  
    private:

        std::vector< std::shared_ptr<QMFLIB_POLYPHASE_ANA_FILTERBANK> > m_vshptrAnalysisQmf;
        std::vector< std::vector<float> > m_vvfInputBuffer;
    
        CqmfAnalysis( const CqmfAnalysis & );
        const CqmfAnalysis &operator=( const CqmfAnalysis & );
};


class CqmfSynthesis: public CqmfWrapper
{
    public:
        CqmfSynthesis(){};

        bool init(  unsigned int p_uiNumChannels, 
                    unsigned int p_uiFrameSize,
                    QMFLIB_HYBRID_FILTER_MODE p_eHybridMode);
        
        void process( const std::vector<std::vector<std::vector<std::complex<FLOAT> > > >  &p_rvvvcFFreqDomInputBuffer,
                            std::vector<std::vector<FLOAT> > &p_rvvFTimeDomOutputBuffer);
  
    private:

        std::vector< std::shared_ptr<QMFLIB_POLYPHASE_SYN_FILTERBANK> > m_vshptrSynthesisQmf; 
        std::vector< std::vector<float> > m_vvfOutputBuffer;

        CqmfSynthesis( const CqmfSynthesis & );
        const CqmfSynthesis &operator=( const CqmfSynthesis & );
};



#endif
