cmake_minimum_required(VERSION 3.9.2)

###############################################################################
# project name
###############################################################################
project(Modules)

###############################################################################
# target sources
###############################################################################
add_subdirectory(3DAudioCoder)
add_subdirectory(ascParser)
add_subdirectory(binaural)
add_subdirectory(coreCoder)
add_subdirectory(decoderInterfaces)
add_subdirectory(dmxMatrixCoder)
add_subdirectory(domainSwitcher)
add_subdirectory(drcCoder)
add_subdirectory(dynamicObjectPriorityGenerator)
add_subdirectory(formatConverter)
add_subdirectory(gVBAPRenderer)
add_subdirectory(hoaCoder)
add_subdirectory(hoaMatrixCoder)
add_subdirectory(iarFormatConverter)
add_subdirectory(metadataPreprocessor)
add_subdirectory(mixer)
add_subdirectory(oamCoder)
add_subdirectory(saocCoder)
add_subdirectory(earconCoder)
