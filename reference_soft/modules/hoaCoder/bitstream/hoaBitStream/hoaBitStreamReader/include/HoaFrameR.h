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
 $Id: HoaFrameR.h 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#ifndef __HoaFrameR__
#define __HoaFrameR__

#include "choabitstream.h"
#include "basicbitstreamfieldsread.h"
#include "vectorBasedPreDomSoundHuffman_decode.h"
#include <set>

/**
  * @brief Implementation of the HOA Frame reader 
  * 
  * Implements the HoaFrameIO for reading data from the bit stream.
  *
  */
class HoaFrameR : public IBitStreamFieldRead,
                        public HoaFrameIO<UCHAR_FR,
                                              INT_FR,
                                              UINT_FR> 
{
  public:
     /**
       * @brief constructor
       * 
       * Initializes the HOA frame fields with default values
       *
       */ 
    HoaFrameR();

     /**
       * @brief reads the HOA standard frame
       * 
       * reads the HOA frame 
       * the bit stream reader has to be set by 
       * IBitStreamFieldRead::setBitStreamReader() for file or stream access
       * Reads AAC data if m_bReadAac==true, otherwise only the spat. info is read
       *
       * @retval number of bytes read (<=0 on errors)
       */ 
    int read();

     /**
       * @brief get access frame fields 
       * 
       *
       * @retval reference to the HOA access frame field class (valid until read() is called again)
       *
       */ 
    const HoaFrame& getFields();

     /**
       * @brief set the global Independency Flag to overwrite the
       *        internal HOA independency flag
       * 
       *  @param[in] bFlag the state of the global flag
       *
       *
       */ 
    void setGlobalIndyFlag(bool bFlag);


  private:
     /**
       * @brief counts number of active bits in a bit array
       * 
       * counts all true elements of the input vector
       *
       * @param [in] bits the bit (bool) array to be counted
       *
       * @retval number of true elements in bits
       *
       */ 
    unsigned int cntActiveBits(std::vector<bool> &bits); 

     /**
       * @brief function that reads the gain correction field
       * 
       * reads the run length coded gain correction exponent
       * the run length coded values are stored in m_bCodedGainCorrectionExp
       * No decoding of the run length code is performed
       *
       * @param [in] unChannelNumber channel number for the gain correction value 
       *
       * @retval total number of bits read
       *
       */ 
    int readGainCorrectionExponent(const unsigned int unChannelNumber);

  private:
    /**
      * @brief function for reading unary code
      * 
      *
      * @param [out] pVal value to be read
      * @param [in] pBool1Bit 1 bit reader
      *
      * @retval total number of bits read
      *
      */ 
    int readUnaryCode(unsigned int *pVal, UCHAR_FR *pBool1Bit);

    /**
      * @brief helper function for reading angle and magnitude differences
      *        used in the dir. predication and the 
      * 
      *
      * @param [in] bUseRealCoeffsPerSubband true for reading only magnitude differences            
      * @param [out] nMagDiff output value for read magnitude difference
      * @param [in] bUseHuffmanCodingDiffMag indicates the usage of Huffman 
      *             decoding for reading the magnitude difference
      * @param [in] pDecodedMagDiffTable pointer to the mag. diff. Huffman table for non SBR sub-bands
      * @param [in] unDecodedMagDiffTableSize number of codewords in pDecodedMagDiffTable
      * @param [in] pDirectMagDiffIO integer field for direct writing of the magnitude difference
      * @param [in] pDecodedMagSbrDiffTable pointer to the mag. diff. Huffman table for SBR sub-bands
      * @param [in] unDecodedMagDiffSbrTableSize number of codewords in pDecodedMagSbrDiffTable
      * @param [in] pDirectMagDiffSbrIO  integer filed for direct writing of the magnitude difference in SBR sub-bands
      * @param [out] nAngleDiff read angle difference 
      * @param [in] bUseHuffmanCodingDiffAngle indicates the usage of Huffman 
      *             decoding for reading the angle difference 
      * @param [in] pDecodedAngleDiffTable pointer to the angle diff. Huffman table
      * @param [in] pDirectAngleDiffIO integer field for direct writing of the angle difference
      * @param [in] pBool1Bit integer field of size one bit for writing the Huffman and unary code words
      *
      * @retval total number of bits read
      *
      */
    unsigned int readDiffValues(bool bUseRealCoeffsPerSubband,            
        int &nMagDiff,
        bool bUseHuffmanCodingDiffMag,
        const HuffmanWord<int> *pDecodedMagDiffTable,
        unsigned int unDecodedMagDiffTableSize,
        UINT_FR *pDirectMagDiffIO,
        const HuffmanWord<int> *pDecodedMagSbrDiffTable,
        unsigned int unDecodedMagDiffSbrTableSize,
        UINT_FR *pDirectMagDiffSbrIO,
        int &nAngleDiff,
        bool bUseHuffmanCodingDiffAngle,
        const HuffmanWord<int> *pDecodedAngleDiffTable,
        UINT_FR *pDirectAngleDiffIO,
        UCHAR_FR *pBool1Bit);

  private:
    std::map <IDX1_CB1, Codebook> m_huffmannTable;

};

#endif // __HoaFrameR__