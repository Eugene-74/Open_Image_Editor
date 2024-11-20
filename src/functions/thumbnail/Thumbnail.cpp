// #include "Thumbnail.h"

// namespace fs = std::filesystem;  // Alias pour simplifier le code

// void createThumbnails(const std::vector<std::string>& imagePaths) {
//     for (const auto& imagePath : imagePaths) {
//         createThumbnail(imagePath, 128);
//     }
// }

// void createThumbnail(const std::string& imagePath, const int maxDim) {
//     // QImage image(QString::fromStdString(imagePath));
//     // if (image.isNull()) {
//     //     std::cerr << "Error: Could not load image: " << imagePath << std::endl;
//     //     return;
//     // }

//     QImage thumbnail = data.loadImage(nullptr, imagePath, QSize(maxDim, maxDim), false, false);


//     // Calculate the scaling factor to maintain aspect ratio
//     // double scale = std::min(static_cast<double>(maxDim) / image.width(), static_cast<double>(maxDim) / image.height());

//     // Resize the image
//     // QImage thumbnail = image.scaled(image.width() * scale, image.height() * scale, Qt::KeepAspectRatio);

//     // Generate the output path for the thumbnail
//     std::hash<std::string> hasher;
//     size_t hashValue = hasher(imagePath);

//     std::string extension = fs::path(imagePath).extension().string();

//     std::string outputImage;
//     if (maxDim == 128) {
//         outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
//     }
//     else if (maxDim == 256) {
//         outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
//     }
//     else if (maxDim == 512) {
//         outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
//     }

//     // Save the thumbnail image
//     if (!thumbnail.save(QString::fromStdString(outputImage))) {
//         std::cerr << "Error: Could not save thumbnail: " << outputImage << std::endl;
//     }
// }
// void createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim) {
//     for (const auto& imagePath : imagePaths) {
//         createThumbnailIfNotExists(imagePath, maxDim);
//     }
// }


// /**
//  * @brief Creates a thumbnail for the given image if it does not already exist.
//  *
//  * This function checks if a thumbnail for the specified image exists. If it does not,
//  * it creates a new thumbnail.
//  *
//  * @param imagePath The path to the image file for which the thumbnail should be created.
//  */
// void createThumbnailIfNotExists(Data* data, const std::string& imagePath, const int maxDim) {
//     if (!hasThumbnail(imagePath, maxDim)) {

//         // std::cout << "creating Thumbnail for: " << imagePath << std::endl;
//         createThumbnail(imagePath, maxDim);
//     }
//     else {
//         // std::cout << "Thumbnail already exists for: " << maxDim << " : " << imagePath << std::endl;
//     }
// }
// bool hasThumbnail(const std::string& imagePath, const int maxDim) {

//     std::hash<std::string> hasher;
//     size_t hashValue = hasher(imagePath);
//     std::string extension = fs::path(imagePath).extension().string();
//     std::string outputImage;

//     if (maxDim == 128) {
//         outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
//     }
//     else if (maxDim == 256) {
//         outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
//     }
//     else if (maxDim == 512) {
//         outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
//     }

//     // Check if the thumbnail file exists
//     return fs::exists(outputImage);
// }

// void createAllThumbnail(const std::string& imagePath, const int size) {
//     std::vector<std::string> imagePaths;

//     if (size > 128) {
//         createThumbnailIfNotExists(imagePath, 128);
//     }
//     if (size > 256) {
//         createThumbnailIfNotExists(imagePath, 256);
//     }
//     if (size > 512) {
//         createThumbnailIfNotExists(imagePath, 512);
//     }
// }

// std::string getThumbnailPath(const std::string& imagePath, const int size){
//     std::hash<std::string> hasher;
//     size_t hashValue = hasher(imagePath);
//     std::string extension = fs::path(imagePath).extension().string();
//     std::string outputImage;

//     if (size == 128) {
//         outputImage = THUMBNAIL_PATH + "/normal/" + std::to_string(hashValue) + extension;
//     }
//     else if (size == 256) {
//         outputImage = THUMBNAIL_PATH + "/large/" + std::to_string(hashValue) + extension;
//     }
//     else if (size == 512) {
//         outputImage = THUMBNAIL_PATH + "/x-large/" + std::to_string(hashValue) + extension;
//     }
//     return outputImage;
// }