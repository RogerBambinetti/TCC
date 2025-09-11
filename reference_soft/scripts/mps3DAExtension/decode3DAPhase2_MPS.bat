@echo off

set exe=%0
set inputdir=%1
set outputdir=%2
set bitrate=%3
set tmpdir=.\tmp

set coredecbin=".\bin\3DAudioCoreDecoder.exe"
set mpsdecbin=".\bin\spatialdec.exe"
set wavCutter=".\bin\wavCutterCmdl.exe"
set Resamp=".\bin\ResampAudio.exe"
set wavM2N=".\bin\wavM2NCmdl.exe"



IF "%inputdir%" == "" (
    echo invalid arguments
    echo Usage
    echo %exe% ^<path_to_input_mp4_directory^> ^<path_to_output_wav^> 
	echo Example:
	echo %exe% .\input .\output 
	
    GOTO END    

) 

echo %coredecbin%

if exist %coredecbin% (
    echo Using decoder binary %coredecbin%
) else (
    echo Decoder binary %coredecbin% not found
    echo Exiting
    GOTO END
)

if exist %mpsdecbin% (
    echo Using MPS Decoder binary  %mpsdecbin%
) else (
    echo MPS Decoder binary  %mpsdecbin% not found
    echo Exiting
    GOTO END
)

if "%bitrate%"=="48" (
	set delayMPS=5057
	set flags=-cicpIn -1 -inGeo .\cfg\geo_9.1a.txt
)
if "%bitrate%"=="64" (
	set delayMPS=5057
	set flags=-cicpIn -1 -inGeo .\cfg\geo_9.1a.txt
)

if "%bitrate%"=="96" (
	set delayMPS=2241
	set flags=-cicpIn -1 -inGeo .\cfg\geo_22.2_MPS.txt
)
if "%bitrate%"=="128" (
	set delayMPS=2241
	set flags=-cicpIn -1 -inGeo .\cfg\geo_22.2_MPS.txt
)

FOR  %%f IN ("%inputdir%\\*.mp4") DO (
        mkdir %outputdir%\%%~nf
        echo %basefile%
        echo Decoding:
        echo %%f
		
	
		echo %coredecbin% -if %%f -of %tmpdir%\%%~nf_outCore.wav
		%coredecbin% -if %%f -of %tmpdir%\%%~nf_outCore.wav
				
		echo %mpsdecbin% 0 %tmpdir%\%%~nf_outCore.wav %inputdir%\%%~nf.bs  %tmpdir%\%%~nf_outMPS.wav	
		%mpsdecbin% 0 %tmpdir%\%%~nf_outCore.wav %inputdir%\%%~nf.bs  %tmpdir%\%%~nf_outMPS.wav
				
		echo %wavCutter% -delay  %delayMPS% -if %tmpdir%\%%~nf_outMPS.wav -of %tmpdir%\%%~nf_outMPS_noDelay.wav	
		%wavCutter% -delay  %delayMPS% -if %tmpdir%\%%~nf_outMPS.wav -of %tmpdir%\%%~nf_outMPS_noDelay.wav
		
		echo %Resamp% -s 48000 -D integer24 %tmpdir%\%%~nf_outMPS_noDelay.wav %tmpdir%\%%~nf_outMPS_noDelay_resamp.wav 	
		%Resamp% -s 48000 -D integer24 %tmpdir%\%%~nf_outMPS_noDelay.wav %tmpdir%\%%~nf_outMPS_noDelay_resamp.wav 
		
		echo %wavM2N% -if %tmpdir%\%%~nf_outMPS_noDelay_resamp -of %outputdir%\%%~nf  %flags% -dir 1	
		%wavM2N% -if %tmpdir%\%%~nf_outMPS_noDelay_resamp -of %outputdir%\%%~nf\%%~nf %flags% -dir 1

)


:END