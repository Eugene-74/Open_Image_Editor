#pragma once

#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <opencv2/opencv.hpp>

class Person {
   private:
    std::string name;
    cv::Rect face;

   public:
    void save(std::ofstream& out) const;
    void load(std::ifstream& in);

    std::string getName() const;
    cv::Rect getFace() const;

    void setName(std::string name);
    void setFace(cv::Rect face);
};

bool is_slow_cpu();
bool startDlib();
std::vector<Person> detectFaces(std::string imagePath, QImage image);
void detectFacesAsync(std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback);
