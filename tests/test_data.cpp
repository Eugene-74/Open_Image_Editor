
#include <gtest/gtest.h>

#include "Data.hpp"
class DataTest : public ::testing::Test {
   protected:
    std::vector<std::string> imagePaths = {
        "test/resources/images/jpeg.jpeg",
        "test/resources/images/png.png",
        "test/resources/images/bmp.bmp",
        "test/resources/images/gif.gif"};
    std::vector<int> thumbnailSizes = {16, 128, 256, 512};
    Data data;
};

TEST_F(DataTest, CreateThumbnail) {
    for (const auto& imagePath : imagePaths) {
        for (const auto& size : thumbnailSizes) {
            data.createThumbnail(imagePath, size);
        }
    }
}
