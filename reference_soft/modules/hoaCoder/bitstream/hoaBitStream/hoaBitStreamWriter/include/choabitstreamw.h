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
 $Rev: 188 $
 $Author: technicolor-kf $
 $Date: 2015-07-22 11:37:58 +0200 (Mi, 22 Jul 2015) $
 $Id: choabitstreamw.h 188 2015-07-22 09:37:58Z technicolor-kf $
*/
#ifndef __HOABITSTREAMWRITE__
#define __HOABITSTREAMWRITE__

#include "HoaConfigW.h"
#include "HoaFrameW.h"
#include <memory>


/**
  * @brief the HOA file format writer
  * 
  * Class for writing the HOA file format
  *
  *
  */ 
class CHoaBitStreamW
{
  public:
      /**
       * @brief Constructor creates an bit stream writer
       * 
       */ 
    CHoaBitStreamW();
      
      /**
       * @brief Destructor 
       * 
       */ 
    ~CHoaBitStreamW();

      /**
       * @brief Opens a new HOA file for writing
       * 
       *  @param [in] sName path and file name of the new HOA file
       *
       *
       */ 
    bool openStream(const char * sName);

      /**
       * @brief closes an open HOA file
       * 
       * @retval true error
       * @retval false okay
       *
       */ 
    bool closeStream();
      
      /**
       * @brief Writes the HOA access frame to the file
       * 
       * Get a reference to the access frame struct from getHoaConfig().
       * Edit set the parameter and call this function to write the 
       * bit stream fields to the file
       *
       * @retval number of bits written
       *
       */    
      int writeHoaConfig(); 

      /**
       * @brief Writes the HOA frame to the file
       * 
       * @param [in] bGlobalIndependencyFlag the state of the 
       *                                     global Independency Flag
       *
       * Get a reference to the frame struct from getFrame().
       * Edit set the parameter and call this function to write the 
       * bit stream fields to the file
       *
       *
       * @retval number of bits written
       *
       */ 
    int writeFrame(bool  bGlobalIndependencyFlag);    

     /**
       * @brief byte align bit buffer
       *
       * This function filles the bit buffer to the next multiple
       * of eight bits. Call this function after  writeHoaConfig() 
       * and writeFrame(bool  bGlobalIndependencyFlag) to byte align
       * the frame buffer. 
       *
       * @retval number of bits written for alignment
       * 
       */   
    int byteAlignFrameBuffer();  


      /**
       * @brief Initializes the HOA frame parameter with default values
       *
       * Allocates and initializes the parameter of the standard HOA frame from
       * a the parameter of the current access frame setting.
       * Is called automatically from writeHoaConfig()
       * 
       */
    void initFrame();


      /**
       * @brief get a reference to the HOA access frame
       *
       * @retval reference to the HOA access frame member instance
       * 
       */
    HoaConfig& getHoaConfig(unsigned int unCoreCoderFrameLength);    

      /**
       * @brief get a reference to the HOA standard frame
       *
       * @retval reference to the HOA standard frame member instance
       * 
       */
    HoaFrame& getFrame();

      /**
       * @brief get the size of the current standard frame
       *
       * @retval size in bit
       * 
       */
    unsigned int getFrameSize(bool  bGlobalIndependencyFlag);

     /**
       * @brief get the size of the current Access Unit frame
       *
       * @retval size in bit
       * 
       */
    unsigned int getHoaConfigSize();

      /**
       * @brief create a new Channel Side Info Data table
       *
       *  @comments: Use always this function to create a new channel because
       *             actually a derived class is created instead of the actual base class. 
       *
       * @retval shared pointer to the new channel
       * 
       */
    static std::shared_ptr<CChannelSideInfoData> getChannelInfo(CHANNEL_TYPE type);

  private:
    std::shared_ptr<CBitStreamWriter> m_bitWriter; /*< class for writing bits and bytes */
    std::shared_ptr<CStreamWriter> m_fileWriter; /*< file access for flushing the m_bitWriter buffer to the file */ 
    HoaConfigW m_hoaConfigWrite; /*< the HOA access frame */
    HoaFrameW m_hoaFrameWrite; /*< the HOA standard frame */
};

#endif // __HOABITSTREAMWRITE__
