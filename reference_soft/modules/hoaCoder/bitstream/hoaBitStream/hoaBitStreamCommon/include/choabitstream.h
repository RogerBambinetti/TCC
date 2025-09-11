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
$Id: choabitstream.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#ifndef __HOABITSTREAM__
#define __HOABITSTREAM__

#include <cmath>
#include <memory>
#include <map>
#include "basicbitstreamfields.h"
#include "stdHoaFrame.h"
#define USAC_CORE
#include "TabulatedValuesHoaBitstream.h"
#include "integerFunctions.h"


/**
* This file include the definition of the HOA compression bit stream
* The bit stream consists of three classes
*
*  - HoaConfig: Including information that are required for the initialization of the HOA decoder
*  - HoaFrame: Data required for decoding a HOA frame (spatial side info and AAC frames)
*
*  The bit stream classes comprise the variables that are read from or written to the bit stream. 
*  The actual file IO is not included here so that these classes can be used for reading and writing.
*  The required bit stream fields (from bitStreamIOLib) are defined in the IO classes (i.e. CHoaBsFileHeaderIO). 
*  The IO classes set the field properties for the required field types so that they are identical for reading and writing. 
*
* An actual bit stream reader or writer should derive from the IO class using the respective read or write bit stream field types as template parameters.
* The additional read or write function uses the bit stream field members to write or read from the bit stream. 
*
* For writing, the variable from the bit stream class is copied into the respective bit stream field and written 
* with the bit stream field write function. 
* For reading, the bit stream field read function is used to read the bit stream field from the stream and the 
* value of the bit stream field can then be copied to the respective variable of the bit stream class.
*
*/


/******************************************************** 
definition of the HOA Access Frame class              */
class HoaConfig
{
public:
   /** sets the default parameter of the fields */
   HoaConfig(const unsigned int unTotalNumCoders, const unsigned int unCoreCoderFrameLength) 
      :  m_unTotalNumCoders(unTotalNumCoders), m_unCoreCoderFrameLength(unCoreCoderFrameLength)
   {      

      // 3 Bit
      m_unHoaOrder = 0;
	  
	  // 1 Bit
      m_bIsScreenRelative = false;

      // 1 Bit
      m_bUsesNfc = 0;

      // 32 bit float 
      m_ufAmbNfcReferenceDistance = 0.0;
      // size is counted later

      // 3Bit
      m_nMinAmbHoaOrder = 0;

      // is computed from other BS values
      m_unTotalNumCoders = 0;


      // 4Bit writes m_unTotalNumCoders - (m_nMinAmbHoaOrder+1)^2 -1;
      m_unNumOfAdditionalCoders = 0;

      // 2Bit writes value-1
      m_unMaxNoOfDirSigsForPrediction = 0;

      // 4Bit writes value-1
      m_unNoOfBitsPerScaleFactor = 0;

      // 3Bit 
      m_unSpatialInterpolationTime = 256;

      // 1Bit
      m_unSpatInterpMethod = 0;

      // 2Bit
      m_unCodedVVecLength = 1;

      //
      m_unMaxVVecDirections = 8;

      // 3Bit
      m_unMaxGainCorrAmpExp = 4;

      // 1Bit
      m_bSingleLayer = 1;
      // 2Bit
      m_unFrameLengthIndicator = 0;
      m_unHoaFrameLength = 1024;

      // 3Bit
      m_unMaxNumOfPredDirs = 0; 

      // escaped value (3,2,5)
      m_unMaxNumOfPredDirsPerBand = 0;

      // escaped value (2,5,0)
      m_nMaxHoaOrderToBeTransmitted = 0;

      // 2Bit
      m_unDirGridTableIdx = 0;

      // 4 Bit
      m_unFirstSBRSubbandIdx = 0;        

      // 2 bit 
      m_unSubbandConfigIdx = 1;

      // 2Bit 
      m_unParSubBandTableIdx = 1;

      // create pred. sub band table
      TabulatedValuesHoaBitstream Tabval;
      Tabval.getPredSubbandTable(m_vvunPredSubbandTable);

      m_vunPredSubbandWidths = m_vvunPredSubbandTable[m_unSubbandConfigIdx];
      m_unNumOfPredSubbands = m_vunPredSubbandWidths.size();

      // create PAR sub band table
      Tabval.getParSubbandTable(m_vvunParSubbandTable);
      m_vunParSubbandWidths = m_vvunParSubbandTable[m_unParSubBandTableIdx];

      m_unLastFirstOrderSubBandIdx = 0;

      m_vunParUpmixHoaOrderPerParSubBandIdx.assign(m_vunParSubbandWidths.size(), 2);

      m_vbUseRealCoeffsPerParSubband.assign(m_vunParSubbandWidths.size(), false);


      // 1Bit // AK: indicate the use of decorrelation method proposed by QUALCOMM
      m_bUsePhaseShiftDecorr = false;

   };

