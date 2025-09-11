cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(Tools)

###############################################################################
# target sources (libraries)
###############################################################################

if(UNIX AND NOT APPLE)
  set(ADD_MATH_LIB m)
endif()

# cicp2geometryLib
add_library(cicp2geometryLib
  cicp2geometry/cicp2geometryLib/include/cicp2geometry.h
  cicp2geometry/cicp2geometryLib/src/cicp2geometry.c
)
target_include_directories(cicp2geometryLib
  PUBLIC "${PROJECT_SOURCE_DIR}/cicp2geometry/cicp2geometryLib/include"
)
target_compile_definitions(cicp2geometryLib PRIVATE _CRT_SECURE_NO_WARNINGS)
set_target_properties(cicp2geometryLib PROPERTIES FOLDER "Libraries/tools")

# fftlib_3da
add_library(fftlib_3da
  fftlib/include/fftlib.h
  fftlib/src/fftlib.c
)
target_include_directories(fftlib_3da PUBLIC "${PROJECT_SOURCE_DIR}/fftlib/include")
target_link_libraries(fftlib_3da ${ADD_MATH_LIB})
set_target_properties(fftlib_3da PROPERTIES FOLDER "Libraries/tools")

# libisomediafile
add_subdirectory(IsoLib/libisomediafile)

