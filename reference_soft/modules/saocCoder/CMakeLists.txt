cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(saocCoder)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# saocDecoderLib
add_library(saocDecoderLib
  saocDecoderLib/include/error.h
  saocDecoderLib/include/hierChnlGrouping.h
  saocDecoderLib/include/saoc_bitinput.h
  saocDecoderLib/include/saoc_bitstream.h
  saocDecoderLib/include/saoc_const.h
  saocDecoderLib/include/saoc_decode.h
  saocDecoderLib/include/saoc_decor.h
  saocDecoderLib/include/saoc_decorr_tables.h
  saocDecoderLib/include/saoc_dmx_tables.h
  saocDecoderLib/include/saoc_drc_lfe.h
  saocDecoderLib/include/saoc_getinfo.h
  saocDecoderLib/include/saoc_huff_nodes.h
  saocDecoderLib/include/saoc_interpolate.h
  saocDecoderLib/include/saoc_kernels.h
  saocDecoderLib/include/saoc_machine.h
  saocDecoderLib/include/saoc_nlc_dec.h
  saocDecoderLib/include/saoc_render.h
  saocDecoderLib/include/saoc_spatial_filereader.h
  saocDecoderLib/include/saoc_svd_tool.h
  saocDecoderLib/include/saocdeclib.h
  saocDecoderLib/src/error.c
  saocDecoderLib/src/hierChnlGrouping.c
  saocDecoderLib/src/saoc_bitinput.c
  saocDecoderLib/src/saoc_bitstream.c
  saocDecoderLib/src/saoc_decode.c
  saocDecoderLib/src/saoc_decor.c
  saocDecoderLib/src/saoc_drc_lfe.c
  saocDecoderLib/src/saoc_interpolate.c
  saocDecoderLib/src/saoc_kernels.c
  saocDecoderLib/src/saoc_nlc_dec.c
  saocDecoderLib/src/saoc_nodes_cld.c
  saocDecoderLib/src/saoc_nodes_cpc.c
  saocDecoderLib/src/saoc_nodes_icc.c
  saocDecoderLib/src/saoc_nodes_nrg.c
  saocDecoderLib/src/saoc_nodes_old.c
  saocDecoderLib/src/saoc_nodes_pt0.c
  saocDecoderLib/src/saoc_render.c
  saocDecoderLib/src/saoc_spatial_filereader.c
  saocDecoderLib/src/saoc_svd_tool.c
  saocDecoderLib/src/saocdeclib.c
)
target_compile_definitions(saocDecoderLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(saocDecoderLib PUBLIC "${PROJECT_SOURCE_DIR}/saocDecoderLib/include")
target_link_libraries(saocDecoderLib qmflib_3da wavIO_3da cicp2geometryLib oamCommonLib)
set_target_properties(saocDecoderLib PROPERTIES FOLDER "Libraries/modules")

# saocEncoderLib
add_library(saocEncoderLib
  saocEncoderLib/include/const.h
  saocEncoderLib/include/defines.h
  saocEncoderLib/include/error.h
  saocEncoderLib/include/saoc_bitinput.h
  saocEncoderLib/include/saoc_bitstream.h
  saocEncoderLib/include/saoc_enc.h
  saocEncoderLib/include/saoc_huff_tab.h
  saocEncoderLib/include/saoc_nlc_enc.h
  saocEncoderLib/include/saoc_sbrconst.h
  saocEncoderLib/include/saoc_stream.h
  saocEncoderLib/include/saoc_types.h
  saocEncoderLib/include/spatial_filereader.h
  saocEncoderLib/src/saoc_bitinput.c
  saocEncoderLib/src/saoc_bitstream.c
  saocEncoderLib/src/saoc_enc.c
  saocEncoderLib/src/saoc_huff_cld.c
  saocEncoderLib/src/saoc_huff_cpc.c
  saocEncoderLib/src/saoc_huff_icc.c
  saocEncoderLib/src/saoc_huff_nrg.c
  saocEncoderLib/src/saoc_huff_old.c
  saocEncoderLib/src/saoc_nlc_enc.c
  saocEncoderLib/src/saoc_stream.c
)
target_compile_definitions(saocEncoderLib PRIVATE RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX _CRT_SECURE_NO_WARNINGS)
target_include_directories(saocEncoderLib PUBLIC "${PROJECT_SOURCE_DIR}/saocEncoderLib/include")
target_link_libraries(saocEncoderLib qmflib_3da)
set_target_properties(saocEncoderLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# saocDecoderCmdl
add_executable(saocDecoderCmdl
  saocDecoderCmdl/include/saocDecoder.h
  saocDecoderCmdl/src/saocDecoder.c
)
# increase stack size to prevent stack overflow
target_link_options(
    saocDecoderCmdl
    PRIVATE
        $<$<C_COMPILER_ID:MSVC>:/STACK:5000000>
    )

target_compile_definitions(saocDecoderCmdl PRIVATE RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX
                                                   NO_WD2_SAOC_FLEXIBLE_LAYOUT_3D
                                                   NO_RM2_SAOC_USE_HACCED
                                                   NO_RM2_3D_BUGFIX_SAOC_1
                                                   NO_RM2_3D_BUGFIX_SAOC_2
                                                   NO_RM2_3D_BUGFIX_SAOC_3
                                                   _CRT_SECURE_NO_WARNINGS)
target_include_directories(saocDecoderCmdl PRIVATE "${PROJECT_SOURCE_DIR}/saocDecoderCmdl/include")
target_link_libraries(saocDecoderCmdl saocDecoderLib oamEncoderLib gVBAPRendererLib)
set_target_properties(saocDecoderCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS saocDecoderCmdl DESTINATION bin)

# saocEncoderCmdl
add_executable(saocEncoderCmdl
  saocEncoderCmdl/include/saoc_encoder.h
  saocEncoderCmdl/src/saoc_encoder.c
)
target_compile_definitions(saocEncoderCmdl PRIVATE RM0_3D_BUGFIX_SAOC_BITSTREAM_SYNTAX _CRT_SECURE_NO_WARNINGS)
target_include_directories(saocEncoderCmdl PRIVATE "${PROJECT_SOURCE_DIR}/saocEncoderCmdl/include")
target_link_libraries(saocEncoderCmdl saocEncoderLib wavIO_3da cicp2geometryLib ${ADD_MATH_LIB})
set_target_properties(saocEncoderCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS saocEncoderCmdl DESTINATION bin)
