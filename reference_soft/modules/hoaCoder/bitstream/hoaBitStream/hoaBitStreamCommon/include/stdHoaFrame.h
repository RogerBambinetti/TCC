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
$Id: stdHoaFrame.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#ifndef __STDHOAFRAME__
#define __STDHOAFRAME__

#include <cmath>
#include <memory>
#include <map>
#include <algorithm>
#include "basicbitstreamfields.h"
#include "integerFunctions.h"
#include "hoaHuffmanTables.h"
#include "DataTypes.h"
#include "TabulatedValuesHoaBitstream.h"
#include <string>

static const unsigned int c8BitQuantizerWord = 5;			// mask to signal the use of 8-bit quantizer
static const unsigned int cVVecVQWord = 4;					// mask to signal the use of VQ for VVec  
static const unsigned int cInvalidHuffmanTable = 99;		// invalid table number to indicate first frame


/*********************************************************************
* HELPER CLASS for computation of size in bits of coded
* magnitude and angle differences used by the dir. prediction module
* and the PAR module
*********************************************************************/
class DiffValueIO
{
public:
   /** Function for the computation of the size in bits for writing or reading angle and magnitude differences */
   static unsigned int getSizeOfDiffValue(bool bUseRealCoeffsPerParSubband,            
      int nMagDiff,
      bool bUseParHuffmanCodingDiffMag,
      const HuffmanWord<int> *pDecodedMagDiffTable,
      unsigned int unDecodedMagDiffTableSize,
      IBitStreamField *pDirectMagDiffIO,
      const HuffmanWord<int> *pDecodedMagSbrDiffTable,
      unsigned int unDecodedMagDiffSbrTableSize,
      IBitStreamField *pDirectMagDiffSbrIO,
      int nAngleDiff,
      bool bUseParHuffmanCodingDiffAngle,
      const HuffmanWord<int> *pDecodedAngleDiffTable,
      IBitStreamField *pDirectAngleDiffIO
      )
   {
      unsigned int unSize = 0;
      const HuffmanWord<int> *pSelectedMagDiffTable = 0;
      unsigned int unSelectedTableSize = 0;
      IBitStreamField *pSelectedDirectMagDiffIO = 0;
      if(bUseRealCoeffsPerParSubband)
      {
         pSelectedMagDiffTable = pDecodedMagSbrDiffTable;
         unSelectedTableSize = unDecodedMagDiffSbrTableSize;
         pSelectedDirectMagDiffIO = pDirectMagDiffSbrIO;
      }
      else
      {
         pSelectedMagDiffTable = pDecodedMagDiffTable;
         unSelectedTableSize = unDecodedMagDiffTableSize;
         pSelectedDirectMagDiffIO = pDirectMagDiffIO;
      }


      // switch between Huffman decoding and plain bits
      if(bUseParHuffmanCodingDiffMag)
      {                
         int idx = 0;
         // Size of Magnitude value
         if(nMagDiff <= pSelectedMagDiffTable[0].m_codedValue)
         {
            // escape word for negative values
            unSize += pSelectedMagDiffTable[0].m_unCodeWordLength;
            // unary code of the out of range magnitude difference
            unSize += -nMagDiff + pSelectedMagDiffTable[0].m_codedValue + 1; // bits for unary code
         }
         else if (nMagDiff >= pSelectedMagDiffTable[unSelectedTableSize-1].m_codedValue)
         {
            // escape word for positive values
            unSize += pSelectedMagDiffTable[unSelectedTableSize-1].m_unCodeWordLength;
            // unary code of the out of range magnitude difference
            unSize += nMagDiff - pSelectedMagDiffTable[unSelectedTableSize-1].m_codedValue + 1; // bits for unary code
         }
         else
         {
            // Huffman coded magnitude difference
            idx = getIdxFromValue(nMagDiff,  pSelectedMagDiffTable);
            if(idx>=0)
               unSize += pSelectedMagDiffTable[idx].m_unCodeWordLength;
         }
      }
      else
      {
         unSize += pSelectedDirectMagDiffIO->getSizeInBit(); 
         // Size of Magnitude value
         if(nMagDiff <= pSelectedMagDiffTable[1].m_codedValue)
         {
            // unary code of the out of range magnitude difference
            unSize += -nMagDiff + pSelectedMagDiffTable[1].m_codedValue + 1; // bits for unary code
         }
         else if (nMagDiff >= pSelectedMagDiffTable[unSelectedTableSize-2].m_codedValue)
         {
            // unary code of the out of range magnitude difference
            unSize += nMagDiff - pSelectedMagDiffTable[unSelectedTableSize-2].m_codedValue + 1; // bits for unary code
         }
      }

      // switch between complex and real valued parameter
      if ( !bUseRealCoeffsPerParSubband )
      {
         // switch between Huffman decoding and plain bits
         if(bUseParHuffmanCodingDiffAngle)
         {                
            int idx = 0;
            // read angle value
            idx =  getIdxFromValue(nAngleDiff, pDecodedAngleDiffTable);
            if(idx>=0)
               unSize += pDecodedAngleDiffTable[idx].m_unCodeWordLength;
         }
         else
         {
            unSize += pDirectAngleDiffIO->getSizeInBit(); 
         }
      }
      return unSize;
   }
};

/*********************************************************************
* GAIN CORRECTION DATA 
*********************************************************************/
class CHoaGainCorrection
{
public:
   CHoaGainCorrection() : m_bGainCorrectionException(false),
      m_nGainCorrAbsAmpExp(0)
   {};

   virtual ~CHoaGainCorrection(){};

public:
   std::vector<bool> m_bCodedGainCorrectionExp; // max. (ceil(log2(compressionState.NoOfHOACoeffs))
   //      +compressionState.MaxAmplifyExp)+2 entries
   // length read as long as a one has been read                                       
   bool m_bGainCorrectionException; // HoaConfig::m_unTotalNumCoders entries  
   int m_nGainCorrAbsAmpExp;  
};


template <class UCHAR_FT,
class UINT_FT>
class CHoaGainCorrectionIO //: public CHoaGainCorrection
{
public:
   CHoaGainCorrectionIO() : m_unMaxGainCorrectionExpSize(0), m_nMaxLog2ExpOfTransportSigs(0)
   {
      m_bool1Bit.setFieldSizeInBits(1);
      m_uintGainCorrAbsAmpExpIO.setByteAlignFlag(false);
   };

   ~CHoaGainCorrectionIO(){};

   void initArrays(CHoaGainCorrection &data,
      unsigned int unNoOfHoaCoeffs,
      unsigned int unMaxGainCorrAmpExp)
   {
      m_nMaxLog2ExpOfTransportSigs = getCeilLog2( static_cast<int>(std::ceil( 1.5 * static_cast<double>(unNoOfHoaCoeffs))));
      m_uintGainCorrAbsAmpExpIO.setFieldSizeInBits(getCeilLog2(  m_nMaxLog2ExpOfTransportSigs + unMaxGainCorrAmpExp + 1 ));

      m_unMaxGainCorrectionExpSize = m_nMaxLog2ExpOfTransportSigs + unMaxGainCorrAmpExp + 1 + 1; // the additional 1 is for the exponent -1
      data.m_bCodedGainCorrectionExp.resize(m_unMaxGainCorrectionExpSize , false );
      data.m_bCodedGainCorrectionExp.resize(1);
      data.m_bCodedGainCorrectionExp[0] = true;
      data.m_bGainCorrectionException = false;  

   };

