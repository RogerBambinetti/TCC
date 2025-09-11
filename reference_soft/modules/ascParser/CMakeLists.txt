cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(ascParser)

# include_directories("${PROJECT_SOURCE_DIR}/ascParserLib/include")
# include_directories("${PROJECT_SOURCE_DIR}/ascParserLib/src")
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/cicp2geometry/cicp2geometryLib/include")
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/IsoLib/libisomediafile/src")
# if(WIN32)
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/IsoLib/libisomediafile/w32")
# endif()
# if(APPLE)
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/IsoLib/libisomediafile/macosx")
# endif()
# if(UNIX AND NOT APPLE)
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/IsoLib/libisomediafile/linux")
# endif()
# include_directories("${PROJECT_SOURCE_DIR}/../../tools/IsoLib/mpegh_in_isom/src")

###############################################################################
# target sources (libraries)
###############################################################################
# ascParserLib
add_library(ascParserLib
  ascParserLib/include/ascparser.h
  ascParserLib/src/ascparser_aots.h
  ascParserLib/src/ascparser_asc.h
  ascParserLib/src/ascparser_bitbuf.h
  ascParserLib/src/ascparser_bitstream.h
  ascParserLib/src/ascparser_machine.h
  ascParserLib/src/ascparser_metadata.h
  ascParserLib/src/ascparser_mhas.h
  ascParserLib/src/ascparser_parse.h
  ascParserLib/src/ascparser_stream_wrap.h
  ascParserLib/src/ascparser_usacconfig.h
  ascParserLib/src/ascparser.c
  ascParserLib/src/ascparser_asc.c
  ascParserLib/src/ascparser_bitbuf.c
  ascParserLib/src/ascparser_bitstream.c
  ascParserLib/src/ascparser_metadata.c
  ascParserLib/src/ascparser_mhas.c
  ascParserLib/src/ascparser_parse.c
  ascParserLib/src/ascparser_stream_wrap.c
  ascParserLib/src/ascparser_usacconfig.c
)
target_compile_definitions(ascParserLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(ascParserLib PUBLIC "${PROJECT_SOURCE_DIR}/ascParserLib/include"
                                        PUBLIC "${PROJECT_SOURCE_DIR}/ascParserLib/src")

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()
target_link_libraries(ascParserLib libisomediafile mpegh_in_isom cicp2geometryLib ${ADD_MATH_LIB})
set_target_properties(ascParserLib PROPERTIES FOLDER "Libraries/modules")

###############################################################################
# target sources (executables)
###############################################################################
# ascParserCmdl
add_executable(ascParserCmdl
  ascParserCmdl/src/ascParserCmdl.c
)
target_compile_definitions(ascParserCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(ascParserCmdl ascParserLib)
set_target_properties(ascParserCmdl PROPERTIES FOLDER "Executables/modules")
install(TARGETS ascParserCmdl DESTINATION bin)
