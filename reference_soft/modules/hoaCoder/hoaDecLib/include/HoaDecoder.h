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
 $Rev: 203 $
 $Author: technicolor-ks $
 $Date: 2016-01-19 14:45:41 +0100 (Di, 19 Jan 2016) $
 $Id: HoaDecoder.h 203 2016-01-19 13:45:41Z technicolor-ks $
*/
#ifndef _HOADECODER_
#define _HOADECODER_
#include <vector>
#include <memory>
#include "DataTypes.h"

#define EXACTNESS_TEST /*< enables sample conversions to create 
                           results that are as close as possible to the CFP proposal */

  /**
    * @brief class providing general information on the HOA file
    * 
    * The class provides information about the HOA file
    *
    */ 
class HoaInfo
{
  public:
      /**
       * @brief constructor
       * 
       * Initializes the member variables with default values
       *
       */ 
    HoaInfo();

  public:
    unsigned int m_unHoaOrder; /*< the HOA order of the decoded HOA PCM signal */
    unsigned int m_unNumHoaCoefficients; /*< the number of HOA channels (signals or coefficients) */
    UINTEGER64 m_unFileSizeInSample; /*< the original file length of the compressed HOA signal in samples */
    bool m_bNFC; /*< flag that indicates that near field compensation has been applied to the HOA represeantation */
    float m_fNfcRadius; /*< the radius used for the NFC compensations (invalied if m_bNFC==false)*/
	bool m_bIsScreenRelative; /*< flag to indicate if HOA content is relative to screen size*/
    unsigned int m_unSampleRate; /*< the samples rate in Hz of the PCM signals */
    unsigned int m_unNumberOfSpeakers; /*< number of output speaker signals */
    std::vector<std::string>  m_vsSpeakerPositions; /*< vector containing a string of the positions of each speaker */ 
    std::vector<std::vector<FLOAT> > m_vvfRenderingMatrix; /*< vector of vector of float containing the elements of the channelxHOA rendering matrix */
};

/** forward declaration of used classes */
class CHoaBitStreamR;
class SpatialDecoder;
class hoaRenderer;
class WavReader;
class DRC1hoa;
class hoaRotation;

  /**
    * @brief class for decoding HOA PCM samples from the HOA file
    * 
    * This function is for the decoding of the HOA PCM signals from 
    * the compressed HOA bit stream. 
    * It provides decoding of HOA files, which means that HOA streaming is not supported.
    * The reading of the HOA file is handled by the class.
    * The PCM samples of a HOA frame are decoded frame by frame by the decode() function.
    *
    */ 
