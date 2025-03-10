#include <gtest/gtest.h>

#include <filesystem>

#include "Conversion.hpp"

namespace fs = std::filesystem;

TEST(ConversionTest, readHeicAndHeif) {
    std::string imagePath;
    imagePath = TESTS_PATH.toStdString() + "/ressources/images/heic.heic";
    EXPECT_FALSE(readHeicAndHeif(imagePath).isNull());

    imagePath = TESTS_PATH.toStdString() + "/ressources/images/heif.heif";
    EXPECT_FALSE(readHeicAndHeif(imagePath).isNull());
}

// also test read and write
TEST(ConversionTest, convertion) {
    for (const auto& imageFormat1 : IMAGE_CONVERTION) {
        QDir dir(TESTS_PATH + "/ressources/images/" + QString::fromStdString(imageFormat1));
        if (dir.exists()) {
            dir.removeRecursively();
        }
        dir.mkpath(".");

        std::string imagePath = TESTS_PATH.toStdString() + "/ressources/images/" + imageFormat1 + "." + imageFormat1;
        std::string outputPath;

        for (const auto& imageFormat2 : IMAGE_CONVERTION) {
            outputPath = TESTS_PATH.toStdString() + "/ressources/images/" + imageFormat1 + "/" + imageFormat1 + "." + imageFormat2;
            EXPECT_TRUE(convertImageWithMetadata(imagePath, outputPath));
            EXPECT_TRUE(QFile::exists(QString::fromStdString(outputPath)));
            QImage image;
            if (isHeicOrHeif(outputPath)) {
                image = readHeicAndHeif(outputPath);
            } else if (isRaw(outputPath)) {
            } else {
                image = QImage(QString::fromStdString(outputPath));
            }
            EXPECT_FALSE(image.isNull());
        }
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }
}
