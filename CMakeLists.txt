cmake_minimum_required(VERSION 3.10)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/AppConfig.txt" app_config)
foreach(config_line IN LISTS app_config)
    string(REGEX MATCH "([^=]+)=(.*)" _ ${config_line})
    set(${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
endforeach()

project(${APP_NAME}-${APP_VERSION} VERSION ${APP_VERSION})


if(NOT CMAKE_GENERATOR)
    set(CMAKE_GENERATOR "Ninja")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage -O0")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fprofile-arcs -ftest-coverage -O0")

if(WIN32)
    message(STATUS "Configuring for Windows")
    set(CMAKE_PREFIX_PATH "C:/msys64/mingw64")

    if(NOT CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/clang.exe")
    endif()
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/clang++.exe")
    endif()

    include_directories("C:/msys64/mingw64/include")
    link_directories("C:/msys64/mingw64/lib")

    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC OFF)
    set(CMAKE_AUTOUIC OFF)

    set(Qt6_USE_STATIC_LIBS OFF)
    set(Qt6_USE_STATIC_RUNTIME OFF)

    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Os -s")

    set(ENV{QTFRAMEWORK_BYPASS_LICENSE_CHECK} "1")

    find_package(Qt6 COMPONENTS Core Gui Widgets Multimedia Location Positioning Quick QuickControls2 Network REQUIRED)
    find_package(OpenCV REQUIRED)
    find_package(CURL REQUIRED)
    find_package(jsoncpp REQUIRED)

    find_library(EXIV2_LIBRARIES exiv2 REQUIRED)
    find_library(HEIF_LIBRARIES heif REQUIRED)
    set(HEIF_INCLUDE_DIRS "C:/msys64/mingw64/include/libheif")

    
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${EXIV2_INCLUDE_DIRS})
    include_directories(${HEIF_INCLUDE_DIRS})
    include_directories(${Qt6Core_INCLUDE_DIRS})
    include_directories(${Qt6Multimedia_INCLUDE_DIRS})
    include_directories(${Qt6Gui_INCLUDE_DIRS})
    include_directories(${Qt6Widgets_INCLUDE_DIRS})
    include_directories(${Qt6Location_INCLUDE_DIRS})
    include_directories(${Qt6Positioning_INCLUDE_DIRS})
    include_directories(${Qt6Quick_INCLUDE_DIRS})
    include_directories(${Qt6QuickControls2_INCLUDE_DIRS})
    include_directories(${Qt6Network_INCLUDE_DIRS})

    





    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/AppConfig.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/../include/AppConfig.hpp
        @ONLY
    )
    
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/install/nsis_variables.nsh.in
        ${CMAKE_CURRENT_BINARY_DIR}/../install/nsis_variables.nsh
        @ONLY
        )
        
    include_directories(include)

    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()

    file(GLOB_RECURSE SOURCES "src/*.cpp")
    file(GLOB_RECURSE INCLUDE_HEADERS "include/*.hpp")
    
    
    set(RESOURCE_FILES src/ressources/ressources.qrc)

    qt6_add_resources(QRC_SOURCES ${RESOURCE_FILES})

    qt6_wrap_cpp(MOC_SOURCES ${INCLUDE_HEADERS})

    add_library(OpenImageEditorLib ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})
    
    target_link_libraries(OpenImageEditorLib 
        PRIVATE
        Qt6::Multimedia
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Quick
        Qt6::QuickControls2
        Qt6::Location
        Qt6::Positioning
        Qt6::Network
        CURL::libcurl 
        jsoncpp_lib
        ${OpenCV_LIBS}
        ${EXIV2_LIBRARIES}
        ${HEIF_LIBRARIES}
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})

        target_link_libraries(${PROJECT_NAME} PRIVATE OpenImageEditorLib)

        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
        endif()

        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
            target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
        endif()
    endif()
    
    # -------------------CREATE TESTS------------------------
    if(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "Test")
        enable_testing()
    
        find_package(GTest REQUIRED)
        include_directories(${GTEST_INCLUDE_DIRS})
        include_directories(${CMAKE_SOURCE_DIR}/src)
    
        file(GLOB TEST_SOURCES "tests/*.cpp")
    
        add_executable(runTests ${TEST_SOURCES} ${QRC_SOURCES})
        
        target_link_libraries(runTests PRIVATE ${GTEST_LIBRARIES} OpenImageEditorLib)
    
        add_test(NAME runTests COMMAND runTests)
    endif()
endif()
