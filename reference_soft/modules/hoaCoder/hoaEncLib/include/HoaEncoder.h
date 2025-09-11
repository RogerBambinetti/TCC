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
 $Rev: 196 $
 $Author: technicolor-ks $
 $Date: 2015-10-12 13:45:11 +0200 (Mo, 12 Okt 2015) $
 $Id: HoaEncoder.h 196 2015-10-12 11:45:11Z technicolor-ks $
*/
#ifndef _HOADECODER_
#define _HOADECODER_
#include <vector>
#include <map>
#include <memory>
#include <fstream>
#include "DataTypes.h"


/** forward declaration of used classes */
class CHoaBitStreamW;
class SpatialEncoder;
class BitRateControl;
class WavWriter;
class HoaFrame;

  /**
    * @brief class for encoding HOA PCM samples to an compressed HOA file
    * 
    */ 
class HoaEncoder
  {
  public:
      /**
       * @brief constructor
       * 
       * Initializes the member variables with default values
       * Call initHoaEncoder() to initialize the HOA encoder 
       *
       */ 
    HoaEncoder();

      /**
       * @brief destructor
       *
       */ 
    ~HoaEncoder();

     /**
       * @brief initialized the HOA encoder
       * 
       * This functions initializes the HOA encoder.
       * It writes the file header and the access frame 
       * and initializes all member variables. 
       * The function can be used to re-open another file.
       *
       * @param [in] hoaTransportChanFile path and file name for the HOA transport channels
       * @param [in] hoaSideInfoFile path and file name for the HOA side information
       * @param [in] hoaSideInfoSizeFile path and file name for the HOA side information size file
       * @param [in] unSampleRate sampling frequency of the input signal
       * @param [in] unTotalBitRate average bit rate for the coded file
       * @param [in] unHoaOrder order of the HOA input signal
       * @param [in] bUseNfc near field compensation has been applied to the HOA signal
       * @param [in] fNfcDistance radius used for the NFC
       * @param [in] bCoreCoderUsesSbr set this flag if the core coder uses the SBR tool
       * @param [in] unCoreFrameLength frame (Access Unit) length in samples used by the Core Encoder
       * @param [in] unHoaFrameLength frame length in samples used by the spatial coding see standard for allowed lengths
       *
       *
       * @retval true error
       * @retval false okay
       *
       */ 
    bool initHoaEncoder(const std::string &hoaTransportChanFile,
                        const std::string &hoaSideInfoFile, 
                        const std::string &hoaSideInfoSizeFile,
                        const unsigned int unSampleRate,
                        const unsigned int unTotalBitRate,
                        const unsigned int unHoaOrder,
                        const bool bUseNfc,
                        float fNfcDistance,
                        bool bCoreCoderUsesSbr,
                        unsigned int unCoreFrameLength,
                        unsigned int unHoaFrameLength);


     /**
       * @brief closes an open HOA file
       * 
       * @retval true error
       * @retval false okay
       *
       */ 
    bool closeHoaFile();

      /**
       * @brief encodes the PCM HOA signals to the input file
       * 
       * The HOA signals of the input vector are encoded to an HOA frame.
       * If not all input samples are required for encoding an frame, the
       * input vector is resized to an vector of the remaining samples. 
       * The first vector holds one vector of samples for each HOA coefficient.
       * If setEndOfFile() has been called the input vector is ignored.
       *
       * @param [out] vvfHoaSignals int 2D signal buffer vvfHoaSignals[channel][sample]
       *
       * @retval true error
       * @retval false okay
       *
       */ 
    bool encode(std::vector<std::vector<FLOAT>> &vvfHoaSignals);

     /**
       * @brief indicate the end of the input file
       * 
       * No more input samples are given to the encoder.
       *
       *
       */ 
     void setEndOfFile();

     /**
       * @brief check for end of file
       * 
       * @retval true end of file has been reached
       * @retval false end of file has not been reached
       *
       */ 
    bool isEndOfFile();


  private:
    std::shared_ptr<CHoaBitStreamW> m_bitStreamWriter; /*< the bit stream writer for writing bits and bytes to the file */
    std::shared_ptr<WavWriter> m_WavWriter; /*< for writing the HOA transport channels into a multichannel file */
    std::shared_ptr<SpatialEncoder> m_spatialEncoder; /*< for the spatial domain encoding (creates the signals to be AAC encoded*/
    std::vector<std::vector<FLOAT> > m_fHoaSamples; /*< buffer for the HOA input samples */
    std::vector<std::vector<FLOAT> > m_fTransportChannels; /*< buffer for the AAC input signals */
    std::fstream m_sideInfoSizeStream; /*< stream for writing the side info size file */
    unsigned int m_unHoaFrameLength; /*< indicates the number of samples per channel decoded from one HOA frame */
    bool m_bHoaFileIsOpen; /*< state variable that indicates that a file has been initialized */
    bool m_bEndOfFileSet; /*< the end of input samples has been indicated */
    bool m_bEndOfFileReached; /*< the end of the file has been reached */
    unsigned int m_unHoaSiPerFrame; /*< the number of HOA Frames that are accumulated into one core coder frame */
    unsigned int m_unHoaSiCnt; /*< number of HOA Frames accumulated */
    unsigned int m_unHoaSiBits; /*< size of the HOA side info frame in bit */
    int64_t m_nSamplesRead; /*< number of sample per channel read */
    int64_t m_nSamplesWritten; /*< number of sample per channel written */
    int m_nEncoderDelaySamples; /*< number of delay samples per channel added by the spatial encoding */
    unsigned int m_unAddDelayInSamples; /*< additional delay to align core coder delay to frame size */
    std::vector<std::vector<FLOAT> > m_fTransportChannelsDelayBuffer; /*< vector to buffer delayed samples */
    unsigned int m_unNumOfDummyFrames; /*< number of additional HOA dummy frames for synchronization of the side info 
                                           and the decoded and delayed transport signals (only core coder delay is considered)*/ 
    const std::map<unsigned int, unsigned int> m_CoreCoderDelayTable; /*< table for core coder delays in samples first->bitrate second->delay*/
    std::shared_ptr<HoaFrame> m_dummyHoaFrame; /*< empty HOA frame  */

  private:
    static const std::map<unsigned int, unsigned int> createCoreCoderDelayTable(); /*< function to fill m_AddDelayTable */
  };


#endif //_HOADECODER_
