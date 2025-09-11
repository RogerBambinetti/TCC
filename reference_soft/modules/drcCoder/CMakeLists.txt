cmake_minimum_required(VERSION 3.9.2)

# Attempt to beautify MSVC solution explorer
macro(group_targets TARGETS FOLDERNAME)
  foreach(TGT IN LISTS ${TARGETS})
    set_target_properties(${TGT} PROPERTIES FOLDER ${FOLDERNAME})
  endforeach()
endmacro()

get_directory_property(DRC_UTILITY_TARGETS DIRECTORY ${drclib_SOURCE_DIR} BUILDSYSTEM_TARGETS)
group_targets(DRC_UTILITY_TARGETS "MPEG-D DRC/Utility")

set(
  DRC_EXECUTABLE_TARGETS
    drcToolDecoder
    drcToolEncoder
    dmxCmdl
    loudnessNormalizerCmdl
    peakLimiterCmdl
    uniDrcBitstreamDecoderCmdl
    uniDrcGainDecoderCmdl
    uniDrcInterfaceDecoderCmdl
    uniDrcInterfaceEncoderCmdl
    uniDrcSelectionProcessCmdl
    qmf_example
    fft_example
    wavio_example
)

group_targets(DRC_EXECUTABLE_TARGETS "MPEG-D DRC/Executables")

set(
  DRC_LIBRARY_TARGETS
    peaklimiter
    readonlybitbuf
    uniDrcBitstreamDecoder
    uniDrcCommon_hostparams
    uniDrcCommon_tables
    uniDrcGainDecoder
    uniDrcInterfaceDecoder
    uniDrcInterfaceEncoder
    uniDrcSelectionProcess
    writeonlybitbuf
    xmlReader
    fft
    qmf
    wavio
)

group_targets(DRC_LIBRARY_TARGETS "MPEG-D DRC/Libraries")