   unsigned int getSize(const CHoaGainCorrection &data,
      bool bIndependencyFlag)
   {
      unsigned int nSize = 0;
      if(bIndependencyFlag)
         nSize += m_uintGainCorrAbsAmpExpIO.getSizeInBit();
      nSize += data.m_bCodedGainCorrectionExp.size() * m_bool1Bit.getSizeInBit();
      nSize +=  m_bool1Bit.getSizeInBit();
      return nSize;
   }

public:
   unsigned int m_unMaxGainCorrectionExpSize;
   int m_nMaxLog2ExpOfTransportSigs;
   UCHAR_FT m_bool1Bit;
   UINT_FT m_uintGainCorrAbsAmpExpIO;
};

/*********************************************************************
* SPATIAL PREDICTION DATA 
*********************************************************************/
class CSpatialPredictionData
{
public:
   CSpatialPredictionData() : m_bPerformPrediction(false), m_bKindOfCodedPredIds(false)
   {};

   virtual ~CSpatialPredictionData(){};

public:
   bool m_bPerformPrediction;       // one bit that signalizes that the prediction is used or not used
   bool m_bKindOfCodedPredIds;      // one bit that signalizes the kind of coding of prediction indices   
   std::vector<bool> m_bActivePred; // m_unNoOfHoaCoeffs entries
   std::vector<bool> m_bPredType; // max. m_unNoOfHoaCoeffs entries only the sum of the active m_bActivePred are used
   std::vector<unsigned int> m_unPredIds;
   std::vector<int> m_nPredGains; // size as m_unPredIds uses HoaConfig::m_unNoOfBitsPerScaleFactor bits

};

template <class UCHAR_FT,
class INT_FT,
class UINT_FT>
class CSpatialPredictionDataIO //: public CSpatialPredictionData
{
public:
   CSpatialPredictionDataIO() 
   {
      m_unMaxNoOfDirSigsForPrediction = 0;
      m_unNoOfPredIdsBetterToCodeExplicitly = 1;
      m_bool1Bit.setFieldSizeInBits(1);
   };

   ~CSpatialPredictionDataIO(){};

   void initArrays(CSpatialPredictionData &data,
      unsigned int unMaxNoOfDirSigsForPrediction,
      unsigned int unNoOfHoaCoeffs,
      unsigned int unNoOfBitsPerScaleFactor)
   {
      // set number of bits for explicit coding of pred. index
      m_unActivePredIdsIO.setFieldSizeInBits(getCeilLog2(unNoOfHoaCoeffs));

      // compute the number of prediction indices that is better to code explicitly
      m_unNoOfPredIdsBetterToCodeExplicitly = 1;
      while( (m_unNoOfPredIdsBetterToCodeExplicitly*m_unActivePredIdsIO.getSizeInBit() 
         + getCeilLog2(m_unNoOfPredIdsBetterToCodeExplicitly)) < unNoOfHoaCoeffs)
      {
         m_unNoOfPredIdsBetterToCodeExplicitly++;
      }
      m_unNoOfPredIdsBetterToCodeExplicitly--;
      m_unNoActivePredIdsIO.setFieldSizeInBits(getCeilLog2(m_unNoOfPredIdsBetterToCodeExplicitly));

      data.m_bActivePred.resize(unNoOfHoaCoeffs, false);

      data.m_bPredType.resize(unNoOfHoaCoeffs, false);

      data.m_unPredIds.resize(unNoOfHoaCoeffs*unMaxNoOfDirSigsForPrediction, 0);
      m_unPredIdsIO.setFieldSizeInBits(1); // variable size
      m_vunDirSigChannelIds.reserve(unNoOfHoaCoeffs+1); 

      data.m_nPredGains.resize(unNoOfHoaCoeffs*unMaxNoOfDirSigsForPrediction, 0);
      unsigned int nBits = unNoOfBitsPerScaleFactor;
      m_nPredGainsIO.setFieldSizeInBits(nBits);
      m_unMaxNoOfDirSigsForPrediction = unMaxNoOfDirSigsForPrediction;
   };

   unsigned int getSize(const CSpatialPredictionData &data)
   {
      unsigned int nSizeInBit =  m_bool1Bit.getSizeInBit(); // one bit for pred performed field
      if(data.m_bPerformPrediction)
      {
         nSizeInBit += m_bool1Bit.getSizeInBit(); // one bit for active pred ids signalization type
         if(data.m_bKindOfCodedPredIds)
         {
            nSizeInBit += m_unNoActivePredIdsIO.getSizeInBit(); // bits for number of active pred ids
            // get number of active prediction ids
            unsigned int unNoActivePredictions = 0;
            for(unsigned int n = 0; n < data.m_bActivePred.size(); ++n)
            {
               if(data.m_bActivePred[n]!=0)
                  unNoActivePredictions++;
            }
            nSizeInBit += unNoActivePredictions*m_unActivePredIdsIO.getSizeInBit(); // bits for explicit ids.
         }
         else
         {
            nSizeInBit += data.m_bActivePred.size()*m_bool1Bit.getSizeInBit(); // bits for active pred array
         }
         //nSizeInBit += data.m_bPredType.size()*m_bool1Bit.getSizeInBit();  // bits for pred type
         m_unPredIdsIO.setFieldSizeInBits(getCeilLog2(m_vunDirSigChannelIds.size()+1)); // set number of bits to read
         nSizeInBit += data.m_unPredIds.size()*m_unPredIdsIO.getSizeInBit(); // bits for dir signal ids
         nSizeInBit += data.m_nPredGains.size()*m_nPredGainsIO.getSizeInBit(); // bits for pred. gains
      }

      return nSizeInBit;
   };

public:
   UCHAR_FT m_bool1Bit;
   UINT_FT m_unPredIdsIO;
   INT_FT m_nPredGainsIO;
   UINT_FT m_unNoActivePredIdsIO;
   UINT_FT m_unActivePredIdsIO;
   unsigned int m_unNoOfPredIdsBetterToCodeExplicitly; 
   unsigned int m_unMaxNoOfDirSigsForPrediction;
   std::vector<unsigned int> m_vunDirSigChannelIds;
};

/*********************************************************************
* Channel Side Info Data
*********************************************************************/
/**
* @brief HOA std. frame types
* 
*/
typedef enum 
{
   DIR_CHANNEL = 0,
   VEC_CHANNEL = 1,
   ADD_HOA_CHANNEL = 2,
   EMPTY_CHANNEL = 3
}CHANNEL_TYPE;


class CChannelSideInfoData
{
public:
   CChannelSideInfoData(CHANNEL_TYPE ChannelType) : m_unChannelType(ChannelType)
   {};

   virtual ~CChannelSideInfoData(){};

public:
   CHANNEL_TYPE m_unChannelType;
};

/*********************************************************************
* Directional Info Channel 
*********************************************************************/
class CDirectionalInfoChannel : public CChannelSideInfoData
{
public: 
   CDirectionalInfoChannel() : CChannelSideInfoData(DIR_CHANNEL), m_unActiveDirIds(0)
   {};
   ~CDirectionalInfoChannel(){};
public:
   unsigned int m_unActiveDirIds;
};

template <class UCHAR_FT,
class UINT_FT>
class CDirectionalInfoChannelIO //: public CDirectionalInfoChannel
{
public:
   CDirectionalInfoChannelIO() 
   {
      m_uint2Bit.setFieldSizeInBits(2);
      m_dirIdxIO.setFieldSizeInBits(0);
   };

