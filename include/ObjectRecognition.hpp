#pragma once

#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>

// Forward declaration
class QImage;
class Data;
class ImageData;

class DetectedFaces {
   public:
    DetectedFaces(const cv::Rect& faceRect, float confidence, const cv::Mat& embedding = cv::Mat(), int personId = -1)
        : faceRect(faceRect), confidence(confidence), embedding(embedding), personId(personId) {}

    cv::Rect getFaceRect() const;
    float getConfidence() const;
    cv::Mat* getEmbeddingPtr();
    int* getPersonIdPtr();
    int getPersonIdConst() const;

   private:
    cv::Rect faceRect;
    float confidence;

    cv::Mat embedding;
    int personId;
};

class DetectedObjects {
   public:
    void save(std::ofstream& out) const;
    void load(std::ifstream& in);
    void clear();
    auto getDetectedObjects() -> std::map<std::string, std::vector<std::pair<cv::Rect, float>>>;
    auto getDetectedObjectsConst() const -> std::map<std::string, std::vector<std::pair<cv::Rect, float>>>;

    auto getDetectedFacesPtr() -> std::vector<DetectedFaces>*;
    auto getDetectedFacesConst() const -> std::vector<DetectedFaces>;

    void setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects);
    void detectFaces(ImageData imageData);

    std::string getModelUsed() const;
    void setModelUsed(const std::string& modelName);

   private:
    std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjects;
    std::vector<DetectedFaces> detectedFaces;
    std::string modelUsed;
};

bool isCudaAvailable();
void detectObjectsAsync(std::shared_ptr<Data> data, std::string imagePath, QImage image, std::function<void(DetectedObjects)> callback, bool toFront = false);
void computeFaces(std::shared_ptr<Data> data, std::string imagePath);
cv::Mat QImageToCvMat(const QImage& inImage);
QImage CvMatToQImage(const cv::Mat& inImage);
