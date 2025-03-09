#include <gtest/gtest.h>

#include <fstream>

#include "Folders.hpp"

TEST(FoldersTest, AddFolder) {
    Folders root("root");
    root.addFolder("child1");
    root.addFolder("child2");

    ASSERT_EQ(root.getFolders()->size(), 2);
    EXPECT_EQ(root.getFolders()->at(0).getName(), "child1");
    EXPECT_EQ(root.getFolders()->at(1).getName(), "child2");
}

TEST(FoldersTest, modifyParent) {
    Folders* parent = new Folders("parent");

    Folders root("root");
    root.addFolder("child");

    EXPECT_EQ(root.getFolder(0)->getParent()->getName(), "root");

    root.setParent(parent);

    EXPECT_EQ(root.getParent()->getName(), "parent");
}

TEST(FoldersTest, modifyFolderPtr) {
    Folders* parent = new Folders("parent");

    Folders root("root");
    root.addFolder("child1");

    root.getFolder(0)->setParent(parent);

    EXPECT_EQ(root.getFolder(0)->getParent()->getName(), "parent");
}

TEST(FoldersTest, AddFile) {
    Folders root("root");
    root.addFile("image1.jpeg");
    root.addFile("image2.jpeg");

    ASSERT_EQ(root.getFiles().size(), 2);
    EXPECT_EQ(root.getFiles().at(0), "image1.jpeg");
    EXPECT_EQ(root.getFiles().at(1), "image2.jpeg");
}

TEST(FoldersTest, SaveAndLoad) {
    Folders root("root");
    root.addFolder("folder1");
    root.addFile("image1.jpeg");

    std::ofstream out(TESTS_PATH.toStdString() + "/test_save.dat", std::ios::binary);
    root.save(out);
    out.close();

    Folders loadedRoot;
    std::ifstream in(TESTS_PATH.toStdString() + "/test_save.dat", std::ios::binary);
    loadedRoot.load(in);
    in.close();

    EXPECT_EQ(loadedRoot.getName(), "root");
    ASSERT_EQ(loadedRoot.getFolders()->size(), 1);
    EXPECT_EQ(loadedRoot.getFolders()->at(0).getName(), "folder1");
    ASSERT_EQ(loadedRoot.getFiles().size(), 1);
    EXPECT_EQ(loadedRoot.getFiles().at(0), "image1.jpeg");
}

TEST(FoldersTest, GetIfExist) {
    Folders root("root");
    root.addFolder("folder1");

    EXPECT_TRUE(getIfExist(&root, "folder1"));
    EXPECT_FALSE(getIfExist(&root, "folder2"));
}

TEST(FoldersTest, ContainImage) {
    EXPECT_TRUE(containImage(TESTS_PATH.toStdString() + "/images"));
    QDir dir(TESTS_PATH + "/test");
    dir.mkpath(".");

    EXPECT_FALSE(containImage(TESTS_PATH.toStdString() + "/test"));

    if (dir.exists()) {
        dir.removeRecursively();
    }
}