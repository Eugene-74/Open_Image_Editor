cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(EasyImageEditor VERSION 1.0.0)

# Configuration C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Configuration Qt dynamique
set(Qt5_USE_STATIC_LIBS OFF)
set(Qt5_USE_STATIC_RUNTIME OFF)

# Chemin Qt
find_program(QMAKE_EXECUTABLE NAMES qmake)
get_filename_component(QT_BIN_DIR ${QMAKE_EXECUTABLE} DIRECTORY)
get_filename_component(QT_ROOT_DIR ${QT_BIN_DIR} DIRECTORY)
set(CMAKE_PREFIX_PATH "${QT_ROOT_DIR}/lib/cmake")

# Ajout du répertoire pour FindExiv2.cmake
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Trouver les packages
find_package(Qt5 COMPONENTS Core Gui Widgets Location REQUIRED)
find_package(OpenCV REQUIRED)
find_package(Exiv2 REQUIRED)

# Inclure les répertoires d'en-têtes
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})

# Sources et en-têtes
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "src/*.h")
set(RESOURCE_FILES src/ressources/ressources.qrc)

# Génération Qt
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})
qt5_add_resources(QRC_SOURCES ${RESOURCE_FILES})

# Création de l'exécutable
add_executable(${PROJECT_NAME} ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})

# Liaison des bibliothèques
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Location
    ${OpenCV_LIBS}
    ${EXIV2_LIBRARIES}
    ${CMAKE_DL_LIBS}  # Ajout de la bibliothèque de chargement dynamique
)

# Support GCC filesystem
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

# Configuration des plugins Qt statiques
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        Qt5::QWindowsIntegrationPlugin
        Qt5::QWindowsVistaStylePlugin
    )
endif()

# Installation de l'exécutable
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Installation des bibliothèques Qt
get_target_property(Qt5_Core_LOCATION Qt5::Core LOCATION)
get_filename_component(QT_LIB_DIR ${Qt5_Core_LOCATION} DIRECTORY)
file(GLOB QT_LIBS "${QT_LIB_DIR}/libQt5*.so*")
install(FILES ${QT_LIBS} DESTINATION lib)

# Installation des dépendances OpenCV
foreach(lib ${OpenCV_LIBS})
    if(TARGET ${lib})
        get_target_property(lib_location ${lib} LOCATION)
        if(lib_location)
            install(FILES ${lib_location} DESTINATION lib)
        endif()
    endif()
endforeach()

# Installation des dépendances Exiv2
foreach(lib ${EXIV2_LIBRARIES})
    if(IS_ABSOLUTE ${lib})
        install(FILES ${lib} DESTINATION lib)
    else()
        find_library(FULL_LIB_PATH ${lib})
        if(FULL_LIB_PATH)
            install(FILES ${FULL_LIB_PATH} DESTINATION lib)
        endif()
    endif()
endforeach()

# Configuration CPack
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Open Image Editor Application")
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
endif()
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${PROJECT_NAME})

# Configuration des bibliothèques système
set(CMAKE_INSTALL_SYSTEM_RUNTIME_LIBS_SKIP FALSE)
include(InstallRequiredSystemLibraries)
include(CPack)