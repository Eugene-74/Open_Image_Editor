# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "Debug")
  file(REMOVE_RECURSE
  "CMakeFiles\\OpenImageEditor-1.0.10_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\OpenImageEditor-1.0.10_autogen.dir\\ParseCache.txt"
  "CMakeFiles\\OpenImageEditorLib_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\OpenImageEditorLib_autogen.dir\\ParseCache.txt"
  "OpenImageEditor-1.0.10_autogen"
  "OpenImageEditorLib_autogen"
  )
endif()
