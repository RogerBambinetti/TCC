cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(earconCoder)

###############################################################################
# target sources (libraries)
###############################################################################
# earconCommonLib
add_library(earconCommonLib
  src/common/earconInfo.h
  src/common/mhas.h
  src/common/pcmDataConfig.h
  src/common/pcmDataPayload.h
  src/common/printHelper.h
  src/common/twosComplement.h
  src/common/earconInfo.c
  src/common/pcmDataConfig.c
  src/common/pcmDataPayload.c
  src/common/printHelper.c
  src/common/twosComplement.c
)
target_compile_definitions(earconCommonLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(earconCommonLib PUBLIC "${PROJECT_SOURCE_DIR}/src/common")
set_target_properties(earconCommonLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# earconDecoder
add_executable(earconDecoder
  src/earconDecoder/bitstreamReader.h
  src/earconDecoder/bsWriter.h
  src/earconDecoder/earconInfoReader.h
  src/earconDecoder/mhasReader.h
  src/earconDecoder/pcmDataConfigReader.h
  src/earconDecoder/pcmDataPayloadReader.h
  src/earconDecoder/wavWriter.h
  src/earconDecoder/bitstreamReader.c
  src/earconDecoder/bsWriter.c
  src/earconDecoder/earconDecoder.c
  src/earconDecoder/earconInfoReader.c
  src/earconDecoder/mhasReader.c
  src/earconDecoder/pcmDataConfigReader.c
  src/earconDecoder/pcmDataPayloadReader.c
  src/earconDecoder/wavWriter.c
)
target_compile_definitions(earconDecoder PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(earconDecoder PUBLIC "${PROJECT_SOURCE_DIR}/src/earconDecoder")
target_link_libraries(earconDecoder earconCommonLib readonlybitbuf_3da wavIO_3da)
set_target_properties(earconDecoder PROPERTIES FOLDER "Executables/earconCoder")
install(TARGETS earconDecoder DESTINATION bin)

# earconEncoder
add_executable(earconEncoder
  src/earconEncoder/earconEncoder.c
  src/earconEncoder/earconInfoWriter.h
  src/earconEncoder/mhasWriter.h
  src/earconEncoder/pcmDataConfigWriter.h
  src/earconEncoder/pcmDataPayloadWriter.h
  src/earconEncoder/earconInfoWriter.c
  src/earconEncoder/mhasWriter.c
  src/earconEncoder/pcmDataConfigWriter.c
  src/earconEncoder/pcmDataPayloadWriter.c
)
target_compile_definitions(earconEncoder PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(earconEncoder PUBLIC "${PROJECT_SOURCE_DIR}/src/earconEncoder")
target_link_libraries(earconEncoder earconCommonLib writeonlybitbuf_3da wavIO_3da)
set_target_properties(earconEncoder PROPERTIES FOLDER "Executables/earconCoder")
install(TARGETS earconEncoder DESTINATION bin)
