// #include <gtest/gtest.h>

// #include "ImageData.hpp"

// class ImageDataTest : public ::testing::Test {
//    protected:
//     void SetUp() override {
//         // Setup code here
//     }

//     void TearDown() override {
//         // Cleanup code here
//     }

//     ImageData imageData;
// };

// TEST_F(ImageDataTest, TestAssignmentOperator) {
//     ImageData other;
//     other.setDate(123456789);
//     imageData = other;
//     EXPECT_EQ(imageData.getDate(), 123456789);
// }

// TEST_F(ImageDataTest, TestPrint) {
//     // Assuming print() outputs to qDebug, we can't capture it directly in a test
//     // This test is just a placeholder
//     imageData.print();
// }

// TEST_F(ImageDataTest, TestGet) {
//     std::string result = imageData.get();
//     EXPECT_FALSE(result.empty());
// }

// TEST_F(ImageDataTest, TestGetMetaDataPtr) {
//     MetaData* metaDataPtr = imageData.getMetaDataPtr();
//     EXPECT_NE(metaDataPtr, nullptr);
// }

// TEST_F(ImageDataTest, TestGetMetaData) {
//     MetaData metaData = imageData.getMetaData();
//     // Add checks for metaData if needed
// }

// TEST_F(ImageDataTest, TestAddFolder) {
//     imageData.addFolder("testFolder");
//     auto folders = imageData.getFolders();
//     EXPECT_EQ(folders.size(), 1);
//     EXPECT_EQ(folders[0].getName(), "testFolder");
// }

// TEST_F(ImageDataTest, TestAddFolders) {
//     std::vector<std::string> folders = {"folder1", "folder2"};
//     imageData.addFolders(folders);
//     auto resultFolders = imageData.getFolders();
//     EXPECT_EQ(resultFolders.size(), 2);
//     EXPECT_EQ(resultFolders[0].getName(), "folder1");
//     EXPECT_EQ(resultFolders[1].getName(), "folder2");
// }

// TEST_F(ImageDataTest, TestGetImageName) {
//     // Assuming setImagePath is a method to set the image path
//     // imageData.setImagePath("path/to/image.jpg");
//     std::string imageName = imageData.getImageName();
//     EXPECT_EQ(imageName, "image.jpg");
// }

// TEST_F(ImageDataTest, TestEqualityOperator) {
//     ImageData other;
//     // Assuming setImagePath is a method to set the image path
//     // imageData.setImagePath("path/to/image.jpg");
//     // other.setImagePath("path/to/image.jpg");
//     EXPECT_TRUE(imageData == other);
// }

// TEST_F(ImageDataTest, TestSetExifMetaData) {
//     Exiv2::ExifData exifData;
//     // Populate exifData with some values
//     imageData.setExifMetaData(exifData);
//     // Add checks to verify metadata was set correctly
// }

// TEST_F(ImageDataTest, TestLoadData) {
//     imageData.loadData();
//     // Add checks to verify data was loaded correctly
// }

// TEST_F(ImageDataTest, TestSaveMetaData) {
//     imageData.saveMetaData();
//     // Add checks to verify metadata was saved correctly
// }

// TEST_F(ImageDataTest, TestGetImageDimensions) {
//     int width = imageData.getImageWidth();
//     int height = imageData.getImageHeight();
//     EXPECT_GT(width, 0);
//     EXPECT_GT(height, 0);
// }

// TEST_F(ImageDataTest, TestTurnImage) {
//     imageData.turnImage(90);
//     EXPECT_EQ(imageData.getOrientation(), 90);
// }

// TEST_F(ImageDataTest, TestSaveLoad) {
//     std::ofstream out("test.dat", std::ios::binary);
//     imageData.save(out);
//     out.close();

//     ImageData loadedImageData;
//     std::ifstream in("test.dat", std::ios::binary);
//     loadedImageData.load(in);
//     in.close();

//     EXPECT_EQ(imageData.getOrientation(), loadedImageData.getOrientation());
//     EXPECT_EQ(imageData.getDate(), loadedImageData.getDate());
// }

// TEST_F(ImageDataTest, TestCropSizes) {
//     std::vector<std::vector<QPoint>> cropSizes = {{{0, 0}, {1, 1}}, {{2, 2}, {3, 3}}};
//     imageData.setCropSizes(cropSizes);
//     auto resultCropSizes = imageData.getCropSizes();
//     EXPECT_EQ(resultCropSizes.size(), 2);
//     EXPECT_EQ(resultCropSizes[0].size(), 2);
//     EXPECT_EQ(resultCropSizes[1].size(), 2);
// }

// TEST_F(ImageDataTest, TestPersonStatus) {
//     // imageData.setPersonStatus(ImageData::PersonStatus::Loading);
//     // EXPECT_TRUE(imageData.isPersonStatusLoading());

//     // imageData.setPersonStatus(ImageData::PersonStatus::NotLoaded);
//     // EXPECT_TRUE(imageData.isPersonStatusNotLoaded());

//     // imageData.setPersonStatus(ImageData::PersonStatus::Loaded);
//     // EXPECT_TRUE(imageData.isPersonStatusLoaded());
// }

// TEST_F(ImageDataTest, TestMetaData) {
//     MetaData metaData;
//     imageData.setMetaData(metaData);
//     EXPECT_EQ(imageData.getMetaData(), metaData);
// }

// TEST_F(ImageDataTest, TestPersons) {
//     std::vector<Person> persons = {Person(), Person()};
//     imageData.setpersons(persons);
//     auto resultPersons = imageData.getpersons();
//     EXPECT_EQ(resultPersons.size(), 2);
// }

// TEST_F(ImageDataTest, TestDate) {
//     imageData.setDate(123456789);
//     EXPECT_EQ(imageData.getDate(), 123456789);
// }

// TEST_F(ImageDataTest, TestOrientation) {
//     imageData.setOrientation(90);
//     EXPECT_EQ(imageData.getOrientation(), 90);
// }
