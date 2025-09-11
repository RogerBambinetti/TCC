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
$Id: HoaFrameR.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#include "HoaFrameR.h"

template <class T,  class T2>
static unsigned int readHuffmanCode(T2 &Value, const HuffmanWord<T2> *table, T boolIO)
{
   unsigned int codeWordLength = 0;
   unsigned int codedWord = 0;
   int idx = 0;
   while( (codeWordLength < 32))
   {
      codeWordLength += boolIO.read();
      codedWord += (boolIO*1)<<(codeWordLength-1);
      idx = getIdxFromCodeWord(codedWord, table, codeWordLength);
      if(idx >= 0)
      {
         Value = table[idx].m_codedValue;
         return codeWordLength;
      }
   }
   return codeWordLength;
}

HoaFrameR::HoaFrameR() 
{
   getHuffmanTables(m_huffmannTable);
}

void HoaFrameR::setGlobalIndyFlag(bool bFlag)
{
   m_bGlobalIndependencyFlag = bFlag;
}

int HoaFrameR::read()
{
   int nBitsRead = 0;
   if(m_bitReader)
   {
      // set bit stream reader
      m_spatPredictionDataIO.m_bool1Bit.setBitStreamReader(m_bitReader);
      m_spatPredictionDataIO.m_nPredGainsIO.setBitStreamReader(m_bitReader);
      m_spatPredictionDataIO.m_unPredIdsIO.setBitStreamReader(m_bitReader);
      m_spatPredictionDataIO.m_unActivePredIdsIO.setBitStreamReader(m_bitReader);
      m_spatPredictionDataIO.m_unNoActivePredIdsIO.setBitStreamReader(m_bitReader);
      m_addAmbHoaInfoChannelIO.m_bool1Bit.setBitStreamReader(m_bitReader);
      m_addAmbHoaInfoChannelIO.m_uint2Bit.setBitStreamReader(m_bitReader);
      m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO.setBitStreamReader(m_bitReader);
      m_directionalInfoChannelIO.m_dirIdxIO.setBitStreamReader(m_bitReader);
      m_directionalInfoChannelIO.m_uint2Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_bool1Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uint2Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uint3Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uint4Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uint8Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uint10Bit.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uintAddValue.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uintNumberOfDirections.setBitStreamReader(m_bitReader);
      m_vectorBasedInfoChannelIO.m_uintDirIdx.setBitStreamReader(m_bitReader);    
      m_emptyInfoChannelIO.m_uint2Bit.setBitStreamReader(m_bitReader);
      m_gainCorrectionIO.m_bool1Bit.setBitStreamReader(m_bitReader);
      m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO.setBitStreamReader(m_bitReader);
      m_bool1Bit.setBitStreamReader(m_bitReader);
      m_hoaDirectionalPredictionInfoIO.m_bool1Bit.setBitStreamReader(m_bitReader);
      m_hoaDirectionalPredictionInfoIO.m_globalPredDirsIdsIO.setBitStreamReader(m_bitReader);
      m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO.setBitStreamReader(m_bitReader);
      m_hoaDirectionalPredictionInfoIO.m_numOfGlobalPredDirsIO.setBitStreamReader(m_bitReader);
      m_hoaDirectionalPredictionInfoIO.m_uint4Bit.setBitStreamReader(m_bitReader);
      m_hoaParInfoIO.m_bool1Bit.setBitStreamReader(m_bitReader);
      m_hoaParInfoIO.m_uint2Bit.setBitStreamReader(m_bitReader);
      m_hoaParInfoIO.m_uint4Bit.setBitStreamReader(m_bitReader);

      // reset add. HOA ambient array and count
      m_vunAddHoaCoeff.reserve(m_unNumOfAdditionalCoders);
      m_vunAddHoaCoeff.resize(0);

      // read independency flag
      nBitsRead += m_bool1Bit.read();
      m_bHoaIndependencyFlag = m_bool1Bit==1;
      if(m_bGlobalIndependencyFlag && !m_bHoaIndependencyFlag)
      {  
         return 0;
      }

      // read channel information data
      std::set<unsigned int> suiNonTransitionalAddHoaChannels;
      std::set<unsigned int> suiFadeInAddHoaChannels;
      std::set<unsigned int> suiNewVecChannels;
      unsigned int unChannelNumber = 0;	
      m_spatPredictionDataIO.m_vunDirSigChannelIds.resize(0);
      for(std::vector<std::shared_ptr<CChannelSideInfoData> >::iterator it = m_vChannelSideInfo.begin();
         it < m_vChannelSideInfo.end(); it++)
      {
         CHANNEL_TYPE channelType = EMPTY_CHANNEL;
         nBitsRead += m_emptyInfoChannelIO.m_uint2Bit.read();
         channelType = static_cast<CHANNEL_TYPE>(static_cast<unsigned int>(m_emptyInfoChannelIO.m_uint2Bit));

         switch(channelType)
         {
            // directional channel
         case DIR_CHANNEL:
            // check if channel type has been changed
            if((*it)->m_unChannelType != DIR_CHANNEL)
            {
               // allocate a directional channel
               *it = std::shared_ptr<CChannelSideInfoData>(new CDirectionalInfoChannel());
            }
            nBitsRead += m_directionalInfoChannelIO.m_dirIdxIO.read();
            dynamic_cast<CDirectionalInfoChannel*>(it->get())->m_unActiveDirIds =  m_directionalInfoChannelIO.m_dirIdxIO;
            // store channel index of current dir sig
            m_spatPredictionDataIO.m_vunDirSigChannelIds.push_back(unChannelNumber);
            break;
            // add HOA Ambient channel
         case ADD_HOA_CHANNEL:
            // check if channel type has been changed
            if((*it)->m_unChannelType != ADD_HOA_CHANNEL)
            {
               // allocate a directional channel
               *it = std::shared_ptr<CChannelSideInfoData>(new CAddAmbHoaInfoChannel());
            }
            if(m_bHoaIndependencyFlag)
            {
               nBitsRead += m_addAmbHoaInfoChannelIO.m_uint2Bit.read();
               dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState = 
                  m_addAmbHoaInfoChannelIO.m_uint2Bit;
               nBitsRead += m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO.read();
               dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx = m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO
                  + 1 + m_addAmbHoaInfoChannelIO.m_unMinNumOfCoeffsForAmbHOA;
            }
            else
            {
               nBitsRead += m_addAmbHoaInfoChannelIO.m_bool1Bit.read();
               dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_bAmbCoeffIdxChanged = m_addAmbHoaInfoChannelIO.m_bool1Bit > 0;
               if(m_addAmbHoaInfoChannelIO.m_bool1Bit)
               { 
                  if (1 < dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState)
                  {
                     nBitsRead += m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO.read();
                     dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx = m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO
                        + 1 + m_addAmbHoaInfoChannelIO.m_unMinNumOfCoeffsForAmbHOA;
                     dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState = 1;
                  }
                  else
                  {
                     dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState = 2;
                  }
               }
               else
               {
                  dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState = 0;	
               }
            }	

            if (1== m_vectorBasedInfoChannelIO.m_unCodedVVecLength)
            {
               switch (dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState)
               {
               case 0:
                  suiNonTransitionalAddHoaChannels.insert(dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx);
                  break;
               case 1:
                  suiFadeInAddHoaChannels.insert(dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx);
                  break;
               }
            }
            m_vunAddHoaCoeff.push_back(dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx - 1);
            break;
         case VEC_CHANNEL:
            {
               // check if channel type has been changed
               if((*it)->m_unChannelType != VEC_CHANNEL)
               {
                  // allocate a directional channel
                  *it = std::shared_ptr<CChannelSideInfoData>(new CVectorBasedInfoChannel());
               }
               CVectorBasedInfoChannel *pCurChan = dynamic_cast<CVectorBasedInfoChannel*>(it->get());              
              
               if(m_bHoaIndependencyFlag)
               {  
				   if (1 == m_vectorBasedInfoChannelIO.m_unCodedVVecLength) {
					   nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
					   pCurChan->m_bNewChannelTypeOne = m_vectorBasedInfoChannelIO.m_bool1Bit == 1;
					   if (1==pCurChan->m_bNewChannelTypeOne)
						   suiNewVecChannels.insert(unChannelNumber);					   
				   }
                  pCurChan->m_bSameHeaderPrevFrame = false;
                  nBitsRead += m_vectorBasedInfoChannelIO.m_uint4Bit.read();
                  pCurChan->m_unNbitsQ = static_cast<unsigned short>(m_vectorBasedInfoChannelIO.m_uint4Bit);
                  if(pCurChan->m_unNbitsQ == cVVecVQWord) 
                  {
                     //Indicies Codebook
                     nBitsRead += m_vectorBasedInfoChannelIO.m_uint3Bit.read();
                     pCurChan->m_unIndicesCodebookIdx = m_vectorBasedInfoChannelIO.m_uint3Bit;
                     nBitsRead += m_vectorBasedInfoChannelIO.m_uintNumberOfDirections.read();
                     pCurChan->m_unVVecDirections = m_vectorBasedInfoChannelIO.m_uintNumberOfDirections + 1;							
                  }
                  if(pCurChan->m_unNbitsQ > c8BitQuantizerWord) 
                  {
                     pCurChan->m_bPFlag = 0;
                     nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                     pCurChan->m_bCbFlag = m_vectorBasedInfoChannelIO.m_bool1Bit == 1;
                  }
               }
               else
               {
				  if ((1 == m_vectorBasedInfoChannelIO.m_unCodedVVecLength) && (m_vPrevFrameChannelSideInfo[unChannelNumber]->m_unChannelType != VEC_CHANNEL))
				  {						
					  suiNewVecChannels.insert(unChannelNumber);
				  }
                  unsigned short bA, bB; // bool
                  nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                  bA = m_vectorBasedInfoChannelIO.m_bool1Bit;
                  nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                  bB = m_vectorBasedInfoChannelIO.m_bool1Bit;
                  if (0 == bA + bB)
                  {
                     pCurChan->m_bSameHeaderPrevFrame = true;
                     // using previous frame's nBitsQ, bPFlag and bCbFlag
                  }
                  else 
                  {
                     pCurChan->m_bSameHeaderPrevFrame = false;
                     nBitsRead += m_vectorBasedInfoChannelIO.m_uint2Bit.read();
                     unsigned short tmp2bit;
                     tmp2bit = static_cast<unsigned short>(m_vectorBasedInfoChannelIO.m_uint2Bit);
                     pCurChan->m_unNbitsQ = 8*bA + 4*bB + tmp2bit;

                     if (pCurChan->m_unNbitsQ == cVVecVQWord)
                     {
                        // Indicies Codebook
                        nBitsRead += m_vectorBasedInfoChannelIO.m_uint3Bit.read();
                        pCurChan->m_unIndicesCodebookIdx = m_vectorBasedInfoChannelIO.m_uint3Bit;
                        nBitsRead += m_vectorBasedInfoChannelIO.m_uintNumberOfDirections.read();
                        pCurChan->m_unVVecDirections = m_vectorBasedInfoChannelIO.m_uintNumberOfDirections + 1;												
                     }

                     if(pCurChan->m_unNbitsQ > c8BitQuantizerWord)
                     {
                        // read prediction flag
                        nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                        pCurChan->m_bPFlag = m_vectorBasedInfoChannelIO.m_bool1Bit == 1;
                        // write Codebook flag
                        nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                        pCurChan->m_bCbFlag = m_vectorBasedInfoChannelIO.m_bool1Bit == 1;
                     }
                  }
               }
            }
            break;
            // Empty and default channel
         default:
            // check if channel type has been changed
            if((*it)->m_unChannelType != EMPTY_CHANNEL)
            {
               // allocate a directional channel
               *it = std::shared_ptr<CChannelSideInfoData>(new CEmptyInfoChannel());
            }
         }


         // read gain correction data

         // read last gain correction amplification exponent
         if(m_bHoaIndependencyFlag)
         {
            nBitsRead += m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO.read();
            m_vGainCorrectionData[unChannelNumber].m_nGainCorrAbsAmpExp =  m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO
               - m_gainCorrectionIO.m_nMaxLog2ExpOfTransportSigs;
         }
         else
            m_vGainCorrectionData[unChannelNumber].m_nGainCorrAbsAmpExp = 0;

         // read gain correction exception fags
         nBitsRead += readGainCorrectionExponent(unChannelNumber);

         // read gc exception bit
         nBitsRead += m_gainCorrectionIO.m_bool1Bit.read();
         m_vGainCorrectionData[unChannelNumber].m_bGainCorrectionException = m_gainCorrectionIO.m_bool1Bit>0;

         // increase channel number
         unChannelNumber++;
      }    


      for(; unChannelNumber < m_vGainCorrectionData.size(); unChannelNumber++)
      {
         // read gain correction data

         // read last gain correction amplification exponent
         if(m_bHoaIndependencyFlag)
         {
            nBitsRead += m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO.read();
            m_vGainCorrectionData[unChannelNumber].m_nGainCorrAbsAmpExp =    m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO 
               - m_gainCorrectionIO.m_nMaxLog2ExpOfTransportSigs;
         }
         else
            m_vGainCorrectionData[unChannelNumber].m_nGainCorrAbsAmpExp = 0;

         // read gain correction exception fags
         nBitsRead += readGainCorrectionExponent(unChannelNumber);

         // read gc exception bit
         nBitsRead += m_gainCorrectionIO.m_bool1Bit.read();
         m_vGainCorrectionData[unChannelNumber].m_bGainCorrectionException = m_gainCorrectionIO.m_bool1Bit>0;  
      }

      // process remaining portion of vec channel info
      unChannelNumber = 0;
      for(std::vector<std::shared_ptr<CChannelSideInfoData> >::iterator it = m_vChannelSideInfo.begin();
         it < m_vChannelSideInfo.end(); it++)
      {
         if(VEC_CHANNEL == (*it)->m_unChannelType)
         {
            CVectorBasedInfoChannel *pCurChan = dynamic_cast<CVectorBasedInfoChannel*>(it->get());		
            std::set<unsigned int>::iterator suiItNewVecChannels;
            std::set<unsigned int>::iterator suiItTransitionalAddHoaChannel;
            std::set<unsigned int>::iterator suiItFadeInAddHoaChannel;
            unsigned int unHoaCoeffIdx = m_vectorBasedInfoChannelIO.m_unHoaIdxOffset;
            suiItNewVecChannels = suiNewVecChannels.find(unChannelNumber);

            if(cVVecVQWord == pCurChan->m_unNbitsQ)
            {   

               pCurChan->m_vunDirectionIndices.resize(pCurChan->m_unVVecDirections, 0);
               pCurChan->m_vbSign.resize(pCurChan->m_unVVecDirections, 0);

               switch (pCurChan->m_unIndicesCodebookIdx)
               {
               case 0:
                  m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(10); //3D fine grid
                  break;
               case 1: 
				    m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(6); // CICP
                  break;
               case 2:
				   m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(6);
                  break;
               case 3:
				    m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(6); //2D
                  break;
               case 4: // reserved
               case 5: // reserved
               case 6: // reserved
               case 7: 	
                  m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(getCeilLog2(m_unNoOfHoaCoeffs));                 						
               }

               if (1 < pCurChan->m_unVVecDirections)
               {
                  nBitsRead += m_vectorBasedInfoChannelIO.m_uint8Bit.read();
                  pCurChan->m_unWeightingCodebookIdx = m_vectorBasedInfoChannelIO.m_uint8Bit;
               }

               for(unsigned int nEl = 0; nEl < pCurChan->m_vunDirectionIndices.size(); nEl++)
               {
                  nBitsRead += m_vectorBasedInfoChannelIO.m_uintDirIdx.read();
                  pCurChan->m_vunDirectionIndices[nEl] = m_vectorBasedInfoChannelIO.m_uintDirIdx+1;      			

                  nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                  pCurChan->m_vbSign[nEl] = (m_vectorBasedInfoChannelIO.m_bool1Bit == 1);      			
               }

               pCurChan->m_vbAdditionalInfo.assign(m_vectorBasedInfoChannelIO.m_unNumOfVecElements, 0);
               for(unsigned int nEl = 0; nEl < pCurChan->m_vbAdditionalInfo.size(); nEl++, ++unHoaCoeffIdx)
               {				
                  suiItFadeInAddHoaChannel = suiFadeInAddHoaChannels.find( unHoaCoeffIdx ); 
                  suiItTransitionalAddHoaChannel = suiNonTransitionalAddHoaChannels.find( unHoaCoeffIdx ); 
                  if ((suiItTransitionalAddHoaChannel == suiNonTransitionalAddHoaChannels.end()) && !(suiItNewVecChannels!=suiNewVecChannels.end() && suiItFadeInAddHoaChannel!=suiFadeInAddHoaChannels.end()) ) 
                  {
                     pCurChan->m_vbAdditionalInfo[nEl] = 1;
                  }
               } 
            }
            else if(c8BitQuantizerWord == pCurChan->m_unNbitsQ)
            {
               pCurChan->m_vun8bitCodedVelement.resize(m_vectorBasedInfoChannelIO.m_unNumOfVecElements, 0);
               for(unsigned int nEl = 0; nEl < pCurChan->m_vun8bitCodedVelement.size(); nEl++, ++unHoaCoeffIdx)
               {				
                  suiItFadeInAddHoaChannel = suiFadeInAddHoaChannels.find( unHoaCoeffIdx ); 
                  suiItTransitionalAddHoaChannel = suiNonTransitionalAddHoaChannels.find( unHoaCoeffIdx ); 
                  if ((suiItTransitionalAddHoaChannel == suiNonTransitionalAddHoaChannels.end()) && !(suiItNewVecChannels!=suiNewVecChannels.end() && suiItFadeInAddHoaChannel!=suiFadeInAddHoaChannels.end()) ) 
                  {
                     nBitsRead += m_vectorBasedInfoChannelIO.m_uint8Bit.read();
                     pCurChan->m_vun8bitCodedVelement[nEl] = m_vectorBasedInfoChannelIO.m_uint8Bit;
                  }
                  else
                  {
                     pCurChan->m_vun8bitCodedVelement[nEl] = 128; // 128 is the quantized value for 0.0
                  }
               }  
            }
            else if (pCurChan->m_unNbitsQ > c8BitQuantizerWord)
            {
               // read each vector element 
               pCurChan->m_vsCodedHuffmannWord.resize( m_vectorBasedInfoChannelIO.m_unNumOfVecElements, std::string(""));
               pCurChan->m_vunDecodedHuffmannWord.resize( m_vectorBasedInfoChannelIO.m_unNumOfVecElements, 0);
               pCurChan->m_vbSign.resize(m_vectorBasedInfoChannelIO.m_unNumOfVecElements, 1);
               pCurChan->m_vunAdditionalValue.resize(m_vectorBasedInfoChannelIO.m_unNumOfVecElements, 0);
               pCurChan->m_vbElementBitmask.resize(m_vectorBasedInfoChannelIO.m_unNumOfVecElements, 0); //bugfix NP 29-05-2015
               unsigned int nEl = 0;		  
               std::vector<unsigned short>::iterator itDec = pCurChan->m_vunDecodedHuffmannWord.begin();
               for(std::vector<std::string>::iterator it = pCurChan->m_vsCodedHuffmannWord.begin();
                  it < pCurChan->m_vsCodedHuffmannWord.end(); ++it, ++itDec, nEl++, ++unHoaCoeffIdx)
               {
                  // read Huffman word
                  unsigned int unHuffamCodeLength = 0;
                  bool bCodeWordFound = false;
                  IDX4_SYMBOL unSymbol  = 0;
                  it->clear();			
                  suiItFadeInAddHoaChannel = suiFadeInAddHoaChannels.find( unHoaCoeffIdx ); 
                  suiItTransitionalAddHoaChannel = suiNonTransitionalAddHoaChannels.find( unHoaCoeffIdx ); 
                  if ((suiItTransitionalAddHoaChannel == suiNonTransitionalAddHoaChannels.end()) && !(suiItNewVecChannels!=suiNewVecChannels.end() && suiItFadeInAddHoaChannel!=suiFadeInAddHoaChannels.end()) ) 
                  {   				
                     pCurChan->m_vbElementBitmask[nEl] = 1; //bugfix NP 29-05-2015
                     while(!bCodeWordFound)
                     {
                        // read new bit
                        nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                        // add to code word string
                        if(m_vectorBasedInfoChannelIO.m_bool1Bit==1)
                           it->append("1");
                        else
                           it->append("0");
                        // try to decode the code word
                        unSymbol = decodeHuffmanSymbol(m_huffmannTable, pCurChan->m_unNbitsQ, 
                           pCurChan->m_bPFlag, pCurChan->m_bCbFlag, *it, 
                           unHoaCoeffIdx); 
                        // check for maximal code word length 
                        unHuffamCodeLength++;
                        if(unHuffamCodeLength==16)
                           return -1;
                        // check if code word has been found in the table
                        bCodeWordFound = unSymbol != CODEWORD_NOT_FOUND;
                     }
                     // set decode code word
                     *itDec = unSymbol;
                     if(*itDec>0)
                     {
                        // read sign bit
                        nBitsRead += m_vectorBasedInfoChannelIO.m_bool1Bit.read();
                        pCurChan->m_vbSign[nEl] = m_vectorBasedInfoChannelIO.m_bool1Bit == 1;
                        pCurChan->m_vunAdditionalValue[nEl] = 0; 
                        if(*itDec>1)
                        {
                           // read additional value
                           m_vectorBasedInfoChannelIO.m_uintAddValue.setFieldSizeInBits(*itDec-1);
                           nBitsRead += m_vectorBasedInfoChannelIO.m_uintAddValue.read();
                           pCurChan->m_vunAdditionalValue[nEl] = m_vectorBasedInfoChannelIO.m_uintAddValue;
                        }
                     }
                  }
                  else
                  {
                     pCurChan->m_vunAdditionalValue[nEl] = 0;
                     pCurChan->m_vunDecodedHuffmannWord[nEl] = 0;				
                     pCurChan->m_vbElementBitmask[nEl] = 0; //bugfix NP 29-05-2015
                  }
               }		  
            }
         }
         unChannelNumber++;
      }

      // read perform prediction bit
      if(m_spatPredictionDataIO.m_vunDirSigChannelIds.size()>0)
      {
         nBitsRead += m_spatPredictionDataIO.m_bool1Bit.read();
         m_spatPredictionData.m_bPerformPrediction = m_spatPredictionDataIO.m_bool1Bit>0;
      }
      else
         m_spatPredictionData.m_bPerformPrediction = false;

      unsigned int unNumActivePred = 0;
      if(m_spatPredictionData.m_bPerformPrediction)
      {
         // read active prediction coding type 
         nBitsRead += m_spatPredictionDataIO.m_bool1Bit.read();
         m_spatPredictionData.m_bKindOfCodedPredIds = m_spatPredictionDataIO.m_bool1Bit>0;

         if(m_spatPredictionData.m_bKindOfCodedPredIds)
         {
            // read number of active pred indices
            nBitsRead += m_spatPredictionDataIO.m_unNoActivePredIdsIO.read();
            unNumActivePred = m_spatPredictionDataIO.m_unNoActivePredIdsIO+1;
            m_spatPredictionData.m_bActivePred.assign(m_spatPredictionData.m_bActivePred.size(), 0);
            for(unsigned int n=0; n < unNumActivePred; ++n)
            {
               nBitsRead += m_spatPredictionDataIO.m_unActivePredIdsIO.read();
               m_spatPredictionData.m_bActivePred[m_spatPredictionDataIO.m_unActivePredIdsIO] = 1;
            }
         }
         else
         {
            // read number of active predictions
            nBitsRead += readArray(m_spatPredictionData.m_bActivePred.begin(), m_spatPredictionData.m_bActivePred.end(), 
               m_spatPredictionDataIO.m_bool1Bit);
            unNumActivePred = cntActiveBits(m_spatPredictionData.m_bActivePred);
         }

         // read the prediction type of each of the active predictions
         //m_spatPredictionData.m_bPredType.resize(unNumActivePred);
         //nBitsRead += readArray(m_spatPredictionData.m_bPredType.begin(), m_spatPredictionData.m_bPredType.end(), 
         //                       m_spatPredictionDataIO.m_bool1Bit);

         // read the index of each of the active predictions
         m_spatPredictionData.m_unPredIds.resize(unNumActivePred*m_spatPredictionDataIO.m_unMaxNoOfDirSigsForPrediction);
         m_spatPredictionDataIO.m_unPredIdsIO.setFieldSizeInBits(getCeilLog2(m_spatPredictionDataIO.m_vunDirSigChannelIds.size()+1));
         nBitsRead += readArray(m_spatPredictionData.m_unPredIds.begin(), m_spatPredictionData.m_unPredIds.end(), 
            m_spatPredictionDataIO.m_unPredIdsIO);
         // map bit stream value to channel index
         for(std::vector<unsigned int>::iterator it = m_spatPredictionData.m_unPredIds.begin();
            it <  m_spatPredictionData.m_unPredIds.end(); ++it)
         {
            if(*it)
               *it = m_spatPredictionDataIO.m_vunDirSigChannelIds[*it-1]+1;
         }

         // count number of active gains
         unsigned int nActivePredGains = 0;
         for(unsigned int n=0; n < m_spatPredictionData.m_unPredIds.size(); ++n)
            nActivePredGains += m_spatPredictionData.m_unPredIds[n]>0;

         // read prediction gains for each of the active predictions
         m_spatPredictionData.m_nPredGains.resize(nActivePredGains);
         nBitsRead += readArray(m_spatPredictionData.m_nPredGains.begin(), m_spatPredictionData.m_nPredGains.end(), 
            m_spatPredictionDataIO.m_nPredGainsIO);
      }
      m_vPrevFrameChannelSideInfo = m_vChannelSideInfo;

      // read HOA Directional Prediction Info
      if(m_hoaDirectionalPredictionInfoIO.m_unNumOfPredSubbands > 0)
      {
         // Use directional prediction flag
         nBitsRead += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.read();
         m_hoaDirectionalPredictionInfo.m_bUseDirPred = m_hoaDirectionalPredictionInfoIO.m_bool1Bit==1;
         if(m_hoaDirectionalPredictionInfo.m_bUseDirPred)
         {
            // read keep global dirs flag
            if(!m_bHoaIndependencyFlag)
            {
               nBitsRead += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.read();
               m_hoaDirectionalPredictionInfo.m_bKeepPreviouseGlobalPredDirsFlag = m_hoaDirectionalPredictionInfoIO.m_bool1Bit == 1;
            }
            else
               m_hoaDirectionalPredictionInfo.m_bKeepPreviouseGlobalPredDirsFlag = false;

            if(!m_hoaDirectionalPredictionInfo.m_bKeepPreviouseGlobalPredDirsFlag)
            {
               // NumOfGlobalPredDirs
               nBitsRead += m_hoaDirectionalPredictionInfoIO.m_numOfGlobalPredDirsIO.read(); 
               m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs = m_hoaDirectionalPredictionInfoIO.m_numOfGlobalPredDirsIO + 1; 
               m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.resize(m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs);

               // set size of umBitsForRelDirGridIdxIO
               m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO.setFieldSizeInBits(
                  getCeilLog2(m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs));

               // for loop over m_unNumOfGlobalPredDirs
               nBitsRead += readArray(m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.begin(),
                  m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.end(),
                  m_hoaDirectionalPredictionInfoIO.m_globalPredDirsIdsIO);
            }

            // create sorted AddHoaCoeff index array
            std::vector<unsigned int> vunSortedAddHoaCoeff = m_vunAddHoaCoeff;
            std::sort(vunSortedAddHoaCoeff.begin(), vunSortedAddHoaCoeff.end());

            // loop over subbands and directions for reading prediction parameter for each combination
            for ( unsigned int band = 0; band < m_hoaDirectionalPredictionInfoIO.m_unNumOfPredSubbands; ++band)
            {
               // read keep prediction matrix flag
               if(!m_bHoaIndependencyFlag)
               {
                  nBitsRead += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.read();
                  m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[band] = m_hoaDirectionalPredictionInfoIO.m_bool1Bit == 1;
               }
               else
                  m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[band] = false;         

               if(!m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[band])
               {
                  // read huffman decision
                  nBitsRead += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.read(); 
                  m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffMag[band] = m_hoaDirectionalPredictionInfoIO.m_bool1Bit==1;
                  if(band < m_hoaDirectionalPredictionInfoIO.m_unFirstSBRSubbandIdx)
                  {
                     nBitsRead += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.read(); 
                     m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffAngle[band] = m_hoaDirectionalPredictionInfoIO.m_bool1Bit==1;
                  }

                  for ( unsigned int dir = 0; dir < m_hoaDirectionalPredictionInfoIO.m_unMaxNumOfPredDirsPerBand; ++dir)
                  {
                     // reset parameter buffer
                     m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[band][dir].assign(
                        m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[band][dir].size(), 0);
                     m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[band][dir].assign(
                        m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[band][dir].size(), 0);

                     // read active direction bit
                     nBitsRead += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.read();
                     m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[band][dir] = m_hoaDirectionalPredictionInfoIO.m_bool1Bit==1;
                     if(m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[band][dir])
                     {
                        // read RelDirGridIdx
                        nBitsRead += m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO.read();
                        m_hoaDirectionalPredictionInfo.m_vvunRelDirGridIdx[band][dir] = m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO;

                        // loop over const HOA channels 
                        for ( unsigned int hoaIdx = 0; hoaIdx < m_hoaDirectionalPredictionInfoIO.m_unMinNumOfCoeffsForAmbHOA ; hoaIdx++)
                        {
                           // read angle and magnitude differences
                           nBitsRead += readDiffValues(band >= m_hoaDirectionalPredictionInfoIO.m_unFirstSBRSubbandIdx,            
                              m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[band][dir][hoaIdx],
                              m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffMag[band],
                              g_pDecodedMagDiffTable,
                              sizeof(g_pDecodedMagDiffTable)/sizeof(HuffmanWord<int>),
                              &m_hoaDirectionalPredictionInfoIO.m_uint4Bit,
                              g_pDecodedMagDiffSbrTable,
                              sizeof(g_pDecodedMagDiffSbrTable)/sizeof(HuffmanWord<int>),
                              &m_hoaDirectionalPredictionInfoIO.m_uint4Bit,
                              m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[band][dir][hoaIdx],
                              m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffAngle[band],
                              g_pDecodedAngleDiffTable,
                              &m_hoaDirectionalPredictionInfoIO.m_uint4Bit,
                              &m_hoaDirectionalPredictionInfoIO.m_bool1Bit);
                        }

                        // loop over add HOA channels
                        for ( unsigned int hoaIdx = 0; hoaIdx < vunSortedAddHoaCoeff.size() ; hoaIdx++)
                        {
                           // read angle and magnitude differences
                           nBitsRead += readDiffValues(band >= m_hoaDirectionalPredictionInfoIO.m_unFirstSBRSubbandIdx,            
                              m_hoaDirectionalPredictionInfo.m_vvvfDecodedMagDiff[band][dir][vunSortedAddHoaCoeff[hoaIdx]],
                              m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffMag[band],
                              g_pDecodedMagDiffTable,
                              sizeof(g_pDecodedMagDiffTable)/sizeof(HuffmanWord<int>),
                              &m_hoaDirectionalPredictionInfoIO.m_uint4Bit,
                              g_pDecodedMagDiffSbrTable,
                              sizeof(g_pDecodedMagDiffSbrTable)/sizeof(HuffmanWord<int>),
                              &m_hoaDirectionalPredictionInfoIO.m_uint4Bit,
                              m_hoaDirectionalPredictionInfo.m_vvvfDecodedAngleDiff[band][dir][vunSortedAddHoaCoeff[hoaIdx]],
                              m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffAngle[band],
                              g_pDecodedAngleDiffTable,
                              &m_hoaDirectionalPredictionInfoIO.m_uint4Bit,
                              &m_hoaDirectionalPredictionInfoIO.m_bool1Bit);
                        }
                     }
                  }
               }
            }
         }
      }

      // Read PAR Data
      if(m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband.size() > 0)
      {
         nBitsRead +=  m_hoaParInfoIO.m_bool1Bit .read();
         m_hoaParInfo.m_bPerformPar = (m_hoaParInfoIO.m_bool1Bit==1);

         if(m_hoaParInfo.m_bPerformPar)
         {
            // update selected tables 
            for(unsigned int band=0; band < m_hoaParInfo.m_vbKeepPrevMatrixFlag.size(); band++)
            {
               // keep matrix flag
               nBitsRead +=  m_hoaParInfoIO.m_bool1Bit.read();
               m_hoaParInfo.m_vbKeepPrevMatrixFlag[band] = (m_hoaParInfoIO.m_bool1Bit==1);

               if(!m_hoaParInfo.m_vbKeepPrevMatrixFlag[band])
               {
                  // ParDecorrSigsSelectionTableIdx
                  nBitsRead += m_hoaParInfoIO.m_uint2Bit.read();
                  m_hoaParInfo.m_vunParDecorrSigsSelectionTableIdx[band] = m_hoaParInfoIO.m_uint2Bit;
                  m_hoaParInfo.updateSelectionTableInSubBand(band,
                     m_hoaParInfoIO.m_vunParUpmixHoaOrderPerParSubBandIdx[band]);

                  // reduce matrix size
                  nBitsRead +=  m_hoaParInfoIO.m_bool1Bit.read();
                  m_hoaParInfo.m_vbUseReducedNoOfUpmixSigs[band] = (m_hoaParInfoIO.m_bool1Bit==1);
                  if(m_hoaParInfo.m_vbUseReducedNoOfUpmixSigs[band])
                  {
                     if(m_hoaParInfo.m_vvbUseParUpmixSig[band].size() != m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band].size())
                        return nBitsRead;
                     nBitsRead += readArray(m_hoaParInfo.m_vvbUseParUpmixSig[band].begin(), m_hoaParInfo.m_vvbUseParUpmixSig[band].end(),
                        m_hoaParInfoIO.m_bool1Bit);
                  }   
                  else
                  {
                     m_hoaParInfo.m_vvbUseParUpmixSig[band].assign(m_hoaParInfo.m_vvbUseParUpmixSig[band].size(), true);
                  }

                  // Huffman table index
                  nBitsRead += m_hoaParInfoIO.m_bool1Bit.read();
                  m_hoaParInfo.m_vbUseParHuffmanCodingDiffAbs[band] = (m_hoaParInfoIO.m_bool1Bit==1);
                  if ( !m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband[band] )
                  {
                     nBitsRead += m_hoaParInfoIO.m_bool1Bit.read();
                     m_hoaParInfo.m_vbUseParHuffmanCodingDiffAngle[band] = (m_hoaParInfoIO.m_bool1Bit==1);
                  }
                  // Loop over each up-mix signal
                  for(unsigned int n=0; n < m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band].size(); ++n)
                  {
                     // clear matrix row
                     m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band][n].assign(
                        m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band][n].size(), 0);
                     m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[band][n].assign(
                        m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[band][n].size(), 0);
                     // Loop over all active parameters for each active up-mix signal
                     for(unsigned int m=0; (m < m_hoaParInfo.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n].size()) 
                        & m_hoaParInfo.m_vvbUseParUpmixSig[band][n]; ++m)
                     {
                        // read angle and magnitude differences
                        nBitsRead += readDiffValues(m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband[band],            
                           m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band][n][m_hoaParInfo.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n][m]],
                           m_hoaParInfo.m_vbUseParHuffmanCodingDiffAbs[band],
                           g_pDecodedParMagDiffTable,
                           sizeof(g_pDecodedParMagDiffTable)/sizeof(HuffmanWord<int>),
                           &m_hoaParInfoIO.m_uint4Bit,
                           g_pDecodedParMagDiffSbrTable,
                           sizeof(g_pDecodedParMagDiffSbrTable)/sizeof(HuffmanWord<int>),
                           &m_hoaParInfoIO.m_uint4Bit,
                           m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[band][n][m_hoaParInfo.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n][m]],
                           m_hoaParInfo.m_vbUseParHuffmanCodingDiffAngle[band],
                           g_pDecodedParAngleDiffTable,
                           &m_hoaParInfoIO.m_uint4Bit,
                           &m_hoaParInfoIO.m_bool1Bit);
                     }
                  }
               }
               else
               {
                  for(unsigned int n=0; n < m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band].size(); ++n)
                  {
                     // clear matrix row
                     m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band][n].assign(
                        m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band][n].size(), 0);
                     m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[band][n].assign(
                        m_hoaParInfo.m_vvvnParMixingMatrixDiffAngle[band][n].size(), 0);
                  }
               }
            }
         }
      }
   }
   return nBitsRead;
};

