#include "thumbnail.h"

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <iostream>

#include <filesystem>  // Pour vérifier l'existence des fichiers

namespace fs = std::filesystem;  // Alias pour simplifier le code

// Fonction pour créer des miniatures de 128x128 maximum
void createThumbnails(const std::vector<std::string>& imagePaths, const std::string& outputDir) {
    for (const std::string& imagePath : imagePaths) {
        // Créer le chemin de sortie pour l'image miniature
        std::string filename = outputDir + "/thumbnail_" + std::to_string(std::hash<std::string>{}(imagePath)) + ".jpg";

        // Vérifier si la miniature existe déjà
        if (fs::exists(filename)) {
            std::cout << "Thumbnail already exists: " << filename << std::endl;
            continue;
        }

        // Charger l'image depuis le fichier
        cv::Mat image = cv::imread(imagePath);

        // Vérifier si l'image a été chargée correctement
        if (image.empty()) {
            std::cerr << "Erreur lors de la lecture de l'image : " << imagePath << std::endl;
            continue;
        }

        // Calculer les nouvelles dimensions tout en gardant le ratio
        int maxWidth = 128;
        int maxHeight = 128;
        int width = image.cols;
        int height = image.rows;

        // Calculer l'échelle pour garder le ratio
        float scale = std::min(static_cast<float>(maxWidth) / width, static_cast<float>(maxHeight) / height);

        // Calculer les nouvelles dimensions de l'image
        int newWidth = static_cast<int>(width * scale);
        int newHeight = static_cast<int>(height * scale);

        // Redimensionner l'image
        cv::Mat thumbnail;
        cv::resize(image, thumbnail, cv::Size(newWidth, newHeight), 0, 0, cv::INTER_LINEAR);

        // Enregistrer la miniature
        cv::imwrite(filename, thumbnail);

        std::cout << "Thumbnail created: " << filename << std::endl;
    }
}