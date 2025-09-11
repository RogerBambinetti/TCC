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
/*   spacePositions.h 
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
$Id: spacePositions.h 157 2015-01-16 14:00:35Z technicolor-kf $
*/


#ifndef __spacePositions_H__
#define __spacePositions_H__ 


#include <string>
#include <stdexcept>
#include "SimpleMtrx.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif  


 class spacePositions
 {
  public:
    
    // create an empty object 
    spacePositions();

    // construct from spherical positions, 
    spacePositions(  int numberOfPositions,            // number of speaker or source positions   
                     double *radius,                   // pointer to vector of size numberOfPositions with radius in m,
                     double *speakerInclination,       // pointer to vector of size numberOfPositions with inclination in rad 
                     double *speakerAzimuth);          // pointer to vector of size numberOfPositions with azimuth in rad 
    
    // Construct from positions in file, 
   
    // b_forceLFE may be 
    spacePositions( const std::string &fileName,   // name of file with position data
                    int fileType,                  // fileType - 1: ASCII RXXXX_AXXXX_EXXX  (MPEG-H RM0); 2: Fliege type (TC internally)
                    bool b_forceLFE=false);        // used to read in LFE channels for fileType 1 files - crates an LFE only spacePositions object  

    // Copy Constructor
    spacePositions(const spacePositions&);

   // Assignment Operator 
    spacePositions& operator=(const spacePositions&);           
 
   // Destructor
    ~spacePositions();                                          
 
   // add south and north pole to the positions
   // warning: no check is done if this positions already exist
   // best use is2D before call
    void addPoles( double r=1.);                     // radius in m

   // Test if positions are 2D
    bool is2D(  double n2DTheshold=(7.*PI/(180.)));  // threshold value,  +/-   value to horizontal pane in rad     
 
    // Return number of positions 
    int getNumPositions()                                       
        {
         return m_numPositions;
        };      
                                                       
    // operator overload to address position as two dim array ([m_numPositions][3])                                                       
    double* operator[](int row)                                 
        { 
            return & m_pArr[row * m_Cols];   // [l1_coord1, l1ccord2, l3coord3; l2_coord1, l2_coord2, ...]  in memory
        } 

    // Copy and convert to Cartesian positions, 
    // bForceR1==true to force radius to one (default) 
    spacePositions convertCopyToCartesian(bool bForceR1=true); 

    // Copy and convert to Spherical positions    
    spacePositions convertCopyToSpherical();                       
   
    // see if positions are Spherical or Cartesian    
    int getType()                                                 
        {
         return m_type;
        };           
      
    // string to indicate  if positions are Spherical or Cartesian                          
    std::string getTypeString()
        {
         switch (m_type) {
         case 1: return "Spherical";
         case 2: return "Cartesian";
         default: return "Undefined";
          }
        };
    
    //bool writeToFile(  std::string filename );

    // returns a 3x3 matrix holding the the Cartesian positions (m_Cols) of l1,l2, l3 (rows), 
    // to speed up use Cartesian m_type positions
    simpleMtrx getTriplet(int l1,int l2, int l3);     // indices to triplets to be returned       
    
    // returns a 2x2 matrix (for 2D)
    simpleMtrx getDoublet(int l1,int l2);  

    // get raw pointer of positions (maybe used for linear addressing or be converted to  double (*m_numPositions)[3] to access like [l1][0|1|3])
    double* getPositionsArrayRaw()                                
    { return m_pArr;}                              

    // returns maximal and minimal inclination values  in rad found within positions
    void getMaxMinInclination(double *maxInclination,   // the lowest position
                              double *minInclination);  // the highest position

    // returns maximal distance of position in space to origin 
    double getMaxDistance();

  private:
    double *m_pArr;      //  holds positions, row major storage map (default C), rows: position indices, cols: corordinates (rtp, or xyz), m_Cols=3;  
    int m_numPositions;  //  rows
    int  m_type;         //  0: undefined, 1 spherical, 2 Cartesian
    int m_Cols;          // 3 rtp or xyz
 };


#endif // __spacePositions_H__
