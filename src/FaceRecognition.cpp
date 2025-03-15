#include "FaceRecognition.hpp"

#include <dlib/cuda/cuda_dlib.h>
#include <dlib/dnn.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <opencv2/opencv.hpp>

#include "Data.hpp"

using namespace dlib;
using namespace std;

void Person::save(std::ofstream& out) const {
    size_t nameSize = name.size();
    out.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
    out.write(name.c_str(), nameSize);
    out.write(reinterpret_cast<const char*>(&face.x), sizeof(face.x));
    out.write(reinterpret_cast<const char*>(&face.y), sizeof(face.y));
    out.write(reinterpret_cast<const char*>(&face.width), sizeof(face.width));
    out.write(reinterpret_cast<const char*>(&face.height), sizeof(face.height));
}

void Person::load(std::ifstream& in) {
    size_t nameSize;
    in.read(reinterpret_cast<char*>(&nameSize), sizeof(nameSize));
    name.resize(nameSize);
    in.read(&name[0], nameSize);
    in.read(reinterpret_cast<char*>(&face.x), sizeof(face.x));
    in.read(reinterpret_cast<char*>(&face.y), sizeof(face.y));
    in.read(reinterpret_cast<char*>(&face.width), sizeof(face.width));
    in.read(reinterpret_cast<char*>(&face.height), sizeof(face.height));
}

bool is_slow_cpu() {
    auto start = std::chrono::high_resolution_clock::now();
    volatile double sum = 0;
    for (int i = 0; i < 10000000; i++) {
        sum += i * 0.00001;
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> elapsed = end - start;
    return elapsed.count() > 1.0;
}

bool isCudaAvailable() {
    try {
        dlib::cuda::get_num_devices();
        return true;
    } catch (dlib::cuda_error& e) {
        return false;
    }
}

bool startDlib() {
    return true;
}

cv::Mat QImageToCvMat(const QImage& inImage) {
    switch (inImage.format()) {
        case QImage::Format_RGB32: {
            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC4, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine());
            return mat.clone();
        }
        case QImage::Format_RGB888: {
            QImage swapped = inImage.rgbSwapped();
            return cv::Mat(swapped.height(), swapped.width(), CV_8UC3, const_cast<uchar*>(swapped.bits()), swapped.bytesPerLine()).clone();
        }
        case QImage::Format_Indexed8: {
            cv::Mat mat(inImage.height(), inImage.width(), CV_8UC1, const_cast<uchar*>(inImage.bits()), inImage.bytesPerLine());
            return mat.clone();
        }
        default:
            break;
    }
    return cv::Mat();
}

std::vector<Person> detectFacesCPU(std::string imagePath, QImage image) {
    try {
        array2d<unsigned char> img;
        cv::Mat mat = QImageToCvMat(image);

        int newSize = std::max(1, std::min(mat.cols, mat.rows) / 1000);
        float invNewSize = 1.0f / newSize;

        cv::Mat resizedMat;
        cv::resize(mat, resizedMat, cv::Size(), invNewSize, invNewSize);

        cv::Mat gray;
        cv::cvtColor(resizedMat, gray, cv::COLOR_BGR2GRAY);

        dlib::cv_image<unsigned char> dlibImage(gray);

        frontal_face_detector detector = get_frontal_face_detector();

        std::vector<rectangle> dets = detector(dlibImage);

        qDebug() << "Number of faces detected: " << dets.size();

        std::vector<Person> persons;

        for (const auto& d : dets) {
            Person person;
            cv::Rect face;
            face.x = d.left() * newSize;
            face.y = d.top() * newSize;
            face.width = d.width() * newSize;
            face.height = d.height() * newSize;
            person.setFace(face);
            person.setName("Unknown");
            persons.push_back(person);
        }
        return persons;
    } catch (std::exception& e) {
        qDebug() << "Exception: " << e.what();
    }
    return {};
}

std::vector<Person> detectFacesCUDA(std::string imagePath, QImage image) {
    qDebug() << "not working";
    try {
        array2d<unsigned char> img;
        cv::Mat mat = QImageToCvMat(image);

        int newSize = std::max(1, std::min(mat.cols, mat.rows) / 1000);
        float invNewSize = 1.0f / newSize;

        cv::Mat resizedMat;
        cv::resize(mat, resizedMat, cv::Size(), invNewSize, invNewSize);

        cv::Mat gray;
        cv::cvtColor(resizedMat, gray, cv::COLOR_BGR2GRAY);

        dlib::cv_image<unsigned char> dlibImage(gray);

        // Use CUDA for face detection
        frontal_face_detector detector = get_frontal_face_detector();
        std::vector<rectangle> dets = detector(dlibImage);

        qDebug() << "Number of faces detected: " << dets.size();

        std::vector<Person> persons;

        for (const auto& d : dets) {
            Person person;
            cv::Rect face;
            face.x = d.left() * newSize;
            face.y = d.top() * newSize;
            face.width = d.width() * newSize;
            face.height = d.height() * newSize;
            person.setFace(face);
            person.setName("Unknown");
            persons.push_back(person);
        }
        return persons;
    } catch (std::exception& e) {
        qDebug() << "Exception: " << e.what();
    }
    return {};
}

void detectFacesAsync(Data* data, std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback) {
    data->addHeavyThread([=]() {
        std::vector<Person> persons;
        if (isCudaAvailable()) {
            qDebug() << "launch CUDA";
            persons = detectFacesCUDA(imagePath, image);
        } else {
            qDebug() << "launch CPU";

            persons = detectFacesCPU(imagePath, image);
        }
        try {
            callback(persons);
        } catch (const std::exception& e) {
            qCritical() << e.what();
        } });
}

std::string Person::getName() const {
    return name;
}

cv::Rect Person::getFace() const {
    return face;
}

void Person::setName(std::string name) {
    this->name = name;
}

void Person::setFace(cv::Rect face) {
    this->face = face;
}