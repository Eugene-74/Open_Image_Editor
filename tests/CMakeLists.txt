# Minimum CMake version required
cmake_minimum_required(VERSION 3.14)

# Project name
project(OpenImageEditorTests)

# Enable testing
enable_testing()

# Find necessary packages
find_package(GTest REQUIRED)
find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(jsoncpp REQUIRED)

find_library(EXIV2_LIBRARIES exiv2 REQUIRED)
find_library(HEIF_LIBRARIES heif REQUIRED)
set(HEIF_INCLUDE_DIRS "C:/msys64/mingw64/include/libheif")

# Include directories
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})
include_directories(${HEIF_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${jsoncpp_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Test sources
file(GLOB_RECURSE TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Main project sources
set(SOURCE_FILES
    "${CMAKE_SOURCE_DIR}/src/Verification.cpp"

    # Add other necessary source files here
)

# Add executable for tests
add_executable(${PROJECT_NAME} ${TEST_SOURCES} ${SOURCE_FILES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    GTest::GTest
    GTest::Main
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    CURL::libcurl
    jsoncpp_lib
    ${OpenCV_LIBS}
    ${EXIV2_LIBRARIES}
    ${HEIF_LIBRARIES}
)

# Add tests
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