class HoaDecoder
  {
  public:
      /**
       * @brief constructor
       * 
       * Initializes the member variables with default values
       * Call openHoaFile() to initialize the HOA decoder 
       *
       */ 
    HoaDecoder();

      /**
       * @brief destructor
       *
       */ 
    ~HoaDecoder();

     /**
       * @brief initialized the HOA decoder and renderer from an HOA file
       * 
       * This functions initializes the HOA decoder and renderer.
       * It reads the file header and the access frame 
       * and initializes all member variables. 
       * The function can be used to re-open another file.
       * After successfully calling this function the
       * HoaInfo can be access from getHoaInfo(). 
       * The positions of the loud-speakers are read from the provided text file.
       *
       * @param [in] transportChanFileName path and file name of the PCM transport channel WAV file
       * @param [in] hoaFileName path and file name of the binary HOA SI file
       * @param [in] speakerFileName path and file name to the speaker file
       * @param [in] drcFlag: <1: drc1 off, ==1: TD drc, ==2 FD drc
       * @param [in] drc1FileName  path and file name to the drc 1 coefficient WAV file 
       * @param [in] disableRendering disables rendering to speakers (default false)
       * @param [in] sHoaMtxFile path and file name to an optional HOA rendering matrix file
       *
       * @retval true error
       * @retval false okay
       *
       */ 
    bool initHoaDecoder(const std::string &transportChanFileName, 
                        const std::string &hoaSiFileName,
                        const std::string & speakerFileName,
                        const std::string & sScreenSizeName,
                        const std::vector<std::string> &sHoaMtxFile,
                        int rotationFlag,
                        const std::string &rotationFileName,
                        int drcFlag, 
                        const std::string & drc1FileName,
                        unsigned int unCoreCoderFrameLength,
                        bool disableRendering=false
                        );

     /**
       * @brief closes an open HOA file
       * 
       * @retval true error
       * @retval false okay
       *
       */ 
    bool closeHoaFile();

      /**
       * @brief decodes the PCM HOA signals and renders them to speaker signals
       * 
       * The size of the input vector is controlled and resized if required.
       * The outer vector holds one vector of PCM signals for each channel (coefficient) vvfSpeakerPcmSamplesCS[channel][sample].
       * Basically, the decoding of a HOA frame should result in sample vectors of 1024 samples.
       * For the firstly decoded frames the size of the sample vectors are zero to handle the delay. 
       * The size of the last sample vectors could be less than 1024 samples to handle the OFL.
       * But all samples vectors should always have the same size.
       *
       * @param [out] vvfSpeakerPcmSamplesCS output 2D speaker signal buffer vvfSpeakerPcmSamplesCS[channel][sample]
       *
       * @retval true error
       * @retval false okay
       *
       */ 
    bool decode(std::vector<std::vector<FLOAT> > &vvfSpeakerPcmSamplesCS);

      /**
       * @brief get the PCM signals of the HOA representation of the last frame
       * 
       * Returns a constant reference to the signals of the lastly decoded HOA 
       * representation. 
       *
       * @retval 2D HOA signal buffer vvfHoaPcmSamplesCS[channel][sample]
       *
       */ 
    const std::vector<std::vector<FLOAT> > &getHoaSamples();

     /**
       * @brief get information about the HOA file
       * 
       *  Returns only valid information if an HOA file has been successfully opened before.
       *
       * @retval A copy of the HoaInfo member variable
       *
       */ 
    HoaInfo getHoaInfo();

     /**
       * @brief check for end of file
       * 
       * @retval true end of file has been reached
       * @retval false end of file has not been reached
       *
       */ 
    bool isEndOfFile();

  private:
    HoaInfo m_hoaInfo; /*< hoa information class */
    std::shared_ptr<WavReader> m_WavReader; /*< class for reading the PCM transport channel */
    std::shared_ptr<CHoaBitStreamR> m_bitStreamReader; /*< the bit stream reader for bit and byte access to the file */
    std::shared_ptr<SpatialDecoder> m_spatialDecoder; /*< for the spatial domain decoding (reconstructs the HOA signals from the decoded AAC signals)*/
    std::shared_ptr<hoaRenderer> m_hoaRenderer; /*< renderer for conversion of HOA representation to a PCM loudspeaker signals of a given setup*/
    std::vector<std::vector<FLOAT> > m_fTimeDomainSamples; /*< buffer for the decoded AAC signals */
    std::vector<std::vector<FLOAT> > m_fHoaSamples; /*< buffer for the decoded HOA signals */
    unsigned int m_unSpatDelayFrames; /*< counter to handle the spatial delay (OFL processing) */
    unsigned int m_unSpatEofFrames; /*< number of frames to process to empty the spatial decoder pipeline when end of the file has been reached */
    UINTEGER64 m_unNumSamplesDecoded; /*< counts number of decoded samples per channel */
    unsigned int m_unFrameLength; /*< indicates the number of samples per channel decoded from one HOA frame */
    bool m_bHoaFileIsOpen; /*< state variable that indicates that a file has been initialized */

    int m_drc1mode; 
    std::shared_ptr<WavReader> m_DRCwavReader; /*< class for reading the DRC 1 coefficients  */
    std::shared_ptr<DRC1hoa> m_drc1;
    std::vector<std::vector<FLOAT> > m_DCR1Samples; /*< buffer for the DRC gain sequences */

    int m_rotationFlag;
    std::shared_ptr<WavReader> m_hoaRotationWavReader; /*< class for reading the HOA rotation parameter */
    std::shared_ptr<hoaRotation> m_hoaRotation;
    std::vector<std::vector<FLOAT> > m_hoaRotationSamples; /*< buffer for the HOA rotation parameter */

    bool m_disableRendering;
    unsigned int m_unHoaFramesPerCoreFrame; /*< number of HOAFrames included in one core coder frame */
    unsigned int m_unHoaFrameCnt; /*< index of current HOAFrame with respect to the core coder frame [0, ..., m_unHoaFramesPerCoreFrame-1] */
  };


#endif //_HOADECODER_
