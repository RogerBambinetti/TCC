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

#include <algorithm>
#include <numeric>
#include "parDecorrFilter.h"

static const unsigned int unOrder = 3;
static const unsigned int unNumFilters = 10;

static float lattice_coeff_3[unNumFilters][unOrder] = {
{  0.1358f, -0.0373f,  0.0357f },
{ -0.4428f, -0.2286f, -0.4690f },
{  0.4041f, -0.2044f,  0.4640f },
{ -0.4388f,  0.3804f, -0.4791f },
{  0.3004f,  0.2375f,  0.4400f },
{  0.4334f,  0.3321f, -0.1605f },
{ -0.1511f,  0.0254f, -0.1600f },
{ -0.3740f,  0.1246f,  0.2468f },
{ -0.4942f, -0.3039f,  0.4780f },
{  0.3816f, -0.3901f, -0.4434f }
};


ParDecorrFilter::ParDecorrFilter()
      : m_unFilterIdx(0),
        m_vfNumerator(unOrder+1, std::complex<float>(0.0f, 0.0f)),
        m_vfDenominator(unOrder+1, std::complex<float>(0.0f, 0.0f)),
        m_vfX(unOrder+1, std::complex<float>(0.0f, 0.0f)),
        m_vfY(unOrder, std::complex<float>(0.0f, 0.0f))
{
}


ParDecorrFilter::~ParDecorrFilter()
{
}


void ParDecorrFilter::init(unsigned int filterIdx) 
{
   /* set decorrelator index */
   if(filterIdx < unNumFilters)
      m_unFilterIdx = filterIdx;
   else
      return;
   /* compute direct form filter coefficients for the denominator*/
   computeFilterCoeffs();
   /* compute numerator coefficients from denominator coefficients*/
   std::reverse_copy(m_vfDenominator.begin(), m_vfDenominator.end(), m_vfNumerator.begin());
   /* reset state buffers */
   reset();
}


void ParDecorrFilter::reset()
{
   /* reset state buffer to zero */
   m_vfX.assign(unOrder+1, std::complex<float>(0.0f, 0.0f));
   m_vfY.assign(unOrder, std::complex<float>(0.0f, 0.0f));
}


void ParDecorrFilter::process_sample(const std::complex<FLOAT>  &p_rcQmfSampleIn, 
                                           std::complex<FLOAT>  &p_rcQmfSampleOut)
{
      /* update input state buffer */
      std::copy_backward(m_vfX.begin(), m_vfX.end()-1, m_vfX.end());
      m_vfX[0] = p_rcQmfSampleIn;
      /* apply numerator coefficients to input state buffer */
      p_rcQmfSampleOut = static_cast<std::complex<FLOAT> >(std::inner_product(m_vfX.begin(), m_vfX.end(), 
         m_vfNumerator.begin(), std::complex<float>(0.0f, 0.0f)));
      /* apply numerator coefficients to output state buffer */
      p_rcQmfSampleOut -= static_cast<std::complex<FLOAT> >(std::inner_product(m_vfY.begin(), m_vfY.end(), 
         m_vfDenominator.begin()+1, std::complex<float>(0.0f, 0.0f)));
      /* update output state buffer */
      std::copy_backward(m_vfY.begin(), m_vfY.end()-1, m_vfY.end());
      m_vfY[0] = p_rcQmfSampleOut;
}

void ParDecorrFilter::process(const std::vector< std::complex<FLOAT> > &p_rvcQmfSamplesIn, 
                              std::vector< std::complex<FLOAT> > &p_rvcQmfSamplesOut)
{
   for (unsigned int n = 0; n < p_rvcQmfSamplesIn.size(); ++n)
   {
      process_sample(p_rvcQmfSamplesIn[n], p_rvcQmfSamplesOut[n]);
   }
}

void ParDecorrFilter::computeFilterCoeffs()
{
   int     p, j;
   float   tmp[unOrder+1];
   float  *pLatticeCoeff = lattice_coeff_3[m_unFilterIdx];

   m_vfDenominator[0] = 1.0f;
   for (p = 0; p < unOrder; p++)
   {
      m_vfDenominator[p+1] = pLatticeCoeff[p];
      for (j = 0; j < p; j++)
      {
         m_vfDenominator[j+1] = tmp[j] + pLatticeCoeff[p] * tmp[p-j-1];
      }
      for (j = 0; j <= p; j++)
      {
         tmp[j] = m_vfDenominator[j+1].real();        
      }
   }
}