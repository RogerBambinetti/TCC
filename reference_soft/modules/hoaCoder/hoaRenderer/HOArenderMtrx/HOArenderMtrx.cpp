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
/*   HOArenderMtrx.cpp 
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    calculate a HOA rendering matrix or load from file or table
* uses:             spacePositions to handle the speaker positions, robustPan to calculate the panning functions
*                 
* authors:          Johannes Boehm (jb), 
* log:              created 23.08.2013, 
*/
/*
$Rev: 203 $
$Author: technicolor-ks $
$Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
$Id: HOArenderMtrx.cpp 203 2016-01-19 13:45:41Z technicolor-ks $
*/

#include "HOArenderMtrx.h"
#include "robustPan.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "modeMtrxRom.h"
using namespace std;


//-----------------------------------------------------------------------------------------------------------------------------------------
// construct an empty object 
 HOArenderMtrx::HOArenderMtrx()
 : m_Lspeakers(0), m_uiNumSubwoofer(0),	m_uiNumSatellites(0), m_HOAorder(0), m_cols(0), m_renderID(0)
{
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// construct from file
HOArenderMtrx::HOArenderMtrx( unsigned long renderId, const std::string &matrixFileName,  int addNlfeCh, bool reTwiddleHOAcoeffs )
: m_renderID(renderId)
{
  string line;
  ifstream myfile (matrixFileName);
  if (myfile.is_open())
    {
         getline (myfile,line);
         m_Lspeakers = atoi(line.c_str());
         getline (myfile,line);
         m_cols = atoi(line.c_str());
         m_HOAorder = int(sqrt(double(m_cols)))-1;
         m_Dmatrix.resize(m_Lspeakers+addNlfeCh, m_cols); 
        for (int c=0; c< m_cols; c++)
         {
            for (int r=0; r<m_Lspeakers; r++)
            {
              if (!myfile.good()) throw(std::runtime_error("Error reading Matrix file"));
              getline (myfile,line);
              m_Dmatrix[r][c] = atof(line.c_str());
            }
            
            for (int l=0; l<addNlfeCh; l++)
                    m_Dmatrix[m_Lspeakers+l][c] =0.0;  
         }
        if(reTwiddleHOAcoeffs) retwiddle();
		 m_uiNumSatellites = m_Lspeakers;
		 m_uiNumSubwoofer = addNlfeCh;
         m_Lspeakers+=m_uiNumSubwoofer;
		 
    }
    else throw(std::runtime_error("Cannot open Matrix file"));
  
}
//-----------------------------------------------------------------------------------------------------------------------------------------

 HOArenderMtrx::HOArenderMtrx(const std::string &matrixFileName)
: m_renderID(0)
{
  string line;
  ifstream myfile (matrixFileName);
  if (myfile.is_open())
    {
         getline (myfile,line); // # Number of Loudspeaker
         getline (myfile,line);
         m_Lspeakers = atoi(line.c_str());
         getline (myfile,line); // # Number of HOA Coefficients
         getline (myfile,line);
         m_cols = atoi(line.c_str());
         m_HOAorder = int(sqrt(double(m_cols)))-1;
         getline (myfile,line); // # CICP Loudspeaker Config
         m_vvFsignaledSpeakerPosRad.resize(m_Lspeakers);
         m_uiLsTypes.resize(m_Lspeakers);
         unsigned int spkIdx = 0;		
         unsigned int parseOffset = 0;	
         m_uiNumSubwoofer = 0;
         m_uiNumSatellites = 0;
         for (int c=0; c< m_Lspeakers; c++){
             getline (myfile,line);
             // parse speaker position
             if (line.compare(0,4,"LFE_") != 0) {
                 m_uiLsTypes[c] = 0;  //satellite = 0
                 m_uiNumSatellites++;
                 parseOffset = 0;	
             }
             else{
                 m_uiLsTypes[c] = 1; //subwoofer = 1
                 m_uiNumSubwoofer++;
                 parseOffset = 4;
             }
             string sR = line.substr(1+parseOffset,4);
             string sF = line.substr(7+parseOffset,4);
             string sT = line.substr(13+parseOffset,3);
             m_vvFsignaledSpeakerPosRad[spkIdx].resize(3);
             m_vvFsignaledSpeakerPosRad[spkIdx][0] = atof(sR.c_str())/100.0;
             m_vvFsignaledSpeakerPosRad[spkIdx][1] = (90-atof(sT.c_str())) *PI/180.0;
             m_vvFsignaledSpeakerPosRad[spkIdx][2] = atof(sF.c_str())*PI/180.0;  
             spkIdx++;
         }
         //
         getline (myfile,line); // # HOA Matrix Values
         m_Dmatrix.resize(m_Lspeakers, m_cols); 
        for (int c=0; c< m_cols; c++)
         {
            for (int r=0; r<m_Lspeakers; r++)
            {
              if (!myfile.good()) throw(std::runtime_error("Error reading Matrix file"));
              getline (myfile,line);
              m_Dmatrix[r][c] = atof(line.c_str());
            }   
         }
    }
    else throw(std::runtime_error("Cannot open Matrix file"));
  
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// construct from table
HOArenderMtrx::HOArenderMtrx( unsigned long renderId, double *pTable, int hoaOrder, int nChannels, int addNlfeCh, bool reTwiddleHOAcoeffs)
: m_Lspeakers(nChannels), m_HOAorder(hoaOrder), m_cols((hoaOrder+1)*(hoaOrder+1)), m_renderID(renderId)
{
  m_Dmatrix.resize(m_Lspeakers+addNlfeCh, m_cols); 
  for (int c=0; c< m_cols; c++)
    {
        for (int r=0; r<m_Lspeakers; r++)
             m_Dmatrix[r][c] = *pTable++;        
        for (int l=0; l<addNlfeCh; l++)
            m_Dmatrix[m_Lspeakers+l][c] =0.0;  
    }
    if(reTwiddleHOAcoeffs) retwiddle(); 
    m_uiNumSatellites = m_Lspeakers;
	m_uiNumSubwoofer = addNlfeCh;
    m_Lspeakers+=m_uiNumSubwoofer;
}


//-----------------------------------------------------------------------------------------------------------------------------------------
// construct from spacePositions object
  HOArenderMtrx::HOArenderMtrx(spacePositions *speakerPos, int HOAorder, unsigned long renderID, int addNlfeCh)
  : m_HOAorder(HOAorder), m_renderID(renderID), m_cols((HOAorder+1)*(HOAorder+1))
  {
    init(speakerPos, addNlfeCh); 
  }

//-----------------------------------------------------------------------------------------------------------------------------------------
// construct from speaker positions in file
 HOArenderMtrx::HOArenderMtrx(const std::string &fileNameWithLoudSpeakerPos, int fileType, int HOAorder, unsigned long renderID, bool addlfeCh)
  : m_HOAorder(HOAorder), m_renderID(renderID), m_cols((HOAorder+1)*(HOAorder+1))
  {
    spacePositions speakerPos( fileNameWithLoudSpeakerPos, fileType);	
    int addNlfeCh=0;
    if (addlfeCh)
      {
       spacePositions lfePos( fileNameWithLoudSpeakerPos, fileType, true);
       addNlfeCh= lfePos.getNumPositions();  
      }
    init(&speakerPos, addNlfeCh);

	m_uiLsTypes.assign(m_Lspeakers,0);
	if (addNlfeCh)
	{
		unsigned int spkIdx = 0;
		std::ifstream file(fileNameWithLoudSpeakerPos);
		while(file.good())
		{
			std::string line;
			std::getline(file, line);
			if(line.size() > 3) {			
				if(line.compare(0, 4, "LFE_")==0)
					m_uiLsTypes[spkIdx]=1; //subwoofer = 1
				spkIdx++;
			}
		}
	}	
  }


//-----------------------------------------------------------------------------------------------------------------------------------------
void getHOAWindow(double*pw, unsigned winLen, unsigned HOAoder, unsigned numberOfLoudSpeakers);  // declaration
//-----------------------------------------------------------------------------------------------------------------------------------------
// calculate the rendering matrix from speaker positions
void HOArenderMtrx::init(spacePositions *speakerPos, int addNlfeCh)
{
   bool special2Dhandling=false;
   if (speakerPos->is2D())    // special 2D handling
     {
      special2Dhandling=true;
      speakerPos->addPoles();
     }    
    m_Lspeakers = speakerPos->getNumPositions();
    m_Dmatrix.resize(m_Lspeakers, m_cols); 

    // build a mixing matrix G of equal distributed directions (on the unit sphere) to the speaker setup 
    robustPan rPan(speakerPos);
    double r [N_PSI_POINTS ];
    for (int i=0; i<N_PSI_POINTS; i++) r[i]=1;
    spacePositions virtSorces_rtp( N_PSI_POINTS, r, (double*)vS1__theta, (double*) vS1__phi);   
    double beta;
    {
        const double betaTab[5]={1000,1000,100,100,50};
        int f=m_HOAorder-1;
        if(f>4) f=4;
        beta=betaTab[f];
    }
    simpleMtrx * pG = rPan.calculateGains(virtSorces_rtp,beta );    

    // read from ROM a spherical mode matrix of the equal distributed positions and map it to an Eigen matrix
    double *pdPsi = reinterpret_cast  <double*> (psiN6);  // this loads the unsigned char table as a double table
    if (m_HOAorder>6) throw(std::runtime_error("HOArenderMtrx: ModematrixTables only provided up to HOA order 6"));
    simpleMtrx mPsi(m_cols, N_PSI_POINTS, pdPsi );

    // Perform an SVD of mPsi * pG->transpose()    
    pG->transpose();
    SVD svd( mPsi * (*pG) );

    // every singular value is forced to one for building the energy preserving pseudo inverse, 
    // except if the singular value is 60db below the strongest singular value (suppressed)
    // this is done to suppress left-right hemisphere asymmetries 
    int numS= svd.w_s.size();
    int cnt;
    const double thresh=1e-3*svd.w_s[0];  
    for (cnt=1; cnt<numS; cnt++)
        if (svd.w_s[cnt]<thresh) break;
    simpleMtrx I(numS, numS); I.setIdentity(); 
    if(cnt<numS)
        for (cnt; cnt<numS; cnt++) I[cnt][cnt]=0.0;

    // build energy preserving decoding matrix proto type
    svd.U.transpose();
    m_Dmatrix =  svd.V * I * svd.U;

    //  window in coefficient domain to attenuate rendering side/back lobes 
    std::vector<double>  ew(m_cols);    
    getHOAWindow(ew.data(), m_cols, m_HOAorder, m_Lspeakers);  // get window  - scaling factors per HOA order  
    simpleMtrx eW(m_cols,m_cols); eW.zeros();
    for(int c=0; c<m_cols; c++) eW[c][c]=ew[c];   
    m_Dmatrix = m_Dmatrix * eW;                   //window

    // normalize rendering matrix to preserve energy
    double scale=1.0 / m_Dmatrix.normFro();
    m_Dmatrix.multScalar(scale); 

    if(special2Dhandling)  // 2D handling
    {   
      m_Lspeakers = m_Lspeakers-2;
      simpleMtrx M(m_Lspeakers, m_cols);
      double gain=1./sqrt(double(m_Lspeakers));      
      for(int r=0; r<m_Lspeakers; r++)  // down mix virtual speaker in matrix form 
         for(int c=0; c<m_cols; c++)
            M[r][c] =  m_Dmatrix[r][c] + gain*( m_Dmatrix[m_Lspeakers][c] + m_Dmatrix[m_Lspeakers+1][c]);          
      M.multScalar( (1.0/M.normFro()));
      m_Dmatrix = M;         // normalize again
    }

    if (addNlfeCh>0)  // add zero mixing coefficients to the end of the matrix (muted LFEs)
    {
    simpleMtrx  Mtmp=m_Dmatrix;
    m_Dmatrix.resize(m_Lspeakers+addNlfeCh, m_cols); 
	for (int c=0; c< m_cols; c++)
	{
		for (int r=0; r<m_Lspeakers; r++)                   
			m_Dmatrix[r][c] = Mtmp[r][c];    
		for (int l=0; l<addNlfeCh; l++)
			m_Dmatrix[m_Lspeakers+l][c] =0.0;  
	}
    }
	m_uiNumSatellites = m_Lspeakers;
	m_uiNumSubwoofer = addNlfeCh;
	m_Lspeakers+=m_uiNumSubwoofer;
}

//-----------------------------------------------------------------------------------------------------------------------------------------
// resort HOA coefficients in render matrix from MPEG4 sequence ordering to linear sequence ordering
void HOArenderMtrx::retwiddle()
{
    // create an index vector to resort HOA coefficients in MPEG4 sequence ordering to linear ordering 
    int cnt=1;;
    std::vector<int> indxVec(m_cols);
    indxVec[0]=1;
    for (int n=1; n< m_HOAorder+1; n++)
    {
        int k=n*n+2;
        indxVec[cnt++]=k;
        for(int m=1; m<n; m++)
        {
            k+=2;
            indxVec[cnt++]=k;
        }
        k++;
        indxVec[cnt++]=k;
        for(int m=1; m<n+1; m++)
        {
            k-=2; 
            indxVec[cnt++]=k;
        }
    }  
  // resort the matrix     
  simpleMtrx  Mtmp=m_Dmatrix;
  for(int c=0; c< m_Lspeakers; c++)
      for(int v=0; v<m_cols; v++)
      {
          int idxx=indxVec[v]-1;
          m_Dmatrix[c][v]=Mtmp[c][idxx];
      }
}
//-----------------------------------------------------------------------------------------------------------------------------------------
 int HOArenderMtrx::resortSpeakerPositions(const std::vector< unsigned int> order)
 {
     if ( (order.size() != m_uiLsTypes.size()) || (order.size() != m_vvFsignaledSpeakerPosRad.size()) )
         return 0;
     else 
     {   //m_Lspeakers 
         std::vector<unsigned int> tempLsType;
         tempLsType.resize(m_Lspeakers);

         std::vector<std::vector<double >> tempSignaledSpeakerPosRad;
         tempSignaledSpeakerPosRad.resize(m_Lspeakers);

         for (int n=0; n<m_Lspeakers; ++n)
         {
             tempLsType[order[n]] =  m_uiLsTypes[n];
         }
         m_uiLsTypes = tempLsType;

         for (int n=0; n<m_Lspeakers; ++n)
         {
             tempSignaledSpeakerPosRad[order[n]].resize(3); // [radius, inclination, azi]
             for (unsigned int m=0; m<3; ++m)
             {
                 tempSignaledSpeakerPosRad[order[n]][m] =  m_vvFsignaledSpeakerPosRad[n][m];
             }
         }
         m_vvFsignaledSpeakerPosRad = tempSignaledSpeakerPosRad;
         return 1;
     }
     
 }

//-----------------------------------------------------------------------------------------------------------------------------------------
// helper functions
//-----------------------------------------------------------------------------------------------------------------------------------------


  // BesselI0 -- Regular Modified Cylindrical Bessel Function (Bessel I).

  double BesselI0(double x) {
      double denominator;
      double numerator;
      //double z;

      if (x == 0.0) {
          return 1.0;
      } else {
          double z = x * x;
          numerator = (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* (z* 
              (z* 0.210580722890567e-22  + 0.380715242345326e-19 ) +
              0.479440257548300e-16) + 0.435125971262668e-13 ) +
              0.300931127112960e-10) + 0.160224679395361e-7  ) +
              0.654858370096785e-5)  + 0.202591084143397e-2  ) +
              0.463076284721000e0)   + 0.754337328948189e2   ) +
              0.830792541809429e4)   + 0.571661130563785e6   ) +
              0.216415572361227e8)   + 0.356644482244025e9   ) +
              0.144048298227235e10);

          denominator = (z*(z*(z-0.307646912682801e4)+
              0.347626332405882e7)-0.144048298227235e10);
      }

      return -numerator/denominator;
  }


