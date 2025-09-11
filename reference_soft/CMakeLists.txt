cmake_minimum_required(VERSION 3.9.2)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE})

enable_testing()

###############################################################################
# project name
###############################################################################
project(MPEG-H3DAudioCodec)

include(FetchContent)
set(FETCHCONTENT_QUIET No)

# Create AFsp directory and download/unpack (UNIX requires chmod afterwards)
file(DOWNLOAD
    https://www.mmsp.ece.mcgill.ca/Documents/Downloads/AFsp/AFsp-v9r0.tar.gz
    ${CMAKE_SOURCE_DIR}/tools/AFsp/AFsp-v9r0.tar.gz
    SHOW_PROGRESS
)
execute_process(
    COMMAND ${CMAKE_COMMAND} -E tar xzf AFsp-v9r0.tar.gz 
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/tools/AFsp
)
if(UNIX)
    execute_process(
        COMMAND chmod -R 755 ${CMAKE_SOURCE_DIR}/tools/AFsp/AFsp-v9r0
        OUTPUT_VARIABLE chmod_output
        ERROR_VARIABLE chmod_error
        RESULT_VARIABLE chmod_result
    )
endif()

# Could use existing libisomediafile targets to the build
FetchContent_Populate(
    IsoLib
    GIT_REPOSITORY https://github.com/MPEGGroup/isobmff.git
    GIT_TAG 7eaf0482137da37afd60418930d86943e33e909b
    GIT_SUBMODULES "IsoLib/audio_example" # Workaround to avoid recursive cloning
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_directory IsoLib/libisomediafile ../../tools/IsoLib/libisomediafile
)

# DRC targets are added directly to the build
set(MPEGH On)
set(AMD1 Off)
set(WRITE_FRAMESIZE Off)

FetchContent_Declare(
    DrcLib
    PATCH_COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_CURRENT_LIST_DIR}/modules/drcCoder/CMakeLists_drcToolEncoder.txt ${CMAKE_CURRENT_BINARY_DIR}/_deps/drclib-src/modules/drcTool/drcToolEncoder/CMakeLists.txt
    URL https://standards.iso.org/iso-iec/23003/-4/ed-2/en/amd/2/MPEG_D_DRC_refsoft.zip
)

FetchContent_MakeAvailable(DrcLib)

add_subdirectory(tools)
add_subdirectory(modules)

###############################################################################
# version info
###############################################################################
string(TIMESTAMP DATE "%Y-%m-%d")

message("\n******************** MPEG-H 3D Audio Coder - Edition 4.0 (FDIS) ****************")
message("*                                                                             *")
message("*                                  ${DATE}                                 *")
message("*                                                                             *")
message("*    This software may only be used in the development of the MPEG 3D Audio   *")
message("*    standard, ISO/IEC 23008-3 or in conforming implementations or products.  *")
message("*                                                                             *")
message("*******************************************************************************\n")
