#include "FaceRecognition.hpp"

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
    return elapsed.count() > 1.0;  // Si > 1 sec, CPU lent
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

std::vector<Person> detectFaces(std::string imagePath, QImage image) {
    try {
        // Load image using dlib
        array2d<unsigned char> img;
        cv::Mat mat = QImageToCvMat(image);

        int newSize = 2;
        float invNewSize = 1.0f / newSize;

        if (invNewSize <= 0) {
            qDebug() << "Error: invNewSize must be greater than 0";
            return {};
        }

        cv::Mat resizedMat;
        cv::resize(mat, resizedMat, cv::Size(), invNewSize, invNewSize);

        qDebug() << "Image dimensions: " << mat.cols << "x" << mat.rows;
        cv::Mat gray;
        cv::cvtColor(resizedMat, gray, cv::COLOR_BGR2GRAY);

        qDebug() << "resized Image dimensions: " << resizedMat.cols << "x" << resizedMat.rows;

        // Convert to dlib::cv_image
        dlib::cv_image<unsigned char> dlibImage(gray);

        // Initialize face detector
        frontal_face_detector detector = get_frontal_face_detector();

        // Detect faces in the image
        std::vector<rectangle> dets = detector(dlibImage);

        // Print the number of faces detected
        qDebug() << "Number of faces detected: " << dets.size();
        // std::vector<cv::Rect> faces;
        std::vector<Person> persons;

        for (const auto& d : dets) {
            Person person;
            // faces.emplace_back(cv::Rect(d.left() * newSize, d.top() * newSize, d.width() * newSize, d.height() * newSize));
            person.face.x = d.left() * newSize;
            person.face.y = d.top() * newSize;
            person.face.width = d.width() * newSize;
            person.face.height = d.height() * newSize;
            person.name = "Unknown";
            persons.push_back(person);
        }
        return persons;

        // return faces;
    } catch (std::exception& e) {
        qDebug() << "Exception: " << e.what();
    }
    return {};
}

void detectFacesAsync(std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback) {
    std::thread([=]() {
        auto persons = detectFaces(imagePath, image);
        qDebug() << "Detected bid faces:" << persons.size();
        callback(persons);
    }).detach();
}