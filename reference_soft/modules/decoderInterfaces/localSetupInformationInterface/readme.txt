

--- Local Setup Information Interface Module - Software Structure ---

This module provides reading functionality of files containing bitstream in the local setup information interface syntax.
Besides, this module provides the possibility to generate such bitstream files from a dedicated .txt file and the commandline 
switches of the included commandline tool.

The module consists of a commandline tool that illustrates the writing and reading functionality and a library that holds the 
corresponding funtions.


To build the command line tool, open and compile localSetupInterfaceExample.sln. 


The resulting commandline tool has following arguments :

	-of  	<path of the output bitstream file>   										/* Required */
	-wire 	<number of defined wire outputs followed by a list of the wireIDs> 			/* Required */
	-screen	<size of the local screen in the form: 
				hasLocalScreenInfo, (subsequent data only if hasLocalScreenInfo == 1)
				az_left, az_right,  
				hasElevationInfo, 	(subsequent data only if hasElevationInfo == 1)
				el_bottom, el_top>														/* Required */
	ONE OF THE FOLLOWING:
	-speakers 		<path to a text file that defines the speaker setup for loudspeaker playback>
	-brirsWav 		<path to a folder of BRIRs in the form of stereo wave files for binaural playback>
	-brirsBitstream	<path to a BRIR bitstream file (FIR data or low-level parameters) for binaural playback>
	
	ONE OF THE FOLLOWING ARGUMENTS IN CASE OF -BRIRSWAV:
	-geo	<path to a text file that contains the geometric information of the BRIR measurement positions>
	-hoa	<hoa order of the BRIRs>


The commandline tool reads the loudspeaker setup from the dedicated textfile or the BRIRs from the wavefiles or bistream file.
It then writes a bitstream file in the local setup information interface syntax.

This file is then read again by the commandline tool to show the reading functionality.

The txt file for the local loudspeaker setup has to have the following form:

line 1: numSpeakers, CICPLayoutIdx (-1 if none), externalDistanceCompensation (0|1)

  *     if CICPLayoutIdx == -1:
  *         line 2 to Line 2+numSpeakers-1 either:
  *         g, az, el, isLfe, LSdist, LScalibGain
  *         c, cicpSpeakerIdx, knownAz, knownEl, LSdist, LScalibGain
  *
  *     if CICPLayoutIdx >= 0:
  *       either no more lines following, or
  *         line 2 to Line 2+numSpeakers-1:
  *         knownAz, knownEl, LSdist, LScalibGain
  
Example files is included for the following cases:
- loudspeaker setup is defined by a CICP index
- loudspeaker setup is defined by a CICP index, known positions and calibration gains are given for (some of) the speakers
- loudspeaker setup is defined by a list of CicpSpeakerIdxs
- loudspeaker setup is defined by a list of CicpSpeakerIdxs with additional known positions and calibration gains given for (some of) the speakers
- loudspeaker setup is defined by directly giving the positions of the speakers
  

Usage example: 

-of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -brirsWav "C:\tmp\BRIR_Medium_IIS\IIS_BRIR_"  -geo "..\..\..\..\binaural\BRIRdata\geo_28_2.txt"

-of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -brirsBitstream "..\..\..\..\binaural\BRIRdata\IIS_BRIR_FD_lowlevel.bs"

-of "test.bs" -wire 3 1 2 3 -screen 1 15 -15 1 -5 10 -speakers "geo_CICPindex_knownPos.txt" 
	