   void init(unsigned int unBitsPerDir) 
   {
      m_dirIdxIO.setFieldSizeInBits(unBitsPerDir);
   }

   ~CDirectionalInfoChannelIO(){};

   unsigned int getSize()
   {  
      return m_dirIdxIO.getSizeInBit() + m_uint2Bit.getSizeInBit();
   };

public:
   UCHAR_FT m_uint2Bit;
   UINT_FT m_dirIdxIO;
};


/*********************************************************************
* Empty Info Channel 
*********************************************************************/
class CEmptyInfoChannel : public CChannelSideInfoData
{
public: 
   CEmptyInfoChannel() : CChannelSideInfoData(EMPTY_CHANNEL){};
   ~CEmptyInfoChannel(){};
};

template <class UCHAR_FT>
class CEmptyInfoChannelIO //: public CEmptyInfoChannel
{
public:
   CEmptyInfoChannelIO()
   {
      m_uint2Bit.setFieldSizeInBits(2);
   };

   ~CEmptyInfoChannelIO(){};

   unsigned int getSize()
   {  
      return m_uint2Bit.getSizeInBit();
   };

public:
   UCHAR_FT m_uint2Bit;
};

/*********************************************************************
* Vector-based Predominant Sound Info Channel 
*********************************************************************/
class CVectorBasedInfoChannel : public CChannelSideInfoData
{
public: 
   CVectorBasedInfoChannel() : CChannelSideInfoData(VEC_CHANNEL)
   {
      m_unNbitsQ = cInvalidHuffmanTable;
      m_bPFlag = false;
      m_bCbFlag = false;
      m_bSameHeaderPrevFrame = false;
	  m_bNewChannelTypeOne = false;
      m_unVVecDirections = 8;
      m_unMaxVVecDirections = 8;
      m_unIndicesCodebookIdx = 0;
      m_unWeightingCodebookIdx = 0;
   };
   ~CVectorBasedInfoChannel(){};
public:
   unsigned short m_unNbitsQ;
   bool m_bPFlag;
   bool m_bCbFlag;
   bool m_bSameHeaderPrevFrame;
   bool m_bNewChannelTypeOne;
   std::vector<unsigned int> m_vun8bitCodedVelement;
   std::vector<std::string> m_vsCodedHuffmannWord;
   std::vector<unsigned short> m_vunDecodedHuffmannWord;    
   std::vector<bool> m_vbElementBitmask; //bugfix NP 29-05-2015
   std::vector<bool> m_vbAdditionalInfo;
   std::vector<bool> m_vbSign;
   std::vector<unsigned int> m_vunNumBitsAdditional;
   std::vector<unsigned int> m_vunAdditionalValue;
   // Vector Quantization
   unsigned int m_unIndicesCodebookIdx;	
   unsigned int m_unMaxVVecDirections;	
   unsigned int m_unVVecDirections;
   std::vector<unsigned int> m_vunDirectionIndices;
   unsigned int m_unWeightingCodebookIdx;
};

template <class UCHAR_FT,
class UINT_FT>
class CVectorBasedInfoChannelIO //: public CEmptyInfoChannel
{
public:
   CVectorBasedInfoChannelIO()
   {
      m_unMaxNumOfVecs = 0;
      m_unCodedVVecLength = 0;
      m_unMaxVVecDirections = 2;
      m_unNumOfVecElements = 0;
      m_unHoaIdxOffset = 0;
      m_bool1Bit.setFieldSizeInBits(1);
      m_uint2Bit.setFieldSizeInBits(2);
      m_uint3Bit.setFieldSizeInBits(3);
      m_uint4Bit.setFieldSizeInBits(4);
      m_uint8Bit.setFieldSizeInBits(8);
      m_uint10Bit.setFieldSizeInBits(10);
      m_uint8Bit.setByteAlignFlag(false);
      m_uintAddValue.setFieldSizeInBits(0);
      m_uintAddValue.setByteAlignFlag(false);
   };

   ~CVectorBasedInfoChannelIO(){};

   void initArrays(unsigned int unMaxNumOfVecs,
      int unMinHoaOrder,
      unsigned int unNoOfHoaCoeffs,
      unsigned int unCodedVVecLength,
      unsigned int unMaxVVecDirections)

   {

      m_unMaxNumOfVecs = unMaxNumOfVecs;
      m_unCodedVVecLength = unCodedVVecLength;
      m_unMaxVVecDirections = unMaxVVecDirections;
      switch (m_unCodedVVecLength){
      case 1: // minimized VVec length (lower orders and AddHoaChannels are removed)
      case 2: // lower orders are removed
         m_unHoaIdxOffset = (unMinHoaOrder+1)*(unMinHoaOrder+1) + 1;
         break;
      case 0: //full VVec
      default:
         m_unHoaIdxOffset = 1;		  
      }
      m_unNumOfVecElements = unNoOfHoaCoeffs - m_unHoaIdxOffset + 1;
      unsigned int numOfDirLength = getCeilLog2(m_unMaxVVecDirections);
      m_uintNumberOfDirections.setFieldSizeInBits(numOfDirLength);
      unsigned int indexLength = getCeilLog2(unNoOfHoaCoeffs);
      m_uintDirIdx.setFieldSizeInBits(indexLength);
   }

