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
 $Rev: 157 $
 $Author: technicolor-kf $
 $Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
 $Id: ChannelReAssignment.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#include "ChannelReAssignment.h"

// constructor
ChannelReAssignment::ChannelReAssignment()
{
		m_bIsInit = false;
};


// reset
bool ChannelReAssignment::init(unsigned int p_uiNoOfChannels, unsigned int p_uiMaxNoOfDirSigs, unsigned int p_uiNoOfHOACoeffs, unsigned int p_uiFrameSize)
{
		// init global parameters
		m_uiNoOfAvailableChannels  = p_uiNoOfChannels;
		m_uiMaxNoOfDirSigs         = p_uiMaxNoOfDirSigs;
		m_uiNoOfHOACoeffs          = p_uiNoOfHOACoeffs;
		m_uiFrameSize			   = p_uiFrameSize;

		if (m_uiNoOfAvailableChannels < m_uiMaxNoOfDirSigs)
		{
			std::cout<< "Number of available channels must not be lower than maximum number of directional signals!" << std::endl;
			return true;
		}
		else
		{
			if (m_uiNoOfAvailableChannels < 1)
			{
				std::cout<< "Number of available channels must not be lower than one!" << std::endl;
				return true;
			}
		}

		m_bIsInit = true;

		return false;
}


bool ChannelReAssignment::process(const	  std::vector<std::vector<FLOAT> > & p_rvvFInputAllDecodedSigsFrame,
									  std::vector<std::vector<FLOAT> > & p_rvvFOutputDecPSSigsFrame,
									  std::vector<std::vector<FLOAT> > & p_rvvFOutputDecAmbHOACoeffsFrame,
									  const std::vector<unsigned int> & p_rvuiAmbHOAAssignmentVec,
									  const std::map<unsigned int, unsigned int> &p_rmuiActiveDirAndGridDirIndices,
									  const std::map<unsigned int, std::vector<FLOAT> > &p_rmuivFVectors)
{
	 // check if ChannelReAssignment object has already been initialized
	 if ( !m_bIsInit )
	 {
		 return true;
	 }
	
	 // check if the buffer sizes are correct
	 if ( checkBufferSizes(p_rvvFInputAllDecodedSigsFrame, p_rvvFOutputDecPSSigsFrame, p_rvvFOutputDecAmbHOACoeffsFrame) )
	 {
	 	return true;
	 }

	 // init current frame of all directional signals to zero
	 for (unsigned int uiDirIdx=0; uiDirIdx < m_uiMaxNoOfDirSigs; uiDirIdx++)
	 {
			 p_rvvFOutputDecPSSigsFrame[uiDirIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0) );
	 }

	 // extract directional signals frame from multichannel signal frame
	 for ( std::map<unsigned int, unsigned int>::const_iterator cmuiIt=p_rmuiActiveDirAndGridDirIndices.begin(); cmuiIt != p_rmuiActiveDirAndGridDirIndices.end(); cmuiIt++)
	 {
		 unsigned int uiCurrChanIndexWithDirSig = cmuiIt->first;
		 p_rvvFOutputDecPSSigsFrame[uiCurrChanIndexWithDirSig-1].assign(p_rvvFInputAllDecodedSigsFrame[uiCurrChanIndexWithDirSig-1].begin(), p_rvvFInputAllDecodedSigsFrame[uiCurrChanIndexWithDirSig-1].end() );
	 }

	 // extract vector based signals frame from multichannel signal frame
	 for ( std::map<unsigned int, std::vector<FLOAT> >::const_iterator cmuivFIt = p_rmuivFVectors.begin(); cmuivFIt != p_rmuivFVectors.end(); cmuivFIt++)
	 {
		 unsigned int uiCurrChanIndexWithVecSig = cmuivFIt->first;
		 p_rvvFOutputDecPSSigsFrame[uiCurrChanIndexWithVecSig-1].assign(p_rvvFInputAllDecodedSigsFrame[uiCurrChanIndexWithVecSig-1].begin(), p_rvvFInputAllDecodedSigsFrame[uiCurrChanIndexWithVecSig-1].end() );
	 }

	 
	 // extract ambient HOA coefficient frame from multichannel signal frame
		 // init to zero
		 for (unsigned int uiCoeffIdx=0; uiCoeffIdx < m_uiNoOfHOACoeffs; uiCoeffIdx++)
		 {
			 p_rvvFOutputDecAmbHOACoeffsFrame[uiCoeffIdx].assign( m_uiFrameSize, static_cast<FLOAT>(0.0) );
		 }

	 for ( unsigned int uiChanIdx = 0; uiChanIdx < m_uiNoOfAvailableChannels; uiChanIdx++)
	 {
		 if ( p_rvuiAmbHOAAssignmentVec[uiChanIdx] != 0)
		 {
			 p_rvvFOutputDecAmbHOACoeffsFrame[ p_rvuiAmbHOAAssignmentVec[uiChanIdx] - 1].assign( p_rvvFInputAllDecodedSigsFrame[uiChanIdx].begin(), p_rvvFInputAllDecodedSigsFrame[uiChanIdx].end() );
		 }
	 }

	return false;
};




bool ChannelReAssignment::checkBufferSizes(const std::vector<std::vector<FLOAT> > & p_rvvFInputAllDecodedSigsFrame,
										   const std::vector<std::vector<FLOAT> > & p_rvvFOutputDecDirSigsFrame,
										   const std::vector<std::vector<FLOAT> > & p_rvvFOutputPreliminaryDecAmbHOACoeffsFrame)
{
	 //// check if the sizes of the buffers are correct
	 if ( (p_rvvFInputAllDecodedSigsFrame.size() != m_uiNoOfAvailableChannels) || 
		  (p_rvvFOutputDecDirSigsFrame.size() != m_uiMaxNoOfDirSigs) ||
		  (p_rvvFOutputPreliminaryDecAmbHOACoeffsFrame.size() != m_uiNoOfHOACoeffs) )
	 {
		std::cout<< "ChannelReAssignment: Buffer dimensions don't match with expected dimensions!" << std::endl;
		return true;
	 }


	 bool bError = false;
	 unsigned int uiExpectedBufferLength = p_rvvFInputAllDecodedSigsFrame[0].size();

	 for( unsigned int uiRowIdx=1; uiRowIdx < m_uiNoOfAvailableChannels; uiRowIdx++)
	 {
		 if( p_rvvFInputAllDecodedSigsFrame[uiRowIdx].size() != uiExpectedBufferLength)
		 {
			 bError = true;
		 }
	 }

	 for (unsigned int uiRowIdx=0; uiRowIdx < m_uiMaxNoOfDirSigs; uiRowIdx++)
	 {
		 if( p_rvvFOutputDecDirSigsFrame[uiRowIdx].size() != uiExpectedBufferLength )
		 {
			 bError = true;
		 }
	 }

	 for (unsigned int uiRowIdx=0; uiRowIdx < m_uiNoOfHOACoeffs; uiRowIdx++)
	 {
			if( p_rvvFOutputPreliminaryDecAmbHOACoeffsFrame[uiRowIdx].size() != uiExpectedBufferLength)
			{
				bError = true;
			}
	 }

	 if (bError)
	 {
		 std::cout<< "ChannelReAssignment: Buffer lengths don't match!" << std::endl;
	 }

	 return bError;
}
