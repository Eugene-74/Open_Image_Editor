cmake_minimum_required(VERSION 3.10)
project(EasyImageEditor)

# Régler les politiques CMake
if(POLICY CMP0003)
  cmake_policy(SET CMP0003 NEW)
endif()

if(POLICY CMP0080)
  cmake_policy(SET CMP0080 NEW)
endif()

# Ajouter le répertoire contenant FindExiv2.cmake à CMAKE_MODULE_PATH
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Trouver les packages Qt nécessaires
find_package(Qt5 COMPONENTS Core Gui Widgets Location REQUIRED)

# Trouver les packages OpenCV nécessaires
find_package(OpenCV REQUIRED)

# Trouver les packages Exiv2 nécessaires
find_package(Exiv2 REQUIRED)

# Inclure les répertoires d'en-têtes
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${EXIV2_INCLUDE_DIRS})

# Ajouter les fichiers source
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Ajouter les fichiers d'en-tête pour moc
file(GLOB_RECURSE HEADERS "src/*.h")

# Ajouter le fichier de ressources
set(RESOURCE_FILES src/ressources/ressources.qrc)

# S'assurer que moc est exécuté sur les fichiers d'en-tête
qt5_wrap_cpp(MOC_SOURCES ${HEADERS})

# Compiler les ressources
qt5_add_resources(QRC_SOURCES ${RESOURCE_FILES})

# Ajouter l'exécutable
add_executable(EasyImageEditor ${SOURCES} ${MOC_SOURCES} ${QRC_SOURCES})

# Lier les bibliothèques Qt
target_link_libraries(EasyImageEditor Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Location)

# Lier les bibliothèques OpenCV
target_link_libraries(EasyImageEditor ${OpenCV_LIBS})

# Lier les bibliothèques Exiv2
target_link_libraries(EasyImageEditor ${EXIV2_LIBRARIES})

# Lier stdc++fs si vous utilisez GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(EasyImageEditor stdc++fs)
endif()

# Installer l'exécutable
install(TARGETS EasyImageEditor
        RUNTIME DESTINATION bin)

# Inclure les modules nécessaires
include(InstallRequiredSystemLibraries)

# Exécuter le script FixupBundle.cmake après l'installation
install(SCRIPT "${CMAKE_MODULE_PATH}/FixupBundle.cmake")

# Configuration de CPack
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "EasyImageEditor")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

include(CPack)