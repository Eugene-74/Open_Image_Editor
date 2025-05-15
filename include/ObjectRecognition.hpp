#pragma once

#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>

// Forward declaration
class QImage;
class Data;

class DetectedObjects {
   private:
    std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjects;

   public:
    void save(std::ofstream& out) const;
    void load(std::ifstream& in);
    void clear();
    auto getDetectedObjects() -> std::map<std::string, std::vector<std::pair<cv::Rect, float>>>;
    auto getDetectedObjectsConst() const -> std::map<std::string, std::vector<std::pair<cv::Rect, float>>>;

    void setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects);
};

bool isCudaAvailable();
void detectObjectsAsync(std::shared_ptr<Data> data, std::string imagePath, QImage image, std::function<void(DetectedObjects)> callback, bool toFront = false);
void computeFaces(std::shared_ptr<Data> data, std::string imagePath);
cv::Mat QImageToCvMat(const QImage& inImage);
QImage CvMatToQImage(const cv::Mat& inImage);
