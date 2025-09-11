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
/*   spacePositions.cpp                                     
*                   Copyright (c) 2013, Technicolor Germany, Deutsche Thomson OHG (DTO)
*                        All Rights Reserved. Technicolor Confidential.
* project:          3D_Audio - MPEG H
* functionality:    hold spherical or Cartesian position coordinates
* authors:          Johannes Boehm (jb), 
* log:              created 17.07.2013, 
*/

/*
$Rev: 157 $
$Author: technicolor-kf $
$Date: 2015-01-16 15:00:35 +0100 (Fr, 16 Jan 2015) $
$Id: spacePositions.cpp 157 2015-01-16 14:00:35Z technicolor-kf $
*/

#include "spacePositions.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

using namespace std;

//--------------------------------------------------------------------------------------------------------------------------
spacePositions::spacePositions()
: m_numPositions( 0), m_Cols(0), m_type(0),  m_pArr(NULL)
{}

//--------------------------------------------------------------------------------------------------------------------------
 spacePositions::spacePositions( int numberOfPositions, double *radius, double *speakerInclination, double *speakerAzimuth)
 : m_numPositions( numberOfPositions), m_Cols(3), m_type(1)

 {
    m_pArr = new double [3 * m_numPositions];
  
   for (int i=0; i<m_numPositions; i++)
   {
    if(radius==NULL)
        m_pArr[i*m_Cols]=1;
    else
        m_pArr[i*m_Cols]=radius[i];
    m_pArr[i*m_Cols+1]=speakerInclination[i];
    m_pArr[i*m_Cols+2]=speakerAzimuth[i];
   }
 }


//--------------------------------------------------------------------------------------------------------------------------
spacePositions:: spacePositions( const std::string &fileName, int fileType, bool b_forceLFE)
{
    string line;
    ifstream myfile (fileName);
    // get number of lines in file
    if (myfile.is_open())
    {
        int  cnt=0;
        while ( myfile.good() )
        {
            getline (myfile,line);
            cnt++;
        }
         myfile.close();
        //myfile.seekg (0, ios::beg);
        //myfile.clear();
        ifstream myfile (fileName);
        if (fileType==1)  // ASCCII string m_type positions
        {
          m_pArr = new double [3 * cnt];
          m_Cols=3;
          m_numPositions=0;
          m_type=1;
          while ( myfile.good() )
          {
              getline (myfile,line);
              if (line.compare(0,4,"LFE_") != 0      && !b_forceLFE)
              {   
                if(line.size()>15)   // feature fix 03.12.13 jb - this helps to skip blank lines
                {         
                if (line.compare(0,1,"R") != 0)
                    throw(std::runtime_error("file error - ASCCII positions"));
                string sR= line.substr(1,4);
                if (line.compare(5,2,"_A") != 0)
                    throw(std::runtime_error("file error - ASCCII positions"));
                string sF= line.substr(7,4);
                if (line.compare(11,2,"_E") != 0)
                    throw(std::runtime_error("file error - ASCCII positions"));
                string sT= line.substr(13,3);
                m_pArr[m_numPositions*m_Cols]  =atof(sR.c_str())/100.0;
                m_pArr[m_numPositions*m_Cols+1]=(90-atof(sT.c_str())) *PI/180.0   ;
                m_pArr[m_numPositions*m_Cols+2]=atof(sF.c_str())*PI/180.0;
                m_numPositions++;
                }
              }
              else if (b_forceLFE && line.compare(0,4,"LFE_") == 0 )
              {
                  if (line.compare(0+4,1,"R") != 0)
                      throw(std::runtime_error("file error - ASCCII positions"));
                  string sR= line.substr(4+1,4);
                  if (line.compare(5+4,2,"_A") != 0)
                      throw(std::runtime_error("file error - ASCCII positions"));
                  string sF= line.substr(7+4,4);
                  if (line.compare(11+4,2,"_E") != 0)
                      throw(std::runtime_error("file error - ASCCII positions"));
                  string sT= line.substr(13+4,3);
                  m_pArr[m_numPositions*m_Cols]  =atof(sR.c_str())/100.0;
                  m_pArr[m_numPositions*m_Cols+1]=(90-atof(sT.c_str())) *PI/180.0   ;
                  m_pArr[m_numPositions*m_Cols+2]=atof(sF.c_str())*PI/180.0;
                  m_numPositions++;
              }

          }
        }
        else if  (fileType==2)  // Fliege positions
        {
            m_pArr = new double [3 * cnt];
            m_Cols=3;
            m_numPositions=0;
            m_type=2;
            while ( myfile.good() )
            {
            getline (myfile,line);
            if (line.size()>86)
            {
                string sX=line.substr(0, 22);
                string sY=line.substr(23, 22);
                string sZ=line.substr(47, 22);
                m_pArr[m_numPositions*m_Cols]  = atof(sX.c_str());
                m_pArr[m_numPositions*m_Cols+1]= atof(sY.c_str());
                m_pArr[m_numPositions*m_Cols+2]= atof(sZ.c_str());
                m_numPositions++;
            }            
            }
        }
        else
        {
         throw(std::runtime_error("Unsupported position file m_type"));
        }

        myfile.close();
    }

    else throw(std::runtime_error("Unable to open file")); 

} 


