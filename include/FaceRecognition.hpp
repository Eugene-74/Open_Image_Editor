#pragma once

// #include <dlib/opencv.h>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>

// Forward declaration
class QImage;
class Data;

class DetectedObjects {
   private:
    std::map<std::string, std::vector<cv::Rect>> detectedObjects;

   public:
    void save(std::ofstream& out) const;
    void load(std::ifstream& in);
    void clear();
    std::map<std::string, std::vector<cv::Rect>> getDetectedObjects();
    void setDetectedObjects(const std::map<std::string, std::vector<cv::Rect>>& detectedObjects);
};

bool is_slow_cpu();
bool isCudaAvailable();
void detectFacesAsync(Data* data, std::string imagePath, QImage image, std::function<void(DetectedObjects)> callback);
std::pair<int, double> recognize_face(cv::Ptr<cv::face::LBPHFaceRecognizer> model, const cv::Mat& test_image);
void computeFaces(Data* data, std::string imagePath);