const HoaFrame& HoaFrameR::getFields()
{
   return *this;
};

unsigned int HoaFrameR::cntActiveBits(std::vector<bool> &bits)
{
   unsigned int nActiveBits = 0;
   for(unsigned int n=0; n < bits.size(); ++n)
      nActiveBits += bits[n]==true;
   return nActiveBits;
}


int HoaFrameR::readGainCorrectionExponent(const unsigned int unChannelNumber)
{
   unsigned int nNumBitsRead = 0;
   // allocate max length of vector
   m_vGainCorrectionData[unChannelNumber].m_bCodedGainCorrectionExp.resize(0 , false );
   // read until unValuesToRead times a one has been read
   unsigned int nBit = 0;
   while(!nBit)
   {
      if(!m_bitReader->readBit(nBit))
         return -1;
      m_vGainCorrectionData[unChannelNumber].m_bCodedGainCorrectionExp.push_back(nBit>0);
      nNumBitsRead++;
   }
   return nNumBitsRead;
}

unsigned int HoaFrameR::readDiffValues(bool bUseRealCoeffsPerSubband,            
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
                                       UCHAR_FR *pBool1Bit)
{
   unsigned int nNumBitsRead = 0;
   const HuffmanWord<int> *pSelectedMagDiffTable = 0;
   unsigned int unSelectedTableSize = 0;
   UINT_FR *pSelectedDirectMagDiffIO = 0;
   if(bUseRealCoeffsPerSubband)
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
   if(bUseHuffmanCodingDiffMag)
   {                
      // read Huffman code word
      nNumBitsRead += readHuffmanCode(nMagDiff, 
         pSelectedMagDiffTable,
         *pBool1Bit);
      // read Magnitude difference value
      if(nMagDiff == pSelectedMagDiffTable[0].m_codedValue)
      {
         // read unary code of the out of range magnitude difference
         unsigned int val = 0;
         nNumBitsRead += readUnaryCode(&val, pBool1Bit);
         nMagDiff -= val;
      }
      else if (nMagDiff == pSelectedMagDiffTable[unSelectedTableSize-1].m_codedValue)
      {
         // read unary code of the out of range magnitude difference
         unsigned int val = 0;
         nNumBitsRead += readUnaryCode(&val, pBool1Bit);
         nMagDiff += val;
      }
   }
   else
   {
      // read difference index as integer value with a fixed number of bits
      nNumBitsRead += pSelectedDirectMagDiffIO->read();
      int idx = *pSelectedDirectMagDiffIO + 1;
      nMagDiff = pSelectedMagDiffTable[idx].m_codedValue;
      // read Magnitude difference value
      if(nMagDiff == pSelectedMagDiffTable[1].m_codedValue)
      {
         // read unary code of the out of range magnitude difference
         unsigned int val = 0;
         nNumBitsRead += readUnaryCode(&val, pBool1Bit);
         nMagDiff -= val;
      }
      else if (nMagDiff == pSelectedMagDiffTable[unSelectedTableSize-2].m_codedValue)
      {
         // read unary code of the out of range magnitude difference
         unsigned int val = 0;
         nNumBitsRead += readUnaryCode(&val, pBool1Bit);
         nMagDiff += val;
      }
      idx ++;
   }

   // switch between complex and real valued parameter
   if ( !bUseRealCoeffsPerSubband )
   {  
      int idx = 0;
      // read angle value
      idx =  getIdxFromValue(nAngleDiff, pDecodedAngleDiffTable);
      // switch between Huffman decoding and plain bits
      if(bUseHuffmanCodingDiffAngle)
      {                
         // read Huffman code word
         nNumBitsRead += readHuffmanCode(nAngleDiff, 
            pDecodedAngleDiffTable,
            *pBool1Bit);
      }
      else
      {
         // read difference index as integer value with a fixed number of bits
         nNumBitsRead += pDirectAngleDiffIO->read();
         nAngleDiff = pDecodedAngleDiffTable[*pSelectedDirectMagDiffIO].m_codedValue;
      }
   }
   return nNumBitsRead;

}

int HoaFrameR::readUnaryCode(unsigned int *pVal, UCHAR_FR *pBool1Bit)
{
   unsigned int nBitsRead = 0;
   *pVal = 0; // init
   nBitsRead += pBool1Bit->read();
   *pVal += *pBool1Bit;
   while(*pBool1Bit == 1)
   {
      nBitsRead += pBool1Bit->read();
      *pVal += *pBool1Bit;
   }

   return nBitsRead;
}

