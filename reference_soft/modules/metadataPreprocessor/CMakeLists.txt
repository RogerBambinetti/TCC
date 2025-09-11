cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(metadataPreprocessor)

###############################################################################
# target sources (libraries)
###############################################################################
# metadataPreprocessorLib
add_library(metadataPreprocessorLib
  metadataPreprocessorLib/include/metadataPreprocessor.h
  metadataPreprocessorLib/include/sceneDisplacementProcessing.h
  metadataPreprocessorLib/include/screenRelatedProcessing.h
  metadataPreprocessorLib/src/metadataPreprocessor.c
  metadataPreprocessorLib/src/sceneDisplacementProcessing.c
  metadataPreprocessorLib/src/screenRelatedProcessing.c
)
target_compile_definitions(metadataPreprocessorLib PRIVATE _FTRD _X86_VC8_TARGET_ CODE_ANALYSIS CODE_ANALYSIS _CRT_SECURE_NO_WARNINGS)
target_include_directories(metadataPreprocessorLib PUBLIC "${PROJECT_SOURCE_DIR}/metadataPreprocessorLib/include")
target_link_libraries(metadataPreprocessorLib
  ascParserLib
  elementInteractionInterfaceLib
  localSetupInformationInterfaceLib
  sceneDisplacementInterfaceLib
  oamEncoderLib
  qmflib_3da
  wavIO_3da
)
set_target_properties(metadataPreprocessorLib PROPERTIES FOLDER "Libraries/modules")
