#include <gtest/gtest.h>

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

TEST_F(ImageDataTest, TestAssignmentOperator) {
    ImageData other;
    other.setDate(123456789);
    imageData = other;
    EXPECT_EQ(imageData.getDate(), 123456789);
}

// TEST_F(ImageDataTest, TestPrint) {
//     imageData.print();
// }

TEST_F(ImageDataTest, TestGet) {
    std::string result = imageData.get();
    EXPECT_FALSE(result.empty());
}

TEST_F(ImageDataTest, TestGetMetaDataPtr) {
    MetaData* metaDataPtr = imageData.getMetaDataPtr();
    EXPECT_NE(metaDataPtr, nullptr);
}

TEST_F(ImageDataTest, TestGetMetaData) {
    MetaData metaData = imageData.getMetaData();
    // Add checks for metaData if needed
}

TEST_F(ImageDataTest, TestAddFolder) {
    imageData.addFolder("testFolder");
    auto folders = imageData.getFolders();
    EXPECT_EQ(folders.size(), 1);
    EXPECT_EQ(folders[0].getName(), "testFolder");
}

TEST_F(ImageDataTest, TestAddFolders) {
    std::vector<std::string> folders = {"folder1", "folder2"};
    imageData.addFolders(folders);
    auto resultFolders = imageData.getFolders();
    EXPECT_EQ(resultFolders.size(), 2);
    EXPECT_EQ(resultFolders[0].getName(), "folder1");
    EXPECT_EQ(resultFolders[1].getName(), "folder2");
}

TEST_F(ImageDataTest, TestGetImageName) {
    imageData = Folders("path/to/image.jpg");
    std::string imageName = imageData.getImageName();
    EXPECT_EQ(imageName, "");
}

TEST_F(ImageDataTest, TestEqualityOperator) {
    ImageData other;
    imageData = ImageData("path/to/image.jpg");
    imageData.setDate(123456789);
    imageData.setOrientation(1);
    other = ImageData("path/to/image.jpg");
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

TEST_F(ImageDataTest, TestTurnImage) {
    imageData.turnImage(90);
    EXPECT_EQ(imageData.getOrientation(), 90);
}

TEST_F(ImageDataTest, TestSaveLoad) {
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

TEST_F(ImageDataTest, TestCropSizes) {
    std::vector<std::vector<QPoint>> cropSizes = {{{0, 0}, {1, 1}}, {{2, 2}, {3, 3}}};
    imageData.setCropSizes(cropSizes);
    auto resultCropSizes = imageData.getCropSizes();
    EXPECT_EQ(resultCropSizes.size(), 2);
    EXPECT_EQ(resultCropSizes[0].size(), 2);
    EXPECT_EQ(resultCropSizes[1].size(), 2);
}

TEST_F(ImageDataTest, TestPersonStatus) {
    imageData.setPersonStatusLoading();
    EXPECT_TRUE(imageData.isPersonStatusLoading());

    imageData.setPersonStatusNotLoaded();
    EXPECT_TRUE(imageData.isPersonStatusNotLoaded());

    imageData.setPersonStatusLoaded();
    EXPECT_TRUE(imageData.isPersonStatusLoaded());
}

TEST_F(ImageDataTest, TestMetaData) {
    MetaData metaData;
    imageData.setMetaData(metaData);
    EXPECT_EQ(imageData.getMetaData(), metaData);
}

TEST_F(ImageDataTest, TestPersons) {
    std::vector<Person> persons = {Person(), Person()};
    imageData.setpersons(persons);
    auto resultPersons = imageData.getpersons();
    EXPECT_EQ(resultPersons.size(), 2);
}

TEST_F(ImageDataTest, TestDate) {
    imageData.setDate(123456789);
    EXPECT_EQ(imageData.getDate(), 123456789);
}

TEST_F(ImageDataTest, TestOrientation) {
    for (int i = 1; i <= 8; i++) {
        imageData.setOrientation(i);
        EXPECT_EQ(imageData.getOrientation(), i);
    }
}