   unsigned int getSize(const CVectorBasedInfoChannel &data, 
      bool bIndependencyFlag,
      unsigned int unNumAdditionalHoaCoeffs, 
      unsigned int unNoOfHoaCoeffs,
      unsigned int unCodedVVecLength)
   {  
      unsigned int unSizeInBit = m_uint2Bit.getSizeInBit();
      if(bIndependencyFlag)
	  {  
		 if (1 == unCodedVVecLength) {
		    unSizeInBit += m_bool1Bit.getSizeInBit(); // NewChannelTypeOne flag
		 }
         unSizeInBit += m_uint4Bit.getSizeInBit(); // NbitsQ
         if(data.m_unNbitsQ == cVVecVQWord)
         {
            unSizeInBit += m_uint3Bit.getSizeInBit();  //codebook Idx
            unSizeInBit += m_uintNumberOfDirections.getSizeInBit(); // number of directions		
         }
         else if(data.m_unNbitsQ > c8BitQuantizerWord)
         {				
            unSizeInBit += m_bool1Bit.getSizeInBit(); // CbFlag
         }		
      }
      else
      {
         if (data.m_bSameHeaderPrevFrame)
         {
            unSizeInBit += m_uint2Bit.getSizeInBit(); 
         }
         else
         {
            unSizeInBit += m_uint4Bit.getSizeInBit(); 
            if(data.m_unNbitsQ == cVVecVQWord)
            {
               unSizeInBit += m_uint3Bit.getSizeInBit();  //codebook Idx
               unSizeInBit += m_uintNumberOfDirections.getSizeInBit(); // number of directions		
            }
            else if(data.m_unNbitsQ > c8BitQuantizerWord) 
            {
               unSizeInBit += m_bool1Bit.getSizeInBit(); //cbflag
               unSizeInBit += m_bool1Bit.getSizeInBit(); //pflag
            }			 
         }
      }
      if(data.m_unNbitsQ > c8BitQuantizerWord)
      {       
         std::vector<bool>::const_iterator itAddInfo = data.m_vbAdditionalInfo.begin();
         std::vector<unsigned int>::const_iterator itAddBits = data.m_vunNumBitsAdditional.begin();
         for(std::vector<std::string>::const_iterator it=data.m_vsCodedHuffmannWord.begin();
            it < data.m_vsCodedHuffmannWord.end(); ++it, ++itAddInfo, ++itAddBits)
         {
            unSizeInBit += it->length();
            if(*itAddInfo)
            {
               unSizeInBit += m_bool1Bit.getSizeInBit(); // sign bit
               if(*itAddBits)
               {
                  unSizeInBit += *itAddBits; // bits for mantisse
               }
            }
         }
      }
      else if (data.m_unNbitsQ == c8BitQuantizerWord)
      {
         unSizeInBit += (data.m_vun8bitCodedVelement.size()-unNumAdditionalHoaCoeffs) * m_uint8Bit.getSizeInBit();
      }
      else if (data.m_unNbitsQ == cVVecVQWord)
      { 	  
         switch (data.m_unIndicesCodebookIdx)
         {
         case 0:
            m_uintDirIdx.setFieldSizeInBits(10);
            break;
         case 1:	
			 m_uintDirIdx.setFieldSizeInBits(6);
            break;
         case 2:
			  m_uintDirIdx.setFieldSizeInBits(6);
            break;
         case 3:
         case 4:
         case 5:
         case 6:
         case 7:			
            m_uintDirIdx.setFieldSizeInBits(getCeilLog2(unNoOfHoaCoeffs));           			
         }		


         if (1 < data.m_unVVecDirections)
         {
            unSizeInBit += m_uint8Bit.getSizeInBit(); // weighting cdbk
         }

         unSizeInBit += data.m_unVVecDirections * m_uintDirIdx.getSizeInBit(); // directions  
         unSizeInBit += data.m_unVVecDirections * m_bool1Bit.getSizeInBit(); //signs
      }
      return unSizeInBit;
   };

public:
   UCHAR_FT m_bool1Bit;
   UCHAR_FT m_uint2Bit;
   UINT_FT m_uint3Bit;
   UINT_FT m_uint4Bit;
   UINT_FT m_uint8Bit;
   UINT_FT m_uint10Bit;
   UINT_FT m_uintAddValue;
   UINT_FT m_uintDirIdx;  
   UINT_FT m_uintNumberOfDirections;
   unsigned int m_unCodedVVecLength;
   unsigned int m_unMaxNumOfVecs;
   unsigned int m_unNumOfVecElements;
   unsigned int m_unHoaIdxOffset;
   unsigned int m_unMaxVVecDirections;
   unsigned int m_unVVecDirections;
};


/*********************************************************************
* Additional Ambient HOA Info Channel 
*********************************************************************/
class CAddAmbHoaInfoChannel : public CChannelSideInfoData
{
public: 
   CAddAmbHoaInfoChannel() : CChannelSideInfoData(ADD_HOA_CHANNEL)       
   {
      m_bAmbCoeffIdxChanged = true;
      m_unAmbCoeffIdxTransitionState = 2;
      m_unAmbCoeffIdx = 0;
   };

   ~CAddAmbHoaInfoChannel(){};

public:
   bool m_bAmbCoeffIdxChanged;
   unsigned int m_unAmbCoeffIdxTransitionState;
   unsigned int m_unAmbCoeffIdx;

};

template <class UCHAR_FT,
class UINT_FT>
class CAddAmbHoaInfoChannelIO //: public CAddAmbHoaInfoChannel
{
public:
   CAddAmbHoaInfoChannelIO() : m_unMinNumOfCoeffsForAmbHOA(0)
   {
      m_bool1Bit.setFieldSizeInBits(1);
      m_uint2Bit.setFieldSizeInBits(2);
   };

   ~CAddAmbHoaInfoChannelIO(){};

   void initArrays(unsigned int unMaxNumberOfCoeffsToBeTransmitted,
      int nMinAmbHoaOrder)
   {
      m_unMinNumOfCoeffsForAmbHOA = (nMinAmbHoaOrder+1)*(nMinAmbHoaOrder+1);
      unsigned int nBits = getCeilLog2( unMaxNumberOfCoeffsToBeTransmitted
         - m_unMinNumOfCoeffsForAmbHOA);
      m_unAmbHoaCoeffsAssignmentIO.setFieldSizeInBits(nBits);
   }

   unsigned int getSize(const CAddAmbHoaInfoChannel &data,
      bool bIndependencyFlag)
   {
      unsigned int unSizeInBit = 0; 

      unSizeInBit += m_uint2Bit.getSizeInBit();

      if(bIndependencyFlag)
      {
         // Transition state
         unSizeInBit += m_uint2Bit.getSizeInBit(); 
         // Coded Amb Coeff Idx
         unSizeInBit += m_unAmbHoaCoeffsAssignmentIO.getSizeInBit(); 
      }
      else
      {
         unSizeInBit += m_bool1Bit.getSizeInBit();
         if( (1==data.m_unAmbCoeffIdxTransitionState) & data.m_bAmbCoeffIdxChanged) 
            unSizeInBit += m_unAmbHoaCoeffsAssignmentIO.getSizeInBit(); 
      }
      return unSizeInBit;
   };

public:
   UCHAR_FT m_uint2Bit;
   UCHAR_FT m_bool1Bit;
   UINT_FT m_unAmbHoaCoeffsAssignmentIO;    
   unsigned int m_unMinNumOfCoeffsForAmbHOA;
};

/********************************************************** 
definition of the HOA Directional Prediction Info class */
class HoaDirPredInfo
{
public:
   HoaDirPredInfo() : m_bUseDirPred(false),
      m_bKeepPreviouseGlobalPredDirsFlag(false),
      m_unNumOfGlobalPredDirs(0)
   {
   };

   virtual ~HoaDirPredInfo(){};

public:
   bool m_bUseDirPred; // 1 Bit
   bool m_bKeepPreviouseGlobalPredDirsFlag; // one bit for keeping global dirs constant
   std::vector<bool> m_vbKeepPrevDirPredMatrixFlag; // [band]
   unsigned int m_unNumOfGlobalPredDirs; // MaxNumOfPredDirsLog2+1 Bits
   std::vector<unsigned int> m_vunGlobalPredDirIds; //  m_unNumOfGlobalPredDirs * NumOfBitsPerDirIdx bits
   std::vector<std::vector<bool> > m_vvbDirIsActive; // NumOfPredSubbands * MaxNumOfPredDirsPerBand * 1 bit
   std::vector<std::vector<unsigned int> > m_vvunRelDirGridIdx; //   NumOfPredSubbands 
   // * MaxNumOfPredDirsPerBand * NumBitsForRelDirGridIdx bit
   std::vector<bool> m_vbUseHuffmanCodingDiffAngle; // NumOfPredSubbands
   std::vector<bool> m_vbUseHuffmanCodingDiffMag; // NumOfPredSubbands
   std::vector<std::vector<std::vector<int> > > m_vvvfDecodedMagDiff; // variable length [band][dir][hoaIdx]
   std::vector<std::vector<std::vector<int> > > m_vvvfDecodedAngleDiff; // variable length [band][dir][hoaIdx]
};

