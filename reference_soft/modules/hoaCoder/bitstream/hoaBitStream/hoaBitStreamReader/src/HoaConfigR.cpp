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
$Id: HoaConfigR.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#include "HoaConfigR.h"

HoaConfigR::HoaConfigR() : HoaConfigIO<UCHAR_FR, UINT_FR, FLOAT_FR> (20,1024)
{
};

int HoaConfigR::read()
{
   int nBitsRead = 0;
   if(m_bitReader)
   {
      /** set bit stream reader */
      m_ucharByte.setBitStreamReader(m_bitReader);
      m_bool1Bit.setBitStreamReader(m_bitReader);
      m_uint4Bit.setBitStreamReader(m_bitReader);
      m_uint3Bit.setBitStreamReader(m_bitReader);
      m_uint2Bit.setBitStreamReader(m_bitReader);
      m_uint5Bit.setBitStreamReader(m_bitReader);
      m_float4Byte.setBitStreamReader(m_bitReader);
      m_unLastFirstOrderSubBandIdxIO.setBitStreamReader(m_bitReader);
      m_unVqConfBits.setBitStreamReader(m_bitReader);
      m_unFirstSBRSubbandIdxIO.setBitStreamReader(m_bitReader);
	   m_unDiffOrder.setBitStreamReader(m_bitReader);//NP

      // read HOA order
      nBitsRead += m_uint3Bit.read();
      m_unHoaOrder = m_uint3Bit;
      if(m_unHoaOrder==7)
      {
         nBitsRead += m_uint5Bit.read();
         m_unHoaOrder += m_uint5Bit;
      } 

      // read m_bIsScreenRelative
      nBitsRead += m_bool1Bit.read();
      m_bIsScreenRelative = m_bool1Bit==1;

      // read near field HOA compensation flag
      nBitsRead += m_bool1Bit.read();
      m_bUsesNfc = m_bool1Bit==1;

      // read NFC radius
      if(m_bUsesNfc)
      {
         nBitsRead += m_float4Byte.read();
         m_ufAmbNfcReferenceDistance = m_float4Byte;
      }

      // read min. HOA order
      nBitsRead += m_uint3Bit.read();
      m_nMinAmbHoaOrder = m_uint3Bit - 1;
      if(m_nMinAmbHoaOrder==6)
      {
         nBitsRead += m_uint5Bit.read();
         m_nMinAmbHoaOrder += m_uint5Bit;
      } 

      // compute number of additional HOA channels
      m_unNumOfAdditionalCoders = m_unTotalNumCoders - 
         ((unsigned int)m_nMinAmbHoaOrder+1) * ((unsigned int)m_nMinAmbHoaOrder+1);

      nBitsRead += m_bool1Bit.read();
      m_bSingleLayer = m_bool1Bit==1;


      // read spatial interpolation time
      nBitsRead += m_uint3Bit.read();
      unsigned int codedSpatialInterpolationTime = m_uint3Bit;

      // read spatial interpolation method flag
      nBitsRead += m_bool1Bit.read();
      m_unSpatInterpMethod = m_bool1Bit==1;

      // read codedVVec Length
      nBitsRead += m_uint2Bit.read();
      m_unCodedVVecLength = m_uint2Bit;

      // read maximal gain correction amplification exponent
      nBitsRead += m_uint3Bit.read();
      m_unMaxGainCorrAmpExp = m_uint3Bit;

      // read FrameLengthFactor
      nBitsRead += m_uint2Bit.read();
      m_unFrameLengthIndicator = m_uint2Bit;
      if(m_unFrameLengthIndicator < 3)
         m_unHoaFrameLength = m_mHoaFrameLengthTable[m_unCoreCoderFrameLength][m_unFrameLengthIndicator];
      else
         return -1;
      m_unSpatialInterpolationTime = m_mSpatInterpTimeCodeTable[m_unHoaFrameLength][codedSpatialInterpolationTime];

      // Phase 2 extensions

      // read maximum HOA order to be transmitted

      if( (unsigned int)(m_nMinAmbHoaOrder+1) < (m_unHoaOrder+1) ) {
         m_unDiffOrder.setFieldSizeInBits(getCeilLog2(m_unHoaOrder - m_nMinAmbHoaOrder + 1));
         nBitsRead += m_unDiffOrder.read();
         m_nMaxHoaOrderToBeTransmitted = m_unDiffOrder + m_nMinAmbHoaOrder;
      }
      else
      {
         m_nMaxHoaOrderToBeTransmitted = m_unHoaOrder;
      }

      m_unVqConfBits.setFieldSizeInBits(getCeilLog2(getCeilLog2( (m_unHoaOrder+1)*(m_unHoaOrder+1) +1) ) );  //bugfix 2016-03-01 [NP]
      nBitsRead += m_unVqConfBits.read();
      m_unNumVVecVqElementsBits = m_unVqConfBits;//+1; //bugfix 2016-03-01 [NP]
      m_unMaxVVecDirections = getPow2(m_unNumVVecVqElementsBits);

      // read flag to indicate the usage of decorrelation method proposed by QUALCOMM
      m_bUsePhaseShiftDecorr = false; // default setting

      if (m_nMinAmbHoaOrder == 1)
      {
         nBitsRead += m_bool1Bit.read();
         m_bUsePhaseShiftDecorr = (m_bool1Bit == 1);
      }

      // read number of directional signals are used in the predictions method
      nBitsRead += m_uint2Bit.read();
      m_unMaxNoOfDirSigsForPrediction = m_uint2Bit + 1;

      // read number of bits per scale factor
      nBitsRead += m_uint4Bit.read();
      m_unNoOfBitsPerScaleFactor = m_uint4Bit + 1;

      // read configuration of prediction sub bands
      nBitsRead += m_uint2Bit.read();
      m_unSubbandConfigIdx = m_uint2Bit;

      if(m_unSubbandConfigIdx == m_vvunPredSubbandTable.size())
      {
         // read custom subband configuration
         decodeSubbandConfig(m_vunPredSubbandWidths);
      }
      else
      {
         m_vunPredSubbandWidths= m_vvunPredSubbandTable[m_unSubbandConfigIdx];
      }
      m_unNumOfPredSubbands = m_vunPredSubbandWidths.size();

      if(m_unNumOfPredSubbands>0)
      {
         // read first SBR sub band Index
         m_unFirstSBRSubbandIdxIO.setFieldSizeInBits(getCeilLog2(m_unNumOfPredSubbands+1));
         nBitsRead += m_unFirstSBRSubbandIdxIO.read();
         m_unFirstSBRSubbandIdx = m_unFirstSBRSubbandIdxIO;

         // read maximum number of predicted directions 
         nBitsRead += m_uint3Bit.read();
         m_unMaxNumOfPredDirs = 1<<m_uint3Bit;

         // read maximum number of predicted directions per subband
         nBitsRead += m_uint3Bit.read();
         m_unMaxNumOfPredDirsPerBand = m_uint3Bit + 1;
         if( ( (m_unMaxNumOfPredDirsPerBand-1) == ((1<<3)-1))  ) 
         {
            nBitsRead += m_uint2Bit.read(); 
            m_unMaxNumOfPredDirsPerBand += m_uint2Bit;       
         } 
         // escaped value with 2 extra bits and 5 extra bits 
         if((m_unMaxNumOfPredDirsPerBand-1) == ((1<<3)-1 + (1<<2) - 1))
         {
            nBitsRead += m_uint5Bit.read();
            m_unMaxNumOfPredDirsPerBand += m_uint5Bit; 
         }

         // read table index for direction grid
         nBitsRead += m_uint2Bit.read(); 
         m_unDirGridTableIdx = m_uint2Bit;      

      }


      // read PAR sub band table index
      nBitsRead += m_uint2Bit.read();
      m_unParSubBandTableIdx = m_uint2Bit;

      if(m_unParSubBandTableIdx>0)
      {
         if(m_unParSubBandTableIdx == m_vvunParSubbandTable.size())
         {
            decodeSubbandConfig(m_vunParSubbandWidths);
         }
         else
         {
            m_vunParSubbandWidths= m_vvunParSubbandTable[m_unParSubBandTableIdx];
         }

         // read last first order sub band index
         m_unLastFirstOrderSubBandIdxIO.setFieldSizeInBits(getCeilLog2(m_vunParSubbandWidths.size()+1));
         nBitsRead += m_unLastFirstOrderSubBandIdxIO.read();
         m_unLastFirstOrderSubBandIdx = m_unLastFirstOrderSubBandIdxIO;
         m_vunParUpmixHoaOrderPerParSubBandIdx.assign(m_vunParSubbandWidths.size(), 2);

         // read use real coefficients per par sub band
         m_vbUseRealCoeffsPerParSubband.resize(m_vunParSubbandWidths.size(), 0);
         for(unsigned int n = 0; n < m_vbUseRealCoeffsPerParSubband.size(); ++n)
         {
            nBitsRead += m_bool1Bit.read();
            m_vbUseRealCoeffsPerParSubband[n] = (m_bool1Bit == 1);
            if(n <= m_unLastFirstOrderSubBandIdx)
               m_vunParUpmixHoaOrderPerParSubBandIdx[n] = 1;
         }
      }
      else
      {
         m_vunParSubbandWidths.resize(0);
         m_vunParUpmixHoaOrderPerParSubBandIdx.resize(0);
         m_vbUseRealCoeffsPerParSubband.resize(0, 0);
      }
   }
   return nBitsRead;
};

