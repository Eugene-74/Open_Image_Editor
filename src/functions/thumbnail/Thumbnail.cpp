#include "Thumbnail.h"

namespace fs = std::filesystem;  // Alias pour simplifier le code

void createThumbnails(const std::vector<std::string>& imagePaths) {
    for (const auto& imagePath : imagePaths) {
        createThumbnail(imagePath, 128);
    }
}

void createThumbnail(const std::string& imagePath, const int maxDim) {
    // Load the image using OpenCV

    // cv::Mat image = cv::imread(imagePath);
    // if (image.empty()) {
    //     std::cerr << "Error: Could not load image: " << imagePath << std::endl;
    //     return;
    // }

    // // Calculate the scaling factor to maintain aspect ratio

    // double scale = std::min(static_cast<double>(maxDim) / image.cols, static_cast<double>(maxDim) / image.rows);

    // // Resize the image
    // cv::Mat thumbnail;
    // cv::resize(image, thumbnail, cv::Size(), scale, scale);

    // // Generate the output path for the thumbnail
    // std::hash<std::string> hasher;
    // size_t hashValue = hasher(imagePath);
    // std::string extension = fs::path(imagePath).extension().string();

    // std::string outputImage;
    // if (maxDim == 128) {
    //     outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
    // }
    // else if (maxDim == 256) {
    //     outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
    // }
    // else if (maxDim == 512) {
    //     outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
    // }

    // // Save the thumbnail image
    // if (!cv::imwrite(outputImage, thumbnail)) {
    //     std::cerr << "Error: Could not save thumbnail: " << outputImage << std::endl;
    // }

}
void createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim) {
    for (const auto& imagePath : imagePaths) {
        createThumbnailIfNotExists(imagePath, maxDim);
    }
}


/**
 * @brief Creates a thumbnail for the given image if it does not already exist.
 *
 * This function checks if a thumbnail for the specified image exists. If it does not,
 * it creates a new thumbnail.
 *
 * @param imagePath The path to the image file for which the thumbnail should be created.
 */
void createThumbnailIfNotExists(const std::string& imagePath, const int maxDim) {
    if (!hasThumbnail(imagePath, maxDim)) {

        std::cout << "creating Thumbnail for: " << imagePath << std::endl;
        createThumbnail(imagePath, maxDim);
    }
    else {
        std::cout << "Thumbnail already exists for: " << maxDim << " : " << imagePath << std::endl;
    }
}
bool hasThumbnail(const std::string& imagePath, const int maxDim) {
    // Generate thumbnail path
    // std::string thumbPath = "thumb_" + imagePath;
    // std::string outputImage = THUMBNAIL_PATH + "/thumbnail_" + imagePath.substr(imagePath.find_last_of("/\\") + 1);

    std::hash<std::string> hasher;
    size_t hashValue = hasher(imagePath);
    // std::string outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue);
    std::string extension = fs::path(imagePath).extension().string();
    std::string outputImage;

    if (maxDim == 128) {
        outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
    }
    else if (maxDim == 256) {
        outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
    }
    else if (maxDim == 512) {
        outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
    }
    std::cout << "has  Thumbnail for: " << outputImage << std::endl;


    // Check if the thumbnail file exists
    return fs::exists(outputImage);
}

void createAllThumbnail(const std::string& imagePath, const int size) {
    std::vector<std::string> imagePaths;

    if (size > 128) {
        createThumbnailIfNotExists(imagePath, 128);
    }
    if (size > 256) {
        createThumbnailIfNotExists(imagePath, 256);
    }
    if (size > 512) {
        createThumbnailIfNotExists(imagePath, 512);
    }
}