template <class UCHAR_FT,
class UINT_FT>
class HoaDirPredInfoIO
{
public:
   HoaDirPredInfoIO() : m_unNumOfPredSubbands(0),
      m_unFirstSBRSubbandIdx(0),
      m_unMaxNumOfPredDirs(0), 
      m_unMaxNumOfPredDirsPerBand(0),
      m_unMinNumOfCoeffsForAmbHOA(0),
      m_unMaxNumOfCoeffsPredicted(0)
   {
      m_bool1Bit.setFieldSizeInBits(1);
      m_uint4Bit.setFieldSizeInBits(4);
      m_numOfGlobalPredDirsIO.setFieldSizeInBits(0);
      m_globalPredDirsIdsIO.setFieldSizeInBits(0);
      m_relDirGridIdxIO.setFieldSizeInBits(0);
   };

   void init(HoaDirPredInfo &data,
      unsigned int unNumOfBitsPerDirIdx,
      unsigned int unNumOfPredSubbands,
      unsigned int unFirstSBRSubbandIdx,
      unsigned int unMaxNumOfPredDirs,
      unsigned int unMaxNumOfPredDirsPerBand,
      int nMinAmbHoaOrder,
      int nMaxHoaOrderToBeTransmitted)
   {
      m_unMaxNumOfCoeffsPredicted = (nMaxHoaOrderToBeTransmitted + 1) * (nMaxHoaOrderToBeTransmitted + 1);
      m_unMinNumOfCoeffsForAmbHOA = (nMinAmbHoaOrder + 1) * (nMinAmbHoaOrder + 1);
      m_numOfGlobalPredDirsIO.setFieldSizeInBits(getCeilLog2(unMaxNumOfPredDirs));
      m_globalPredDirsIdsIO.setFieldSizeInBits(unNumOfBitsPerDirIdx);
      m_unNumOfPredSubbands = unNumOfPredSubbands;
      m_unFirstSBRSubbandIdx = unFirstSBRSubbandIdx;
      m_unMaxNumOfPredDirs = unMaxNumOfPredDirs;
      m_unMaxNumOfPredDirsPerBand = unMaxNumOfPredDirsPerBand;
      data.m_vbKeepPrevDirPredMatrixFlag.assign(m_unNumOfPredSubbands, false);
      data.m_vvbDirIsActive.assign(m_unNumOfPredSubbands, std::vector<bool>(unMaxNumOfPredDirsPerBand, false));
      data.m_vunGlobalPredDirIds.assign(m_unMaxNumOfPredDirs, 0);
      data.m_vvunRelDirGridIdx.assign(m_unNumOfPredSubbands, std::vector<unsigned int>(unMaxNumOfPredDirsPerBand, 0));
      data.m_vvvfDecodedMagDiff.assign(m_unNumOfPredSubbands, 
         std::vector<std::vector<int>>(unMaxNumOfPredDirsPerBand, 
         std::vector<int>(m_unMaxNumOfCoeffsPredicted, 0)));
      data.m_vvvfDecodedAngleDiff.assign(m_unNumOfPredSubbands, 
         std::vector<std::vector<int>>(unMaxNumOfPredDirsPerBand, 
         std::vector<int>(m_unMaxNumOfCoeffsPredicted, 0)));
      data.m_vbUseHuffmanCodingDiffMag.resize(m_unNumOfPredSubbands, true);
      data.m_vbUseHuffmanCodingDiffAngle.resize(m_unNumOfPredSubbands, true);
   };

   unsigned int getSize(HoaDirPredInfo &data,
      std::vector<unsigned int> vunSortedAddHoaCoeffs,
      bool bIndependencyFlag)
   {
      unsigned int unSize = 0;
      // Use directional prediction flag
      unSize += m_bool1Bit.getSizeInBit(); 
      if(data.m_bUseDirPred)
      {
         // Keep previous global directions
         if(!bIndependencyFlag)
            unSize += m_bool1Bit.getSizeInBit();
         else
            data.m_bKeepPreviouseGlobalPredDirsFlag = false;

         if(!data.m_bKeepPreviouseGlobalPredDirsFlag)
         {
            // NumOfGlobalPredDirs
            unSize += m_numOfGlobalPredDirsIO.getSizeInBit(); 

            // set size of umBitsForRelDirGridIdxIO
            m_relDirGridIdxIO.setFieldSizeInBits(getCeilLog2(data.m_unNumOfGlobalPredDirs));

            // for loop over m_unNumOfGlobalPredDirs
            unSize += data.m_unNumOfGlobalPredDirs * m_globalPredDirsIdsIO.getSizeInBit(); 
         }


         for ( unsigned int band = 0; band < m_unNumOfPredSubbands; ++band)
         {
            // Keep previous direction prediction matrix 
            if(!bIndependencyFlag)
               unSize += m_bool1Bit.getSizeInBit();
            else
               data.m_vbKeepPrevDirPredMatrixFlag[band] = false;

            if(!data.m_vbKeepPrevDirPredMatrixFlag[band])
            {
               // Huffman table decision
               unSize += m_bool1Bit.getSizeInBit(); 
               if(band < m_unFirstSBRSubbandIdx)
               {
                  // Huffman table decision for angle differences 
                  unSize += m_bool1Bit.getSizeInBit(); 
               }

               for ( unsigned int dir = 0; dir < m_unMaxNumOfPredDirsPerBand; ++dir)
               {
                  unSize += m_bool1Bit.getSizeInBit(); 
                  if(data.m_vvbDirIsActive[band][dir])
                  {
                     // read RelDirGridIdx
                     unSize += m_relDirGridIdxIO.getSizeInBit();

                     // loop over const HOA channels 
                     for ( unsigned int hoaIdx = 0; hoaIdx < m_unMinNumOfCoeffsForAmbHOA ; hoaIdx++)
                     {
                        unSize += DiffValueIO::getSizeOfDiffValue(band >= m_unFirstSBRSubbandIdx,            
                           data.m_vvvfDecodedMagDiff[band][dir][hoaIdx],
                           data.m_vbUseHuffmanCodingDiffMag[band],
                           g_pDecodedMagDiffTable,
                           sizeof(g_pDecodedMagDiffTable)/sizeof(HuffmanWord<int>),
                           &m_uint4Bit,
                           g_pDecodedMagDiffSbrTable,
                           sizeof(g_pDecodedMagDiffSbrTable)/sizeof(HuffmanWord<int>),
                           &m_uint4Bit,
                           data.m_vvvfDecodedAngleDiff[band][dir][hoaIdx],
                           data.m_vbUseHuffmanCodingDiffAngle[band],
                           g_pDecodedAngleDiffTable,
                           &m_uint4Bit);
                     }

                     // loop over add HOA channels
                     for ( unsigned int hoaIdx = 0; hoaIdx < vunSortedAddHoaCoeffs.size() ; hoaIdx++)
                     {
                        unSize += DiffValueIO::getSizeOfDiffValue(band >= m_unFirstSBRSubbandIdx,            
                           data.m_vvvfDecodedMagDiff[band][dir][vunSortedAddHoaCoeffs[hoaIdx]],
                           data.m_vbUseHuffmanCodingDiffMag[band],
                           g_pDecodedMagDiffTable,
                           sizeof(g_pDecodedMagDiffTable)/sizeof(HuffmanWord<int>),
                           &m_uint4Bit,
                           g_pDecodedMagDiffSbrTable,
                           sizeof(g_pDecodedMagDiffSbrTable)/sizeof(HuffmanWord<int>),
                           &m_uint4Bit,
                           data.m_vvvfDecodedAngleDiff[band][dir][vunSortedAddHoaCoeffs[hoaIdx]],
                           data.m_vbUseHuffmanCodingDiffAngle[band],
                           g_pDecodedAngleDiffTable,
                           &m_uint4Bit);
                     }
                  }
               }
            }
         }
      }
      return unSize;
   }

