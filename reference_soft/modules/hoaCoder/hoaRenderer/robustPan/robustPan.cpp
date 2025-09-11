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
/*   robustPan.cpp
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    calculate panning gains
* uses:             spacePositions to handle the speaker positions, 
*                 
* authors:          Johannes Boehm (jb),
* log:              created 19.08.2013, 
*/
/*
$Rev: 196 $
$Author: technicolor-ks $
$Date: 2015-10-12 13:45:11 +0200 (Mo, 12 Okt 2015) $
$Id: robustPan.cpp 196 2015-10-12 11:45:11Z technicolor-ks $
*/




#include "robustPan.h"
#include <memory.h>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <cmath>
using namespace std;


#define bPLANEWAVEMODEL true // don't change this (spherical wave model is not implemented)
//#define RM0 1

//----------------------------------------------------------------------------------------------------------------------------------------- 
robustPan::robustPan(  spacePositions *speakerPos, double freq, double micRadius, bool clipSourceHight)
: m_SpeakerPos_Cart(*speakerPos), m_clipSourceHight(clipSourceHight), m_vMradius(micRadius), m_k(2*PI*freq/340.)
{ 
  if (speakerPos->getType()==1)
  {
   m_is2D = speakerPos->is2D();
   speakerPos->getMaxMinInclination(&m_maxInclination, &m_minInclination );
   spacePositions tmpSPC =  speakerPos->convertCopyToCartesian(bPLANEWAVEMODEL);  
   m_SpeakerPos_Cart=tmpSPC;
  } 
  else
  {
   spacePositions tmpSp= speakerPos->convertCopyToSpherical();   // this is done to force radius to 1 (spherical projection)
   m_is2D = tmpSp.is2D();
   tmpSp.getMaxMinInclination(&m_maxInclination, &m_minInclination );
   if(bPLANEWAVEMODEL)  // plane wave model uses positions of spherical projection to unit circle (R=1)
   {
       spacePositions tmpSPC =  tmpSp.convertCopyToCartesian(true);       
       m_SpeakerPos_Cart=tmpSPC;
   }
   else m_SpeakerPos_Cart=*speakerPos;
  }
  init(); 
}
   
  
//-----------------------------------------------------------------------------------------------------------------------------------------
 robustPan::robustPan( const std::string &fileNameWithLoudSpeakerPos, int fileType, double freq, double micRadius, bool clipSourceHight)
: m_clipSourceHight(clipSourceHight), m_vMradius(micRadius), m_k(2*PI*freq/340.)
{
  spacePositions tps(fileNameWithLoudSpeakerPos,fileType );
  spacePositions *speakerPos=&tps;
  if (speakerPos->getType()==1)
  {   
      m_is2D = speakerPos->is2D();
      speakerPos->getMaxMinInclination(&m_maxInclination, &m_minInclination );
      spacePositions tmpSPC =  speakerPos->convertCopyToCartesian(bPLANEWAVEMODEL);  
      m_SpeakerPos_Cart=tmpSPC;
  } 
  else
  {
      spacePositions tmpSp= speakerPos->convertCopyToSpherical();   // this is done to force radius to 1 (spherical projection)
      m_is2D = tmpSp.is2D();
      tmpSp.getMaxMinInclination(&m_maxInclination, &m_minInclination );
      if(bPLANEWAVEMODEL)
      {
          spacePositions tmpSPC =  tmpSp.convertCopyToCartesian(true);       
          m_SpeakerPos_Cart=tmpSPC;
      }
      else m_SpeakerPos_Cart=*speakerPos;
  }
  init(); 
}


std::vector<double> vAdd(const std::vector<double> & a, const std::vector<double> & b)
{
 std::vector<double> retV(b.size());
 for(int i=0; i<(int)a.size(); i++)
    retV[i]=a[i]+b[i];
  return retV;
}