   void setParSubbandTable()
   {
      m_vunParSubbandWidths = m_vvunParSubbandTable[m_unParSubBandTableIdx];
   };

   void setPredSubbandTable()
   {
      m_vunPredSubbandWidths = m_vvunPredSubbandTable[m_unSubbandConfigIdx];
   };

   virtual ~HoaConfig(){};

public:
   unsigned int m_unHoaOrder; // 3Bit writes value-1
   bool m_bUsesNfc; // 1 Bit 
   float m_ufAmbNfcReferenceDistance; // 32 bit float
   int m_nMinAmbHoaOrder; // 3Bit 
   unsigned int m_unTotalNumCoders; // known form USAC Signals3d() numHOATransportChannels
   unsigned int m_unCoreCoderFrameLength; // known form Core Coder
   unsigned int m_unHoaFrameLength;
   bool m_bSingleLayer; // 1Bit
   unsigned int m_unFrameLengthIndicator;  //2Bit
   unsigned int m_unNumOfAdditionalCoders; // m_unTotalNumCoders - (m_nMinAmbHoaOrder+1)^2 -1;
   unsigned int m_unMaxNoOfDirSigsForPrediction; // 2Bit writes value-1
   unsigned int m_unNoOfBitsPerScaleFactor; // 4Bit writes value-1
   unsigned int m_unSpatialInterpolationTime; // 3Bit
   unsigned int m_unSpatInterpMethod; // 1Bit
   unsigned int m_unCodedVVecLength; // 2Bit
   unsigned int  m_unMaxVVecDirections; // 3 bit
   int m_unMaxGainCorrAmpExp; // 3Bit
   bool m_bIsScreenRelative; //1 bit
   unsigned int m_unMaxNumOfPredDirs; // 3Bit
   unsigned int m_unMaxNumOfPredDirsPerBand;  // escaped value (3,2,5)
   int m_nMaxHoaOrderToBeTransmitted; // escaped value (2,5,0)
   unsigned int m_unDirGridTableIdx; // 2Bit
   unsigned int m_unFirstSBRSubbandIdx; // 4Bit
   unsigned int m_unSubbandConfigIdx; // 2Bit
   unsigned int m_unNumOfPredSubbands;
   std::vector<unsigned int> m_vunPredSubbandWidths; // filled from table index

   bool m_bUsePhaseShiftDecorr; // 1 bit
   unsigned int m_unNumVVecVqElementsBits;

   // PAR config data
   unsigned int m_unParSubBandTableIdx; // 2Bit
   std::vector<unsigned int> m_vunParSubbandWidths; // filled from table index
   unsigned int m_unLastFirstOrderSubBandIdx; // written with -1 by ceil(log2(NoOfSubbandGroups))
   std::vector<unsigned int> m_vunParUpmixHoaOrderPerParSubBandIdx; // filled from m_unLastFirstOrderSubBandIdx
   std::vector<bool> m_vbUseRealCoeffsPerParSubband; // 1 bit per element size of NoOfSubbandGroups

protected:
   std::vector<std::vector<unsigned int>> m_vvunPredSubbandTable;
   std::vector<std::vector<unsigned int>> m_vvunParSubbandTable;

};