   virtual ~HoaDirPredInfoIO(){};

public:
   UCHAR_FT m_bool1Bit;
   UINT_FT m_numOfGlobalPredDirsIO;
   UINT_FT m_globalPredDirsIdsIO;
   UINT_FT m_relDirGridIdxIO;
   UINT_FT m_uint4Bit;
   unsigned int m_unNumOfPredSubbands;
   unsigned int m_unFirstSBRSubbandIdx;
   unsigned int m_unMaxNumOfPredDirs; // 3Bit
   unsigned int m_unMaxNumOfPredDirsPerBand;  // escaped value (3,2,5)
   unsigned int m_unMinNumOfCoeffsForAmbHOA;
   unsigned int m_unMaxNumOfCoeffsPredicted;
};

/********************************************************** 
definition of the HOA Spatial Diffuse Field Replication */
class HoaParInfo
{
public:
   HoaParInfo() : m_bPerformPar(false)
   {
      TabulatedValuesHoaBitstream tabVal;
      tabVal.getParSelectionTables(m_SelectionTableMap[1], 1);
      tabVal.getParSelectionTables(m_SelectionTableMap[2], 2);
   };

   virtual ~HoaParInfo(){};

public:
   bool m_bPerformPar;
   std::vector<bool> m_vbKeepPrevMatrixFlag; // [band]
   std::vector<unsigned int> m_vunParDecorrSigsSelectionTableIdx; // [band] 2Bit
   std::vector<std::vector<std::vector<unsigned int> > > m_vvvunParSelectedDecorrSigsIdxMatrix; // [band][sigIdx]
   std::vector<bool> m_vbUseReducedNoOfUpmixSigs; // [band]
   std::vector<std::vector<bool> > m_vvbUseParUpmixSig;// [band][IdxOfUpmixSig]
   std::vector<bool> m_vbUseParHuffmanCodingDiffAbs; // [band] 1bit
   std::vector<bool> m_vbUseParHuffmanCodingDiffAngle; // [band] 1 bit
   std::vector<std::vector<std::vector<int>>> m_vvvnParMixingMatrixDiffAbs; // [band][NumOfUpMixChans][NumOfUpMixChans]
   std::vector<std::vector<std::vector<int>>> m_vvvnParMixingMatrixDiffAngle; // [band][NumOfUpMixChans][NumOfUpMixChans]

public:
   void updateSelectionTableInSubBand(unsigned int band, unsigned int unHoaOrder)
   {
      try{
         m_vvvunParSelectedDecorrSigsIdxMatrix[band] = m_SelectionTableMap[unHoaOrder].at(
            m_vunParDecorrSigsSelectionTableIdx[band]);
      }catch(int e){
         e;
         return;
      }
   }

private:
   std::map<unsigned int, std::map<unsigned int, std::vector<std::vector<unsigned int> > > > m_SelectionTableMap; 
};

template <class UCHAR_FT,
class UINT_FT>
class HoaParInfoIO
{
public:
   HoaParInfoIO() 
   {
      m_bool1Bit.setFieldSizeInBits(1);
      m_uint2Bit.setFieldSizeInBits(2);
      m_uint4Bit.setFieldSizeInBits(4);
   };

   void init(HoaParInfo &data,
      const std::vector<unsigned int> &vunParUpmixHoaOrderPerParSubBandIdx,
      const std::vector<bool> &vbUseRealCoeffsPerParSubband)
   {
      unsigned int unMaxNumOfParSubbands = vunParUpmixHoaOrderPerParSubBandIdx.size();
      m_vbUseRealCoeffsPerParSubband = vbUseRealCoeffsPerParSubband;
      m_vunParUpmixHoaOrderPerParSubBandIdx = vunParUpmixHoaOrderPerParSubBandIdx;
      data.m_vbKeepPrevMatrixFlag.resize(unMaxNumOfParSubbands);
      data.m_vunParDecorrSigsSelectionTableIdx.resize(unMaxNumOfParSubbands);
      data.m_vvvunParSelectedDecorrSigsIdxMatrix.resize(unMaxNumOfParSubbands);
      data.m_vbUseReducedNoOfUpmixSigs.resize(unMaxNumOfParSubbands);
      data.m_vvbUseParUpmixSig.resize(unMaxNumOfParSubbands);
      data.m_vbUseParHuffmanCodingDiffAbs.resize(unMaxNumOfParSubbands);
      data.m_vbUseParHuffmanCodingDiffAngle.resize(unMaxNumOfParSubbands);
      data.m_vvvnParMixingMatrixDiffAbs.resize(unMaxNumOfParSubbands);
      data.m_vvvnParMixingMatrixDiffAngle.resize(unMaxNumOfParSubbands);
      for(unsigned int band = 0; band < unMaxNumOfParSubbands; band++)
      {
         unsigned int unMaxNumOfUpmixSigs = 
            (vunParUpmixHoaOrderPerParSubBandIdx[band] + 1) * (vunParUpmixHoaOrderPerParSubBandIdx[band] + 1);
         std::vector<unsigned int> tmp(unMaxNumOfUpmixSigs, 0);
         data.m_vunParDecorrSigsSelectionTableIdx[band] = 0;
         data.updateSelectionTableInSubBand(band, vunParUpmixHoaOrderPerParSubBandIdx[band]);
         data.m_vvbUseParUpmixSig[band].resize(unMaxNumOfUpmixSigs);
         data.m_vvvnParMixingMatrixDiffAbs[band].resize(unMaxNumOfUpmixSigs, 
            std::vector<int>(unMaxNumOfUpmixSigs, 0));
         data.m_vvvnParMixingMatrixDiffAngle[band].resize(unMaxNumOfUpmixSigs, 
            std::vector<int>(unMaxNumOfUpmixSigs, 0));
      }
   };

   unsigned int getSize(HoaParInfo &data, bool bIndependencyFlag)
   {
      unsigned int unSize = m_bool1Bit.getSizeInBit();
      if(data.m_bPerformPar)
      {
         for(unsigned int band=0; band < data.m_vbKeepPrevMatrixFlag.size(); band++)
         {
            // keep matrix flag
            if(!bIndependencyFlag)
               unSize += m_bool1Bit.getSizeInBit();
            else
               data.m_vbKeepPrevMatrixFlag[band] = false;

            if(!data.m_vbKeepPrevMatrixFlag[band])
            {
               // ParDecorrSigsSelectionTableIdx
               unSize += m_uint2Bit.getSizeInBit();
               data.updateSelectionTableInSubBand(band, 
                  m_vunParUpmixHoaOrderPerParSubBandIdx[band]);

               // reduce matrix size
               unSize += m_bool1Bit.getSizeInBit();
               if(data.m_vbUseReducedNoOfUpmixSigs[band])
               {
                  unSize += m_bool1Bit.getSizeInBit() * data.m_vvbUseParUpmixSig[band].size();
               }   
               // Huffman table index
               unSize += m_bool1Bit.getSizeInBit();
               if( !m_vbUseRealCoeffsPerParSubband[band] )
                  unSize += m_bool1Bit.getSizeInBit();

               // Loop over each up-mix signal
               for(unsigned int n=0; n < data.m_vvvnParMixingMatrixDiffAbs[band].size(); ++n)
               {
                  // Loop over all active parameters for each active up-mix signal
                  for(unsigned int m=0; (m < data.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n].size()) & data.m_vvbUseParUpmixSig[band][n]; ++m)
                  {
                     unSize += DiffValueIO::getSizeOfDiffValue(m_vbUseRealCoeffsPerParSubband[band],            
                        data.m_vvvnParMixingMatrixDiffAbs[band][n][data.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n][m]],
                        data.m_vbUseParHuffmanCodingDiffAbs[band],
                        g_pDecodedParMagDiffTable,
                        sizeof(g_pDecodedParMagDiffTable)/sizeof(HuffmanWord<int>),
                        &m_uint4Bit,
                        g_pDecodedParMagDiffSbrTable,
                        sizeof(g_pDecodedParMagDiffSbrTable)/sizeof(HuffmanWord<int>),
                        &m_uint4Bit,
                        data.m_vvvnParMixingMatrixDiffAngle[band][n][data.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n][m]],
                        data.m_vbUseParHuffmanCodingDiffAngle[band],
                        g_pDecodedParAngleDiffTable,
                        &m_uint4Bit);
                  }
               }
            }
         }
      }
      return unSize;
   }

   virtual ~HoaParInfoIO(){};