std::vector<double> vSub(const std::vector<double> & a, const std::vector<double> & b)
{
 std::vector<double> retV(b.size());
 for(int i=0; i<(int)a.size(); i++)
    retV[i]=a[i]-b[i];
  return retV;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
   simpleMtrx *  robustPan::calculateGains( spacePositions &sourcePositions, double beta, bool bNorm )
  {
    spacePositions sourcePosSp=  sourcePositions.convertCopyToSpherical(); 
    if(m_clipSourceHight)    // do hight and bottom clipping
        for (int ll=0; ll<sourcePosSp.getNumPositions(); ll++)
        {
            if (sourcePosSp[ll][1] > m_maxInclination)
                sourcePosSp[ll][1]=m_maxInclination;
            if (sourcePosSp[ll][1] < m_minInclination)
                sourcePosSp[ll][1]=m_minInclination;
        }
    spacePositions sourcePosCa=sourcePosSp.convertCopyToCartesian(bPLANEWAVEMODEL);
 
    int nL=m_SpeakerPos_Sp.getNumPositions();
    // create mix matrix and initialize with gain zero     
    m_gainMtrx.resize(m_SpeakerPos_Cart.getNumPositions(),sourcePosCa.getNumPositions());
    m_gainMtrx.zeros();
   
   std::vector<double> hms_r(NUM_VIRTMICS), hms_i(NUM_VIRTMICS);
   simpleMtrx BB(nL, nL);
   BB.zeros();

   simpleMtrx HmlTr = m_Hml_r.transposeKeep();
   simpleMtrx HmlTi = m_Hml_i.transposeKeep();    

   // helpers on heap
   simpleMtrx A(nL, nL);
   simpleMtrx C(nL, nL); 
   std::vector<double> b(nL);
   std::vector<double> d(nL);
   std::vector<double> g(nL);
   simpleMtrx r0(nL, nL);
   simpleMtrx T(nL, nL);
   simpleMtrx y00(nL, nL);
   simpleMtrx y01(nL, nL);

   for(int s=0; s<sourcePosCa.getNumPositions(); s++)
   {
          if (bPLANEWAVEMODEL) 
         {
             for (int m=0; m<NUM_VIRTMICS; m++)   // build plane wave transfer function source s to virtual microphones
             {
                 dcmplx cw(0.0, m_k *(  sourcePosCa[s][0]* m_virtMic_xyz[m][0] + sourcePosCa[s][1]* m_virtMic_xyz[m][1] + sourcePosCa[s][2]* m_virtMic_xyz[m][2] )  );
                 cw =exp(cw);
                 hms_r[m]= cw.real();
                 hms_i[m]= cw.imag();
             }
         } else throw(std::runtime_error("spherical wave model not implemented"));
  
         for(int l=0; l<nL; l++)  //  build regularization matrix -> increase costs if sound comes from other direction
         {
           // cos angular distance of plane wave directions of spherical projection of source and speakers:
             //cosGamma = cos(theta1).*cos(theta2)+sin(theta1).*sin(theta2).*cos(fi1-fi2);
            double cosGamma =   cos(sourcePosSp[s][1])*cos(m_SpeakerPos_Sp[l][1])
                              + (sin(sourcePosSp[s][1])*sin(m_SpeakerPos_Sp[l][1]) * cos(sourcePosSp[s][2]-m_SpeakerPos_Sp[l][2]));
             //BB=diag(1-(0.5+0.5.*cosGamma).^2)^2)
             double tmp=0.5+0.5*cosGamma;
             tmp = 1-tmp*tmp; tmp*=tmp;
             BB[l][l]=tmp*beta;
         } 
         //g=(H'*H+beta.* B'*B) \ HT h  - because H, h are complex there are more steps to do ... 
          A =  (HmlTr*m_Hml_r) + (HmlTi*m_Hml_i);
          A = A + BB;
          C =  (HmlTr*m_Hml_i) -( HmlTi*m_Hml_r);
          b =  vAdd( HmlTr.vectorMult(hms_r), HmlTi.vectorMult(hms_i));
          d =  vSub( HmlTr.vectorMult(hms_i), HmlTi.vectorMult(hms_r));
          SVD svdA(A);
          r0 = svdA.pinv() * C;
          T = (C*r0) +A;
          SVD svdT(T);
          y00 = svdT.pinv();
          y01 = r0 * y00;
          g = vAdd( y00.vectorMult(b), y01.vectorMult(d));
    
        
         if (bNorm)
         {      double gN=0.0; 
                 for(int l=0; l<nL; l++)
                 {
                    if ( g[l]<1e-6)g[l]=0.0; 
                    gN +=  g[l]*g[l];
                 }
                 gN=1.0/sqrt(gN);
                 for(int l=0; l<nL; l++)
                        g[l]=g[l]*gN;
                
        }
     
       for(int l=0; l<nL; l++)  
               m_gainMtrx[l][s]=g[l];  

   }
  
   return &m_gainMtrx; 
  }


//-----------------------------------------------------------------------------------------------------------------------------------------
double* robustPan::calculateGains( spacePositions  &sourcePositions, int * rows, int *cols, double beta, bool bNorm)
{
 calculateGains(sourcePositions, beta, bNorm );
 *rows= m_gainMtrx.getRows(); 
 *cols=m_gainMtrx.getCols();
 return m_gainMtrx.data();
}



//-----------------------------------------------------------------------------------------------------------------------------------------
extern const double vM1__theta[256];
extern const double vM1__phi[256]; 
void robustPan::init()
{ 
    m_SpeakerPos_Sp= m_SpeakerPos_Cart.convertCopyToSpherical();  
    //if (!m_is2D)  // 3D case
    {  
      if (m_vMradius==0.0)
      {
          double L=m_SpeakerPos_Cart.getNumPositions();
          //if(L<=3) L=4;//                           ToDo: fix RM= bug - negative radius
#ifdef RM0
          m_vMradius = (sqrt(L)-3)/m_k ;
#else
          m_vMradius = 0.4667*(sqrt(L)-1)/m_k ;
#endif
      }
      double r[NUM_VIRTMICS];
      for (int i=0; i<NUM_VIRTMICS; i++) r[i]=m_vMradius;
      spacePositions virtMic_rtp( NUM_VIRTMICS, r, (double*)vM1__theta, (double*) vM1__phi);             
       m_virtMic_xyz = virtMic_rtp.convertCopyToCartesian(false);
      m_Hml_r.resize(NUM_VIRTMICS, m_SpeakerPos_Cart.getNumPositions());          // plane wave transfer Matrix sources to virtual microphones @ m_k
      m_Hml_i.resize(NUM_VIRTMICS, m_SpeakerPos_Cart.getNumPositions());          // plane wave transfer Matrix sources to virtual microphones @ m_k
      if (bPLANEWAVEMODEL) 
      {
      for(int l=0; l<m_SpeakerPos_Cart.getNumPositions(); l++)
          for (int s=0; s<NUM_VIRTMICS; s++)
          {        // real==0, imag= vector_k *  vector_r'   
           dcmplx cw(0.0, m_k *(  m_SpeakerPos_Cart[l][0]* m_virtMic_xyz[s][0] + m_SpeakerPos_Cart[l][1]* m_virtMic_xyz[s][1] + m_SpeakerPos_Cart[l][2]* m_virtMic_xyz[s][2] )  );
           dcmplx expCw= exp(cw);
           m_Hml_r[s][l]=expCw.real();
           m_Hml_i[s][l]=expCw.imag();
          }
     } else throw(std::runtime_error("spherical wave model not implemented"));                
    } 
    /* else// (m_is2D)  //2D
    {
     throw(std::runtime_error("2D robustPan not implemented"));
       //radius= (L-3)/(2*k);  
   }*/
}



//-----------------------------------------------------------------------------------------------------------------------------------------
// virtual microphone positions
const double vM1__theta[256] = {
    1.57079632679489680000E+00,  7.48119159109959080000E-01,  1.61945615403142360000E+00,  2.79202777481181070000E+00,  2.21378862463945670000E+00, 
    8.20886230340621030000E-01,  1.78366759307903750000E+00,  1.89966204232731160000E+00,  8.46694489001306730000E-01,  9.08919744434375400000E-01, 
    1.23486701777455070000E+00,  1.28475595533830430000E+00,  1.19703970167530230000E+00,  1.02536998801980220000E+00,  2.55273286785724900000E+00, 
    2.02068115135229220000E+00,  1.82403823374224890000E+00,  1.28195289074642790000E+00,  1.59821490439981110000E+00,  1.72670925026131730000E+00, 
    1.74291591192842940000E+00,  2.78607853193511850000E+00,  1.25445042749003210000E+00,  2.19616766509190020000E+00,  1.45040081507041600000E+00, 
    8.86304355418684800000E-01,  1.34796573949331530000E+00,  1.31974357394380390000E+00,  5.36778657038771810000E-01,  2.44516550501147690000E+00, 
    1.29881453436940110000E+00,  1.36863814549139670000E+00,  2.76449305660717130000E+00,  1.68359013637863760000E+00,  1.80905832102253190000E+00, 
    8.97929578891351500000E-01,  1.51242759049367530000E+00,  2.17905612306645890000E+00,  2.14814783515823620000E+00,  3.62663579592911290000E-01, 
    1.25492294678909340000E+00,  5.06091846760101220000E-01,  2.01621528404819190000E+00,  4.87134791622984680000E-01,  2.16062397504249090000E+00, 
    1.50307410033655400000E+00,  8.77281274996954300000E-01,  7.37237541939032680000E-01,  1.22992573190677670000E+00,  2.06493922486461080000E+00, 
    2.40362308074574350000E+00,  1.05567104242901280000E+00,  1.26996982715664730000E+00,  1.51694817856438570000E+00,  2.14889363012526770000E+00, 
    2.80795713393542460000E+00,  2.41240214706776080000E+00,  2.15071948046109360000E+00,  1.36704457555164160000E+00,  9.76361527633288980000E-01, 
    1.57311990576673690000E+00,  1.49441480785821050000E+00,  2.27217337738413820000E+00,  1.94375332578003430000E+00,  2.53144017216910560000E+00, 
    1.52498206294358510000E+00,  4.89335250732200740000E-01,  2.86922465912841980000E+00,  2.63688849390844740000E+00,  8.20996434591139020000E-01, 
    1.34408505284471610000E+00,  1.41192092571787090000E+00,  2.64210582055518120000E-01,  8.77772229361328590000E-01,  1.31605281758952450000E+00, 
    9.52676376479111300000E-01,  1.87482884257886860000E+00,  3.03464670862183360000E+00,  1.74891805763971500000E+00,  1.60288457323762780000E+00, 
    1.74492153902291670000E+00,  1.34279446857697260000E+00,  1.95881572385654360000E+00,  1.98413902776156890000E+00,  2.15456083856762120000E+00, 
    6.46279343722800940000E-01,  8.41061239496152860000E-01,  1.43104283067086820000E+00,  1.73247987835623700000E+00,  1.55304628709043490000E+00, 
    1.05738562491374830000E+00,  2.29775054640864070000E+00,  1.51804668637035480000E+00,  1.60449983497696150000E+00,  4.35279663494976150000E-01, 
    4.79797493138415190000E-01,  1.48049404297564460000E+00,  5.96986460545488980000E-01,  1.16569125649428650000E+00,  2.64388183567918040000E+00, 
    1.80491248683970240000E+00,  2.39953084093261550000E+00,  1.45835665523166580000E+00,  2.30075409798660060000E+00,  1.77683358786067650000E+00, 
    1.04877693625333430000E+00,  6.50971697347227820000E-01,  1.58151770439975750000E+00,  1.00807016788544870000E+00,  2.36212766221545370000E+00, 
    2.89451184683254060000E+00,  2.07443089496168790000E+00,  7.61150323491820100000E-01,  1.69715262451773770000E+00,  1.64118131077173190000E+00, 
    2.43176307588484340000E+00,  1.69455895672002410000E+00,  1.89848683575320410000E+00,  1.45825417517530980000E+00,  7.17276054678394280000E-01, 
    9.61956638563927210000E-01,  1.10379975288701980000E+00,  1.70742613234629450000E+00,  2.76763943023283720000E-01,  9.56376391170976610000E-01, 
    1.27330850425617270000E+00,  1.10645228367897030000E+00,  1.23656026940377690000E+00,  5.81212327397610080000E-01,  2.58687208879854410000E+00, 
    1.19391488215604990000E+00,  2.56651120377705060000E+00,  2.23799643165630080000E+00,  1.66330333849810110000E+00,  7.17026295737644430000E-01, 
    2.78155672937854170000E+00,  2.14821097024715610000E+00,  5.98761697195070220000E-01,  2.05086861873314460000E+00,  2.05048547685481890000E+00, 
    2.30814053887293060000E+00,  1.57166273268729300000E+00,  2.05336060651986420000E+00,  2.45700890159390540000E+00,  1.82153556826414050000E+00, 
    1.92454644268586050000E+00,  1.73997160343250610000E+00,  1.67274716860974040000E+00,  1.79555717071882050000E+00,  8.02111767881039550000E-01, 
    1.46700134189359050000E+00,  2.09430623177907860000E+00,  2.62003006821309770000E+00,  1.47618481229293660000E+00,  2.55494620717448480000E+00, 
    6.27269606317793600000E-01,  1.42041119471218180000E+00,  1.37623093952681020000E+00,  2.52459697435483690000E-01,  8.24111524008836690000E-01, 
    1.82711601623519630000E+00,  4.27304994570138540000E-01,  1.91273775653360010000E+00,  2.57715674479757250000E+00,  1.64683465494025480000E+00, 
    1.35989339414535190000E+00,  8.45721343067333820000E-01,  1.42797815647402790000E+00,  2.56445810193711490000E-01,  1.85408086658177470000E+00, 
    7.97352381788569930000E-01,  1.17283784880819700000E+00,  1.97579156953869120000E+00,  1.07827074174899700000E+00,  1.15986771374562280000E+00, 
    1.98315196412113590000E+00,  2.72597873707606770000E+00,  4.07947605442086360000E-01,  1.99419551789364210000E+00,  1.04344497910873410000E+00, 
    1.58418244471760270000E+00,  5.63047937270352230000E-02,  2.66402131680853450000E+00,  1.97715295284895660000E+00,  2.17974977803242440000E+00, 
    2.19490486008502960000E+00,  1.10588745232704900000E+00,  2.56402197986373630000E+00,  2.41640927909702260000E+00,  6.25955308593053590000E-01, 
    1.84252386990174370000E+00,  7.22587551606499570000E-01,  6.59037481867101200000E-01,  1.61338161811008170000E+00,  6.51144769296404190000E-01, 
    1.12044633528560020000E+00,  1.38775635352633530000E+00,  2.81340370436412050000E+00,  1.93254259152481980000E+00,  2.25300620895676220000E+00, 
    4.37798701051926420000E-01,  2.29709128976811880000E+00,  1.13545627435409480000E+00,  1.03447775267640710000E+00,  2.13759209373244730000E+00, 
    2.37824439897954680000E+00,  2.41037883376273370000E+00,  1.54639086438495640000E+00,  2.30171018453533630000E+00,  1.98127287984443660000E+00, 
    9.48982472423353980000E-01,  1.21365381157918280000E+00,  7.99002519437832630000E-01,  1.90427776780085820000E+00,  6.34266954331849870000E-01, 
    3.78834182935344270000E-01,  2.37767588225043400000E+00,  1.68325194312385600000E-01,  9.47085874163107300000E-01,  2.98201672753081360000E+00, 
    1.22628608508116610000E+00,  1.34051422527300850000E+00,  2.39952585415137460000E+00,  1.12788301584038900000E+00,  1.80106308478685690000E+00, 
    1.86488637387984220000E+00,  2.03539602106036450000E+00,  6.56569275660826230000E-01,  1.29279362574528170000E+00,  2.25799055232518240000E+00, 
    2.67335621698771100000E+00,  2.01698670297180540000E+00,  1.91654489619639980000E+00,  2.31683420400382060000E+00,  9.02994428081540620000E-01, 
    2.98834363339483170000E+00,  1.73046826785007110000E+00,  1.70500664117277840000E+00,  2.23976515554358930000E+00,  1.10858144197003420000E+00, 
    2.15499789088186720000E+00,  1.83493009048537850000E+00,  2.06057134818343670000E+00,  2.46983761506714790000E+00,  1.49838462677650570000E+00, 
    1.06754948578989840000E+00,  1.19058565631161510000E+00,  1.39493770707000200000E+00,  1.01012078383716060000E+00,  9.64725567203483240000E-01, 
    2.16510258035889720000E-01,  1.07964216126216410000E+00,  2.61859242789309210000E+00,  2.51782035058782010000E+00,  1.65206753601840410000E+00, 
    4.44298511413458500000E-01, 
};

const double vM1__phi[256] = {
    3.14159265358979310000E+00,  3.14159265358979310000E+00,  -3.38233155670188580000E-01,  1.19263248667349390000E+00,  1.78774514961512090000E+00, 
    3.94277076647476850000E-01,  1.88058232529973850000E+00,  -2.87486999975822280000E+00,  1.58994151524032620000E+00,  1.13832974961638950000E-01, 
    5.30224207719510400000E-01,  -1.28047263282294240000E-01,  2.21295050532664740000E+00,  -1.24025055302630120000E-01,  1.28029192282326140000E+00, 
    3.09254810436726590000E-01,  2.10962502236164000000E+00,  1.58893731727769840000E+00,  2.06941433484836250000E+00,  -1.65205337507968350000E+00, 
    2.54544239768215740000E+00,  2.61325478178388780000E+00,  -1.64340604756801990000E+00,  2.42008985950056400000E+00,  -7.87399119721336360000E-01, 
    1.89086511553808270000E+00,  -1.84297180695540750000E+00,  1.81225921830781210000E+00,  2.96341433570878720000E+00,  1.92251164616041610000E+00, 
    -5.64741951054629650000E-01,  2.03396465900577720000E+00,  -2.91315347685843970000E+00,  5.37346826711131990000E-01,  -3.10434763878932780000E+00, 
    -2.48782826016660200000E+00,  1.62439279974813440000E+00,  3.00048685978168050000E+00,  9.66038612872416320000E-02,  -2.94936178430753590000E+00, 
    1.35925960542501680000E+00,  -1.27701322927373070000E-01,  7.75895532128443870000E-01,  -1.21602994617029350000E+00,  1.04221272038345840000E+00, 
    -2.27649213476922350000E+00,  1.26575454613203810000E+00,  2.78222247379213570000E+00,  -7.97473213352573370000E-01,  -1.46087541350059920000E+00, 
    2.29285565918308090000E+00,  -1.75259308256533020000E+00,  -2.25032253430396610000E+00,  -1.23336596128441780000E-01,  -2.80309585737914710000E+00, 
    -2.28780269766695720000E+00,  5.98519772207203980000E-01,  -1.21234571516480070000E+00,  -1.43371077586454980000E+00,  -1.99802702006227490000E+00, 
    7.47334946232573790000E-01,  -1.63711817097093770000E+00,  -9.29514316789426420000E-01,  2.98169374290159930000E+00,  2.58501915226974120000E+00, 
    -5.62882618295368900000E-01,  9.87158768677366630000E-01,  1.91333027971797790000E+00,  2.17246102739136980000E+00,  -6.56786646513584980000E-01, 
    7.49849547423947160000E-01,  8.93381778594410230000E-02,  9.04606196035349530000E-01,  -1.54533067929257180000E+00,  3.02833725352818550000E-01, 
    2.68574098956293560000E+00,  1.13789634683823210000E-01,  8.82273152732705900000E-01,  -5.53226492715780730000E-01,  1.18997035162157540000E+00, 
    -1.08090884129857890000E-01,  3.12583084761836050000E+00,  -1.68292681616760680000E+00,  -8.89741213680505380000E-02,  1.51925901570025990000E+00, 
    1.96139030082292860000E-01,  7.33869555631094460000E-01,  -2.05952476563115280000E+00,  -2.28047230769678630000E+00,  1.84631267508492240000E+00, 
    2.42034428861174970000E+00,  3.12233575158255840000E-01,  2.50136882298675460000E+00,  -1.42922388689747830000E+00,  -6.53011511459029910000E-01, 
    2.05885123572013650000E+00,  9.57553941825364800000E-01,  -2.03763624330793960000E+00,  2.66141673755585200000E+00,  1.68437049941891210000E+00, 
    -2.49678132418058670000E+00,  2.96424986071720560000E+00,  2.92357917300877100000E+00,  -1.48123000895716330000E+00,  3.24953302370264600000E-01, 
    1.46307657915689580000E+00,  1.76745774241044470000E+00,  -2.49867677551312320000E+00,  -8.13460093474500990000E-01,  -2.66912550467616900000E+00, 
    -3.62426553772725310000E-01,  -3.10578956642281690000E-01,  -1.21707136833319920000E-01,  -1.21598934588177650000E+00,  1.01317587381112020000E-01, 
    5.64733634734549480000E-02,  1.42563942970504500000E+00,  -7.70324877671997750000E-01,  5.26583028378400100000E-01,  2.12735057657464300000E+00, 
    9.81613685577758850000E-01,  -1.46829051474942780000E+00,  9.82404507749796490000E-01,  -1.24912805484685020000E-01,  2.16763628261714820000E+00, 
    -2.70752543698461870000E+00,  -1.04474490274085090000E+00,  9.74509570201486540000E-01,  -2.85804232977289760000E+00,  -2.52035631996872620000E+00, 
    -2.00850679225751570000E+00,  -1.40116340286355710000E+00,  7.83096465105973860000E-01,  -2.06483896522639120000E+00,  1.01152454895076270000E+00, 
    -1.10126312921970660000E+00,  5.26119421125967550000E-01,  2.47785071694334520000E+00,  -3.05882077907637310000E+00,  2.21797957949341960000E+00, 
    2.66799987454084290000E+00,  -1.00074161053306750000E+00,  1.26987903540299610000E+00,  -1.73727733504186890000E+00,  -1.88014381567600510000E+00, 
    -1.21767779491173900000E+00,  1.65604127727832770000E+00,  -7.79472428932382560000E-01,  7.65440536295810550000E-01,  -2.19619818451802470000E+00, 
    -1.21868457910781780000E+00,  2.75786910196225630000E+00,  8.19279516427057210000E-01,  1.39749835477230770000E+00,  -9.64604094451374070000E-01, 
    -1.60425939824598920000E+00,  -2.93308664171005650000E+00,  1.16335452506401250000E+00,  1.83835026255348470000E+00,  2.43203475578229660000E+00, 
    1.22401065664099700000E+00,  4.05953106954272780000E-01,  5.40144236255409190000E-01,  3.57239964605872040000E-01,  2.30149626973443540000E+00, 
    -2.48042644238844990000E+00,  -1.00730282098106970000E+00,  2.26080411938045870000E+00,  -1.10831240850022720000E+00,  2.75622163981889570000E+00, 
    -1.83344082565915590000E+00,  7.98457354443453670000E-02,  1.71142980582194020000E+00,  1.72459462942771700000E+00,  -3.35761010827155980000E-01, 
    -2.63745382090245030000E+00,  -1.72590789160068890000E+00,  -1.74346159083833020000E+00,  2.53909126204389680000E+00,  -2.25391041708895920000E+00, 
    -1.85099833165654840000E+00,  4.92973446828996260000E-02,  -5.65280389447550660000E-01,  -5.36368147855831650000E-01,  -2.49009794256737480000E+00, 
    -1.72738264435041190000E+00,  1.18037416175601510000E+00,  -2.08805666558149560000E+00,  -6.75649749805211420000E-01,  6.20730265273844010000E-01, 
    -3.20711339605275310000E-01,  -1.27757585918209940000E+00,  -2.47093177942271410000E+00,  2.73916249012879430000E+00,  -4.47169712875179760000E-01, 
    3.11017336344553950000E+00,  2.69865282673450400000E+00,  4.60839199364657730000E-01,  1.00969344864302050000E+00,  -4.68925349708950610000E-01, 
    -2.38087159883356850000E+00,  -3.01600877158933000000E+00,  -2.49101315063431490000E+00,  -2.73233946250660600000E+00,  -7.15230316631931040000E-01, 
    -1.19328755653602210000E+00,  9.89157502900199330000E-01,  3.11744187484241990000E-01,  1.28565463545209590000E+00,  -2.35034776485292610000E+00, 
    -3.00107949834506370000E+00,  2.89143038437328940000E+00,  -2.77395439508832190000E+00,  -2.11372988718771370000E+00,  -8.78756140024239740000E-01, 
    2.58022444568272660000E+00,  -2.31982814796406340000E+00,  2.89477763929125500000E+00,  2.95561528910556380000E+00,  -1.56462615576563020000E+00, 
    -1.24899094232993480000E+00,  -1.01654353643395700000E+00,  1.58033667395342900000E+00,  1.97272114765284010000E+00,  -9.97425731523713060000E-01, 
    2.34657634708557870000E+00,  -9.76881444997446670000E-01,  1.36768933089669020000E+00,  2.45367664413740360000E+00,  -1.46702708847078450000E-01, 
    -6.24540788598025660000E-02,  1.95588242767270430000E+00,  1.47356629518574600000E+00,  -1.99296598886383180000E+00,  -3.64134263626974040000E-01, 
    3.09595399513199520000E+00,  -2.71269557548744130000E+00,  2.95347501326734820000E+00,  2.07725486786709460000E+00,  7.74735994356091420000E-01, 
    -2.18934732638894980000E+00,  -1.44215021619825050000E+00,  -1.93523218168275090000E+00,  -3.11059408897301940000E-01,  -2.71472343656252010000E+00, 
    -5.63167371762394400000E-01,  -2.93429637322231060000E+00,  -3.45056805433832210000E-01,  5.45683243282094280000E-01,  -1.26189034461232930000E+00, 
    -2.14498458744113000000E+00,  3.01625347119371060000E-01,  2.97392257256094260000E+00,  -2.92983390950740260000E+00,  -2.92511343432637630000E+00, 
    1.50187365452353380000E+00, 
};
//-----------------------------------------------------------------------------------------------------------------------------------------
