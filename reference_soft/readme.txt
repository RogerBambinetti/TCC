*************************** MPEG-H 3D Audio Coder - Edition 4.0 ****************************
*                                                                                          *
*                       MPEG-H 3D Audio Reference Software - readme                        *
*                                                                                          *
*                                        2024-07-31                                        *
*                                                                                          *
********************************************************************************************


(1) Content of this package
============================================================================================
 - MPEG-H 3D audio reference software containing the combined channel/object (CO) and
   higher order ambisonics (HOA) path written in ANSI C/C++ (CO-Part) and C++11 (HOA-Part).
 - CMake files for configuring and compiling the MPEG-H 3D audio reference software.


(2) Requirements to successfully build and run the MPEG-H 3D audio reference software
============================================================================================
(2.1) General information
=========================
 The following requirements need to be fulfilled to build and run the MPEG-H 3D audio reference software:
 - Additional software packages:
   . AFsp Audio File Programs and Routines
   . ISOBMFF reference software (libisomediafile)
   . MPEG-D DRC reference software
   . CMake version 3.9.2 or higher
 - Linux/Mac:
   . GNU make
   . gcc 10.1 or higher with C++11 support
 - Windows:
   . Microsoft Visual Studio 2019 Win64
   . Microsoft Windows SDK
   . for compatibility to older Microsoft Visual Studio versions see (3)(c)

 All additional software packages are automatically obtained on the fly
 and compiled when executing cmake on any platform.
 No other steps are therefore required to build the MPEG-H 3D audio reference software.
 For additional info about these software packages, see below.

(2.2) Overview of additional software packages
==============================================
(a) AFsp file programs and routines:
------------------------------------
 To provide support for a variety of audio file formats, the MPEG-H 3D audio reference
 software uses the AFsp package for audio file i/o and resampling. The AFsp
 package is written by Prof. Peter Kabal <peter.kabal@mcgill.ca> and is
 available via download.

 Required version:  AFsp-v9r0.tar.gz
 Download site:     http://www.mmsp.ece.mcgill.ca/Documents/Downloads/AFsp/

 For more details, see the readme.txt of the AFsp package.

(b) MPEG-4 Base Media File Format (ISOBMFF) software:
-----------------------------------------------------
 The ISO Base Media File Format (ISOBMFF) is an integral part of the MPEG-H 3D audio
 reference software. It provides the interface to the mp4 container in which the MPEG-H
 3D audio streams are embedded. ISOBMFF is standardized in ISO/IEC 14996-12
 and the software is hosted on GitHub.

 Required version:  7eaf0482137da37afd60418930d86943e33e909b (commit SHA)
 GitHub space:      https://github.com/MPEGGroup/isobmff
 Download site:     https://github.com/MPEGGroup/isobmff/archive/7eaf0482137da37afd60418930d86943e33e909b.zip

(c) MPEG-D DRC reference software:
----------------------------------
 MPEG-D Dynamic Range Control (DRC) is an integral part of the MPEG-H 3D audio
 reference software. It provides efficient control of dynamic range, loudness and clipping
 based on metadata. MPEG-D DRC is standardized in ISO/IEC 23003-4.

 Required edition:  ISO/IEC DIS 23003-4:202x, 3rd edition
 Download site:     https://standards.iso.org/iso-iec/23003/-4/ed-3/en/MPEG_D_DRC_refsoft.zip


(3) MPEG-H 3D audio reference software build instructions
============================================================================================
(a) Linux/Mac/Windows:
----------------------
 - Create and enter a build directory in the top-level folder: mkdir build; cd build
 - Configure the build: cmake ..
 - Build the software: cmake --build .
 - Change directory into ./bin (Linux/Mac) or .\bin\Debug (Windows) folder and check if all 51 files are listed.

(b) General notes:
------------------
 NOTE: After the successful build the general decoding functionality can be verified by executing
       the following ctest command:
       ctest –C Debug .
 NOTE: The path of the MPEG-H 3D audio reference software directory must not
       contain any whitespaces!

(c) Build options (advanced users):
-----------------------------------
 The software can be configured for different compiler environments by using the –G switch
 during the CMake configuration step. Type "cmake --help" for more information.

 NOTE: Please note that in order to fit the conformance requirements the software should be
       configured for
         - Visual Studio 2019 or
         - GNU make / gcc 10.1 or higher with C++11 support.
       Use any divergent compiler environment at your own risk.


