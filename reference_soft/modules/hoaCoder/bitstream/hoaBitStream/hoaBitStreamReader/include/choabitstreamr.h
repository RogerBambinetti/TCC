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
 $Id: choabitstreamr.h 188 2015-07-22 09:37:58Z technicolor-kf $
*/
#ifndef __HOABITSTREAMREAD__
#define __HOABITSTREAMREAD__

#include "HoaConfigR.h"
#include "HoaFrameR.h"

/**
  * @brief Class for reading the HOA compression bit stream
  *
  * Implementation of the HOA bit stream reader   
  * using the bit stream class from the HOA bit   
  * stream definition. 
  *
  * The reader automatically compensated the delay between 
  * the time domain and the spatial domain data. 
  * Furthermore, for HE AAC mode the AAC frames are rearranged 
  * to better match the decoder architecture (Move AAC frames from even frame numbers to odd) 
  * and AAC frames are only returned every second frame. 
  * The first frames only comprise AAC data. Therefore, 
  * the spatial data isn't valid for the first frames. Use
  * spatialDataValid to check whether the spatial data is valid.
  *
  */ 
class CHoaBitStreamR
{
  public:
     /**
       * @brief constructor
       * 
       * Creates an empty (un-initialized) bit stream reader
       *
       */ 
    CHoaBitStreamR();
      
     /**
       * @brief destructor
       * 
       * Clears the bit stream reader
       *
       */ 
    virtual ~CHoaBitStreamR();

     /**
       * @brief closes an open HOA stream
       * 
       * Use this function to re-open a HOA stream
       *
       * @retval true error
       * @retval false okay
       *
       */ 
    bool closeStream();    

     /**
       * @brief check if end of file has been reached
       * 
       * @retval true end of file
       * @retval false not end of file
       *
       */
    bool isEof();

     /**
       * @brief open an HOA file for reading
       * 
       * Actual streaming is not supported yet
       *
       * @param [in] sName file name including the path to the HOA file
       * @param [in] unNumTransportChannels total number of transport channels (form USACconfig())
       *
       * @retval true error
       * @retval false okay
       *
       */ 
    bool initHoaFile(const char * sName, unsigned int unNumTransportChannels, unsigned int unCoreCoderFrameLength);

     /**
       * @brief check if new access frame has been found
       * 
       * HOA parameter could change at access frames. 
       * Call after reading a frame to check if the HOA access frame
       * parameter were reloaded from the stream. 
       *
       * @retval true new access frame has been read
       * @retval false new access frame hasn't been read
       *
       */ 
    bool newHoaConfigFound();

     /**
       * @brief read a new HOA frame from the stream
       * 
       * The HOA bit stream reader has to be initialized to read a frame. 
       *
       * @param [out] failed reading failed bit, true if reading failed
       *
       * @retval reference to the HOA frame structure including the spatial and temporal information
       *         the reference is valid until the next call of this function (not thread safe)
       *
       */ 
    const HoaFrame & readHoaFrame(bool & failed);
    
    /**
       * @brief byte align bit buffer
       * 
       * Reads number of bits to get to the next multiple of eight bits.
       *
       *
       * @retval number of bit read
       *
       */ 
    int byteAlignFrameBuffer();

     /**
       * @brief check for valid spatial data
       * 
       * The first frames only comprise AAC data.
       * To check whether the returned frame structure includes valid spatial data call
       * this function. 
       *
       * @retval true the last frame read included valid spatial data
       * @retval false the last frame read included invalid spatial data
       *
       */     
    bool spatialDataValid();


     /**
       * @brief get a reference to the access frame data
       *
       *
       * @retval reference to the file access frame structure. 
       *         The reference is valid as long as newHoaConfigFound() doesn't return true.
       *
       */ 
    const HoaConfig& getHoaConfig();  

  private:
     /**
       * @brief opens the file using the CStreamReader class
       * 
       *
       * @param [in] sName name and path of the HOA file
       *
       * @retval true error
       * @retval false okay
       *
       */    
    bool openStream(const char * sName);
    
  
  private:
    std::shared_ptr<CBitStreamReader> m_bitReader; /*< class for bit-wise and byte-wise parsing the stream */
    std::shared_ptr<CStreamReader> m_fileReader;  /*< class that reads data from the stream */
    HoaFrameR m_frameRead; /*< class for reading the HOA frame */
    HoaConfigR m_configRead; /*< class for reading the HOA access frame */
    bool m_bValidSpatFrame; /*< indicates if the spatial data of the returned frame is valid */
    bool m_bIsEof; /*< indicates that the end of the stream (file) has been reached */
};

#endif // __HOABITSTREAMREAD__