# libtsplite
file(GLOB_RECURSE libtsplite_cFILES ${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/libtsp/*.c)
file(GLOB_RECURSE libtsplite_hFILES ${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/libtsp/*.h)
add_library(libtsplite
  ${libtsplite_cFILES}
  ${libtsplite_hFILES}
)
target_include_directories(libtsplite PUBLIC "${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/include")
set_target_properties(libtsplite PROPERTIES FOLDER "Libraries/tools")

# libAO
file(GLOB_RECURSE libAO_cFILES ${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/libAO/*.c)
file(GLOB_RECURSE libAO_hFILES ${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/libAO/*.h)
add_library(libAO
  ${libAO_cFILES}
  ${libAO_hFILES}
)
target_include_directories(libAO PUBLIC "${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/include")
target_link_libraries(libAO libtsplite)
set_target_properties(libAO PROPERTIES FOLDER "Libraries/tools")

# mpegh_in_isom
add_library(mpegh_in_isom
  IsoLib/mpegh_in_isom/src/MHAConfigurationAtom.c
  IsoLib/mpegh_in_isom/src/MHASampleEntryAtom.c
  IsoLib/mpegh_in_isom/src/MPEGHAtoms.c
  IsoLib/mpegh_in_isom/src/MPEGHMovies.c
  IsoLib/mpegh_in_isom/src/MPEGHAtoms.h
  IsoLib/mpegh_in_isom/src/MPEGHMovies.h
  IsoLib/mpegh_in_isom/src/MPEGHProfileAndLevelAtom.c
)
target_compile_definitions(mpegh_in_isom PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(mpegh_in_isom PUBLIC "${PROJECT_SOURCE_DIR}/IsoLib/mpegh_in_isom/src")
target_link_libraries(mpegh_in_isom libisomediafile)
set_target_properties(mpegh_in_isom PROPERTIES FOLDER "Libraries/tools")

# parDecorrelator
add_library(parDecorrelator
  parDecorrelator/include/parDecorrFilter.h
  parDecorrelator/include/par_decorr.h
  parDecorrelator/src/par_decorr.cpp
  parDecorrelator/src/parDecorrFilter.cpp
)
target_include_directories(parDecorrelator PUBLIC "${PROJECT_SOURCE_DIR}/parDecorrelator/include"
                                          PRIVATE "${PROJECT_SOURCE_DIR}/../modules/hoaCoder/common/include")
set_target_properties(parDecorrelator PROPERTIES FOLDER "Libraries/tools")

# qmflib_3da
add_library(qmflib_3da
  qmflib/include/qmflib.h
  qmflib/include/qmflib_const.h
  qmflib/include/qmflib_hybfilter.h
  qmflib/src/qmflib.c
  qmflib/src/qmflib_hybfilter.c
)
target_include_directories(qmflib_3da PUBLIC "${PROJECT_SOURCE_DIR}/qmflib/include")
set_target_properties(qmflib_3da PROPERTIES FOLDER "Libraries/tools")

# qmflibCppWrap
add_library(qmflibCppWrap
  qmflibCppWrap/include/QMF_Wrapper.h
  qmflibCppWrap/src/QMF_Wrapper.cpp
)
target_include_directories(qmflibCppWrap PUBLIC "${PROJECT_SOURCE_DIR}/qmflibCppWrap/include"
                                        PRIVATE "${PROJECT_SOURCE_DIR}/../modules/hoaCoder/common/include")
target_link_libraries(qmflibCppWrap qmflib_3da)
set_target_properties(qmflibCppWrap PROPERTIES FOLDER "Libraries/tools")

# readonlybitbuf_3da
add_library(readonlybitbuf_3da
  readonlybitbuf/include/readonlybitbuf.h
  readonlybitbuf/src/readonlybitbuf.c
)
target_include_directories(readonlybitbuf_3da PUBLIC "${PROJECT_SOURCE_DIR}/readonlybitbuf/include")
set_target_properties(readonlybitbuf_3da PROPERTIES FOLDER "Libraries/tools")

# separatorLib
add_library(separatorLib
  separator/separatorLib/include/separatorlib.h
  separator/separatorLib/src/separatorlib.c
)
target_include_directories(separatorLib PUBLIC "${PROJECT_SOURCE_DIR}/separator/separatorLib/include")
target_link_libraries(separatorLib wavIO_3da)
set_target_properties(separatorLib PROPERTIES FOLDER "Libraries/tools")

# wavIO_3da
add_library(wavIO_3da
  wavIO/include/wavIO.h
  wavIO/src/wavIO.c
)
target_compile_definitions(wavIO_3da PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(wavIO_3da PUBLIC "${PROJECT_SOURCE_DIR}/wavIO/include")
set_target_properties(wavIO_3da PROPERTIES FOLDER "Libraries/tools")

# wavReader
add_library(wavReader
  AFspCppWrap/common/include/AFileWrapper.h
  AFspCppWrap/wavReader/include/wavReader.h
  AFspCppWrap/wavReader/src/wavReader.cpp
)
target_include_directories(wavReader PUBLIC "${PROJECT_SOURCE_DIR}/AFspCppWrap/wavReader/include"
                                    PRIVATE "${PROJECT_SOURCE_DIR}/AFspCppWrap/common/include"
                                            "${PROJECT_SOURCE_DIR}/../modules/hoaCoder/common/include")
target_link_libraries(wavReader libtsplite)
set_target_properties(wavReader PROPERTIES FOLDER "Libraries/tools")

# wavWriter
add_library(wavWriter
  AFspCppWrap/common/include/AFileWrapper.h
  AFspCppWrap/wavWriter/include/wavWriter.h
  AFspCppWrap/wavWriter/src/wavWriter.cpp
)
target_include_directories(wavWriter PUBLIC "${PROJECT_SOURCE_DIR}/AFspCppWrap/wavWriter/include"
                                    PRIVATE "${PROJECT_SOURCE_DIR}/AFspCppWrap/common/include"
                                            "${PROJECT_SOURCE_DIR}/../modules/hoaCoder/common/include")
target_link_libraries(wavWriter libtsplite)
set_target_properties(wavWriter PROPERTIES FOLDER "Libraries/tools")

# wavM2NLib
add_library(wavM2NLib
  wavM2N/wavM2NLib/include/wavM2Nlib.h
  wavM2N/wavM2NLib/src/wavM2Nlib.c
)
target_compile_definitions(wavM2NLib PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(wavM2NLib PUBLIC "${PROJECT_SOURCE_DIR}/wavM2N/wavM2NLib/include")
target_link_libraries(wavM2NLib cicp2geometryLib wavIO_3da)
set_target_properties(wavM2NLib PROPERTIES FOLDER "Libraries/tools")

# writeonlybitbuf_3da
add_library(writeonlybitbuf_3da
  writeonlybitbuf/include/writeonlybitbuf.h
  writeonlybitbuf/src/writeonlybitbuf.c
)
target_include_directories(writeonlybitbuf_3da PUBLIC "${PROJECT_SOURCE_DIR}/writeonlybitbuf/include")
set_target_properties(writeonlybitbuf_3da PROPERTIES FOLDER "Libraries/tools")

###############################################################################
# target sources (executables)
###############################################################################
# cicp2geometryCmdl
add_executable(cicp2geometryCmdl
  cicp2geometry/cicp2geometryCmdl/src/cicp2geometryCmdl.c
)
target_compile_definitions(cicp2geometryCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(cicp2geometryCmdl cicp2geometryLib)
set_target_properties(cicp2geometryCmdl PROPERTIES FOLDER "Executables/tools")
install(TARGETS cicp2geometryCmdl DESTINATION bin)

# prodmetadataEncoder
add_executable(prodmetadataEncoder
  prodmetadata/src/prodmetadataEncoder/dynamicProdMetadataFrameWriter.c
  prodmetadata/src/prodmetadataEncoder/dynamicProdMetadataFrameWriter.h
  prodmetadata/src/prodmetadataEncoder/intracodedProdMetadataFrameWriter.c
  prodmetadata/src/prodmetadataEncoder/intracodedProdMetadataFrameWriter.h
  prodmetadata/src/prodmetadataEncoder/encPmcWriter.c
  prodmetadata/src/prodmetadataEncoder/encPmcWriter.h
  prodmetadata/src/prodmetadataEncoder/encPmfReader.c
  prodmetadata/src/prodmetadataEncoder/encPmfReader.h
  prodmetadata/src/prodmetadataEncoder/encPmfWriter.c
  prodmetadata/src/prodmetadataEncoder/encPmfWriter.h
  prodmetadata/src/prodmetadataEncoder/prodMetadataConfigWriter.c
  prodmetadata/src/prodmetadataEncoder/prodMetadataConfigWriter.h
  prodmetadata/src/prodmetadataEncoder/prodMetadataEncoder.c
  prodmetadata/src/prodmetadataEncoder/prodMetadataFrameWriter.c
  prodmetadata/src/prodmetadataEncoder/prodMetadataFrameWriter.h
  prodmetadata/src/prodmetadataEncoder/singleDynamicProdMetadataFrameWriter.c
  prodmetadata/src/prodmetadataEncoder/singleDynamicProdMetadataFrameWriter.h
  prodmetadata/src/common/omfReader.c
  prodmetadata/src/common/omfReader.h
  prodmetadata/src/common/printHelper.c
  prodmetadata/src/common/printHelper.h
  prodmetadata/src/common/tables.h
)
target_compile_definitions(prodmetadataEncoder PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(prodmetadataEncoder PRIVATE "${PROJECT_SOURCE_DIR}/prodmetadata/src/prodmetadataEncoder"
                                                       "${PROJECT_SOURCE_DIR}/prodmetadata/src/common"
                                                       "${PROJECT_SOURCE_DIR}/writeonlybitbuf/include")
target_link_libraries(prodmetadataEncoder writeonlybitbuf_3da ${ADD_MATH_LIB})
set_target_properties(prodmetadataEncoder PROPERTIES FOLDER "Executables/tools")
install(TARGETS prodmetadataEncoder DESTINATION bin)

# prodmetadataDecoder
add_executable(prodmetadataDecoder
  prodmetadata/src/prodmetadataDecoder/decPmcReader.c
  prodmetadata/src/prodmetadataDecoder/decPmcReader.h
  prodmetadata/src/prodmetadataDecoder/decPmfReader.c
  prodmetadata/src/prodmetadataDecoder/decPmfReader.h
  prodmetadata/src/prodmetadataDecoder/dynamicProdMetadataFrameReader.c
  prodmetadata/src/prodmetadataDecoder/dynamicProdMetadataFrameReader.h
  prodmetadata/src/prodmetadataDecoder/intracodedProdMetadataFrameReader.c
  prodmetadata/src/prodmetadataDecoder/intracodedProdMetadataFrameReader.h
  prodmetadata/src/prodmetadataDecoder/prodMetadataConfigReader.c
  prodmetadata/src/prodmetadataDecoder/prodMetadataConfigReader.h
  prodmetadata/src/prodmetadataDecoder/prodMetadataDecoder.c
  prodmetadata/src/prodmetadataDecoder/prodMetadataFrameReader.c
  prodmetadata/src/prodmetadataDecoder/prodMetadataFrameReader.h
  prodmetadata/src/prodmetadataDecoder/singleDynamicProdMetadataFrameReader.c
  prodmetadata/src/prodmetadataDecoder/singleDynamicProdMetadataFrameReader.h
  prodmetadata/src/common/omfReader.c
  prodmetadata/src/common/omfReader.h
  prodmetadata/src/common/printHelper.c
  prodmetadata/src/common/printHelper.h
  prodmetadata/src/common/tables.h
)
target_compile_definitions(prodmetadataDecoder PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(prodmetadataDecoder PRIVATE "${PROJECT_SOURCE_DIR}/prodmetadata/src/prodmetadataDecoder"
                                                       "${PROJECT_SOURCE_DIR}/prodmetadata/src/common"
                                                       "${PROJECT_SOURCE_DIR}/readonlybitbuf/include")
target_link_libraries(prodmetadataDecoder readonlybitbuf_3da)
set_target_properties(prodmetadataDecoder PROPERTIES FOLDER "Executables/tools")
install(TARGETS prodmetadataDecoder DESTINATION bin)

# ResampAudio
file(GLOB_RECURSE ResampAudio_FILES ${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/audio/ResampAudio/*.c)
add_executable(ResampAudio
  ${ResampAudio_FILES}
)
target_link_libraries(ResampAudio libAO ${ADD_MATH_LIB})
set_target_properties(ResampAudio PROPERTIES FOLDER "Executables/tools")
install(TARGETS ResampAudio DESTINATION bin)

# separatorCmdl
add_executable(separatorCmdl
  separator/separatorCmdl/src/separatorCmdl.c
)
target_compile_definitions(separatorCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(separatorCmdl separatorLib wavIO_3da)
set_target_properties(separatorCmdl PROPERTIES FOLDER "Executables/tools")
install(TARGETS separatorCmdl DESTINATION bin)

# ssnrcd
add_executable(ssnrcd
  ssnrcd/src/audiofile_io.h
  ssnrcd/src/cepstrum_analysis.h
  ssnrcd/src/common.h
  ssnrcd/src/common_structs.h
  ssnrcd/src/pns.h
  ssnrcd/src/pns_common.h
  ssnrcd/src/ssnrcd.h
  ssnrcd/src/tables.h
  ssnrcd/src/transfo.h
  ssnrcd/src/audiofile_io.c
  ssnrcd/src/cepstrum_analysis.c
  ssnrcd/src/mpeg4conf.c
  ssnrcd/src/pns.c
  ssnrcd/src/ssnrcd.c
  ssnrcd/src/transfo.c
)
target_compile_definitions(ssnrcd PRIVATE _CRT_SECURE_NO_WARNINGS)
target_include_directories(ssnrcd PRIVATE "${PROJECT_SOURCE_DIR}/ssnrcd/src"
                                          "${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/include"
                                          "${PROJECT_SOURCE_DIR}/AFsp/AFsp-v9r0/include/libtsp")
           
target_link_libraries(ssnrcd libtsplite ${ADD_MATH_LIB})
set_target_properties(ssnrcd PROPERTIES FOLDER "Executables/tools")
install(TARGETS ssnrcd DESTINATION bin)

# wavCutterCmdl
add_executable(wavCutterCmdl
  wavCutter/wavCutterCmdl/src/wavCutterCmdl.c
)
target_compile_definitions(wavCutterCmdl PRIVATE _CRT_SECURE_NO_WARNINGS)
target_link_libraries(wavCutterCmdl wavIO_3da)
set_target_properties(wavCutterCmdl PROPERTIES FOLDER "Executables/tools")
install(TARGETS wavCutterCmdl DESTINATION bin)

# wavM2NCmdl
add_executable(wavM2NCmdl
  wavM2N/wavM2NCmdl/src/wavM2NCmdl.c
)
target_link_libraries(wavM2NCmdl wavM2NLib wavIO_3da)
set_target_properties(wavM2NCmdl PROPERTIES FOLDER "Executables/tools")
install(TARGETS wavM2NCmdl DESTINATION bin)
