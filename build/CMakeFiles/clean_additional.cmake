# Additional clean files
cmake_minimum_required(VERSION 3.16)

if("${CONFIG}" STREQUAL "" OR "${CONFIG}" STREQUAL "")
  file(REMOVE_RECURSE
  "CMakeFiles\\OpenImageEditor-1.0.2_autogen.dir\\AutogenUsed.txt"
  "CMakeFiles\\OpenImageEditor-1.0.2_autogen.dir\\ParseCache.txt"
  "OpenImageEditor-1.0.2_autogen"
  "tests\\CMakeFiles\\OpenImageEditorTests_autogen.dir\\AutogenUsed.txt"
  "tests\\CMakeFiles\\OpenImageEditorTests_autogen.dir\\ParseCache.txt"
  "tests\\OpenImageEditorTests_autogen"
  )
endif()
