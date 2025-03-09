#include <gtest/gtest.h>

#include "Verification.hpp"

TEST(VerificationTest, IsImage) {
    EXPECT_TRUE(isImage("test.jpg"));
    EXPECT_TRUE(isImage("test.png"));
    EXPECT_TRUE(isImage("test.bmp"));
    EXPECT_FALSE(isImage(".hiddenfile"));
    EXPECT_FALSE(isImage("test.txt"));
    EXPECT_FALSE(isImage("test"));
}

TEST(VerificationTest, IsTurnable) {
    EXPECT_TRUE(isTurnable("test.jpg"));
    EXPECT_TRUE(isTurnable("test.jpeg"));
    EXPECT_FALSE(isTurnable("test.gif"));
    EXPECT_FALSE(isTurnable("test.mp4"));
}

TEST(VerificationTest, IsMirrorable) {
    EXPECT_TRUE(isMirrorable("test.jpg"));
    EXPECT_TRUE(isMirrorable("test.png"));
    EXPECT_FALSE(isMirrorable("test.gif"));
    EXPECT_FALSE(isMirrorable("test.mp4"));
}

TEST(VerificationTest, isExif) {
    EXPECT_TRUE(isExif(".jpg"));
    EXPECT_TRUE(isExif(".jpeg"));
    EXPECT_FALSE(isExif(".png"));
    EXPECT_FALSE(isExif(".bmp"));
}

TEST(VerificationTest, isReal) {
    EXPECT_TRUE(isReal(".png"));
    EXPECT_TRUE(isReal(".bmp"));
    EXPECT_FALSE(isReal(".jpeg"));
    EXPECT_FALSE(isReal(".tiff"));
}

TEST(VerificationTest, IsHeicOrHeif) {
    EXPECT_TRUE(isHeicOrHeif("test.heic"));
    EXPECT_TRUE(isHeicOrHeif("test.heif"));
    EXPECT_FALSE(isHeicOrHeif("test.jpg"));
    EXPECT_FALSE(isHeicOrHeif("test.png"));
}

TEST(VerificationTest, IsRaw) {
    EXPECT_TRUE(isRaw("test.cr2"));
    EXPECT_TRUE(isRaw("test.nef"));
    EXPECT_FALSE(isRaw("test.jpg"));
    EXPECT_FALSE(isRaw("test.png"));
}