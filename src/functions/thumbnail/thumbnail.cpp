#include "thumbnail.h"

namespace fs = std::filesystem;  // Alias pour simplifier le code

void createThumbnails(const std::vector<std::string>& imagePaths, const std::string& outputDir) {
    for (const auto& imagePath : imagePaths) {
        try {
            // Generate output thumbnail path
            std::string outputImage = outputDir + "/thumb_" + imagePath.substr(imagePath.find_last_of("/\\") + 1);

            // Check if the thumbnail already exists
            if (std::filesystem::exists(outputImage)) {
                std::cout << "Thumbnail already exists for: " << imagePath << std::endl;
                continue;  // Skip the creation process if thumbnail exists
            }

            // Load image metadata
            std::unique_ptr<Exiv2::Image> image = Exiv2::ImageFactory::open(imagePath);
            assert(image.get() != nullptr);
            image->readMetadata();

            // Get Exif data for image dimensions
            Exiv2::ExifData& exifData = image->exifData();
            if (exifData.empty()) {
                std::cerr << "No Exif data found in the image: " << imagePath << std::endl;
                continue;
            }

            // Extract width and height from Exif data using toInt64()
            int width = exifData["Exif.Photo.PixelXDimension"].toInt64();
            int height = exifData["Exif.Photo.PixelYDimension"].toInt64();

            std::cout << "Width: " << width << ", Height: " << height << std::endl;

            // Placeholder for resizing logic (you'll need to implement this yourself)

            // Save the thumbnail image to the output directory
            std::ofstream outFile(outputImage, std::ios::binary);  // Correct type usage
            if (outFile) {
                // Write the thumbnail data (after resizing)
                // outFile.write(...);  // Assuming you have the resized image data
                outFile.close();
            }

            std::cout << "Thumbnail created for: " << imagePath << std::endl;
        }
        catch (const Exiv2::Error& e) {
            std::cerr << "Error processing file " << imagePath << ": " << e.what() << std::endl;
        }
    }
}