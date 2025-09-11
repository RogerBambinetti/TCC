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
$Id: HoaConfigW.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include "HoaConfigW.h"
#include <iostream>


HoaConfigW::HoaConfigW() : HoaConfigIO<UCHAR_FW, UINT_FW, FLOAT_FW> (20, 1024)
{
};

int HoaConfigW::write()
{
   int nBitsWritten = 0;
   if(m_bitWriter)
   {
      /** set bit stream writer */
      m_ucharByte.setBitStreamWriter(m_bitWriter);
      m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_uint4Bit.setBitStreamWriter(m_bitWriter);
      m_uint3Bit.setBitStreamWriter(m_bitWriter);
      m_uint2Bit.setBitStreamWriter(m_bitWriter);
      m_uint5Bit.setBitStreamWriter(m_bitWriter);
      m_float4Byte.setBitStreamWriter(m_bitWriter);
      m_unLastFirstOrderSubBandIdxIO.setBitStreamWriter(m_bitWriter);
      m_unVqConfBits.setBitStreamWriter(m_bitWriter);
      m_unFirstSBRSubbandIdxIO.setBitStreamWriter(m_bitWriter);
      m_unDiffOrder.setBitStreamWriter(m_bitWriter);

      //converting frameLength to FrameLengthFactor
      std::map<unsigned int, std::vector<unsigned int> >::iterator it = 
         m_mHoaFrameLengthTable.find(m_unCoreCoderFrameLength);
      if(it == m_mHoaFrameLengthTable.end())
         return -1;
      std::vector<unsigned int>::iterator it2 = 
         std::find(it->second.begin(), it->second.end(), m_unHoaFrameLength);
      if(it2 == it->second.end())
         return -1;
      m_unFrameLengthIndicator = std::distance(it->second.begin(), it2);

      // write HOA order
      if(m_unHoaOrder<7)
      {
         m_uint3Bit = m_unHoaOrder;
         nBitsWritten += m_uint3Bit.write();
      }
      else
      {
         // write exception value
         m_uint3Bit = 7;
         nBitsWritten += m_uint3Bit.write();
         // write extended value
         m_uint5Bit = m_unHoaOrder - 7;
         nBitsWritten += m_uint5Bit.write();
      }

      // write m_bIsScreenRelative
      m_bool1Bit = m_bIsScreenRelative;
      nBitsWritten += m_bool1Bit.write();
     

      // write near field HOA compensation flag
      m_bool1Bit = m_bUsesNfc;
      nBitsWritten += m_bool1Bit.write();

      // write NFC radius
      if(m_bUsesNfc)
      {
         m_float4Byte = m_ufAmbNfcReferenceDistance;
         nBitsWritten += m_float4Byte.write();
      }

      // write min. HOA order
      if(m_nMinAmbHoaOrder<6)
      {
         m_uint3Bit = m_nMinAmbHoaOrder + 1;
         nBitsWritten += m_uint3Bit.write();
      }
      else
      {
         // write exception value
         m_uint3Bit = 7;
         nBitsWritten += m_uint3Bit.write();
         // write extended value
         m_uint5Bit = m_nMinAmbHoaOrder + 1 - 7;
         nBitsWritten += m_uint5Bit.write();
      }

      m_bool1Bit = m_bSingleLayer;
      nBitsWritten += m_bool1Bit.write();

      // write spatial interpolation time
      it = m_mSpatInterpTimeCodeTable.find(m_unHoaFrameLength);
      if(it == m_mSpatInterpTimeCodeTable.end())
         return -1;
      it2 = std::find(it->second.begin(), it->second.end(), m_unSpatialInterpolationTime);
      if(it2 == it->second.end())
         return -1;
      m_uint3Bit = std::distance(it->second.begin(), it2);
      nBitsWritten += m_uint3Bit.write();

      // write spatial interpolation method flag
      m_bool1Bit = m_unSpatInterpMethod==1;
      nBitsWritten += m_bool1Bit.write();

      // write coded VVector length
      m_uint2Bit = m_unCodedVVecLength;
      nBitsWritten += m_uint2Bit.write();

      // write maximum gain correction amplification exponent
      m_uint3Bit = m_unMaxGainCorrAmpExp;
      nBitsWritten += m_uint3Bit.write();

      m_uint2Bit = m_unFrameLengthIndicator;
      nBitsWritten += m_uint2Bit.write();


      // Phase 2 extensions

      // write maximum HOA order to be transmitted
      if( m_nMinAmbHoaOrder < static_cast<int>(m_unHoaOrder) )
      {
         m_unDiffOrder.setFieldSizeInBits(
            getCeilLog2(m_unHoaOrder - m_nMinAmbHoaOrder + 1));
         m_unDiffOrder = m_nMaxHoaOrderToBeTransmitted - m_nMinAmbHoaOrder;
         nBitsWritten += m_unDiffOrder.write();
      }
      else
      {
         m_nMaxHoaOrderToBeTransmitted = m_unHoaOrder;
      }

      // write NumVVecVqElementsBits for VVector VQ
      m_unVqConfBits.setFieldSizeInBits(getCeilLog2( getCeilLog2( (m_unHoaOrder+1)*(m_unHoaOrder+1) +1) ));
      m_unVqConfBits = getCeilLog2(m_unMaxVVecDirections);
      nBitsWritten += m_unVqConfBits.write();

      if (m_nMinAmbHoaOrder == 1)
      {
         // write flag to indicate the usage of decorrelation method proposed by QUALCOMM
         m_bool1Bit      = m_bUsePhaseShiftDecorr;
         nBitsWritten += m_bool1Bit.write();
      }

      // write number of directional signals are used in the predictions method
      m_uint2Bit = m_unMaxNoOfDirSigsForPrediction - 1;
      nBitsWritten += m_uint2Bit.write();

      // write number of bits per scale factor
      m_uint4Bit = m_unNoOfBitsPerScaleFactor - 1;
      nBitsWritten += m_uint4Bit.write();

      // write subband configuration index for dir. prediction
      m_uint2Bit = m_unSubbandConfigIdx;
      nBitsWritten += m_uint2Bit.write();

      if(m_uint2Bit==3)
      {
         // write custom subband configuration using diff. coded bandwidths
         nBitsWritten += encodeSubbandConfig(m_vunPredSubbandWidths);
      }

      if(m_unNumOfPredSubbands>0)
      {
         // write first SBR sub band Index
         m_unFirstSBRSubbandIdxIO.setFieldSizeInBits(getCeilLog2(m_unNumOfPredSubbands+1));
         m_unFirstSBRSubbandIdxIO = m_unFirstSBRSubbandIdx;
         nBitsWritten += m_unFirstSBRSubbandIdxIO.write();

         // write maximum number of predicted directions 
         m_uint3Bit = getCeilLog2(m_unMaxNumOfPredDirs);
         nBitsWritten += m_uint3Bit.write();

         // write maximum number of predicted directions per subband
         if( (m_unMaxNumOfPredDirsPerBand-1) < ((1<<3)-1) )
         {
            m_uint3Bit = m_unMaxNumOfPredDirsPerBand-1;
            nBitsWritten += m_uint3Bit.write();
         }

         // escaped value with 2 extra bits
         if(   ( (m_unMaxNumOfPredDirsPerBand-1) >= ((1<<3)-1)) 
            & ( (m_unMaxNumOfPredDirsPerBand-1) < ( (1<<3)-1 + (1<<2)-1) ) ) 
         {
            m_uint3Bit = (1<<3) - 1;
            nBitsWritten += m_uint3Bit.write();      

            m_uint2Bit = m_unMaxNumOfPredDirsPerBand-1 - (1<<3) + 1;
            nBitsWritten += m_uint2Bit.write();      
         } 

         // escaped value with 2 extra bits and 5 extra bits 
         if((m_unMaxNumOfPredDirsPerBand-1) >= ((1<<3)-1 + (1<<2) - 1))
         {
            m_uint3Bit = (1<<3) - 1;
            nBitsWritten += m_uint3Bit.write();      

            m_uint2Bit = (1<<2) - 1;
            nBitsWritten += m_uint2Bit.write();  

            m_uint5Bit = m_unMaxNumOfPredDirsPerBand-1 - (1<<3)+1 - (1<<2)+1;
            nBitsWritten += m_uint5Bit.write();        
         }

         // write table index for direction grid
         m_uint2Bit = m_unDirGridTableIdx;
         nBitsWritten += m_uint2Bit.write(); 

      }

      // write PAR sub band table index
      m_uint2Bit = m_unParSubBandTableIdx;
      nBitsWritten += m_uint2Bit.write();

      if(m_unParSubBandTableIdx>0)
      {
         if(m_uint2Bit == m_vvunParSubbandTable.size())
         {
            // write custom subband configuration using diff. coded bandwidths
            nBitsWritten += encodeSubbandConfig(m_vunParSubbandWidths);
         }
         else
         {
            m_vunParSubbandWidths= m_vvunParSubbandTable[m_unParSubBandTableIdx];
         }
         // write last first order sub band index
         m_unLastFirstOrderSubBandIdxIO.setFieldSizeInBits(getCeilLog2(m_vunParSubbandWidths.size()+1));
         m_unLastFirstOrderSubBandIdxIO = m_unLastFirstOrderSubBandIdx;
         nBitsWritten += m_unLastFirstOrderSubBandIdxIO.write();

         // write use real coefficients per par sub band
         if(m_vbUseRealCoeffsPerParSubband.size() != m_vunParSubbandWidths.size())
            return 0;
         for(unsigned int n = 0; n < m_vbUseRealCoeffsPerParSubband.size(); ++n)
         {
            m_bool1Bit = m_vbUseRealCoeffsPerParSubband[n];
            nBitsWritten += m_bool1Bit.write();
         }
      }
   }
   return nBitsWritten;
};