(4) Quick start: Decoding using the 3DAudioDecoder
============================================================================================
 After having followed the build instructions, change directory into ./bin (Linux/Mac) or .\bin\Debug (Windows) folder.
 Then call 3DAudioDecoder[.exe] without any arguments to see all available command line
 arguments.

 A typical call requires a minimum of two parameters: input file (mp4 or mhas), output file,
 e.g.:
 - Linux/Mac: ./3DAudioDecoder -if <input>.mhas -of <output>.wav
 - Windows:   3DAudioDecoder.exe -if <input>.mhas -of <output>.wav

(4.1) Example Decoding
======================
(a) Get MPEG-H 3D audio conformance streams:
--------------------------------------------
 Download MPEG-H 3D audio conformance streams as defined in:
   ISO/IEC 23008-9 3D audio conformance testing

 from the ISO supplied Uniform Resource Name (URN):
   https://standards.iso.org/iso-iec/23008/-9/ed-3/en

 and put the files (*.mhas and *.wav files) e.g.:
 *.mhas in ./MPEGH_3DA_mhas folder
 *.wav in  ./MPEGH_3DA_ref folder

(b) Decode MPEG-H 3D audio conformance streams:
-----------------------------------------------
 Call 3DAudioDecoder[.exe] as described above (4).
 
 NOTE: To produce decoded waveforms at a specific conformance point (Cpo-<x>), see (4.2).

(c) Compare MPEG-H 3D audio conformance streams:
------------------------------------------------
 The correctness of the decoded output can be checked with either the  MPEG-4 conformance
 tool ssnrcd, or with the AFsp tool compAudio.

 The AFsp audio tools are located at
 - Linux/Mac: ./tools/AFsp/AFsp-v9r0/bin
 - Windows:   ./tools/AFsp/AFsp-v9r0/MSVC/bin

 Use the compAudio command for comparing two audio files:
 - Linux/Mac: ./compAudio <reference>.wav <MPEG-H_3DA_decoded>.wav
 - Windows:   compAudio.exe <reference>.wav <MPEG-H_3DA_decoded>.wav

 The ssnrcd tool is part of the MPEG-4 reference software (ISO/IEC 14496-5).
 The documentation of the MPEG-4 software describes how to build the ssnrcd tool.
 
 Use the following ssnrcd command for comparing two audio files:
 ./ssnrcd <reference>.wav <MPEG-H_3DA_decoded>.wav

(4.2) Conformance testing: Reproducing waveforms at a defined conformance point (Cpo-<x>)
=========================================================================================
 In order to call 3DAudioDecoder[.exe] so as to produce decoded waveforms at a specific
 conformance point (Cpo-<x>) as defined in ISO/IEC 23008-9 3D audio conformance testing, use
 the following decoder command line call:

 - Linux/Mac: ./3DAudioDecoder –if <input>.mhas -of <output>.wav -cpo <x>
 - Windows:   3DAudioDecoder.exe –if <input>.mhas -of <output>.wav -cpo <x>
  
 If <x> is equal to 1 the following conformance point shall be used: the output of the
 decoder before applying resampling and mixing.

 Default behavior:
 If the -cpo <x> command line switch is not specified (or if <x> is equal to 0), the
 conformance point shall be the output of the complete decoder chain.


(5) Tools overview
============================================================================================
 The reference software modules make use of different helper tools which are listed below:

 - AFsp library as described above

 - AFspCppWrap (Library/Commandline)
   . C++ wrapper for C++ interface, allows handling of AFsp raw pointer as shared pointer

 - cicp2geometry (Library/Commandline)
   . Used to convert CICP setups and CICP loudspeakers to geometry information
   . Used to convert geometry information to CICP setups
   . Used to read explicit geometry information from file
   . Used to write geometry information to file

 - fftlib (Library)
   . C-Implementation for the Fast Fourier Transform

 - libisomediafile (Library)
   . Library for reading and writing MP4 files

 - parDecorrelator (Library)
   . Library for decorrelation

 - qmflib (Library)
   . C-Implementation for QMF Analysis and Synthesis
   . C-Implementation for Hybrid Analysis and Synthesis

 - qmflibCppWrap
    . C++ wrapper for C++ interface, allows handling of qmflib raw pointer as shared pointer

 - readonlybitbuf (Library)
   . Library providing bitbuffer reading

 - separator (Library/Commandline)
   . Used to split combined channel/object multichannel wave files
     into a multichannel wave file containing channels only
     and multiple mono wave files containing single object tracks
   . Used to save a number of channels (starting from an offset) of
     an input wave file to a multichannel output wave file

 - wavCutterCmdl (Commandline)
   . Uses the wavIO library to process wave and qmf files
   . Used to cut wave files (remove/add delay in the beginning and remove flushing samples
     at the end of file)

 - wavIO (Library)
   . Library for reading and writing PCM data or qmf data into wave files

 - wavM2N (Commandline/Library)
   . Used to combine multiple mono wave files into a multichannel wave file
   . Used to split a multichannel wave file into multiple mono tracks
   . Used to combine/split object files
   . Supports the MPEG-H 3D audio file naming convention (azimuth, elevation)

 - writeonlybitbuf (Library)
   . Library providing bitbuffer writing


