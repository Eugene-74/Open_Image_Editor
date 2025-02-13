cmake_minimum_required(VERSION 3.14)

file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/AppConfig.txt" app_config)
foreach(config_line IN LISTS app_config)
    string(REGEX MATCH "([^=]+)=(.*)" _ ${config_line})
    set(${CMAKE_MATCH_1} ${CMAKE_MATCH_2})
endforeach()

project(${APP_NAME}-${APP_VERSION} VERSION ${APP_VERSION})


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
    
    # set(LIBRAW_INCLUDE_DIRS "C:/msys64/mingw64/include/libraw")
    # set(LIBRAW_LIBRARIES "C:/msys64/mingw64/lib/libraw.dll.a")

    # Inclure les répertoires d'en-têtes
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${EXIV2_INCLUDE_DIRS})
    include_directories(${HEIF_INCLUDE_DIRS})
    message(STATUS "HEIF library: ${HEIF_INCLUDE_DIRS}")

    # include_directories(${dlib_INCLUDE_DIRS})
    
    # include_directories(${LIBRAW_INCLUDE_DIRS})
    # message(STATUS "LibRaw library: ${LIBRAW_INCLUDE_DIRS}")

    # Générer Version.h
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/include/Version.hpp.in
        ${CMAKE_CURRENT_BINARY_DIR}/../include/Version.hpp
        @ONLY
    )
    
    # Générer nsis_variables.nsh
    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/install/nsis_variables.nsh.in
        ${CMAKE_CURRENT_BINARY_DIR}/../install/nsis_variables.nsh
        @ONLY
        )
        
    # Inclure le répertoire du fichier d'en-tête généré
    include_directories(include)



    # Utiliser ccache si disponible
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()

    # Sources et en-têtes
    file(GLOB_RECURSE SOURCES "src/*.cpp")
    
    # file(GLOB_RECURSE HEADERS "src/*.hpp")
    file(GLOB_RECURSE INCLUDE_HEADERS "include/*.hpp")
    
    set(RESOURCE_FILES src/ressources/ressources.qrc)

    qt6_add_resources(QRC_SOURCES ${RESOURCE_FILES})


    # Génération Qt
    # qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
    qt6_wrap_cpp(MOC_SOURCES ${INCLUDE_HEADERS})



    set(TS_FILES
        # src/ressources/translations/open_image_editor_en.ts
        # src/ressources/translations/open_image_editor_fr.ts
        # Ajoutez d'autres fichiers de traduction ici
    )
    qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${TS_FILES})

    # Création de l'exécutable
    add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES} ${QM_FILES})

    # Liaison des bibliothèques
    target_link_libraries(${PROJECT_NAME} 
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
        # ${LIBRAW_LIBRARIES}
    )

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

    # Support GCC filesystem
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
    endif()

    # Activer l'incrémentation des builds
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ON")

    # Réduire les niveaux d'optimisation pour les builds de développement
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")


elseif(UNIX)
    message(STATUS "Configuring for Linux")
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

    # Chemin Qt
    find_program(QMAKE_EXECUTABLE NAMES qmake)
    get_filename_component(QT_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY)
    get_filename_component(QT_ROOT_DIR ${QT_BIN_DIR} DIRECTORY)
    set(CMAKE_PREFIX_PATH "${QT_ROOT_DIR}/lib/cmake")

    # Ajout du répertoire pour FindExiv2.cmake
    set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

    # Trouver les packages
    find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)
    find_package(OpenCV REQUIRED)
    find_package(Exiv2 REQUIRED)

    # Inclure les répertoires d'en-têtes
    include_directories(${OpenCV_INCLUDE_DIRS})
    include_directories(${EXIV2_INCLUDE_DIRS})

    # Utiliser ccache si disponible
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
        set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
    endif()

    # Sources et en-têtes
    file(GLOB_RECURSE SOURCES "src/*.cpp")
    file(GLOB_RECURSE HEADERS "src/*.hpp")
    set(RESOURCE_FILES src/ressources/ressources.qrc)

    # Génération Qt
    qt6_wrap_cpp(MOC_SOURCES ${HEADERS})
    qt6_add_resources(QRC_SOURCES ${RESOURCE_FILES})

    # Création de l'exécutable
    add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})

    # Liaison des bibliothèques
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        # Qt6::Location
        ${OpenCV_LIBS}
        ${EXIV2_LIBRARIES}
        ${CMAKE_DL_LIBS}  # Ajout de la bibliothèque de chargement dynamique
    )

    # Support GCC filesystem
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
    endif()

    # Activer l'incrémentation des builds
    set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ON")

    # Réduire les niveaux d'optimisation pour les builds de développement
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
    set(CMAKE_CXX_FLAGS_RELEASE "-O3")
endif()

# Include the test configuration file
include(CTestConfig.cmake)

# Enable testing
enable_testing()

# Inclure les tests
add_subdirectory(tests)

# Configuration de CPACK
include(CPack)

set(CPACK_PACKAGE_VERSION ${APP_VERSION})
set(CPACK_NSIS_DISPLAY_NAME ${APP_NAME})
set(CPACK_NSIS_PUBLISHER ${COMPANY_NAME})

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")

# Inclure les fichiers de release dans le package
install(DIRECTORY ${CMAKE_BINARY_DIR}/release/ DESTINATION .)
