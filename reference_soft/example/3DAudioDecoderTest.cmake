###############################################################################
# Set Test Items
###############################################################################
set(inputfile_1 "${CMAKE_SOURCE_DIR}/example/example__C2_3_FD.mhas")
set(inputfile_2 "${CMAKE_SOURCE_DIR}/example/example__C6_3_FD.mhas")
set(inputfile_3 "${CMAKE_SOURCE_DIR}/example/example__C19_3_FD.mhas")

###############################################################################
# Basic Decoding Tests
###############################################################################

# example__C2_3_FD
get_filename_component(outfilebase ${inputfile_1} NAME_WE)
get_filename_component(outfiledir ${inputfile_1} DIRECTORY)
set(cmdl 3DAudioDecoder -if ${inputfile_1} -of ${outfiledir}/${outfilebase}.wav)

add_test(NAME 3DAudioDecoder_${outfilebase}
         COMMAND ${cmdl}
         WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# example__C6_3_FD
get_filename_component(outfilebase ${inputfile_2} NAME_WE)
get_filename_component(outfiledir ${inputfile_2} DIRECTORY)
set(cmdl 3DAudioDecoder -if ${inputfile_2} -of ${outfiledir}/${outfilebase}.wav)

add_test(NAME 3DAudioDecoder_${outfilebase}
         COMMAND ${cmdl}
         WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")

# example__C19_3_FD
get_filename_component(outfilebase ${inputfile_3} NAME_WE)
get_filename_component(outfiledir ${inputfile_3} DIRECTORY)
set(cmdl 3DAudioDecoder -if ${inputfile_3} -of ${outfiledir}/${outfilebase}.wav)

add_test(NAME 3DAudioDecoder_${outfilebase}
         COMMAND ${cmdl}
         WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
