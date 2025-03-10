
#include <gtest/gtest.h>

#include "Data.hpp"
class DataTest : public ::testing::Test {
   protected:
    std::vector<std::string> imagePaths = {
        TESTS_PATH.toStdString() + "/ressources/images/bmp.bmp",
        TESTS_PATH.toStdString() + "/ressources/images/heic.heic",
        TESTS_PATH.toStdString() + "/ressources/images/heif.heif",
        TESTS_PATH.toStdString() + "/ressources/images/jpeg.jpeg",
        TESTS_PATH.toStdString() + "/ressources/images/jpg.jpg",
        TESTS_PATH.toStdString() + "/ressources/images/png.png",
        TESTS_PATH.toStdString() + "/ressources/images/tiff.tiff",
        TESTS_PATH.toStdString() + "/ressources/images/webp.webp"};
    std::vector<int> thumbnailSizes = {16, 128, 256, 512};
    Data data;
};

TEST_F(DataTest, CreateThumbnail) {
    for (const auto& imagePath : imagePaths) {
        for (const auto& size : thumbnailSizes) {
            EXPECT_TRUE(data.createThumbnail(imagePath, size));
            EXPECT_TRUE(data.hasThumbnail(imagePath, size));
            EXPECT_TRUE(data.deleteThumbnail(imagePath, size));
            EXPECT_FALSE(data.hasThumbnail(imagePath, size));
        }
    }
}
