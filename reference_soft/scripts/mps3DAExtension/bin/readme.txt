
(4) Running the software for CO_01 to CO_10
===========================================

Copy the following executables to the following location:

scripts_phase2/bin/3DAudioCoreDecoder.exe
scripts_phase2/bin/spatialdec.exe
scripts_phase2/bin/wavM2NCmdl.exe
scripts_phase2/bin/ResampAudio.exe 

(4) Running the software for decoding Phase 2 RM0
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

  