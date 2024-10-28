cmake_minimum_required(VERSION 3.10)

# Nom du projet
project(EasyImageEditor)

# Définir le standard C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ajouter le répertoire contenant FindExiv2.cmake à CMAKE_MODULE_PATH
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Trouver les packages Qt nécessaires
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

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
target_link_libraries(EasyImageEditor Qt5::Core Qt5::Gui Qt5::Widgets)

# Lier les bibliothèques OpenCV
target_link_libraries(EasyImageEditor ${OpenCV_LIBS})

# Lier les bibliothèques Exiv2
target_link_libraries(EasyImageEditor ${EXIV2_LIBRARIES})

# Lier stdc++fs si vous utilisez GCC
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_link_libraries(EasyImageEditor stdc++fs)
endif()

# Configuration de l'installation
install(TARGETS EasyImageEditor DESTINATION bin)

# Installer les bibliothèques OpenCV
foreach(lib ${OpenCV_LIBS})
    get_filename_component(lib_path ${lib} DIRECTORY)
    if(EXISTS ${lib_path}/${lib})
        install(FILES ${lib_path}/${lib} DESTINATION lib)
    endif()
endforeach()

# Installer les bibliothèques Exiv2
foreach(lib ${EXIV2_LIBRARIES})
    get_filename_component(lib_path ${lib} DIRECTORY)
    if(EXISTS ${lib_path}/${lib})
        install(FILES ${lib_path}/${lib} DESTINATION lib)
    endif()
endforeach()

# Utiliser CPack pour créer un package
include(InstallRequiredSystemLibraries)
set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_NAME "EasyImageEditor")
set(CPACK_PACKAGE_VERSION "1.0.0")
include(CPack)