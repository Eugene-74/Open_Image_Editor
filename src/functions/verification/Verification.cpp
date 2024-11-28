#include "Verification.h"
namespace fs = std::filesystem;


// Revoie True si l'extension du fichier correspond à l'extension d'une image
bool isImage(const std::string& path) {
    // Liste des extensions d'images courantes
    std::vector<std::string> extensionsImages = { ".jpg", ".jpeg", ".png" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(path).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isTurnable(const std::string& path) {
    // Liste des extensions d'images courantes
    std::vector<std::string> extensionsImages = { ".jpg", ".jpeg",".png" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(path).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isMirrorable(const std::string& path) {
    // Liste des extensions d'images courantes
    std::vector<std::string> extensionsImages = { ".jpg", ".jpeg",".png" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(path).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsImages.begin(), extensionsImages.end(), extension) != extensionsImages.end();
}

bool isVideo(const std::string& cheminFichier){
    // Liste des extensions d'images courantes
    // std::vector<std::string> extensionsVideos = { ".mp4", ".avi", ".mov", ".mkv", ".flv", ".webm", ".wmv",".gif" };
    std::vector<std::string> extensionsVideos = { ".gif" };

    // Récupération de l'extension du fichier
    std::string extension = fs::path(cheminFichier).extension().string();

    // Convertir l'extension en minuscule pour éviter les problèmes de casse
    std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

    // Vérifier si l'extension est dans la liste des extensions d'images
    return std::find(extensionsVideos.begin(), extensionsVideos.end(), extension) != extensionsVideos.end();
}