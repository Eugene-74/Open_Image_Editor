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

# Include directories
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${CURL_INCLUDE_DIRS})
include_directories(${jsoncpp_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/include)

# Test sources
file(GLOB_RECURSE TEST_SOURCES "*.cpp")

# Add executable for tests
add_executable(${PROJECT_NAME} ${TEST_SOURCES})

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
)

# Add tests
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME})
