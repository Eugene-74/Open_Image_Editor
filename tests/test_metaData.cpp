#include <gtest/gtest.h>

#include "MetaData.hpp"

class MetaDataTest : public ::testing::Test {
   protected:
    MetaData metaData;

    void SetUp() override {
        // Setup code if needed
    }

    void TearDown() override {
        // Cleanup code if needed
    }
};

TEST_F(MetaDataTest, AssignmentOperator) {
    MetaData other;
    other.modifyExifValue("Exif.Image.Make", "Test Make");
    metaData = other;
    EXPECT_EQ(metaData.getExifData()["Exif.Image.Make"].toString(), "Test Make");
}

TEST_F(MetaDataTest, EqualityOperator) {
    MetaData other;
    EXPECT_TRUE(metaData == other);
}

// TEST_F(MetaDataTest, SaveMetaData) {
//     metaData.modifyExifValue("Exif.Image.Make", "Test Make");
//     metaData.saveMetaData("test.jpg");
// }

TEST_F(MetaDataTest, GetImageWidth) {
    metaData.modifyExifValue("Exif.Image.ImageWidth", "1920");
    EXPECT_EQ(metaData.getImageWidth(), 1920);
}

TEST_F(MetaDataTest, GetImageHeight) {
    metaData.modifyExifValue("Exif.Image.ImageLength", "1080");
    EXPECT_EQ(metaData.getImageHeight(), 1080);
}

TEST_F(MetaDataTest, GetImageOrientation) {
    metaData.modifyExifValue("Exif.Image.Orientation", "1");
    EXPECT_EQ(metaData.getImageOrientation(), 1);
}

TEST_F(MetaDataTest, GetTimestamp) {
    metaData.modifyExifValue("Exif.Image.DateTime", "2023:10:10 10:10:10");
    EXPECT_GT(metaData.getTimestamp(), 0);
}

TEST_F(MetaDataTest, ModifyExifValue) {
    EXPECT_TRUE(metaData.modifyExifValue("Exif.Image.Make", "Test Make"));
    EXPECT_EQ(metaData.getExifData()["Exif.Image.Make"].toString(), "Test Make");
}

TEST_F(MetaDataTest, ModifyXmpValue) {
    EXPECT_TRUE(metaData.modifyXmpValue("Xmp.dc.title", "Test Title"));
    EXPECT_EQ(metaData.getXmpData()["Xmp.dc.title"].toString(), "lang=\"x-default\" Test Title");
}

TEST_F(MetaDataTest, ModifyIptcValue) {
    EXPECT_TRUE(metaData.modifyIptcValue("Iptc.Application2.ObjectName", "Test Object"));
    EXPECT_EQ(metaData.getIptcData()["Iptc.Application2.ObjectName"].toString(), "Test Object");
}

// TEST_F(MetaDataTest, SetOrCreateExifData) {
// metaData.setOrCreateExifData("test.jpg");
// EXPECT_EQ(metaData.getExifData()["Exif.Image.Make"].toString(), "made by photo editor");
// }

// TEST_F(MetaDataTest, LoadData) {
//     metaData.loadData("test.jpg");
// Verify the metadata is loaded correctly
// }

TEST_F(MetaDataTest, Clear) {
    metaData.modifyExifValue("Exif.Image.Make", "Test Make");
    metaData.clear();
    EXPECT_TRUE(metaData.getExifData().empty());
}

// TEST_F(MetaDataTest, DisplayMetaData) {
//     metaData.modifyExifValue("Exif.Image.Make", "Test Make");
//     metaData.displayMetaData();
// }