/******************************************************** 
definition of the HOA Access Frame IO class           */
template <class UCHAR_FT,
class UINT_FT, 
class FLOAT_FT>
class HoaConfigIO : public HoaConfig
{
public:
   HoaConfigIO(const unsigned int unTotalNumCoders, const unsigned int unCoreCoderFrameLength) : HoaConfig(unTotalNumCoders, unCoreCoderFrameLength)
   {
      m_unFrameSize = 0;
      /** init bit stream IO */
      m_ucharByte.setByteAlignFlag(true);
      m_bool1Bit.setFieldSizeInBits(1);
      m_uint4Bit.setFieldSizeInBits(4);
      m_uint3Bit.setFieldSizeInBits(3);
      m_uint2Bit.setFieldSizeInBits(2);
      m_uint5Bit.setFieldSizeInBits(5);
      m_float4Byte.setByteAlignFlag(false);

      // 3 Bit m_unHoaOrder
      m_unFrameSize += m_uint3Bit.getSizeInBit();

      // 1 Bit m_bIsScreenRelative
      m_unFrameSize += m_bool1Bit.getSizeInBit();

      // 1 Bit m_bUsesNfc
      m_unFrameSize += m_bool1Bit.getSizeInBit();

      // 3Bit m_unMinAmbHoaOrder
      m_unFrameSize += m_uint3Bit.getSizeInBit();

      // 1bit m_bSingleLayer
      m_unFrameSize += m_bool1Bit.getSizeInBit();	

      // 2bit m_unFrameLengthIndicator
      m_unFrameSize += m_uint2Bit.getSizeInBit();	

      m_mHoaFrameLengthTable.clear();
      m_mHoaFrameLengthTable[768] = std::vector<unsigned int>(3);	m_mHoaFrameLengthTable[1024] = std::vector<unsigned int>(3);
      m_mHoaFrameLengthTable[2048] = std::vector<unsigned int>(3); m_mHoaFrameLengthTable[4096] = std::vector<unsigned int>(3);
      m_mHoaFrameLengthTable[768][0] = 768; m_mHoaFrameLengthTable[768][1] = 768; m_mHoaFrameLengthTable[768][2] = 768;
      m_mHoaFrameLengthTable[1024][0] = 1024; m_mHoaFrameLengthTable[1024][1] = 1024; m_mHoaFrameLengthTable[1024][2] = 1024; 
      m_mHoaFrameLengthTable[2048][0] = 2048; m_mHoaFrameLengthTable[2048][1] = 1024; m_mHoaFrameLengthTable[2048][2] = 1024; 
      m_mHoaFrameLengthTable[4096][0] = 4096; m_mHoaFrameLengthTable[4096][1] = 2048; m_mHoaFrameLengthTable[4096][2] = 1024;	 

      // 2Bit writes m_unMaxNoOfDirSigsForPrediction
      m_unFrameSize += m_uint2Bit.getSizeInBit();

      // 4Bit m_unNoOfBitsPerScaleFactor 
      m_unFrameSize += m_uint4Bit.getSizeInBit();

      // 3Bit spatial Interpolation Time
      m_unFrameSize += m_uint3Bit.getSizeInBit();

      m_mSpatInterpTimeCodeTable.clear();
      m_mSpatInterpTimeCodeTable[768] = std::vector<unsigned int>(8);	m_mSpatInterpTimeCodeTable[1024] = std::vector<unsigned int>(8);
      m_mSpatInterpTimeCodeTable[2048] = std::vector<unsigned int>(8); m_mSpatInterpTimeCodeTable[4096] = std::vector<unsigned int>(8);
      m_mSpatInterpTimeCodeTable[768][0] =  0; m_mSpatInterpTimeCodeTable[768][1] =  32; m_mSpatInterpTimeCodeTable[768][2] =  64; m_mSpatInterpTimeCodeTable[768][3] = 128; m_mSpatInterpTimeCodeTable[768][4] =  256; m_mSpatInterpTimeCodeTable[768][5] = 384; m_mSpatInterpTimeCodeTable[768][6] = 512; m_mSpatInterpTimeCodeTable[768][7] = 768;
      m_mSpatInterpTimeCodeTable[1024][0] = 0; m_mSpatInterpTimeCodeTable[1024][1] = 64; m_mSpatInterpTimeCodeTable[1024][2] =128; m_mSpatInterpTimeCodeTable[1024][3] =256; m_mSpatInterpTimeCodeTable[1024][4] = 384; m_mSpatInterpTimeCodeTable[1024][5] = 512; m_mSpatInterpTimeCodeTable[1024][6] = 768; m_mSpatInterpTimeCodeTable[1024][7] = 1024;
      m_mSpatInterpTimeCodeTable[2048][0] = 0; m_mSpatInterpTimeCodeTable[2048][1] =128; m_mSpatInterpTimeCodeTable[2048][2] =256; m_mSpatInterpTimeCodeTable[2048][3] =512; m_mSpatInterpTimeCodeTable[2048][4] = 768; m_mSpatInterpTimeCodeTable[2048][5] = 1024; m_mSpatInterpTimeCodeTable[2048][6] = 1536; m_mSpatInterpTimeCodeTable[2048][7] = 2048;
      m_mSpatInterpTimeCodeTable[4096][0] = 0; m_mSpatInterpTimeCodeTable[4096][1] =256; m_mSpatInterpTimeCodeTable[4096][2] =512; m_mSpatInterpTimeCodeTable[4096][3] =768; m_mSpatInterpTimeCodeTable[4096][4] =1024; m_mSpatInterpTimeCodeTable[4096][5] = 1536; m_mSpatInterpTimeCodeTable[4096][6] = 2048; m_mSpatInterpTimeCodeTable[4096][7] = 4096;

      // 1Bit spatial Interpolation Method
      m_unFrameSize += m_bool1Bit.getSizeInBit();

      // 2Bit codedVVecLength 
      m_unFrameSize += m_uint2Bit.getSizeInBit();

      // 3Bit m_nMaxGainCorrAmpExp
      m_unFrameSize += m_uint3Bit.getSizeInBit();

      // 2Bit m_unNumOfPredSubbands
      m_unFrameSize += m_uint2Bit.getSizeInBit();     

      //  m_unParSubBandTableIdx 2 Bit
      m_unFrameSize += m_uint2Bit.getSizeInBit();  
   };

