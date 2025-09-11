#!/bin/bash

############# Variables #################

inputdir=$1
outputdir=$2
configfile=$3
decoderbin=$4

if [ $# != 5 ]; then
    echo invalid arguments
    echo "Usage:"
    echo "$0 <path_to_input_mp4_directory> <path_to_output_wav> <config_file> <decoder_bin> "
	echo "Example:"
	echo "$0 .\input .\output .\cfg-Files\binaries_Win32_Debug.cfg .\make\Debug\3DAudioDecoder.exe" 
	
    exit
fi

if [ -f $decoderbin ]; then
    echo Using decoder binary $decoderbin
else
    echo Decoder binary $decoderbin not found
    echo Exiting
    exit 1;
fi

if [ -f $configfile ]; then
    echo Using config file $configfile
else
    echo Config file $configfile not found
    echo Exiting
    exit 1;
fi


for file in $inputdir/*.mp4
do
basefile=`basename $file .mp4`
mkdir -p $outputdir/$basefile
  echo Decoding: 
  echo $file
  echo  $decoderbin -cfg $configfile -if $file -of $outputdir/$basefile/$basefile  -mono 1
  $decoderbin -cfg $configfile -if $file -of $outputdir/$basefile/$basefile -mono 1
done