HoaConfig& HoaConfigW::getFields()
{
   return *this;
};

int HoaConfigW::encodeSubbandConfig(const std::vector<unsigned int> p_vvunSubbandWidths)
   // encoding of subband configuration
{
   unsigned int nNumSubbands = p_vvunSubbandWidths.size();
   unsigned int nTotalBwSum = 0;
   unsigned int nTotalBits = 0;

   m_uint5Bit = nNumSubbands - 1; // coded number of subbands
   nTotalBits += m_uint5Bit.write();

   if (nNumSubbands > 1)
   {
      // first band
      unsigned int nb = 0; 
      if (p_vvunSubbandWidths[nb] < 1)
         return -1; // first SB must have bandwidth >= 1
      nTotalBits += writeUnaryCode(p_vvunSubbandWidths[nb] - 1); // p_vvunSubbandWidths[nb] - 1 coded with unary code
      nTotalBwSum += p_vvunSubbandWidths[nb];

      if (nNumSubbands > 2)
      {
         for (nb = 1; nb < nNumSubbands-2; nb++)
         {
            if (p_vvunSubbandWidths[nb] < p_vvunSubbandWidths[nb-1])
            {
               std::cout << "subband config: bandwidth difference must be >= 0.\n";
               return -1; 
            }
            unsigned int nBwDiff = p_vvunSubbandWidths[nb] - p_vvunSubbandWidths[nb-1];
            nTotalBits += writeUnaryCode(nBwDiff); // nBwDiff coded with unary code
            nTotalBwSum += p_vvunSubbandWidths[nb];
         }
         // last transmitted subband
         if (p_vvunSubbandWidths[nb] < p_vvunSubbandWidths[nb-1])
         {
            std::cout << "subband config: bandwidth difference must be >= 0.\n";
            return -1; 
         }
         unsigned int nBwDiff = p_vvunSubbandWidths[nb] - p_vvunSubbandWidths[nb-1];
         m_uint5Bit = nBwDiff;
         nTotalBits += m_uint5Bit.write(); // nBwDiff coded with 3 bits
         if (nBwDiff > 31) // max. for 5 bit
         {
            std::cout << "subband config: last subband diff cannot be coded.\n";
            return -1; 
         }
         nTotalBwSum += p_vvunSubbandWidths[nb];
         nTotalBwSum += p_vvunSubbandWidths[nb+1];
      }
   } 

   if (nTotalBwSum != 64) // total bandwidth sum must be 64 for QMF
   {
      std::cout << "subband config: wrong sum of bandwidths.\n";
      return -1;
   }

   return nTotalBits;
}

int HoaConfigW::writeUnaryCode(unsigned int nVal)
{
   int nBitsWritten = 0;
   for(unsigned int nn = 0; nn < nVal; nn++)
   {
      m_bool1Bit = 1;
      nBitsWritten += m_bool1Bit.write();
   }
   m_bool1Bit = 0; // stop bit
   nBitsWritten += m_bool1Bit.write();

   return nBitsWritten;
}