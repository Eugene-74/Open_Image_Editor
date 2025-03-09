#include <gtest/gtest.h>

// #include "Const.hpp"
#include "Conversion.hpp"

TEST(ConversionTest, readHeicAndHeif) {
    std::string imagePath;
    imagePath = std::string(TESTS_PATH.toStdString()) + "/images/heic.heic";
    EXPECT_FALSE(readHeicAndHeif(imagePath).isNull());

    imagePath = std::string(TESTS_PATH.toStdString()) + "/images/heif.heif";
    EXPECT_FALSE(readHeicAndHeif(imagePath).isNull());
}

TEST(ConversionTest, convertion) {
    for (const auto& imageFormat1 : IMAGE_CONVERTION) {
        QDir dir(TESTS_PATH + "/images/" + QString::fromStdString(imageFormat1));
        if (dir.exists()) {
            dir.removeRecursively();
        }
        dir.mkpath(".");

        std::string imagePath = TESTS_PATH.toStdString() + "/images/" + imageFormat1 + "." + imageFormat1;
        std::string outputPath;

        for (const auto& imageFormat2 : IMAGE_CONVERTION) {
            outputPath = TESTS_PATH.toStdString() + "/images/" + imageFormat1 + "/" + imageFormat1 + "." + imageFormat2;
            EXPECT_TRUE(convertImageWithMetadata(imagePath, outputPath));
            EXPECT_TRUE(QFile::exists(QString::fromStdString(outputPath)));
            QImage image(QString::fromStdString(outputPath));
            EXPECT_FALSE(image.isNull());
        }
        if (dir.exists()) {
            dir.removeRecursively();
        }
    }
}