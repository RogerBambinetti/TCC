cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(dmxMatrixCoder)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# dmxMatrixCommonLib
add_library(dmxMatrixCommonLib
  dmxMatrixCommon/include/dmx_matrix_common.h
  dmxMatrixCommon/src/dmx_matrix_common.c
)
target_include_directories(dmxMatrixCommonLib PUBLIC "${PROJECT_SOURCE_DIR}/dmxMatrixCommon/include")
set_target_properties(dmxMatrixCommonLib PROPERTIES FOLDER "Libraries/modules")

# dmxMatrixDecLib
add_library(dmxMatrixDecLib
  dmxMatrixDecoder/dmxMatrixDecLib/include/dmx_matrix_dec.h
  dmxMatrixDecoder/dmxMatrixDecLib/src/dmx_matrix_dec.c
)
target_include_directories(dmxMatrixDecLib PUBLIC "${PROJECT_SOURCE_DIR}/dmxMatrixDecoder/dmxMatrixDecLib/include")
target_link_libraries(dmxMatrixDecLib dmxMatrixCommonLib readonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(dmxMatrixDecLib PROPERTIES FOLDER "Libraries/modules")

# dmxMatrixEncLib
add_library(dmxMatrixEncLib
  dmxMatrixEncoder/dmxMatrixEncLib/include/dmx_matrix_enc.h
  dmxMatrixEncoder/dmxMatrixEncLib/src/dmx_matrix_enc.c
)
target_include_directories(dmxMatrixEncLib PUBLIC "${PROJECT_SOURCE_DIR}/dmxMatrixEncoder/dmxMatrixEncLib/include")
target_link_libraries(dmxMatrixEncLib dmxMatrixCommonLib writeonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(dmxMatrixEncLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# dmxMatrixDecoderCmdl
add_executable(dmxMatrixDecoderCmdl
  dmxMatrixDecoder/dmxMatrixDecCmdl/src/main.c
)
target_compile_definitions(dmxMatrixDecoderCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(dmxMatrixDecoderCmdl dmxMatrixDecLib cicp2geometryLib)
set_target_properties(dmxMatrixDecoderCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS dmxMatrixDecoderCmdl DESTINATION bin)

# dmxMatrixEncoderCmdl
add_executable(dmxMatrixEncoderCmdl
  dmxMatrixEncoder/dmxMatrixEncCmdl/src/main.c
)
target_compile_definitions(dmxMatrixEncoderCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(dmxMatrixEncoderCmdl dmxMatrixEncLib cicp2geometryLib)
set_target_properties(dmxMatrixEncoderCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS dmxMatrixEncoderCmdl DESTINATION bin)
