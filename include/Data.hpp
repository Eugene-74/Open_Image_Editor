#pragma once

#include <QImage>
#include <QMediaPlayer>
#include <QProgressBar>
#include <QRunnable>
#include <QSize>
#include <QTimer>
#include <QWidget>
#include <map>
#include <memory>
#include <mutex>
#include <opencv2/face.hpp>
#include <string>
#include <vector>

#include "AsyncDeque.hpp"
#include "AsyncProgressBar.hpp"
#include "DetectObjectsModel.hpp"
#include "Folders.hpp"
#include "ImagesData.hpp"
#include "Sizes.hpp"
#include "ThreadManager.hpp"

// Forward declarations
class ImageData;
class QApplication;
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

class Data : public std::enable_shared_from_this<Data> {
   public:
    Data();

    void clearCache();

    void preDeleteImage(int imageNbr);
    void unPreDeleteImage(int imageNbr);

    void revocerDeletedImage(int imageNbr);
    void revocerDeletedImage(ImageData& imageData);

    void removeDeletedImages();

    QImage loadImageFromVideo(std::string videoPath, int frameNumber = 1);

    DetectedObjects* detect(std::string imagePath, QImage image, std::string model);

    QImage loadImage(QWidget* parent, std::string imagePath, QSize size, bool setSize, int thumbnail = 0, bool rotation = true, bool square = false, bool crop = true, bool force = false);
    QImage loadImageNormal(std::string imagePath, int thumbnail = 0, bool force = false);

    QImage loadAnImageFromRessources(std::string imagePath, int thumbnail);

    mutable std::mutex imageCacheMutex;
    bool loadInCache(std::string imagePath, bool setSize = false, QSize size = QSize(0, 0), bool force = false);
    void loadInCacheAsync(std::string imagePath, std::function<void()> callback, bool setSize = false, QSize size = QSize(0, 0), int thumbnail = 0, bool force = false);

    bool unloadFromCache(std::string imagePath);

    bool isInCache(std::string imagePath);
    // bool getLoadedImage(std::string imagePath, QImage& image);
    QImage* getImageFromCache(std::string imagePath);
    QImage getImageFromCacheConst(std::string imagePath) const;

    void addImageInCache(const std::string& imagePath, const std::string& thumbnailPath, const QImage& image);

    void createThumbnailAsync(const std::string& imagePath, const int maxDim, std::function<void(bool)> callback = nullptr);
    void createAllThumbnailsAsync(const std::string& imagePath, std::function<void(bool)> callback = nullptr, bool toFront = false);

    bool createThumbnail(const std::string& imagePath, const int maxDim);
    bool deleteThumbnail(const std::string& imagePath, const int maxDim);

    void createThumbnails(const std::vector<std::string>& imagePaths, const int maxDim);

    bool createThumbnailIfNotExists(const std::string& imagePath, const int maxDim);
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

    Folders* getRootFolders();
    Folders* getCurrentFolders();
    void setCurrentFolders(Folders* currentFolders);

    ImagesData* getImagesDataPtr();
    Folders* findFirstFolderWithAllImages();
    void removeImageFromFolders(ImageData& imageData);
    std::string getFolderPath(Folders* folder);
    Folders* findFolderByPath(Folders& root, const std::string& path);

    ImagesData* getDeletedImagesDataPtr();

    void addThread(std::function<void()> job);
    void addThreadToFront(std::function<void()> job);

    void addHeavyThread(std::function<void()> job);
    void addHeavyThreadToFront(std::function<void()> job);

    void stopAllThreads();

    void checkThumbnailAndDetectObjects();
    void checkDetectObjects();
    void checkDetectFaces();

    void checkToUnloadImages(int center, int radius);
    void checkToLoadImages(int center, int radius, int thumbnailSize = 0);

    void setCenterTextLabel(QLabel* centerTextLabel);
    void setCenterText(std::string text);

    DetectObjectsModel getModelConst() const;
    DetectObjectsModel* getModelPtr();
    void setModel(DetectObjectsModel model);

    std::map<std::string, Option> getOptionsConst() const;
    std::map<std::string, Option>* getOptionsPtr();
    void setOptions(std::map<std::string, Option> options);

    bool getSaved();
    void setSaved(bool saved);

    bool getDarkMode();
    void setDarkMode(bool darkMode);

    std::vector<int>* getImagesSelectedPtr();

    Sizes* getSizesPtr();

    bool getConnectionEnabled();
    void setConnectionEnabled(bool connectionEnabled);
    bool checkConnection();

    std::map<int, std::string> getPersonIdNames() const;
    std::map<int, std::string>* getPersonIdNamesPtr();
    void setPersonIdNames(std::map<int, std::string> personIdNames);

    void detectAndRecognizeFaces(ImageData* imageData);

    AsyncProgressBar* getDetectionProgressBarPtr();
    void setDetectionProgressBarPtr(AsyncProgressBar* detectionProgressBar);

   private:
#ifdef _WIN32
    Folders rootFolders = Folders("");
#else
    Folders rootFolders = Folders("/");
#endif
    Folders* currentFolder = &rootFolders;

    bool saved = true;
    bool darkMode = true;
    bool connectionEnabled = true;

    ImagesData imagesData;
    ImagesData deletedImagesData;
    Sizes sizes;

    AsyncProgressBar* detectionProgressBar = nullptr;

    std::vector<int> imagesSelected;
    std::map<int, std::string> personIdNames;

    ThreadManager manager;
    QLabel* centerTextLabel;
    std::unordered_map<std::string, QImageAndPath>* imageCache = nullptr;
    DetectObjectsModel model;
    std::map<std::string, Option> options;

    QTimer* thumbnailTimer = new QTimer();
    int thumbnailWorking = 0;
    AsyncDeque<std::string> hasNoThumbnail;

    QTimer* detectObjectTimer = new QTimer();
    int detectionWorking = 0;
    AsyncDeque<ImageData*> hasNotBeenDetected;

    QTimer* detectFacesTimer = new QTimer();
    int detectionFacesWorking = 0;
    AsyncDeque<ImageData*> hasNotBeenDetectedFaces;

    std::vector<Actions> lastActions = {};
    std::vector<Actions> lastActionsDone = {};

    std::mutex detectAndRecognizeFacesMutex;

    Folders* findFirstFolderWithAllImagesSub(Folders* currentFolder);
    void createFolders(Folders* currentFolders, std::string path);
    void copyTo(Folders rootFolders, std::string destinationPath, bool dateInName);
    };

cv::dnn::Net load_net(std::string model);

QImage rotateQImage(QImage image, ImageData* imageData);

QImage loadAnImage(std::string imagePath, int thumbnail);
QImage loadAnImageWithRotation(ImageData imageData, int thumbnail);
