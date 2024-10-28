cmake_minimum_required(VERSION 3.10)

# Project name
project(EasyImageEditor)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Find Qt packages
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

# Find OpenCV packages
find_package(OpenCV REQUIRED)

# Add the directory containing FindExiv2.cmake to CMAKE_MODULE_PATH
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find Exiv2 packages
find_package(Exiv2 REQUIRED)

# Include directories
include_directories(src)
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add header files for moc
file(GLOB_RECURSE HEADERS "src/*.h")

# Ensure moc is run on header files
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

# Add executable
add_executable(EasyImageEditor ${SOURCES} ${MOC_SOURCES})

# Link Qt libraries
target_link_libraries(EasyImageEditor Qt5::Core Qt5::Gui Qt5::Widgets)

# Link OpenCV libraries
target_link_libraries(EasyImageEditor ${OpenCV_LIBS})

# Link Exiv2 libraries
target_link_libraries(EasyImageEditor ${EXIV2_LIBRARIES})

# Link stdc++fs if using GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(EasyImageEditor stdc++fs)
endif()