//--------------------------------------------------------------------------------------------------------------------------
 spacePositions::spacePositions(const spacePositions&  tp1) 
 {
    m_numPositions= tp1.m_numPositions;
    m_Cols=3;
    m_type=tp1.m_type;
    m_pArr = new double [3 * m_numPositions];
    for (int i=0; i<3 * m_numPositions; i++)
    {
         m_pArr[i]=tp1.m_pArr[i];    
    }
 }  

 //--------------------------------------------------------------------------------------------------------------------------
 spacePositions& spacePositions::operator=( const spacePositions & rhs)
 {    
   if (this == &rhs)      // Same object?
      return *this;        // Yes, so skip assignment, and just return *this.
 
   m_numPositions= rhs.m_numPositions;
   m_Cols=3;
   m_type=rhs.m_type;
   m_pArr = new double [3 * m_numPositions];
   for (int i=0; i<3 * m_numPositions; i++)
   {
       m_pArr[i]=rhs.m_pArr[i];    
   }
    return *this;   
 };



//--------------------------------------------------------------------------------------------------------------------------
 spacePositions::~spacePositions()
 {
  delete [] m_pArr;
 }

 void spacePositions::addPoles( double r)
 {
   double *pt= new double [3 * (m_numPositions+2)];
   for (int i=0; i<3*m_numPositions; i++)
       pt[i] = m_pArr[i];
    delete [] m_pArr;
    m_pArr = pt;
    pt = pt+(3*m_numPositions);
    if(m_type==1) //rtp
    {
     *pt++=r; *pt++=0; *pt++=0;
     *pt++=r; *pt++=PI; *pt=0;  
    }
    else   // xyz
    {
     *pt++=0; *pt++=0; *pt++=r;
     *pt++=0; *pt++=0; *pt=-r;  
    }
    m_numPositions=m_numPositions+2;
}
       
//-------------------------------------------------------------------------------------------------------------------------------
 bool  spacePositions::is2D( double n2DTheshold)
 {
  bool is2Dflag=true; 
    double lt=PI/2-n2DTheshold;
    double ut=PI/2+n2DTheshold;
  if (m_type ==1)
  {
  for (int i=0; i<m_numPositions; i++)
     if(m_pArr[i*m_Cols+1]<lt || m_pArr[i*m_Cols+1]>ut)
        {is2Dflag=false;break;}
   }
   else 
   {// convert to rtp and call again, delete copy
    spacePositions tmp = this->convertCopyToSpherical();      
    for (int i=0; i<tmp.getNumPositions(); i++)
        if(tmp[i][1]<lt || tmp[i][1]>ut)
           {is2Dflag=false;break;}
   }
   return is2Dflag;
 }


