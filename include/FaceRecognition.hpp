#pragma once

// #include <dlib/opencv.h>
#include <opencv2/face.hpp>
#include <opencv2/opencv.hpp>

// Forward declaration
class QImage;
class Data;

class Person {
   private:
    std::string name;
    cv::Rect face;
    std::vector<cv::Point2f> landmarks;

   public:
    bool operator==(const Person& other) const {
        return this->face == other.face &&
               this->name == other.name;
    }

    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

    std::string getName() const;
    cv::Rect getFace() const;
    std::vector<cv::Point2f> getLandmarks() const;

    void setName(std::string name);
    void setFace(cv::Rect face);
    void setLandmarks(std::vector<cv::Point2f> landmarks);
};

bool is_slow_cpu();
bool isCudaAvailable();
std::vector<Person> detectFacesCUDA(std::string imagePath, QImage image);
std::vector<Person> detectFacesCPU(std::string imagePath, QImage image);
void detectFacesAsync(Data* data, std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback);
std::pair<int, double> recognize_face(cv::Ptr<cv::face::LBPHFaceRecognizer> model, const cv::Mat& test_image);
void computeFaces(Data* data, std::string imagePath);
