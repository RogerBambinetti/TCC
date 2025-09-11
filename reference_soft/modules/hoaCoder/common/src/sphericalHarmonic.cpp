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
/*   sphericalHarmonic.cpp
*                   Copyright (c) 2014, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    calculate a spherical Harmonics value
* authors:          Johannes Boehm (jb), 
* log:              created 25.07.2014, 
*/
/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: sphericalHarmonic.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include <cstdlib>
#include <cmath>
using namespace std;


//--------------------------------------------------------------------------------------------------------------
// spherical harmonics function - good enough till N<10
// many optimizations regarding speed and accuracy are possible
//-------------------------------------------------------------------------------------------------------------



double sphericalHarmonics(int n, int m, double theta, double phi)
{

 double cosTheta=cos(theta);
 double  sh, dd = 1.0;
 int abs_m=abs(m);
 // calculate part of N3D norm factor
 for( int k = n + abs_m; k > (n - abs_m); k--) 
    dd *= k;
 // calculate Legendre 
double legendre=1.0; 
 if (abs_m > 0) 
    {
    double prt1=sqrt((1.0-cosTheta)*(1.0+cosTheta));
    double prt2=1.0;
    for (int i=1;i<=abs_m;i++) 
        {
        legendre *= -prt2*prt1;
        prt2 += 2.0;
        }
    }
 if(abs_m &1)
       legendre=-legendre;

 if (n!=abs_m)
    {                            
    double legTmp=cosTheta*(2.0*abs_m+1)*legendre;
    if (n == (abs_m+1))
         legendre = legTmp;
    else 
        {   
        double pll=1.0;                     
        for (int ll=abs_m+2;ll<=n;ll++)                 
            {            
            pll=(cosTheta*(2.0*ll-1)*legTmp-(ll+abs_m-1)*legendre)/(ll-abs_m);
            legendre=legTmp;
            legTmp=pll;
            }
        legendre = pll;
        }
    }
 // apply N3D norm factor
  if (abs_m==0)  
       sh= sqrt( (2.0*n + 1.0)/ ( dd ) )*legendre;
  else
       sh= sqrt( 2.0*(2.0*n + 1.0)/ ( dd ) )*legendre; 

// apply azimuthal part

 if (m >= 0)
    return (sh*cos(m*phi));
 else
   return (sh*sin(abs(m)*phi));
}