(6) Modules overview
============================================================================================
 The reference software is based on different separate modules which are listed below:

 - ascParser (Commandline/Library)
   . Parser for the Audio Specific Config of an mp4 file
   . Writes optional downmix matrices into files

 - binaural
   . All modules required for Frequency-domain and Time-domain binaural processing
   . Frequency Domain Binaural Renderer (Commandline/Library)
   . Time Domain Binaural Renderer (Commandline/Library)
   . Parametrization of BRIRs for the Frequency Domain Binaural Renderer (Commandline/Library)
   . Parametrization of BRIRs for the Time Domain Binaural Renderer (Commandline/Library)
   . BRIR Bitstream Writer (Commandline/Library)

 - coreCoder (Commandline)
   . USAC Reference Software based core encoder and decoder implementation

 - decoderInterfaces
   . elementInteractionInterface
     Module for reading and writing data according to the element interaction interface
   . localSetupInformationInterface
     Module for reading and writing data according to the local setup information interface

 - dmxMatrixCoder (Commandline/Library)
   . Downmix Matrix En- and Decoder with optional input channel equalizers

 - domainSwitcherCmdl (Commandline)
   . Perform a QMF Analysis and Synthesis to files
   . Read/write TD files with "*.wav" input/output
   . Read/write FD files with "*_real.qmf" and "*_imag.qmf" input/output

 - drcCoder (Commandline/Library)
   . DRC module for decoding DRC sequences and applying them to the audio signal
   . After executing the cmake build steps, the source files are located in the build folder

 - dynamicObjectPriorityGenerator
   . Module that generates dynamic object priority data

 - formatConverter (Commandline/Library)
   . Format Converter module converting input signals to different loudspeaker setups

 - gainDelayTrim (Shell Script Example)
   . Apply trim gains and delays to compensate for varying loudspeaker distances
   . Trim parameters are generated by the formatConverter module if
     loudspeaker distances are input to formatConverter module

 - gVBAPRenderer (Commandline/Library)
   . Generalized VBAP Renderer Module

 - hoaCoder
   . All modules required for HOA encoding and decoding

 - hoaMatrixCoder
   . Module to encode and decode a signaled HOA rendering matrix

 - iarFormatConverter (Commandline/Library)
   . Immersive format converter module converting input signals to different loudspeaker setups

 - metadataPreprocessor (Library)
   . Processing and parsing of metadata

 - mixer (Commandline/Library)
   . Mixer for combining two wave files (like objects and channels)
   . Offers delay adjustment between input files
   . Offers possibility to apply gains to the input files

 - oamCoder (Commandline/Library)
     Object Metadata En- and Decoder

 - saocCoder (Commandline/Library)
     Spatial Audio Object Coding En- and Decoder


(7) 3D audio encoder reference implementation
============================================================================================
 An example reference implementation of the MPEG-H 3D audio encoder can be found in:
   /modules/3DAudioCoder/3DAudioEncoder

 This encoder example shows how the different modules are connected to encode:
   - 22.2 channel based encoding
   - Objects based encoding
   - SAOC based object encoding
   - HOA based encoding


(8) 3D audio decoder reference implementation
============================================================================================
 The reference implementation of the MPEG-H 3D audio decoder can be found in:
   ./build/bin (Linux/Mac) or .\build\bin\Debug (Windows)

 This decoder connects the separate modules to decode the submitted test
 bitstreams for all test points including HOA and additionally supports
 metadata decoding and object rendering. Running the 3DAudioDecoder requires
 all separate modules to be located in the same directory.


(9) Technical Support
============================================================================================
 For technical support to build and run the MPEG-H 3D audio reference software,
 please direct your correspondence to:
   mpeg-h-3da-maintenance@iis.fraunhofer.de


(10) Revision History
============================================================================================
 See file changes.txt for a complete list of changes
