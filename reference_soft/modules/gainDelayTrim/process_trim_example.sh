#! /bin/csh -f

#/***********************************************************************************
# 
# This software module was originally developed by 
# 
# Fraunhofer IIS
# 
# in the course of development of the ISO/IEC 23008-3 for reference purposes and its 
# performance may not have been optimized. This software module is an implementation
# of one or more tools as specified by the ISO/IEC 23008-3 standard. ISO/IEC gives 
# you a royalty-free, worldwide, non-exclusive, copyright license to copy, distribute, 
# and make derivative works of this software module or modifications  thereof for use
# in implementations or products claiming conformance to the ISO/IEC 23008-3 standard 
# and which satisfy any specified conformance criteria. Those intending to use this 
# software module in products are advised that its use may infringe existing patents. 
# ISO/IEC have no liability for use of this software module or modifications thereof. 
# Copyright is not released for products that do not conform to the ISO/IEC 23008-3 
# standard.
# 
# Fraunhofer IIS retains full right to modify and use the code for its own purpose,
# assign or donate the code to a third party and to inhibit third parties from using 
# the code for products that do not conform to MPEG-related ITU Recommendations and/or 
# ISO/IEC International Standards.
# 
# This copyright notice must be included in all copies or derivative works. 
# 
# Copyright (c) ISO/IEC 2013.
# 
# ***********************************************************************************/

# =================================================
# specify parameters:
# -------------------------------------------------
set ITEM = 'trim_test_22_2'
# -------------------------------------------------
set DMX_WAV = ${ITEM}_dmx
set DMX_EXEC = './formatConverterCmdl'
set INFMT = 0  # 22.2 input format
set OUTFMT = 2 # 5.1 output format
set DIST_FILE = './distance_example.txt' # input: file containing loudspeaker distances, 1 line per dmx output channel, no trailing CR/LF after last line
# -------------------------------------------------
set SPLIT_EXEC =  './wavM2NCmdl'
set SPLIT_OUT = ${ITEM}_splitdmx
set SPLIT_CFG = '54' # config index for splitting of 5.1 multichannel WAV
# -------------------------------------------------
set TRIM_EXEC = './mixerCmdl'
set CH_SFX_LIST = '_A+030_E+00, _A-030_E+00, _A+000_E+00, _LFE1, _A+110_E+00, _A-110_E+00'
set TRIM_PARAM_FILE = 'trim_parameters.txt'

echo ==============================================
echo downmixing input file
echo ==============================================
${DMX_EXEC} -if ${ITEM}.wav -of ${DMX_WAV}.wav -inConf ${INFMT} -outConf ${OUTFMT} -dist ${DIST_FILE}

echo ==============================================
echo splitting dmx output into single channel WAVs
echo ==============================================
${SPLIT_EXEC} -if ${DMX_WAV} -of ${SPLIT_OUT} -cfg ${SPLIT_CFG} -dir 1
\rm -f ${DMX_WAV}

echo ==============================================
echo applying trim gain and delay to each channel
echo ==============================================
set count = 0
foreach TRIM_PARAMS ("`cat ${TRIM_PARAM_FILE}`")
   @ count = ($count + 1)
   set TRIM_GAIN = `echo ${TRIM_PARAMS} | cut -d',' -f1`
   set TRIM_DELAY = `echo ${TRIM_PARAMS} | cut -d',' -f2`
   set CH_WAV = ${SPLIT_OUT}`echo ${CH_SFX_LIST} | cut -d',' -f${count}`
   echo "--------------------------"
   echo channel = ${count}, file = ${CH_WAV}.wav, gain = ${TRIM_GAIN}, delay = ${TRIM_DELAY} samples
   ${TRIM_EXEC} -if1 ${CH_WAV}.wav -if2  ${CH_WAV}.wav -of ${CH_WAV}_trim.wav -d1 ${TRIM_DELAY} -d2 ${TRIM_DELAY} -g1 ${TRIM_GAIN} -g2 0.0
end


