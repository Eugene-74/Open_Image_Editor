#pragma once
#include <QRunnable>
#include <QSize>
#include <map>
#include <string>
#include <vector>

// #include "Folders.hpp"
#include <QImage>

#include "ImagesData.hpp"
#include "Sizes.hpp"

// Forward declarations
// class ImagesData;
class ImageData;
class QApplication;
class Folders;
class Option;

class QImageAndPath {
   public:
    QImage image;
    std::string imagePath;
};

class Actions {
   public:
    Actions() = default;
    std::function<void()> unDo;
    std::function<void()> reDo;
};

class Data {
   public:
    QApplication* app;
    ImagesData imagesData;

    int imageNumber = 0;
    ImagesData deletedImagesData;

#ifdef _WIN32
    Folders rootFolders;
#else
    Folders rootFolders;
#endif
    Folders* currentFolder = &rootFolders;

    std::map<std::string, Option> options;
    Sizes* sizes = new Sizes();
    std::map<std::string, QImageAndPath>* imageCache = nullptr;
    bool saved = true;

    std::vector<int> imagesSelected;
    std::vector<int> imagesDeleted;

    bool darkMode = true;

    Data();

    void clearCache();

    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);

    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();

    QImage loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0, bool rotation = true, bool square = false, bool crop = true, bool force = false);
    QImage loadImageNormal(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0, bool force = false);

    bool loadInCache(std::string imagePath, bool setSize = false, QSize size = QSize(0, 0), bool force = false);
    void loadInCacheAsync(std::string imagePath, std::function<void()> callback, bool setSize = false, QSize size = QSize(0, 0), bool force = false);

    bool unloadFromCache(std::string imagePath);

    bool isInCache(std::string imagePath);
    bool getLoadedImage(std::string imagePath, QImage& image);

    bool createThumbnail(const std::string& imagePath, const int maxDim);
    bool deleteThumbnail(const std::string& imagePath, const int maxDim);

    void createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim);

    void createThumbnailIfNotExists(const std::string& imagePath, const int maxDim);
    void createThumbnailsIfNotExists(const std::vector<std::string>& imagePaths, const int maxDim);

    void createAllThumbnail(const std::string& imagePath, const int maxDim);
    void createAllThumbnailIfNotExists(const std::string& imagePath, const int maxDim);

    bool hasThumbnail(const std::string& imagePath, const int maxDim);

    std::string getThumbnailPath(const std::string& imagePath, const int size);

    void exportImages(std::string exportPath, bool dateInName);

    void saveData();
    void loadData();
    void clear();

    bool isDeleted(int imageNbr);

    void addAction(std::function<void()> unDo, std::function<void()> reDo);
    void addActionDone(Actions action);

    void unDoAction();
    void reDoAction();

    void sortCurrentImagesData();

    void rotateLeft(int nbr, std::string extension, std::function<void()> reload, bool action = true);
    void rotateRight(int nbr, std::string extension, std::function<void()> reload, bool action = true);

    void mirrorLeftRight(int nbr, std::string extension, std::function<void()> reload, bool action = true);
    void mirrorUpDown(int nbr, std::string extension, std::function<void()> reload, bool action = true);

    void realRotate(int nbr, int rotation, std::function<void()> reload);
    void exifRotate(int nbr, int rotation, std::function<void()> reload);

    void exifMirror(int nbr, bool UpDown, std::function<void()> reload);
    void realMirror(int nbr, bool UpDown, std::function<void()> reload);

    void clearActions();
    QImage rotateQImage(QImage image, ImageData* imageData);

    Folders* getRootFolders();
    Folders* getCurrentFolders();

    ImagesData* getImagesData();
    Folders* findFirstFolderWithAllImages(const ImagesData& imagesData, const Folders& currentFolder) const;
    void removeImageFromFolders(ImageData& imageData);
    std::string getFolderPath(Folders* folder);
    Folders* findFolderByPath(Folders& root, const std::string& path);

   private:
    Folders* findFirstFolderWithAllImagesSub(const Folders& currentFolder) const;
    std::vector<Actions> lastActions = {};
    std::vector<Actions> lastActionsDone = {};

    void createFolders(Folders* currentFolders, std::string path);
    void copyTo(Folders rootFolders, std::string destinationPath, bool dateInName);
};

class LoadImageTask : public QRunnable {
   public:
    LoadImageTask(Data* data, const std::string& imagePath, bool setSize, QSize size, bool force, std::function<void()> callback)
        : data(data), imagePath(imagePath), setSize(setSize), size(size), force(force), callback(callback) {
    }

    void run() override;

   private:
    Data* data;
    std::string imagePath;
    bool setSize;
    QSize size;
    bool force;
    std::function<void()> callback;
};