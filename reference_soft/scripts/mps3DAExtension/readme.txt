MPEG 3D Audio Reference Software of 3D Audio: Phase 2 MPS 3DA Extension
========================================

1) Content of this package
===========================
* Reference software for Phase 2 MPS 3DA Extension for Channel/object (CO) 3D Audio Phase 2 
* Reference software written in ANSI C/C++ (CO-Part)

(2) Prerequisites to successfully build and run the reference software
======================================================================

(2.1) Download and build AFsp Audio File I/O package
====================================================
The AFsp Audio File I/O Package
-------------------------------

current version:  AFsp-v9r0.tar.gz
download site:    http://www-mmsp.ece.mcgill.ca/documents/Downloads/AFsp/


Build the libtsplite library
----------------------------
Unpack the downloaded file and follow the instructions below:

*) Linux/Mac:
   - change to unpacked folder and start make
   - 64bit builds only:
     rename the resulting library libtsplite.a to libtsplite_x86_64.a 
     for proper linking on 64bit platforms
	 
*) Win32:
   - use the MSVC.sln project file in AFsp-v9r0\MSVC and select libtsplite

Finally the following files have to be copied to the following locations:

../../modules/mps3DAExtension/import/AFsp/include/libtsp.h
../../modules/import/AFsp/include/libtsp/UTpar.h
../../modules/import/AFsp/include/libtsp/AFpar.h
../../modules/import/AFsp/lib/libtsp.[a|lib]
bin/CopyAudio.exe (from /AFsp-v9r0/MSVC/bin/CopyAudio.exe) 
bin/ResampAudio.exe (from /AFsp-v9r0/MSVC/bin/ResampAudio.exe) 

Note:  In newer AFsp versions the libtsp library was named libtsplite. 
       Rename the library to libtsp to avoid linker errors.

	   
(2.2) Download and build the MPEG Surround reference software 

Download the MPEG-D MPEG Surround reference software v1.20 from: 
w15832, WD2 of ISO/IEC 23003-1:2007/AMD 4 Reference Software for MPEG Surround Extensions for 3D Audio, 113th MPEG meeting (Geneva, October 2015)

Unzip the content of mps-refsoft-v1.20(mps_3da).zip to the following location:

../../modules/mps3DAExtension/	   
	   
Finally build the MPS reference software
	
(3) Build Instructions for CO-Part
====================================

Build the 3D Audio Phase 1 RM6 reference software packages as described in the contained readme.txt file.

bin/3DAudioCoreDecoder.exe (from ../../modules/3DAudioCoder/3DAudioDecoder/make/Debug/3DAudioCoreDecoder.exe) 
bin/wavCutterCmdl.exe from ../../tools/wavCutter/wavCutterCmdl/make/Debug/wavCutterCmdl.exe) 
bin/wavM2NCmdl.exe (from ../../tools/wavM2N/wavM2NCmdl/make/Debug/wavM2NCmdl.exe) 

(4) Running the software for decoding Phase 2 bitstreams
===========================================

(4.1) Configurations without MPS Extension:
- 48, 64, 96 and 128 kbps RM0    items CO_11 and CO_12
- 96 and 128 kbps RM0            items CO_04, CO_05, CO_06, CO_07, CO_08 and CO_09
- 96 and 128 kbps IIS Benchmark  items CO_01 : CO_10

run the script "decode3DAPhase2.[bat/sh]" by giving the input and output directories, the path to the binary configuration file and the decoder module
  e.g. decode3DAPhase2.bat .\input .\output .\cfg-Files\binaries_Win32_Debug.cfg .\make\Debug\3DAudioDecoder.exe 

(4.2) Configurations with MPS Extension:
 - 96 and 128 kbps RM0          items CO_01, CO_02, CO_03, CO_10, Air, Car and Gun
 - 48 and 64 kbps RM0           items CO_01 : CO_10 
 
 - run the script "decode3DAPhase2_MPS.[bat]" by giving the input, output directories and the bitrate
  e.g. decode3DAPhase2_MPS.bat .\input .\output 48
  



  

  


  
  

