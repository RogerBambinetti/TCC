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
 $Id: integerFunctions.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/
#ifndef __INTEGERFUNCTIONS__
#define __INTEGERFUNCTIONS__

/**
  * @brief computes the n=ceil(log2(X)) so that 2^n>=X
  * 
  *
  * @param [in] unX function value
  *
  * @retval ceil(log2(X))
  *
  */    
inline static unsigned int getCeilLog2(const unsigned int unX)
{
  if(unX==0)
    return 0;
  unsigned int unTmp = 1;
  unsigned int unN = 0;
  while( (unTmp<<(unN)) < unX)
  {
    unN++;
  }
  return unN;
};

inline static unsigned int getPow2(const unsigned int p_uiExponent)
{
	unsigned int uiPowResult = 1;

	for (unsigned int uiExpIdx=0; uiExpIdx < p_uiExponent; uiExpIdx++)
	{
		uiPowResult <<= 1;
	}

	return uiPowResult;
}

#endif
