#include <gtest/gtest.h>

#include <QPoint>

#include "ImageData.hpp"

class ImageDataTest : public ::testing::Test {
   protected:
    void SetUp() override {
        // Setup code here
    }

    void TearDown() override {
        // Cleanup code here
    }

    ImageData imageData;
};

TEST_F(ImageDataTest, AssignmentOperator) {
    ImageData other("path/to/image.jpeg");
    other.setDate(123456789);
    // other.addFolder("folder");
    other.setOrientation(5);
    // other.setCropSizes({QPoint(0, 0), QPoint(1, 1)});
    // other.setMetaData(MetaData());
    // other.setpersons();
    other.setPersonStatusLoading();
    imageData = other;
    EXPECT_EQ(imageData.getDate(), 123456789);
    // EXPECT_EQ(imageData.getFolders()[0].getName(), "folder");
    EXPECT_EQ(imageData.getOrientation(), 5);
    // EXPECT_EQ(imageData.getCropSizes(), 5);
    // EXPECT_EQ(imageData.getMetaData(), 5);
    // EXPECT_EQ(imageData.getpersons(), 5);
    EXPECT_TRUE(imageData.isPersonStatusLoading());
}

TEST_F(ImageDataTest, AddFolder) {
    imageData.addFolder("testFolder");
    auto folders = imageData.getFolders();
    EXPECT_EQ(folders.size(), 1);
    EXPECT_EQ(folders[0].getName(), "testFolder");
}

TEST_F(ImageDataTest, AddFolders) {
    std::vector<std::string> folders = {"folder1", "folder2"};
    imageData.addFolders(folders);
    auto resultFolders = imageData.getFolders();
    EXPECT_EQ(resultFolders.size(), 2);
    EXPECT_EQ(resultFolders[0].getName(), "folder1");
    EXPECT_EQ(resultFolders[1].getName(), "folder2");
}

TEST_F(ImageDataTest, GetImageName) {
    // ImageData imageData = ImageData("path/to/image.jpg");
    // std::string imageName = imageData.getImageName();
    // EXPECT_EQ(imageName, "");

    // ImageData imageData = ImageData(TESTS_PATH.toStdString() + "/images/jpeg.jpeg");
    // std::string imageName = imageData.getImageName();
    // EXPECT_EQ(imageName, "jpeg.jpeg");
}

TEST_F(ImageDataTest, EqualityOperator) {
    ImageData other;
    imageData = ImageData("path/to/image.jpeg");
    imageData.setDate(123456789);
    imageData.setOrientation(1);
    other = ImageData("path/to/image.jpeg");
    other.setDate(987654321);
    other.setOrientation(2);
    EXPECT_TRUE(imageData == other);
}

// TEST_F(ImageDataTest, TestSetExifMetaData) {
//     Exiv2::ExifData exifData;
//     imageData.setExifMetaData(exifData);
// }

// TEST_F(ImageDataTest, TestLoadData) {
//     imageData.loadData();
// }

// TEST_F(ImageDataTest, TestSaveMetaData) {
//     imageData.saveMetaData();
// }

// TEST_F(ImageDataTest, TestGetImageDimensions) {
// int width = imageData.getImageWidth();
// int height = imageData.getImageHeight();
// EXPECT_GT(width, -1);
// EXPECT_GT(height, -1);
// -1 = undefined
// }

TEST_F(ImageDataTest, TurnImage) {
    imageData.turnImage(90);
    EXPECT_EQ(imageData.getOrientation(), 90);
}

TEST_F(ImageDataTest, SaveLoad) {
    std::ofstream out("test.dat", std::ios::binary);
    imageData.save(out);
    out.close();

    ImageData loadedImageData;
    std::ifstream in("test.dat", std::ios::binary);
    loadedImageData.load(in);
    in.close();

    EXPECT_EQ(imageData.getOrientation(), loadedImageData.getOrientation());
    EXPECT_EQ(imageData.getDate(), loadedImageData.getDate());
}

TEST_F(ImageDataTest, CropSizes) {
    std::vector<std::vector<QPoint>> cropSizes = {{{0, 0}, {1, 1}}, {{2, 2}, {3, 3}}};
    imageData.setCropSizes(cropSizes);
    auto resultCropSizes = imageData.getCropSizes();
    EXPECT_EQ(resultCropSizes.size(), 2);
    EXPECT_EQ(resultCropSizes[0].size(), 2);
    EXPECT_EQ(resultCropSizes[1].size(), 2);
}

TEST_F(ImageDataTest, PersonStatus) {
    imageData.setPersonStatusLoading();
    EXPECT_TRUE(imageData.isPersonStatusLoading());

    imageData.setPersonStatusNotLoaded();
    EXPECT_TRUE(imageData.isPersonStatusNotLoaded());

    imageData.setPersonStatusLoaded();
    EXPECT_TRUE(imageData.isPersonStatusLoaded());
}

TEST_F(ImageDataTest, MetaData) {
    MetaData metaData;
    imageData.setMetaData(metaData);
    EXPECT_EQ(imageData.getMetaData(), metaData);
}

TEST_F(ImageDataTest, Persons) {
    std::vector<Person> persons = {Person(), Person()};
    imageData.setpersons(persons);
    auto resultPersons = imageData.getpersons();
    EXPECT_EQ(resultPersons.size(), 2);
}

TEST_F(ImageDataTest, Date) {
    imageData.setDate(123456789);
    EXPECT_EQ(imageData.getDate(), 123456789);
}

TEST_F(ImageDataTest, Orientation) {
    for (int i = 1; i <= 8; i++) {
        imageData.setOrientation(i);
        EXPECT_EQ(imageData.getOrientation(), i);
    }
}
