cmake_minimum_required(VERSION 3.14)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/AppConfig.txt" app_config)
foreach(config_line IN LISTS app_config)
    string(REGEX MATCH "([^=]+)=(.*)" _ ${config_line})
    set(${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
endforeach()

project(${APP_NAME}-${APP_VERSION} VERSION ${APP_VERSION})

# Option pour activer les instructions AVX
option(USE_AVX_INSTRUCTIONS "Use AVX instructions" ON)


if(WIN32)
    message(STATUS "Configuring for Windows")

    # Chemin Qt
    set(CMAKE_PREFIX_PATH "C:/msys64/mingw64")

    # Définir les compilateurs uniquement s'ils ne sont pas déjà définis
    if(NOT CMAKE_C_COMPILER)
        set(CMAKE_C_COMPILER "C:/msys64/mingw64/bin/gcc.exe")
    endif()
    if(NOT CMAKE_CXX_COMPILER)
        set(CMAKE_CXX_COMPILER "C:/msys64/mingw64/bin/g++.exe")
    endif()

    # Ajouter les répertoires d'inclusion et de liaison
    include_directories("C:/msys64/mingw64/include")
    link_directories("C:/msys64/mingw64/lib")

    # Configuration C++
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED True)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_AUTOUIC ON)

    # Configuration Qt dynamique
    set(Qt6_USE_STATIC_LIBS OFF)
    set(Qt6_USE_STATIC_RUNTIME OFF)

    # Définir la variable d'environnement
    set(ENV{QTFRAMEWORK_BYPASS_LICENSE_CHECK} "1")

    # Trouver les packages
    find_package(Qt6 COMPONENTS Core Gui Widgets LinguistTools REQUIRED)
    find_package(OpenCV REQUIRED)
    find_package(CURL REQUIRED)
    find_package(jsoncpp REQUIRED)
    find_package(dlib REQUIRED)

    find_library(EXIV2_LIBRARIES exiv2 REQUIRED)
    find_library(HEIF_LIBRARIES heif REQUIRED)
    # find_library(LIBRAW_LIBRARIES libraw REQUIRED)

    message(STATUS "Exiv2 library: ${EXIV2_LIBRARIES}")


    set(HEIF_INCLUDE_DIRS "C:/msys64/mingw64/include/libheif")
    
    # Inclure les répertoires d'en-têtes
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${EXIV2_INCLUDE_DIRS})
    include_directories(${HEIF_INCLUDE_DIRS})
    message(STATUS "HEIF library: ${HEIF_INCLUDE_DIRS}")

    # Générer Version.h
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Version.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/../include/Version.hpp
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


    # Génération Qt
    # qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
    qt6_wrap_cpp(MOC_SOURCES ${INCLUDE_HEADERS})



    set(TS_FILES
        # src/ressources/translations/open_image_editor_en.ts
        # src/ressources/translations/open_image_editor_fr.ts
    )
    qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

    # Création de l'exécutable
    # add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})


    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_link_options(${PROJECT_NAME} PRIVATE -mwindows)
    endif()

    # Liaison des bibliothèques
    # target_link_libraries(${PROJECT_NAME} 
    #     PRIVATE
    #     Qt6::Core
    #     Qt6::Gui
    #     Qt6::Widgets
    #     CURL::libcurl 
    #     jsoncpp_lib
    #     dlib::dlib
    #     ${OpenCV_LIBS}
    #     ${EXIV2_LIBRARIES}
    #     ${HEIF_LIBRARIES}
    # )

    add_custom_target(update_translations
        COMMAND lupdate-qt6 ${SOURCES} -ts ${TS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Updating translations"
    )

    add_custom_target(release_translations
        COMMAND lrelease-qt6 ${TS_FILES}
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Releasing translations"
    )

    add_library(OpenImageEditorLib ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})

    # Lier les bibliothèques nécessaires à la bibliothèque principale
    target_link_libraries(OpenImageEditorLib 
        PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        CURL::libcurl 
        jsoncpp_lib
        dlib::dlib
        ${OpenCV_LIBS}
        ${EXIV2_LIBRARIES}
        ${HEIF_LIBRARIES}
    )

    # Créer l'exécutable principal et le lier à la bibliothèque principale
    add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenImageEditorLib)

    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
    endif()

endif()

# elseif(UNIX)
#     message(STATUS "Configuring for Linux")
#     # Configuration C++
#     set(CMAKE_CXX_STANDARD 17)
#     set(CMAKE_CXX_STANDARD_REQUIRED True)
#     set(CMAKE_AUTOMOC ON)
#     set(CMAKE_AUTORCC ON)
#     set(CMAKE_AUTOUIC ON)