//--------------------------------------------------------------------------------------------------------------------------
 spacePositions spacePositions::convertCopyToCartesian(bool bForceR1)
 {
  
    spacePositions tp(*this);
    if (m_type==1)  // convert
  {
      for (int i=0; i<m_numPositions; i++)
      {
       if (bForceR1)
       {
           tp.m_pArr[i*m_Cols]    = sin(m_pArr[i*3+1])*cos(m_pArr[i*3+2]); // New X-coordinate    
           tp.m_pArr[i*m_Cols+1]  = sin(m_pArr[i*3+1])*sin(m_pArr[i*3+2]); // New Y-coordinate    
           tp.m_pArr[i*m_Cols+2]  = cos(m_pArr[i*3+1]);                    // New Z-coordinate 
       }
       
       else
       {
           tp.m_pArr[i*m_Cols]    =  m_pArr[i*3]*sin(m_pArr[i*3+1])*cos(m_pArr[i*3+2]); // New X-coordinate    
           tp.m_pArr[i*m_Cols+1]  =  m_pArr[i*3]*sin(m_pArr[i*3+1])*sin(m_pArr[i*3+2]); // New Y-coordinate    
           tp.m_pArr[i*m_Cols+2]  =  m_pArr[i*3]*cos(m_pArr[i*3+1]);                    // New Z-coordinate 
       }   
      }
     tp.m_type=2;
  }
  else if (m_type==0)
  {
    throw(std::runtime_error("class spacePrositions, m_type undefined"));
  }
  return tp;
 }

 //--------------------------------------------------------------------------------------------------------------------------
 spacePositions spacePositions::convertCopyToSpherical()
 {
     spacePositions tp(*this);
     if (m_type==2)  // convert
     {
         for (int i=0; i<m_numPositions; i++)
         {             
            tp.m_pArr[i*m_Cols]    =  sqrt(m_pArr[i*3]*m_pArr[i*3] + m_pArr[i*3+1]*m_pArr[i*3+1] + m_pArr[i*3+2]*m_pArr[i*3+2] ); // New radius    
            tp.m_pArr[i*m_Cols+1]  =  atan2( sqrt(m_pArr[i*3]*m_pArr[i*3] + m_pArr[i*3+1]*m_pArr[i*3+1]),  m_pArr[i*3+2] );    // New inclination    
            tp.m_pArr[i*m_Cols+2]  =  atan2(m_pArr[i*3+1], m_pArr[i*3]);                                                 // New azimuth           
         }
         tp.m_type=1;
     }
     else if (m_type==0)
     {
         throw(std::runtime_error("class spacePrositions, m_type undefined"));
     }
     return tp;
 } 

 //--------------------------------------------------------------------------------------------------------------------------
  simpleMtrx spacePositions::getTriplet(int l1,int l2, int l3)
  {
   if (l1>m_numPositions || l2>m_numPositions || l3>m_numPositions)
        throw(std::runtime_error("SpacePosition - getTriplet lx exceeds range of number of  positions"));
   simpleMtrx myMat(3,3);
   if (m_type==1)
   {
       spacePositions tmp = this->convertCopyToCartesian(true);    
       myMat.copyData(tmp.getPositionsArrayRaw());          
   }
   else if (m_type==2)
   {
    myMat.copyData(m_pArr);
   }
   else throw(std::runtime_error("Unsupported SpacePositins m_type"));
   return myMat; 
  } 

 //--------------------------------------------------------------------------------------------------------------------------
simpleMtrx spacePositions::getDoublet(int l1,int l2)
{
    if (l1>m_numPositions || l2>m_numPositions )
        throw(std::runtime_error("SpacePosition - getDoublet lx exceeds range of number of  positions"));
    simpleMtrx myMat(2,2);
    if (m_type==1)
    {
        spacePositions tmp = this->convertCopyToCartesian(true);      
        myMat[0][0]=tmp.m_pArr[l1*m_Cols]; myMat[0][1]=tmp.m_pArr[l1*m_Cols+1]; myMat[1][0]=tmp. m_pArr[l2*m_Cols]; myMat[1][1]=tmp.m_pArr[l2*m_Cols+1];
    }
    else if (m_type==2)
    {
        myMat[0][0]=m_pArr[l1*m_Cols]; myMat[0][1]=m_pArr[l1*m_Cols+1]; myMat[1][0]= m_pArr[l2*m_Cols]; myMat[1][1]=m_pArr[l2*m_Cols+1];
    }
    else throw(std::runtime_error("Unsupported SpacePositins m_type"));
    return myMat; 
}

  //--------------------------------------------------------------------------------------------------------------------------
  void spacePositions::getMaxMinInclination(double *maxInclination, double *minInclination)
  {
    double minI=PI;
    double maxI= 0;
      if (m_type ==1)
      {
          for (int i=0; i<m_numPositions; i++)
          {
              if(m_pArr[i*m_Cols+1]<minI)
                 minI=m_pArr[i*m_Cols+1];
              if(m_pArr[i*m_Cols+1]>maxI)
                  maxI=m_pArr[i*m_Cols+1];
          }
      }
      else 
      {// convert to rtp and call again, delete copy
          spacePositions tmp = this->convertCopyToSpherical();      
          for (int i=0; i<tmp.getNumPositions(); i++)
          {
              if(tmp[i][1]<minI)
                  minI=tmp[i][1];
              if(tmp[i][1]>maxI)
                  maxI=tmp[i][1];
          }
      }
  *maxInclination=maxI;
  *minInclination=minI;
  }
//--------------------------------------------------------------------------------------------------------------------------
double spacePositions::getMaxDistance()
{
 double maxR=0;
    if (m_type ==1)
      {
          for (int i=0; i<m_numPositions; i++)
          {
            if(m_pArr[i*m_Cols]>maxR)
                  maxR=m_pArr[i*m_Cols];
          }
      }
      else 
      {
        for (int i=0; i<m_numPositions; i++)
          {
            double r= sqrt(m_pArr[i*m_Cols]*m_pArr[i*m_Cols]+m_pArr[i*m_Cols+1]*m_pArr[i*m_Cols+1]+m_pArr[i*m_Cols+2]*m_pArr[i*m_Cols+2]);
            if(r>maxR)
                  maxR=r;
          }
         
      }

 return maxR;
}