public:
   UCHAR_FT m_bool1Bit;
   UINT_FT m_uint2Bit;
   UINT_FT m_uint4Bit;
   std::vector<bool> m_vbUseRealCoeffsPerParSubband;
   std::vector<unsigned int> m_vunParUpmixHoaOrderPerParSubBandIdx;

};


/******************************************************** 
definition of the HOA Frame class                     */
class HoaFrame
{
public:
   /** sets the default parameter of the fields */
   HoaFrame( ) : m_bHoaIndependencyFlag(false)
   { };

   HoaFrame(const HoaFrame & rhs)
   {
      copy(rhs);
   }

   HoaFrame& operator=( const HoaFrame& rhs ) 
   {
      // copy only if its not me!
      if(this!= &rhs)
         copy(rhs);
      return *this;
   }

   virtual ~HoaFrame(){};

public:
   std::vector<CHoaGainCorrection> m_vGainCorrectionData;
   std::vector<std::shared_ptr<CChannelSideInfoData> > m_vChannelSideInfo;	
   CSpatialPredictionData m_spatPredictionData;
   HoaDirPredInfo m_hoaDirectionalPredictionInfo;
   HoaParInfo m_hoaParInfo;
   bool m_bHoaIndependencyFlag; // 1Bit

protected:
   void copy(const HoaFrame & rhs) 
   {
      /* do deep copy of all fields */
      m_bHoaIndependencyFlag = rhs.m_bHoaIndependencyFlag;
      m_vGainCorrectionData = rhs.m_vGainCorrectionData;
      m_vChannelSideInfo.clear();
      m_vChannelSideInfo.reserve(rhs.m_vChannelSideInfo.size());
      for(std::vector<std::shared_ptr<CChannelSideInfoData> >::const_iterator it = rhs.m_vChannelSideInfo.begin();
         it < rhs.m_vChannelSideInfo.end(); ++it)
      {
         switch((*it)->m_unChannelType)
         {
         case DIR_CHANNEL:
            m_vChannelSideInfo.push_back(std::shared_ptr<CChannelSideInfoData>(new CDirectionalInfoChannel(*dynamic_cast<CDirectionalInfoChannel*>(it->get()))));
            break;
         case ADD_HOA_CHANNEL:
            m_vChannelSideInfo.push_back(std::shared_ptr<CChannelSideInfoData>(new CAddAmbHoaInfoChannel(*dynamic_cast<CAddAmbHoaInfoChannel*>(it->get()))));
            break;
         case VEC_CHANNEL:
            m_vChannelSideInfo.push_back(std::shared_ptr<CChannelSideInfoData>(new CVectorBasedInfoChannel(*dynamic_cast<CVectorBasedInfoChannel*>(it->get())))); 
            break;
         case EMPTY_CHANNEL:
         default:
            m_vChannelSideInfo.push_back(std::shared_ptr<CChannelSideInfoData>(new CEmptyInfoChannel(*dynamic_cast<CEmptyInfoChannel*>(it->get()))));
         }
      }
      m_spatPredictionData = rhs.m_spatPredictionData;
      m_hoaDirectionalPredictionInfo = rhs.m_hoaDirectionalPredictionInfo;
      m_hoaParInfo = rhs.m_hoaParInfo;
   }
};

/******************************************************** 
definition of the HOA Frame IO class                  */
template <class UCHAR_FT,
class INT_FT,
class UINT_FT>
class HoaFrameIO : public HoaFrame
{
public:
   /** sets the default parameter of the fields */
   HoaFrameIO( ) : m_unNumOfAdditionalCoders(0),
      m_unTotalNumCoders(0),
      m_unNoOfHoaCoeffs(0),
      m_bGlobalIndependencyFlag(false)
   { 
      m_bool1Bit.setByteAlignFlag(false);
      m_bool1Bit.setFieldSizeInBits(1);
   };

   virtual ~HoaFrameIO(){};

   /** sets the default values of the fields */
   void initArrays(unsigned int unNoOfHoaCoeffs,
      unsigned int unMaxNoOfDirSigsForPrediction,
      unsigned int unNoOfBitsPerScaleFactor,
      int          nMinAmbHoaOrder,
      unsigned int unTotalNumCoders,
      unsigned int unNumOfAdditionalCoders,
      unsigned int unCodedVVecLength,
      unsigned int unMaxGainCorrAmpExp,
      unsigned int unVVecDirections,
      unsigned int unDirGridTableIdx,
      unsigned int unMaxNumOfPredDirs,
      unsigned int unMaxNumOfPredDirsPerBand,
      unsigned int unNumOfPredSubbands,
      unsigned int unFirstSBRSubbandIdx,
      int nMaxHoaOrderToBeTransmitted,
      const std::vector<unsigned int> &vunParUpmixHoaOrderPerParSubBandIdx,
      const std::vector<bool> &vbUseRealCoeffsPerParSubband
      )
   {
      // set helper variables 
      m_unNumOfAdditionalCoders = unNumOfAdditionalCoders;
      m_unTotalNumCoders = unTotalNumCoders;
      m_bGlobalIndependencyFlag = false;
      m_unNoOfHoaCoeffs = unNoOfHoaCoeffs;

      // get number of bits for coding the direction index
      TabulatedValuesHoaBitstream tabVal;
      std::vector<unsigned int> vunNumOfBitsPerDirIdxTable;
      tabVal.getNumOfBitsPerDirIdxTable(vunNumOfBitsPerDirIdxTable);
      unsigned int unNumOfBitsPerDirIdx = vunNumOfBitsPerDirIdxTable[unDirGridTableIdx];
      if(unDirGridTableIdx == 3)
      {
         // reserved case -> error set to 0
         unNumOfBitsPerDirIdx = 0;
      }

      /* initialize gain correction data array*/
      m_vGainCorrectionData.resize(m_unTotalNumCoders);
      for(std::vector<CHoaGainCorrection>::iterator it = m_vGainCorrectionData.begin(); 
         it < m_vGainCorrectionData.end(); it++)
      {
         m_gainCorrectionIO.initArrays(*it, unNoOfHoaCoeffs, unMaxGainCorrAmpExp);
      }

      /* initialize spatial prediction data */
      m_spatPredictionDataIO.initArrays(m_spatPredictionData, unMaxNoOfDirSigsForPrediction, 
         unNoOfHoaCoeffs, unNoOfBitsPerScaleFactor);

      /* initialize Side Info Channels */
      m_vChannelSideInfo.assign(unNumOfAdditionalCoders, std::shared_ptr<CChannelSideInfoData>(new CEmptyInfoChannel()));

      /* initialize previous Side Info Channel */
      m_vPrevFrameChannelSideInfo = m_vChannelSideInfo;

      /* init directional channel IO */
      m_directionalInfoChannelIO.init(10);

      /* init add. ambient HOA Info Channel IO class */
      m_addAmbHoaInfoChannelIO.initArrays((nMaxHoaOrderToBeTransmitted+1)*(nMaxHoaOrderToBeTransmitted+1), nMinAmbHoaOrder);


      /* init vector based predominant sounds info channel*/
      m_vectorBasedInfoChannelIO.initArrays(unNumOfAdditionalCoders, nMinAmbHoaOrder, unNoOfHoaCoeffs, unCodedVVecLength, unVVecDirections);

      /* init directional prediction IO */
      m_hoaDirectionalPredictionInfoIO.init(m_hoaDirectionalPredictionInfo, unNumOfBitsPerDirIdx, unNumOfPredSubbands,
         unFirstSBRSubbandIdx, unMaxNumOfPredDirs,
         unMaxNumOfPredDirsPerBand, nMinAmbHoaOrder, nMaxHoaOrderToBeTransmitted);

      /* init HOA PAR IO */
      m_hoaParInfoIO.init(m_hoaParInfo, vunParUpmixHoaOrderPerParSubBandIdx, vbUseRealCoeffsPerParSubband);


   }


