cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(hoaMatrixCoder)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# hoaMatrixComLib
add_library(hoaMatrixComLib
  hoaMatrixCommon/include/hoa_matrix_common.h
  hoaMatrixCommon/src/hoa_matrix_common.c
)
target_include_directories(hoaMatrixComLib PUBLIC "${PROJECT_SOURCE_DIR}/hoaMatrixCommon/include")
set_target_properties(hoaMatrixComLib PROPERTIES FOLDER "Libraries/modules")

# hoaMatrixDecLib
add_library(hoaMatrixDecLib
  hoaMatrixDecoder/hoaMatrixDecLib/include/hoa_matrix_dec.h
  hoaMatrixDecoder/hoaMatrixDecLib/src/hoa_matrix_dec.c
)
target_include_directories(hoaMatrixDecLib PUBLIC "${PROJECT_SOURCE_DIR}/hoaMatrixDecoder/hoaMatrixDecLib/include")
target_link_libraries(hoaMatrixDecLib hoaMatrixComLib readonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(hoaMatrixDecLib PROPERTIES FOLDER "Libraries/modules")

# hoaMatrixEncLib
add_library(hoaMatrixEncLib
  hoaMatrixEncoder/hoaMatrixEncLib/include/hoa_matrix_enc.h
  hoaMatrixEncoder/hoaMatrixEncLib/src/hoa_matrix_enc.c
)
target_include_directories(hoaMatrixEncLib PUBLIC "${PROJECT_SOURCE_DIR}/hoaMatrixEncoder/hoaMatrixEncLib/include")
target_link_libraries(hoaMatrixEncLib hoaMatrixComLib writeonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(hoaMatrixEncLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# hoaMatrixDecoderCmdl
add_executable(hoaMatrixDecoderCmdl
  hoaMatrixDecoder/hoaMatrixDecCmdl/src/main.c
)

target_compile_definitions(hoaMatrixDecoderCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(hoaMatrixDecoderCmdl cicp2geometryLib hoaMatrixDecLib)
set_target_properties(hoaMatrixDecoderCmdl PROPERTIES FOLDER "Executables/HOA")
install(TARGETS hoaMatrixDecoderCmdl DESTINATION bin)

# hoaMatrixEncoderCmdl
add_executable(hoaMatrixEncoderCmdl
  hoaMatrixEncoder/hoaMatrixEncCmdl/src/main.c
)
target_compile_definitions(hoaMatrixEncoderCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(hoaMatrixEncoderCmdl cicp2geometryLib hoaMatrixEncLib)
set_target_properties(hoaMatrixEncoderCmdl PROPERTIES FOLDER "Executables/HOA")
install(TARGETS hoaMatrixEncoderCmdl DESTINATION bin)