#     # Configuration Qt dynamique
#     set(Qt6_USE_STATIC_LIBS OFF)
#     set(Qt6_USE_STATIC_RUNTIME OFF)

#     # Définir la variable d'environnement
#     set(ENV{QTFRAMEWORK_BYPASS_LICENSE_CHECK} "1")

#     # Chemin Qt
#     find_program(QMAKE_EXECUTABLE NAMES qmake)
#     get_filename_component(QT_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY)
#     get_filename_component(QT_ROOT_DIR ${QT_BIN_DIR} DIRECTORY)
#     set(CMAKE_PREFIX_PATH "${QT_ROOT_DIR}/lib/cmake")

#     # Ajout du répertoire pour FindExiv2.cmake
#     set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

#     # Trouver les packages
#     find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
#     find_package(OpenCV REQUIRED)
#     find_package(Exiv2 REQUIRED)

#     # Inclure les répertoires d'en-têtes
#     include_directories(${OpenCV_INCLUDE_DIRS})
#     include_directories(${EXIV2_INCLUDE_DIRS})

#     # Utiliser ccache si disponible
#     find_program(CCACHE_PROGRAM ccache)
#     if(CCACHE_PROGRAM)
#         set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
#         set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
#     endif()

#     # Sources et en-têtes
#     file(GLOB_RECURSE SOURCES "src/*.cpp")
#     file(GLOB_RECURSE HEADERS "src/*.hpp")
#     set(RESOURCE_FILES src/ressources/ressources.qrc)

#     # Génération Qt
#     qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
#     qt6_add_resources(QRC_SOURCES ${RESOURCE_FILES})

#     add_library(OpenImageEditorLib STATIC ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})

#     # Lier les bibliothèques nécessaires à la bibliothèque principale
#     target_link_libraries(OpenImageEditorLib 
#         PRIVATE
#         Qt6::Core
#         Qt6::Gui
#         Qt6::Widgets
#         CURL::libcurl 
#         jsoncpp_lib
#         dlib::dlib
#         ${OpenCV_LIBS}
#         ${EXIV2_LIBRARIES}
#         ${HEIF_LIBRARIES}
#     )

#     # Créer l'exécutable principal et le lier à la bibliothèque principale
#     add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})
#     target_link_libraries(${PROJECT_NAME} PRIVATE OpenImageEditorLib)

#     # Support GCC filesystem
#     if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#         target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
#     endif()

#     # Activer l'incrémentation des builds
#     set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ON")

#     # Réduire les niveaux d'optimisation pour les builds de développement
#     set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
#     set(CMAKE_CXX_FLAGS_RELEASE "-O3")
# endif()

# -------------------CREATE TESTS------------------------

# Enable testing
enable_testing()

# Find GTest package
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

# Include the source directory
include_directories(${CMAKE_SOURCE_DIR}/src)

include_directories(${Qt6Core_INCLUDE_DIRS})
include_directories(${Qt6Gui_INCLUDE_DIRS})
include_directories(${Qt6Widgets_INCLUDE_DIRS})

# Add test source files
file(GLOB TEST_SOURCES "tests/*.cpp")

# Create the test executable
add_executable(runTests ${TEST_SOURCES})

# Link the test executable with GTest and the main project library
target_link_libraries(runTests ${GTEST_LIBRARIES} pthread OpenImageEditorLib)

# Add a command to run the tests
add_test(NAME runTests COMMAND runTests)




































# Include the test configuration file
# include(CTestConfig.cmake)

# # Enable testing
# enable_testing()

# # Inclure les tests
# find_package(GTest REQUIRED)
# include_directories(${GTEST_INCLUDE_DIRS})

# # Ajoutez le fichier de test
# file(GLOB TEST_SOURCES "tests/*.cpp")

# include_directories(${CMAKE_SOURCE_DIR}/include)

# include_directories(${Qt6Core_INCLUDE_DIRS})
# include_directories(${Qt6Gui_INCLUDE_DIRS})
# include_directories(${Qt6Widgets_INCLUDE_DIRS})

# add_executable(tests ${TEST_SOURCES})

# # Liez GoogleTest et pthreads (nécessaire pour GoogleTest)
# target_link_libraries(tests ${GTEST_LIBRARIES} pthread)












# Configuration de CPACK
# include(CPack)

# set(CPACK_PACKAGE_VERSION ${APP_VERSION})
# set(CPACK_NSIS_DISPLAY_NAME ${APP_NAME})
# set(CPACK_NSIS_PUBLISHER ${COMPANY_NAME})

# set(CPACK_GENERATOR "ZIP")
# set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# # Inclure les fichiers de release dans le package
# install(DIRECTORY ${CMAKE_BINARY_DIR}/release/ DESTINATION .)
