
--- Frequency-Domain Binaural Renderer Module - Software Structure ---

The binaural renderer module solution (binauralRendererCmdl.sln) consists of the following projects:
* binauralRendererCmdl : commandline tool to perform binaural rendering
* binauralRendererLib  : library that holds the binauralization functions 
* tools for reading/writing wave files and binaural bitstreams etc.

To build the renderer command line tool, open and compile binauralRendererCmdl.sln. 

The resulting commandline tool (binauralRendererCmdl.exe) has following arguments :
	
-if <path to input audio file>
The commandline tool provides the possibility to directly read qmf data if these are available. As a qmf 
data interface, so-called qmf-files are defined. The qmf-files hold the qmf domain audio data. Two files 
are needed to represent one audio item ("item_real.qmf" and "item_imag.qmf"). They are wave-files but to 
distinguish them from standard wave-files they have the file extension .qmf. If such a file should be 
read, the commandline switch must only contain the base filename (e.g. "item" without "_real.qmf"). The 
paths for the files for the real and imaginary data are constructed internally. These files can be 
created with the qmflib library from the "tools" folder of the reference software.

Example usage:
<folder\\input.wav> for wav input, e.g. "..\\inputData\\inputAudio\\test.wav" (multichannel wave file, no 
mono wave file support)
<folder\\input> for QMF input (assumes two files in folder: "input_real.qmf" and "input_imag.qmf"), only 
the base filename up to the underscore is to be given: e.g. "..\\inputData\\inputAudio\\input"

-of <path to output audio file>
<folder\\output.wav> for wav output, e.g. "..\\outputData\\test2.wav"
<folder\\output> for QMF output, produces two files: "output_real.qmf" and "output_imag.qmf", only the base 
filename up to the underscore is to be given

-fdbs <path to low-level frequency-domain binauralization bitstream file>
<folder\\fd_bitstream.bs> 
This path has to be the path to a low-level frequency-domain binauralization bitstream.

-cicpIn <audio input configuration index>
A CICP configuration index has to be given (CICP Speaker Layout Index as defined in ISO/MPEG 23001-8).
For 22.2 input audio the CICP index is 13, for 5.1 input the CICP index is 6. 
Setups for which no CICP index are known have to be signaled with "-cicpIn -1"

-inGeo <path to geometry file>
This path has to be given in case the CICP index is equal to -1 (arbitrary setup)
Geometry files for all files of the submission set (CO_01 to C0_12) are included in the software (folder 
"inputData"). They contain of the number of channels in the first line, followed by one new line per channel.
The channel entries contain of either a "g" followed by <azimuth_degrees>, <elevation_degrees>, <LFE_flag>. 
The LFE flag signals if this channel is an LFE Channel. A channel could alternatively be signaled by giving a "c" 
followed by a CICP Speaker Index that describes a channel position as defined in ISO/MPEG 23001-8. 
Both possibilities can be mixed in one file. 

-mode (optional) <1/0>
Switch the QTDL processing on or off. QTDL processing is switched on by default

-block (optional) <samples>
Signal the framelength (i.e. number of time domain samples per audio frame) for the binaural rendering. A 
standard framelength of 4096 samples is used as default

-nbs (optional) <integer>
binauralization bitstream does consist of many binaural representations.
This value select the -nbs th binaural representation. Note that the 1st representation is default (i.e. default = 1).

-kconv (optional) <integer>

-kmax (optional) <integer>

Usage example: 
-if CO_01_Church.wav -of test.wav -fdbs IIS_BRIR_FD_lowlevel.bs -cicpIn 13
	

