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

TEST(FoldersTest, AddFile) {
    Folders root("root");
    root.addFile("file1.txt");
    root.addFile("file2.txt");

    ASSERT_EQ(root.getFiles().size(), 2);
    EXPECT_EQ(root.getFiles().at(0), "file1.txt");
    EXPECT_EQ(root.getFiles().at(1), "file2.txt");
}

TEST(FoldersTest, SaveAndLoad) {
    Folders root("root");
    root.addFolder("child1");
    root.addFile("file1.txt");

    std::ofstream out("test_save.dat", std::ios::binary);
    root.save(out);
    out.close();

    Folders loadedRoot;
    std::ifstream in("test_save.dat", std::ios::binary);
    loadedRoot.load(in);
    in.close();

    EXPECT_EQ(loadedRoot.getName(), "root");
    ASSERT_EQ(loadedRoot.getFolders()->size(), 1);
    EXPECT_EQ(loadedRoot.getFolders()->at(0).getName(), "child1");
    ASSERT_EQ(loadedRoot.getFiles().size(), 1);
    EXPECT_EQ(loadedRoot.getFiles().at(0), "file1.txt");
}

TEST(FoldersTest, GetIfExist) {
    Folders root("root");
    root.addFolder("child1");

    EXPECT_TRUE(getIfExist(&root, "child1"));
    EXPECT_FALSE(getIfExist(&root, "child2"));
}

TEST(FoldersTest, ContainImage) {
    // Assuming the function isImage is defined elsewhere and works correctly
    EXPECT_FALSE(containImage("non_existing_path"));
    // Add more tests with actual directories and image files if possible
}