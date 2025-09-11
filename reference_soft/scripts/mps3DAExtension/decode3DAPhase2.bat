@echo off

set exe=%0
set inputdir=%1
set outputdir=%2
set configfile=%3
set decoderbin=%4

IF "%inputdir%" == "" (
    echo invalid arguments
    echo Usage
    echo %exe% ^<path_to_input_mp4_directory^> ^<path_to_output_wav^> ^<config_file^> ^<decoder_bin^> 
	echo Example:
	echo %exe% .\input .\output .\cfg-Files\binaries_Win32_Debug.cfg .\make\Debug\3DAudioDecoder.exe 
	
    GOTO END    

) 


if exist %decoderbin% (
    echo Using decoder binary %decoderbin%
) else (
    echo Decoder binary %decoderbin% not found
    echo Exiting
    GOTO END
)

if exist %configfile% (
    echo Using config file  %configfile%
) else (
    echo Config file %configfile% not found
    echo Exiting
    GOTO END
)

FOR  %%f IN ("%inputdir%\\*.mp4") DO (
        mkdir %outputdir%\%%~nf
        echo %basefile%
        echo Decoding:
        echo %%f
        echo %decoderbin% -cfg %configFile% -if %%f -of %outputdir%\%%~nf\%%~nf.wav -mono 1

        %decoderbin% -cfg %configFile% -if %%f -of %outputdir%\%%~nf\%%~nf.wav -mono 1


)


:END