//-----------------------------------------------------------------------------------------------------------------------------------------
 // create a kaiser window 
  void kaiserWindow(double *pw, unsigned fullWinLength,  double alpha)
  {
      double denominator = abs(BesselI0(alpha));
      int odd=fullWinLength%2;
      double den2= (fullWinLength-1)*(fullWinLength-1);
      unsigned nn= (odd==1)? (fullWinLength+1)/2 : fullWinLength/2;
      int startIdx= (odd==1)? nn-1:nn;
      for ( int i=0; i<(int)nn; i++)
      {
          double x = i + 0.5*(1-odd);
          x= 4*x*x;
          pw[startIdx+i]= abs(  BesselI0(alpha*sqrt(1-x/den2))/denominator); 
      }
      for ( int i=0; i<startIdx; i++)
          pw[i]=pw[fullWinLength-1-i];

  }


//-----------------------------------------------------------------------------------------------------------------------------------------
//legendrePolinomal - calculates value of Legendre polynomial of order n_polOrder for argument x, 
  double  legendrePolinomal(double x, unsigned n_polOrder)
  {
    unsigned et=8;
    double retVal = 0;
    switch (n_polOrder)
    {
    case 0: { retVal=1;	break;}
    case 1: { retVal=x;	break;}
    case 2: { retVal=0.5*(3.*x*x - 1); break;}
    case 3: { retVal=0.5*(5*x*x*x - 3*x);	break;}
    case 4: { retVal=1./8 *(35*x*x*x*x-30*x*x+3) ;	break;}
    case 5: { retVal=1./8 *(63*x*x*x*x*x-70*x*x*x+15*x);	break;}
    case 6: { retVal=1./16 *(231*x*x*x*x*x*x -315*x*x*x*x+105*x*x -5);	break;}
    case 7: { retVal=1./16 *429*x*x*x*x*x*x*x -693*x*x*x*x*x+315*x*x*x-35*x;	break;}
    case 8: { retVal=(6435*x*x*x*x*x*x*x*x-12012*x*x*x*x*x*x+6930*x*x*x*x-1260*x*x+35)/128 ;	break;}
    default: {  // recursive calculation from here on
              double p9=legendrePolinomal(x, et-1);
              double p10=legendrePolinomal(x, et);
              for (int m=et+1; m< (int)n_polOrder+1; m++)
                 {
                  retVal=1./m*(2.*m-1)*x*p10-(m-1)*p9;
                  p9=p10;
                  p10=retVal;
                 }
              }
    }
    return retVal;
  }