   virtual ~HoaConfigIO(){};

   /** returns the size of the access frame in bit */
   unsigned int getFrameSize()
   {
      unsigned int nVariableLength = 0;

      if(m_unHoaOrder>=7)
         nVariableLength += m_uint5Bit.getSizeInBit();

      if(m_bUsesNfc)
         nVariableLength += m_float4Byte.getSizeInBit();

      if(m_nMinAmbHoaOrder>=6)
         nVariableLength += m_uint5Bit.getSizeInBit();

	  if( (unsigned int)(m_nMinAmbHoaOrder+1) < (m_unHoaOrder+1) ) //bugfix NP 2015-11-04
	  {
		  m_unDiffOrder.setFieldSizeInBits(getCeilLog2(m_unHoaOrder - m_nMinAmbHoaOrder + 1));
		  nVariableLength += m_unDiffOrder.getSizeInBit();
	  }
      // VqConfBits for signaling NumVVecVqElementsBits
      m_unVqConfBits.setFieldSizeInBits(getCeilLog2( getCeilLog2( (m_unHoaOrder+1)*(m_unHoaOrder+1) +1)  ) );//bugfix NP
      nVariableLength += m_unVqConfBits.getSizeInBit();

      // 1Bit m_bUsePhaseShiftDecorr
      if ( m_nMinAmbHoaOrder == 1)      
         nVariableLength += m_bool1Bit.getSizeInBit();       

      if(m_unNumOfPredSubbands>0)
      {
         // m_unFirstSBRSubbandIdx 4 Bit
         m_unFirstSBRSubbandIdxIO.setFieldSizeInBits(getCeilLog2(m_unNumOfPredSubbands+1));
         nVariableLength += m_unFirstSBRSubbandIdxIO.getSizeInBit();

         // m_unMaxNumOfPredDirs 3Bit
         nVariableLength += m_uint3Bit.getSizeInBit();

         // m_unMaxNumOfPredDirsPerBand escaped value (3,2,5)
         nVariableLength += m_uint3Bit.getSizeInBit();    
         if( (m_unMaxNumOfPredDirsPerBand-1) >= ( (1<<3)-1) )
            nVariableLength += m_uint2Bit.getSizeInBit();
         if( (m_unMaxNumOfPredDirsPerBand-1) >= ((1<<3)-1 + (1<<2)-1) )
            nVariableLength += m_uint5Bit.getSizeInBit();

         // m_unDirGridTableIdx 2Bit
         m_unFrameSize += m_uint2Bit.getSizeInBit();
      }


      if(m_unParSubBandTableIdx>0)
      {
         if(m_unParSubBandTableIdx== m_vvunParSubbandTable.size() )
         {
            nVariableLength += m_uint5Bit.getSizeInBit();
            nVariableLength += getSizeOfSubbandConfig(m_vunParSubbandWidths);
         }
         else
         {
            m_vunParSubbandWidths = m_vvunParSubbandTable[m_unParSubBandTableIdx];
         }
         m_unLastFirstOrderSubBandIdxIO.setFieldSizeInBits(getCeilLog2(m_vunParSubbandWidths.size()+1));
         nVariableLength += m_unLastFirstOrderSubBandIdxIO.getSizeInBit();
         nVariableLength += m_vbUseRealCoeffsPerParSubband.size() * m_bool1Bit.getSizeInBit();
      }

      return m_unFrameSize + nVariableLength;
   };

protected:
   UCHAR_FT m_ucharByte;
   UCHAR_FT m_bool1Bit;
   UINT_FT m_uint4Bit;
   UINT_FT m_uint3Bit;
   UINT_FT m_uint2Bit;
   UINT_FT m_uint5Bit;
   FLOAT_FT m_float4Byte;
   UINT_FT m_unVqConfBits;
   UINT_FT m_unLastFirstOrderSubBandIdxIO;
   UINT_FT m_unFirstSBRSubbandIdxIO;
   UINT_FT m_unDiffOrder; //NP
   unsigned int m_unFrameSize;
   //std::map<unsigned int, unsigned int> m_mSpatInterpTimeCodeTable;
   std::map<unsigned int, std::vector<unsigned int>> m_mSpatInterpTimeCodeTable;	
   std::map<unsigned int, std::vector<unsigned int>> m_mHoaFrameLengthTable;

private:
   int getSizeOfSubbandConfig(const std::vector<unsigned int> p_vvunSubbandWidths)
      // count required bits for encoding of subband configuration
   {
      unsigned int nNumSubbands = p_vvunSubbandWidths.size();
      unsigned int nTotalBwSum = 0;
      unsigned int nTotalBits = 0;
      if (nNumSubbands > 1)
      {
         // first band
         unsigned int nb = 0; 
         if (p_vvunSubbandWidths[nb] < 1)
            return -1; // first SB must have bandwidth >= 1
         nTotalBits += p_vvunSubbandWidths[nb]; // p_vvunSubbandWidths[nb] - 1 coded with unary code
         nTotalBwSum += p_vvunSubbandWidths[nb];

         if (nNumSubbands > 2)
         {
            for (nb = 1; nb < nNumSubbands-2; nb++)
            {
               unsigned int nBwDiff = p_vvunSubbandWidths[nb] - p_vvunSubbandWidths[nb-1];
               nTotalBits += nBwDiff + 1; // nBwDiff coded with unary code
               nTotalBwSum += p_vvunSubbandWidths[nb];
            }
            // last transmitted subband
            unsigned int nBwDiff = p_vvunSubbandWidths[nb] - p_vvunSubbandWidths[nb-1];
            nTotalBits += 5; // nBwDiff coded with 5 bits
            if (nBwDiff > 31)
               return -1; // cannot be coded with 5 bits
            nTotalBwSum += p_vvunSubbandWidths[nb];
            nTotalBwSum += p_vvunSubbandWidths[nb+1];
         }
      } 

      if (nTotalBwSum != 64) // total bandwidth sum must be 64 for QMF
         return -1;

      return nTotalBits;
   }
};


/**
* @brief HOA std. frame types
* 
*/
typedef enum 
{
   INVALID_HOA_FRAME = -1,
   STD_HOA_FRAME = 0,
}FRAME_TYPE;

#endif // __HOABITSTREAM__