   /** get size of the spatial data in bit*/
   unsigned int getSpatFrameSize()
   {
      /* get size of the Channel Side Info Data */
      unsigned int nSizeInBit = 0;

      if(m_bGlobalIndependencyFlag){
         m_bHoaIndependencyFlag = m_bGlobalIndependencyFlag;
      }
      nSizeInBit += m_bool1Bit.getSizeInBit();

      unsigned int unNumAddHoaChannels = 0;
      unsigned int unNumFadeInAddHoaChannels = 0;
      unsigned int unChannelNumber = 0;
      std::vector<unsigned int>vunNewVecSignals;
      std::vector<std::shared_ptr<CChannelSideInfoData>> pvVecChannels;
      bool bDirChannelUsed = false;

      m_vunAddHoaCoeff.resize(0);

      for(std::vector<std::shared_ptr<CChannelSideInfoData> >::const_iterator it = m_vChannelSideInfo.begin();
         it < m_vChannelSideInfo.end(); it++)
      {
         switch((*it)->m_unChannelType)
         {
         case DIR_CHANNEL:
            nSizeInBit += m_directionalInfoChannelIO.getSize();
            bDirChannelUsed = true;
            break;
         case ADD_HOA_CHANNEL:
            nSizeInBit += m_addAmbHoaInfoChannelIO.getSize(*dynamic_cast<CAddAmbHoaInfoChannel*>(it->get()),
               m_bHoaIndependencyFlag);
            if (1== m_vectorBasedInfoChannelIO.m_unCodedVVecLength)
            {
               switch(dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState){
               case 0:
                  unNumAddHoaChannels++;
                  break;
               case 1:
                  unNumFadeInAddHoaChannels++;
                  break;
               }
            }
            //if ((0==dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState) && (1== m_vectorBasedInfoChannelIO.m_unCodedVVecLength) )
            // unNumAddHoaChannels++;
            m_vunAddHoaCoeff.push_back(dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx - 1);
            break;
         case VEC_CHANNEL:
            pvVecChannels.push_back(*it);
            if ((1 == m_vectorBasedInfoChannelIO.m_unCodedVVecLength) && (m_vPrevFrameChannelSideInfo[unChannelNumber]->m_unChannelType != VEC_CHANNEL))
            {
               vunNewVecSignals.push_back(1);
            }
            else
            {
               vunNewVecSignals.push_back(0);
            }

            break;
         default:
            nSizeInBit += m_emptyInfoChannelIO.getSize();
         }
         unChannelNumber++;
      }
      m_vPrevFrameChannelSideInfo = m_vChannelSideInfo; 
      /* count size of vector-based channels */
      for(unsigned int n = 0; n < pvVecChannels.size(); ++n)
      {
         nSizeInBit += m_vectorBasedInfoChannelIO.getSize(*dynamic_cast<CVectorBasedInfoChannel*>(
            pvVecChannels[n].get()), 
            m_bHoaIndependencyFlag,
            unNumAddHoaChannels+(vunNewVecSignals[n]*unNumFadeInAddHoaChannels),
            m_unNoOfHoaCoeffs,
            m_vectorBasedInfoChannelIO.m_unCodedVVecLength
            );
      }

      /* get HOA Gain Correction Data size */
      for(std::vector<CHoaGainCorrection>::iterator it = m_vGainCorrectionData.begin();
         it < m_vGainCorrectionData.end(); ++it)
      {
         nSizeInBit +=m_gainCorrectionIO.getSize(*it, m_bHoaIndependencyFlag);
      }

      /* get Spatial Prediction Data Size */
      if(bDirChannelUsed)
         nSizeInBit += m_spatPredictionDataIO.getSize(m_spatPredictionData);

      /* get size of directional prediction info */
      if(m_hoaDirectionalPredictionInfoIO.m_unNumOfPredSubbands > 0)
      {
         std::vector<unsigned int> vunSortedAddHoaCoeff = m_vunAddHoaCoeff;
         std::sort(vunSortedAddHoaCoeff.begin(), vunSortedAddHoaCoeff.end());
         nSizeInBit += m_hoaDirectionalPredictionInfoIO.getSize(m_hoaDirectionalPredictionInfo, vunSortedAddHoaCoeff, 
            m_bHoaIndependencyFlag);
      }

      /* get size of PAR data */
      if(m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband.size() > 0)
         nSizeInBit += m_hoaParInfoIO.getSize(m_hoaParInfo, m_bHoaIndependencyFlag);

      return nSizeInBit;
   }

protected:
   CHoaGainCorrectionIO<UCHAR_FT, UINT_FT> m_gainCorrectionIO;
   CSpatialPredictionDataIO<UCHAR_FT, INT_FT, UINT_FT> m_spatPredictionDataIO;
   CDirectionalInfoChannelIO<UCHAR_FT, UINT_FT> m_directionalInfoChannelIO;
   CAddAmbHoaInfoChannelIO<UCHAR_FT, UINT_FT> m_addAmbHoaInfoChannelIO;
   CEmptyInfoChannelIO<UCHAR_FT> m_emptyInfoChannelIO;
   CVectorBasedInfoChannelIO<UCHAR_FT, UINT_FT> m_vectorBasedInfoChannelIO;
   HoaDirPredInfoIO<UCHAR_FT, UINT_FT> m_hoaDirectionalPredictionInfoIO;
   HoaParInfoIO<UCHAR_FT, UINT_FT> m_hoaParInfoIO;
   UCHAR_FT m_bool1Bit;
   unsigned int m_unNumOfAdditionalCoders;
   unsigned int m_unTotalNumCoders;
   unsigned int m_unNoOfHoaCoeffs;
   bool m_bGlobalIndependencyFlag;
   std::vector<std::shared_ptr<CChannelSideInfoData> > m_vPrevFrameChannelSideInfo; // NP bugfix  

   // Phase 2 add-on
   std::vector<unsigned int> m_vunAddHoaCoeff;
};


#endif // __STDHOAFRAME__