//----------------------------------------------------------------------------------------------------------------------------------------- 
// create a maxRE window for 3D
// fullWinLength == HOA order +1 
 void maxRE3D_win(double *pw, unsigned fullWinLength)
 {
  double zerosOfP[12] = {0.5574, 0.7746, 0.8611, 0.9062, 0.9325, 0.9491,  0.9603,0.9682, 0.9739,  0.9782, 0.9816, 0.9842 };
  int N=fullWinLength-1;
  if (N>12) throw(std::runtime_error("maxRE3D_win: only HOA orders up to 12 supported"));
  pw[0]= 1; 
  double  rE=zerosOfP[N-1];
  for(int n=1; n<N+1; n++)
  {
    pw[n]= legendrePolinomal(rE,n);
  }
 }
 
 //-----------------------------------------------------------------------------------------------------------------------------------------
 // create the HOA window. If the number of loudspeakers is larger compared to the number of HOA
 // coefficients, a maxRE window of HOAorder is selected, else - more attenuation of higher order coefficients
 // is needed, which is provided by a kaiser window with alpha=HOAoder+1
 void getHOAWindow(double*pw, unsigned winLen, unsigned HOAoder, unsigned numberOfLoudSpeakers)
 {
  int O_HOA= (HOAoder+1)*(HOAoder+1);
  if ((int) winLen !=O_HOA)
    throw(std::runtime_error("getHOAWindow: window length and HOA order - wrong relationship "));
  double *wT= new double[2*HOAoder+1];
  int strtIx;
  if (numberOfLoudSpeakers<(unsigned)O_HOA)
     { 
       kaiserWindow(wT,2*HOAoder+1, HOAoder+1); // only right half window is needed
       strtIx=HOAoder;
     }
  else
    {
      maxRE3D_win(wT, HOAoder+1); // only half window is needed
      strtIx=0;
    }
   pw[0]=wT[strtIx];
   int c=1, repetitions;
   for (int n=1; n<(int)HOAoder+1; n++)
   {
     repetitions=2*n+1;
     for (int m=0; m< repetitions; m++)
        pw[c+m]=wT[strtIx+n];
    c=c+repetitions;
   }
  delete [] wT;
 }

//-----------------------------------------------------------------------------------------------------------------------------------------

