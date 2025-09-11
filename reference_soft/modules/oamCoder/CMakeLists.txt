cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(oamCoder)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# oamCommonLib
add_library(oamCommonLib
  oamCommon/oam_common.h
  oamCommon/oam_common.c
)
target_compile_definitions(oamCommonLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(oamCommonLib PUBLIC "${PROJECT_SOURCE_DIR}/oamCommon")
set_target_properties(oamCommonLib PROPERTIES FOLDER "Libraries/modules")

# oamEncoderLib
add_library(oamEncoderLib
  oamEncoder/oamEncoderLib/include/oam_encode_ld.h
  oamEncoder/oamEncoderLib/include/oam_read.h
  oamEncoder/oamEncoderLib/include/oam_encode_rapV4.h
  oamEncoder/oamEncoderLib/src/oam_bitbuf_write.h
  oamEncoder/oamEncoderLib/src/oam_bitbuf_write.c
  oamEncoder/oamEncoderLib/src/oam_encode_ld.c
  oamEncoder/oamEncoderLib/src/oam_encode_rapV4.c
  oamEncoder/oamEncoderLib/src/oam_read.c
)
target_compile_definitions(oamEncoderLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(oamEncoderLib PUBLIC "${PROJECT_SOURCE_DIR}/oamEncoder/oamEncoderLib/include"
                                         PUBLIC "${PROJECT_SOURCE_DIR}/oamEncoder/oamEncoderLib/src")
target_link_libraries(oamEncoderLib oamCommonLib writeonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(oamEncoderLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# oamDecoder
add_executable(oamDecoder
  oamDecoder/src/oam_bitbuf_read.h
  oamDecoder/src/oam_decode_ldV1.h
  oamDecoder/src/oam_decode_rapV4.h
  oamDecoder/src/oam_bitbuf_read.c
  oamDecoder/src/oam_decode_ldV1.c
  oamDecoder/src/oam_decode_rapV4.c
  oamDecoder/src/oam_decoder.c
)
target_compile_definitions(oamDecoder PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(oamDecoder oamCommonLib readonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(oamDecoder PROPERTIES FOLDER "Executables/modules")
install(TARGETS oamDecoder DESTINATION bin)

# oamEncoderCmdl
add_executable(oamEncoderCmdl
  oamEncoder/oamEncoderCmdl/src/oam_encoder.c
)
target_compile_definitions(oamEncoderCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(oamEncoderCmdl oamEncoderLib oamCommonLib writeonlybitbuf_3da)
set_target_properties(oamEncoderCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS oamEncoderCmdl DESTINATION bin)