const HoaConfig& HoaConfigR::getFields()
{
   return *this;
};


int HoaConfigR::read(unsigned int unNumTotalCoders, unsigned int unCoreCoderFrameLength)
{
   m_unTotalNumCoders = unNumTotalCoders;
   m_unCoreCoderFrameLength = unCoreCoderFrameLength;
   return read();
}

int HoaConfigR::decodeSubbandConfig(std::vector<unsigned int> &rvunSubbandWidths)
{
   unsigned int nBitsRead = 0;
   unsigned int nTotalBwSum = 0;

   nBitsRead += m_uint5Bit.read(); // coded number of subbands
   unsigned int nNumSubbands = m_uint5Bit + 1;

   rvunSubbandWidths.resize(nNumSubbands);

   if (nNumSubbands > 1)
   {
      unsigned int tmp = 0;
      // first band
      unsigned int nb = 0; 
      nBitsRead += readUnaryCode(&tmp); // unary code of CodedBwFirstBand
      rvunSubbandWidths[nb] = tmp + 1;
      nTotalBwSum += rvunSubbandWidths[nb];

      if (nNumSubbands > 2)
      {
         for (nb = 1; nb < nNumSubbands-2; nb++)
         {
            nBitsRead += readUnaryCode(&tmp); // unary code of bandwidth diff.
            rvunSubbandWidths[nb] = rvunSubbandWidths[nb-1] + tmp;
            nTotalBwSum += rvunSubbandWidths[nb];
         }
         // last transmitted subband
         nBitsRead += m_uint5Bit.read();
         rvunSubbandWidths[nb] = rvunSubbandWidths[nb-1] + m_uint5Bit;
         nTotalBwSum += rvunSubbandWidths[nb];
      }
   }
   // bandwidth of last subband -- nTotalBwSum must be <= 64
   rvunSubbandWidths[nNumSubbands-1] = 64 - nTotalBwSum;

   return nBitsRead;
}

int HoaConfigR::readUnaryCode(unsigned int *pVal)
{
   unsigned int nBitsRead = 0;
   *pVal = 0; // init
   nBitsRead += m_bool1Bit.read();
   *pVal += m_bool1Bit;
   while(m_bool1Bit == 1)
   {
      nBitsRead += m_bool1Bit.read();
      *pVal += m_bool1Bit;
   }

   return nBitsRead;
}

