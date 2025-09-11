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
$Id: HoaFrameW.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include "HoaFrameW.h"
#include <string>
#include <set>

template <class T>
static unsigned int writeHuffmanCode(unsigned int codeWord, unsigned int  codeWordLength, T boolIO)
{
   unsigned int nBitsWritten = 0;
   while(codeWordLength)
   {
      boolIO = static_cast<unsigned char>(0x1 & codeWord);
      nBitsWritten += boolIO.write();
      codeWord >>= 1;
      codeWordLength--;
   }
   return nBitsWritten;
}

HoaFrameW::HoaFrameW() 
{
}

unsigned int HoaFrameW::getFrameSize()
{
   return getSpatFrameSize();
}

void HoaFrameW::setGlobalIndyFlag(bool bFlag)
{
   m_bGlobalIndependencyFlag = bFlag;
}

int HoaFrameW::write()
{   
   // write HOA frame
   int nBitsWritten = 0;
   if(m_bitWriter)
   {
      // set bit stream writer
      m_spatPredictionDataIO.m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_spatPredictionDataIO.m_nPredGainsIO.setBitStreamWriter(m_bitWriter);
      m_spatPredictionDataIO.m_unPredIdsIO.setBitStreamWriter(m_bitWriter);
      m_spatPredictionDataIO.m_unActivePredIdsIO.setBitStreamWriter(m_bitWriter);
      m_spatPredictionDataIO.m_unNoActivePredIdsIO.setBitStreamWriter(m_bitWriter);
      m_addAmbHoaInfoChannelIO.m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_addAmbHoaInfoChannelIO.m_uint2Bit.setBitStreamWriter(m_bitWriter);
      m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO.setBitStreamWriter(m_bitWriter);
      m_directionalInfoChannelIO.m_dirIdxIO.setBitStreamWriter(m_bitWriter);
      m_directionalInfoChannelIO.m_uint2Bit.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uint2Bit.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uint3Bit.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uint4Bit.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uint10Bit.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uintNumberOfDirections.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uintDirIdx.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uintAddValue.setBitStreamWriter(m_bitWriter);
      m_vectorBasedInfoChannelIO.m_uint8Bit.setBitStreamWriter(m_bitWriter);
      m_emptyInfoChannelIO.m_uint2Bit.setBitStreamWriter(m_bitWriter);
      m_gainCorrectionIO.m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO.setBitStreamWriter(m_bitWriter);
      m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_hoaDirectionalPredictionInfoIO.m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_hoaDirectionalPredictionInfoIO.m_globalPredDirsIdsIO.setBitStreamWriter(m_bitWriter);
      m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO.setBitStreamWriter(m_bitWriter);
      m_hoaDirectionalPredictionInfoIO.m_numOfGlobalPredDirsIO.setBitStreamWriter(m_bitWriter);
      m_hoaDirectionalPredictionInfoIO.m_uint4Bit.setBitStreamWriter(m_bitWriter);   
      m_hoaParInfoIO.m_bool1Bit.setBitStreamWriter(m_bitWriter);
      m_hoaParInfoIO.m_uint2Bit.setBitStreamWriter(m_bitWriter);
      m_hoaParInfoIO.m_uint4Bit.setBitStreamWriter(m_bitWriter);

      // reset add. HOA ambient array and count
      m_vunAddHoaCoeff.reserve(m_unNumOfAdditionalCoders);
      m_vunAddHoaCoeff.resize(0);

      // write channel information data
      std::set<unsigned int> suiNonTransitionalAddHoaChannels;
      std::set<unsigned int> suiFadeInAddHoaChannels;
      std::set<unsigned int> suiNewVecChannels;
      unsigned int unChannelNumber = 0;
      m_spatPredictionDataIO.m_vunDirSigChannelIds.resize(0);

      // write independency flag
      if(m_bGlobalIndependencyFlag)
         m_bHoaIndependencyFlag = m_bGlobalIndependencyFlag;
      m_bool1Bit = m_bHoaIndependencyFlag;
      nBitsWritten += m_bool1Bit.write();

      for(std::vector<std::shared_ptr<CChannelSideInfoData>>::const_iterator it = m_vChannelSideInfo.begin();
         it < m_vChannelSideInfo.end(); it++)
      {
         switch((*it)->m_unChannelType)
         {
            // directional channel
         case DIR_CHANNEL:
            m_directionalInfoChannelIO.m_uint2Bit = static_cast<unsigned char>((*it)->m_unChannelType);
            nBitsWritten += m_directionalInfoChannelIO.m_uint2Bit.write();

            m_directionalInfoChannelIO.m_dirIdxIO = dynamic_cast<CDirectionalInfoChannel*>(it->get())->m_unActiveDirIds;
            nBitsWritten += m_directionalInfoChannelIO.m_dirIdxIO.write();
            // store channel index of current dir sig
            m_spatPredictionDataIO.m_vunDirSigChannelIds.push_back(unChannelNumber);
            break;
            // add HOA Ambient channel
         case ADD_HOA_CHANNEL:
            m_addAmbHoaInfoChannelIO.m_uint2Bit = static_cast<unsigned char>((*it)->m_unChannelType);
            nBitsWritten += m_addAmbHoaInfoChannelIO.m_uint2Bit.write();

            if(m_bHoaIndependencyFlag)
            {
               m_addAmbHoaInfoChannelIO.m_uint2Bit = static_cast<unsigned char>(
                  dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState);
               nBitsWritten += m_addAmbHoaInfoChannelIO.m_uint2Bit.write();
               m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO = dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx
                  - 1 - m_addAmbHoaInfoChannelIO.m_unMinNumOfCoeffsForAmbHOA;
               nBitsWritten += m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO.write();
            }
            else
            {  
               m_addAmbHoaInfoChannelIO.m_bool1Bit = dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_bAmbCoeffIdxChanged;
               nBitsWritten += m_addAmbHoaInfoChannelIO.m_bool1Bit.write();
               if((m_addAmbHoaInfoChannelIO.m_bool1Bit) && (1==dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdxTransitionState))
               {
                  m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO = dynamic_cast<CAddAmbHoaInfoChannel*>(it->get())->m_unAmbCoeffIdx
                     - 1 - m_addAmbHoaInfoChannelIO.m_unMinNumOfCoeffsForAmbHOA;
                  nBitsWritten += m_addAmbHoaInfoChannelIO.m_unAmbHoaCoeffsAssignmentIO.write();
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
            // vector-based channel
         case VEC_CHANNEL:
            {
               // vector based pre dominant sounds
               // write channel type
               m_vectorBasedInfoChannelIO.m_uint2Bit = static_cast<unsigned char>((*it)->m_unChannelType);
               nBitsWritten += m_vectorBasedInfoChannelIO.m_uint2Bit.write();

               CVectorBasedInfoChannel *pCurChan = dynamic_cast<CVectorBasedInfoChannel*>(it->get());
               if(m_bHoaIndependencyFlag) // bugfix 2014-05-10 NP
               {
                  if (1 == m_vectorBasedInfoChannelIO.m_unCodedVVecLength) {
                     m_vectorBasedInfoChannelIO.m_bool1Bit = pCurChan->m_bNewChannelTypeOne;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                     if (1==pCurChan->m_bNewChannelTypeOne)
                        suiNewVecChannels.insert(unChannelNumber);					   
                  }

                  m_vectorBasedInfoChannelIO.m_uint4Bit = pCurChan->m_unNbitsQ;
                  nBitsWritten += m_vectorBasedInfoChannelIO.m_uint4Bit.write(); 
                  if(pCurChan->m_unNbitsQ == cVVecVQWord)
                  {					
                     m_vectorBasedInfoChannelIO.m_uint3Bit = pCurChan->m_unIndicesCodebookIdx;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_uint3Bit.write();

                     m_vectorBasedInfoChannelIO.m_uintNumberOfDirections = pCurChan->m_unVVecDirections - 1;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_uintNumberOfDirections.write();					

                  }
                  if(pCurChan->m_unNbitsQ > c8BitQuantizerWord)
                  {
                     // write Codebook flag
                     m_vectorBasedInfoChannelIO.m_bool1Bit = pCurChan->m_bCbFlag;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                  }
               }
               else
               {
                  if ((1 == m_vectorBasedInfoChannelIO.m_unCodedVVecLength) && (m_vPrevFrameChannelSideInfo[unChannelNumber]->m_unChannelType != VEC_CHANNEL))
                  {						
                     suiNewVecChannels.insert(unChannelNumber);
                  }
                  if (pCurChan->m_bSameHeaderPrevFrame)
                  {
                     m_vectorBasedInfoChannelIO.m_bool1Bit = 0;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                     m_vectorBasedInfoChannelIO.m_bool1Bit = 0;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                  } 
                  else
                  {
                     m_vectorBasedInfoChannelIO.m_uint4Bit = pCurChan->m_unNbitsQ;
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_uint4Bit.write(); 
                     if(pCurChan->m_unNbitsQ == cVVecVQWord)
                     {   							
                        //Indicies Codebook
                        m_vectorBasedInfoChannelIO.m_uint3Bit = pCurChan->m_unIndicesCodebookIdx;
                        nBitsWritten += m_vectorBasedInfoChannelIO.m_uint3Bit.write();
                        m_vectorBasedInfoChannelIO.m_uintNumberOfDirections = pCurChan->m_unVVecDirections - 1;
                        nBitsWritten += m_vectorBasedInfoChannelIO.m_uintNumberOfDirections.write();				
                     }
                     if(pCurChan->m_unNbitsQ > c8BitQuantizerWord)
                     {
                        // write prediction flag
                        m_vectorBasedInfoChannelIO.m_bool1Bit = pCurChan->m_bPFlag;
                        nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                        // write Codebook flag
                        m_vectorBasedInfoChannelIO.m_bool1Bit = pCurChan->m_bCbFlag;
                        nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                     }
                  }
               }
            }
            break;
            // Empty and default channel
         default:
            m_emptyInfoChannelIO.m_uint2Bit = static_cast<unsigned char>((*it)->m_unChannelType);
            nBitsWritten += m_emptyInfoChannelIO.m_uint2Bit.write();
         }

         // write last gain correction amplification exponent
         if(m_bHoaIndependencyFlag)
         {
            m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO = (unsigned)(m_vGainCorrectionData[unChannelNumber].m_nGainCorrAbsAmpExp 
               + m_gainCorrectionIO.m_nMaxLog2ExpOfTransportSigs);
            nBitsWritten += m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO.write();

         }

         // write gain correction data
         nBitsWritten += writeArray(m_vGainCorrectionData[unChannelNumber].m_bCodedGainCorrectionExp.begin(), 
            m_vGainCorrectionData[unChannelNumber].m_bCodedGainCorrectionExp.end(), 
            m_gainCorrectionIO.m_bool1Bit);     

         m_gainCorrectionIO.m_bool1Bit = m_vGainCorrectionData[unChannelNumber].m_bGainCorrectionException;
         nBitsWritten += m_gainCorrectionIO.m_bool1Bit.write();

         // increase channel number
         unChannelNumber++;
      }    


      for(; unChannelNumber < m_vGainCorrectionData.size(); unChannelNumber++)
      {
         // write last gain correction amplification exponent
         if(m_bHoaIndependencyFlag)
         {
            m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO = (unsigned)(m_vGainCorrectionData[unChannelNumber].m_nGainCorrAbsAmpExp 
               + m_gainCorrectionIO.m_nMaxLog2ExpOfTransportSigs);
            nBitsWritten += m_gainCorrectionIO.m_uintGainCorrAbsAmpExpIO.write();

         }

         // write gain correction data
         nBitsWritten += writeArray(m_vGainCorrectionData[unChannelNumber].m_bCodedGainCorrectionExp.begin(), 
            m_vGainCorrectionData[unChannelNumber].m_bCodedGainCorrectionExp.end(), 
            m_gainCorrectionIO.m_bool1Bit);

         m_gainCorrectionIO.m_bool1Bit = m_vGainCorrectionData[unChannelNumber].m_bGainCorrectionException;
         nBitsWritten += m_gainCorrectionIO.m_bool1Bit.write();     
      }

      // count number of active prediction indices
      unsigned int unNoOfActivePred = 0;
      std::vector<unsigned int> vIds;
      vIds.reserve(m_spatPredictionData.m_bActivePred.size());
      for(unsigned int n=0; n<m_spatPredictionData.m_bActivePred.size(); ++n)
      {
         if(m_spatPredictionData.m_bActivePred[n])
         {
            unNoOfActivePred++;
            vIds.push_back(n);
         }
      }

      // process remaining portion of vec channel info
      unChannelNumber = 0;
      for(std::vector<std::shared_ptr<CChannelSideInfoData>>::const_iterator it = m_vChannelSideInfo.begin();
         it < m_vChannelSideInfo.end(); it++)
      {
         if(VEC_CHANNEL == (*it)->m_unChannelType)
         {
            std::set<unsigned int>::iterator suiItNewVecChannels;
            std::set<unsigned int>::iterator suiItTransitionalAddHoaChannel;
            std::set<unsigned int>::iterator suiItFadeInAddHoaChannel;
            unsigned int unHoaCoeffIdx = m_vectorBasedInfoChannelIO.m_unHoaIdxOffset;

            suiItNewVecChannels = suiNewVecChannels.find(unChannelNumber); 
            CVectorBasedInfoChannel *pCurChan = dynamic_cast<CVectorBasedInfoChannel*>(it->get());
            if(cVVecVQWord == pCurChan->m_unNbitsQ)
            {   
               //TODO: 
               switch (pCurChan->m_unIndicesCodebookIdx)
               {
               case 0:
                  m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(10);
                  break;
               case 1: 
                  m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(6);
                  break;
               case 2:
                  m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(6);
                  break;
               case 3: // reserved
               case 4: // reserved
               case 5: // reserved
               case 6: // reserved
               case 7:
                  m_vectorBasedInfoChannelIO.m_uintDirIdx.setFieldSizeInBits(getCeilLog2(m_unNoOfHoaCoeffs));
                  break;
               }		

               if (1 < pCurChan->m_unVVecDirections)
               {
                  m_vectorBasedInfoChannelIO.m_uint8Bit = pCurChan->m_unWeightingCodebookIdx;
                  nBitsWritten += m_vectorBasedInfoChannelIO.m_uint8Bit.write();
               }

               for(unsigned int nEl = 0; nEl < pCurChan->m_unVVecDirections; nEl++)
               {
                  m_vectorBasedInfoChannelIO.m_uintDirIdx = pCurChan->m_vunDirectionIndices[nEl]-1; //index 0 is reserved for something else.
                  nBitsWritten += m_vectorBasedInfoChannelIO.m_uintDirIdx.write();

                  m_vectorBasedInfoChannelIO.m_bool1Bit = pCurChan->m_vbSign[nEl];
                  nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
               }				
            }	
            else if(c8BitQuantizerWord == pCurChan->m_unNbitsQ)
            {
               for(unsigned int nEl = 0; nEl < pCurChan->m_vun8bitCodedVelement.size(); nEl++, ++unHoaCoeffIdx)
               {				
                  suiItFadeInAddHoaChannel = suiFadeInAddHoaChannels.find( unHoaCoeffIdx ); 
                  suiItTransitionalAddHoaChannel = suiNonTransitionalAddHoaChannels.find( unHoaCoeffIdx ); 
                  if ((suiItTransitionalAddHoaChannel == suiNonTransitionalAddHoaChannels.end()) && !(suiItNewVecChannels!=suiNewVecChannels.end() && suiItFadeInAddHoaChannel!=suiFadeInAddHoaChannels.end()) ) 
                  {
                     m_vectorBasedInfoChannelIO.m_uint8Bit = pCurChan->m_vun8bitCodedVelement[nEl];
                     nBitsWritten += m_vectorBasedInfoChannelIO.m_uint8Bit.write();							
                  }
               }  
            }
            else if (c8BitQuantizerWord < pCurChan->m_unNbitsQ)
            {
               // write each element of the vector
               unsigned int unEl = 0;
               std::vector<unsigned int>::iterator itAddBits = pCurChan->m_vunNumBitsAdditional.begin();
               std::vector<bool>::iterator itAddInfo = pCurChan->m_vbAdditionalInfo.begin();
               for(std::vector<std::string>::iterator it = pCurChan->m_vsCodedHuffmannWord.begin();
                  it < pCurChan->m_vsCodedHuffmannWord.end(); ++it, ++itAddInfo, ++itAddBits, ++unHoaCoeffIdx)
               {
                  suiItFadeInAddHoaChannel = suiFadeInAddHoaChannels.find( unHoaCoeffIdx ); 
                  suiItTransitionalAddHoaChannel = suiNonTransitionalAddHoaChannels.find( unHoaCoeffIdx ); 
                  if ((suiItTransitionalAddHoaChannel == suiNonTransitionalAddHoaChannels.end()) && !(suiItNewVecChannels!=suiNewVecChannels.end() && suiItFadeInAddHoaChannel!=suiFadeInAddHoaChannels.end()) )  
                  {
                     // write Huffman code word 
                     for(unsigned int idx = 0; idx< it->length(); ++idx)
                     {
                        m_vectorBasedInfoChannelIO.m_bool1Bit = (it->substr(idx,1) == std::string("1") );
                        nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                     }
                     if (*itAddInfo)
                     {
                        // write sign bit
                        m_vectorBasedInfoChannelIO.m_bool1Bit = pCurChan->m_vbSign[unEl]==1;
                        nBitsWritten += m_vectorBasedInfoChannelIO.m_bool1Bit.write();
                        // write additional value
                        if(*itAddBits)
                        {
                           m_vectorBasedInfoChannelIO.m_uintAddValue.setFieldSizeInBits(*itAddBits);
                           m_vectorBasedInfoChannelIO.m_uintAddValue = pCurChan->m_vunAdditionalValue[unEl];
                           nBitsWritten += m_vectorBasedInfoChannelIO.m_uintAddValue.write();
                        }
                     }
                  }
                  unEl++;
               }
            }
         }
         unChannelNumber++;
      }

      // write directional prediction data
      if(m_spatPredictionDataIO.m_vunDirSigChannelIds.size()>0)
      {    
         // write Perform Prediction bit
         m_spatPredictionDataIO.m_bool1Bit = m_spatPredictionData.m_bPerformPrediction;
         nBitsWritten += m_spatPredictionDataIO.m_bool1Bit.write();

         if(m_spatPredictionData.m_bPerformPrediction)
         {
            // select coding type for active prediction indices
            if(unNoOfActivePred <= m_spatPredictionDataIO.m_unNoOfPredIdsBetterToCodeExplicitly)
            {
               // write active prediction coding type
               m_spatPredictionData.m_bKindOfCodedPredIds = 1;
               m_spatPredictionDataIO.m_bool1Bit = m_spatPredictionData.m_bKindOfCodedPredIds;
               nBitsWritten += m_spatPredictionDataIO.m_bool1Bit.write();
               // write number of indices
               m_spatPredictionDataIO.m_unNoActivePredIdsIO = unNoOfActivePred-1;
               nBitsWritten += m_spatPredictionDataIO.m_unNoActivePredIdsIO.write();
               // write indices
               for(unsigned int n = 0; n < vIds.size(); ++n)
               {
                  m_spatPredictionDataIO.m_unActivePredIdsIO = vIds[n];
                  nBitsWritten += m_spatPredictionDataIO.m_unActivePredIdsIO.write();
               }
            }
            else
            {
               // write active prediction coding type
               m_spatPredictionData.m_bKindOfCodedPredIds = 0;
               m_spatPredictionDataIO.m_bool1Bit = m_spatPredictionData.m_bKindOfCodedPredIds;
               nBitsWritten += m_spatPredictionDataIO.m_bool1Bit.write();
               // write number of active predictions
               nBitsWritten += writeArray(m_spatPredictionData.m_bActivePred.begin(), m_spatPredictionData.m_bActivePred.end(), 
                  m_spatPredictionDataIO.m_bool1Bit);
            }

            // write the prediction type of each of the active predictions
            //nBitsWritten += writeArray(m_spatPredictionData.m_bPredType.begin(), m_spatPredictionData.m_bPredType.end(),
            //                           m_spatPredictionDataIO.m_bool1Bit);

            // write the index of each of the active predictions
            m_spatPredictionDataIO.m_unPredIdsIO.setFieldSizeInBits(getCeilLog2(m_spatPredictionDataIO.m_vunDirSigChannelIds.size()+1));
            for(std::vector<unsigned int>::iterator it = m_spatPredictionData.m_unPredIds.begin();
               it <  m_spatPredictionData.m_unPredIds.end(); ++it)
            {
               if(*it)
               {
                  unsigned int idx = 0;
                  while( (*it-1 != m_spatPredictionDataIO.m_vunDirSigChannelIds[idx]) & (idx < m_spatPredictionDataIO.m_vunDirSigChannelIds.size()) )
                     idx++;
                  if(idx < m_spatPredictionDataIO.m_vunDirSigChannelIds.size())
                     m_spatPredictionDataIO.m_unPredIdsIO = idx+1;
                  else
                     return -1;
               }
               else
               {
                  m_spatPredictionDataIO.m_unPredIdsIO = 0;
               }
               nBitsWritten += m_spatPredictionDataIO.m_unPredIdsIO.write();
            }

            // write prediction gains for each of the active predictions
            nBitsWritten += writeArray(m_spatPredictionData.m_nPredGains.begin(), m_spatPredictionData.m_nPredGains.end(), 
               m_spatPredictionDataIO.m_nPredGainsIO);
         }
      }

      // write HOA Directional Prediction Info
      if(m_hoaDirectionalPredictionInfoIO.m_unNumOfPredSubbands > 0)
      {
         // Use directional prediction flag
         m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaDirectionalPredictionInfo.m_bUseDirPred;
         nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
         if(m_hoaDirectionalPredictionInfo.m_bUseDirPred)
         {
            // Keep previous global directions
            if(!m_bHoaIndependencyFlag)
            {
               m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaDirectionalPredictionInfo.m_bKeepPreviouseGlobalPredDirsFlag; 
               nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
            }
            else
               m_hoaDirectionalPredictionInfo.m_bKeepPreviouseGlobalPredDirsFlag = false;

            if(!m_hoaDirectionalPredictionInfo.m_bKeepPreviouseGlobalPredDirsFlag)
            {
               // NumOfGlobalPredDirs
               m_hoaDirectionalPredictionInfoIO.m_numOfGlobalPredDirsIO = m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs - 1; 
               nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_numOfGlobalPredDirsIO.write(); 

               // set size of umBitsForRelDirGridIdxIO
               m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO.setFieldSizeInBits(
                  getCeilLog2(m_hoaDirectionalPredictionInfo.m_unNumOfGlobalPredDirs));

               // for loop over m_unNumOfGlobalPredDirs
               nBitsWritten += writeArray(m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.begin(),
                  m_hoaDirectionalPredictionInfo.m_vunGlobalPredDirIds.end(),
                  m_hoaDirectionalPredictionInfoIO.m_globalPredDirsIdsIO);
            }

            // create sorted AddHoaCoeff index array
            std::vector<unsigned int> vunSortedAddHoaCoeff = m_vunAddHoaCoeff;
            std::sort(vunSortedAddHoaCoeff.begin(), vunSortedAddHoaCoeff.end());


            // loop over subbands and directions for writing prediction parameter for each combination
            for ( unsigned int band = 0; band < m_hoaDirectionalPredictionInfoIO.m_unNumOfPredSubbands; ++band)
            {

               // Keep previous direction prediction matrix
               if(!m_bHoaIndependencyFlag)
               {
                  m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[band]; 
                  nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
               }
               else
                  m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[band] = false;

               if(!m_hoaDirectionalPredictionInfo.m_vbKeepPrevDirPredMatrixFlag[band])
               {
                  // write huffman table decision
                  m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffMag[band];
                  nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
                  if(band < m_hoaDirectionalPredictionInfoIO.m_unFirstSBRSubbandIdx )
                  {
                     m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaDirectionalPredictionInfo.m_vbUseHuffmanCodingDiffAngle[band];
                     nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
                  }

                  for ( unsigned int dir = 0; dir < m_hoaDirectionalPredictionInfoIO.m_unMaxNumOfPredDirsPerBand; ++dir)
                  {
                     // write active direction bit
                     m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[band][dir];
                     nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
                     if(m_hoaDirectionalPredictionInfo.m_vvbDirIsActive[band][dir])
                     {
                        // write RelDirGridIdx
                        m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO = m_hoaDirectionalPredictionInfo.m_vvunRelDirGridIdx[band][dir];
                        nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_relDirGridIdxIO.write();

                        // loop over const HOA channels 
                        for ( unsigned int hoaIdx = 0; hoaIdx < m_hoaDirectionalPredictionInfoIO.m_unMinNumOfCoeffsForAmbHOA ; hoaIdx++)
                        {
                           // write angle and magnitude differences
                           nBitsWritten += writeDiffValues(band >= m_hoaDirectionalPredictionInfoIO.m_unFirstSBRSubbandIdx,            
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
                           // write angle and magnitude differences
                           nBitsWritten += writeDiffValues(band >= m_hoaDirectionalPredictionInfoIO.m_unFirstSBRSubbandIdx,            
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

      // Write PAR Data
      if(m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband.size() > 0)
      {
         m_hoaParInfoIO.m_bool1Bit = m_hoaParInfo.m_bPerformPar;
         nBitsWritten +=  m_hoaParInfoIO.m_bool1Bit .write();
         if(m_hoaParInfo.m_bPerformPar)
         {
            // update selected tables 
            for(unsigned int band=0; band < m_hoaParInfo.m_vbKeepPrevMatrixFlag.size(); band++)
            {
               // keep matrix flag
               if(!m_bHoaIndependencyFlag)
               {
                  m_hoaDirectionalPredictionInfoIO.m_bool1Bit = m_hoaParInfo.m_vbKeepPrevMatrixFlag[band]; 
                  nBitsWritten += m_hoaDirectionalPredictionInfoIO.m_bool1Bit.write();
               }
               else
                  m_hoaParInfo.m_vbKeepPrevMatrixFlag[band] = false;

               if(!m_hoaParInfo.m_vbKeepPrevMatrixFlag[band])
               {
                  // ParDecorrSigsSelectionTableIdx
                  m_hoaParInfoIO.m_uint2Bit = m_hoaParInfo.m_vunParDecorrSigsSelectionTableIdx[band];
                  nBitsWritten += m_hoaParInfoIO.m_uint2Bit.write();
                  m_hoaParInfo.updateSelectionTableInSubBand(band,
                     m_hoaParInfoIO.m_vunParUpmixHoaOrderPerParSubBandIdx[band]);

                  // reduce matrix size
                  m_hoaParInfoIO.m_bool1Bit  = m_hoaParInfo.m_vbUseReducedNoOfUpmixSigs[band];
                  nBitsWritten +=  m_hoaParInfoIO.m_bool1Bit.write();
                  if(m_hoaParInfo.m_vbUseReducedNoOfUpmixSigs[band])
                  {
                     if(m_hoaParInfo.m_vvbUseParUpmixSig[band].size() != m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band].size())
                        return nBitsWritten;
                     nBitsWritten += writeArray(m_hoaParInfo.m_vvbUseParUpmixSig[band].begin(), m_hoaParInfo.m_vvbUseParUpmixSig[band].end(),
                        m_hoaParInfoIO.m_bool1Bit);
                  }   
                  // Huffman table index
                  m_hoaParInfoIO.m_bool1Bit = m_hoaParInfo.m_vbUseParHuffmanCodingDiffAbs[band];
                  nBitsWritten += m_hoaParInfoIO.m_bool1Bit.write();
                  if( !m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband[band] )
                  {
                     m_hoaParInfoIO.m_bool1Bit = m_hoaParInfo.m_vbUseParHuffmanCodingDiffAngle[band];
                     nBitsWritten += m_hoaParInfoIO.m_bool1Bit.write();
                  }

                  // Loop over each up-mix signal
                  for(unsigned int n=0; n < m_hoaParInfo.m_vvvnParMixingMatrixDiffAbs[band].size(); ++n)
                  {
                     // Loop over all active parameters for each active up-mix signal
                     for(unsigned int m=0; (m < m_hoaParInfo.m_vvvunParSelectedDecorrSigsIdxMatrix[band][n].size()) & m_hoaParInfo.m_vvbUseParUpmixSig[band][n]; ++m)
                     {
                        // write angle and magnitude differences
                        nBitsWritten += writeDiffValues(m_hoaParInfoIO.m_vbUseRealCoeffsPerParSubband[band],            
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
            }
         }
      }
   }
   return nBitsWritten;
}

HoaFrame& HoaFrameW::getFields()
{
   return *this;
}

unsigned int HoaFrameW::writeDiffValues(bool bUseRealCoeffsPerSubband,            
                                        int nMagDiff,
                                        bool bUseHuffmanCodingDiffMag,
                                        const HuffmanWord<int> *pDecodedMagDiffTable,
                                        unsigned int unDecodedMagDiffTableSize,
                                        UINT_FW *pDirectMagDiffIO,
                                        const HuffmanWord<int> *pDecodedMagSbrDiffTable,
                                        unsigned int unDecodedMagDiffSbrTableSize,
                                        UINT_FW *pDirectMagDiffSbrIO,
                                        int nAngleDiff,
                                        bool bUseHuffmanCodingDiffAngle,
                                        const HuffmanWord<int> *pDecodedAngleDiffTable,
                                        UINT_FW *pDirectAngleDiffIO,
                                        UCHAR_FW *pBool1Bit)
{
   unsigned int nBitsWritten = 0;
   const HuffmanWord<int> *pSelectedMagDiffTable = 0;
   unsigned int unSelectedTableSize = 0;
   UINT_FW *pSelectedDirectMagDiffIO = 0;
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
      int idx = 0;
      // Write Magnitude difference value
      if(nMagDiff <= pSelectedMagDiffTable[0].m_codedValue)
      {
         // write escape word for negative values
         nBitsWritten += writeHuffmanCode(pSelectedMagDiffTable[0].m_unHuffmanWord, 
            pSelectedMagDiffTable[0].m_unCodeWordLength, *pBool1Bit);
         // write unary code of the out of range magnitude difference
         nBitsWritten += writeUnaryCode((-nMagDiff + pSelectedMagDiffTable[0].m_codedValue),
            pBool1Bit);
      }
      else if (nMagDiff >= pSelectedMagDiffTable[unSelectedTableSize-1].m_codedValue)
      {
         // write escape word for positive values
         nBitsWritten += writeHuffmanCode(pSelectedMagDiffTable[unSelectedTableSize-1].m_unHuffmanWord, 
            pSelectedMagDiffTable[unSelectedTableSize-1].m_unCodeWordLength, *pBool1Bit);
         // write unary code of the out of range magnitude difference
         nBitsWritten += writeUnaryCode((nMagDiff - pSelectedMagDiffTable[unSelectedTableSize-1].m_codedValue),
            pBool1Bit);
      }
      else
      {
         // write Huffman coded magnitude difference
         idx = getIdxFromValue(nMagDiff,  pSelectedMagDiffTable);
         if(idx>=0)
         {
            nBitsWritten += writeHuffmanCode(pSelectedMagDiffTable[idx].m_unHuffmanWord, 
               pSelectedMagDiffTable[idx].m_unCodeWordLength, *pBool1Bit);
         }
      }
   }
   else // use PCM coding for writing magnitude differences
   {
      // Write Magnitude difference value
      if(nMagDiff <= pSelectedMagDiffTable[1].m_codedValue)
      {
         // write escape word for negative values
         *pSelectedDirectMagDiffIO = 0;
         nBitsWritten += pSelectedDirectMagDiffIO->write();
         // write unary code of the out of range magnitude difference
         nBitsWritten += writeUnaryCode((-nMagDiff + pSelectedMagDiffTable[1].m_codedValue),
            pBool1Bit);
      }
      else if (nMagDiff >= pSelectedMagDiffTable[unSelectedTableSize-2].m_codedValue)
      {
         // write escape word for positive values
         *pSelectedDirectMagDiffIO = unSelectedTableSize-3;
         nBitsWritten += pSelectedDirectMagDiffIO->write();
         // write unary code of the out of range magnitude difference
         nBitsWritten += writeUnaryCode((nMagDiff - pSelectedMagDiffTable[unSelectedTableSize-2].m_codedValue),
            pBool1Bit);
      }
      else
      {
         // write magnitude difference direct
         *pSelectedDirectMagDiffIO = nMagDiff - pSelectedMagDiffTable[1].m_codedValue;
         nBitsWritten += pSelectedDirectMagDiffIO->write();
      }
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
         if(idx>=0)
         {
            nBitsWritten += writeHuffmanCode(pDecodedAngleDiffTable[idx].m_unHuffmanWord, 
               pDecodedAngleDiffTable[idx].m_unCodeWordLength, *pBool1Bit);
         }
      }
      else
      {
         *pDirectAngleDiffIO = idx;
         nBitsWritten += pDirectAngleDiffIO->write(); 
      }
   }
   return nBitsWritten;
};

int HoaFrameW::writeUnaryCode(unsigned int nVal, UCHAR_FW *pBool1Bit)
{
   int nBitsWritten = 0;
   // write nVal times a one
   *pBool1Bit = 1;
   for(unsigned int nn = 0; nn < nVal; nn++)
      nBitsWritten += pBool1Bit->write();
   // write a zero as stop bit
   *pBool1Bit = 0;
   nBitsWritten += pBool1Bit->write();

   return nBitsWritten;
}
