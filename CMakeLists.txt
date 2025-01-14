cmake_minimum_required(VERSION 3.14)

# Nom du projet
project(OpenImageEditor VERSION 1.0.0)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configuration Qt dynamique
set(Qt5_USE_STATIC_LIBS OFF)
set(Qt5_USE_STATIC_RUNTIME OFF)

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
file(GLOB_RECURSE HEADERS "src/*.h")
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