#pragma once

#include <dlib/clustering.h>
#include <dlib/dnn.h>
#include <dlib/dnn/loss.h>
#include <dlib/gui_widgets.h>
#include <dlib/image_io.h>
#include <dlib/image_processing.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>
#include <dlib/string.h>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <opencv2/opencv.hpp>

class Person {
   public:
    std::string name;
    cv::Rect face;

    void save(std::ofstream& out) const;
    void load(std::ifstream& in);
};

bool is_slow_cpu();
bool startDlib();
std::vector<Person> detectFaces(std::string imagePath, QImage image);
void detectFacesAsync(std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback);
