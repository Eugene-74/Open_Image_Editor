#include "FaceRecognition.hpp"

using namespace dlib;
using namespace std;

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
            std::cerr << "Error: invNewSize must be greater than 0" << std::endl;
            return {};
        }

        cv::Mat resizedMat;
        cv::resize(mat, resizedMat, cv::Size(), invNewSize, invNewSize);

        std::cout << "Image dimensions: " << mat.cols << "x" << mat.rows << std::endl;
        cv::Mat gray;
        cv::cvtColor(resizedMat, gray, cv::COLOR_BGR2GRAY);

        std::cout << "resized Image dimensions: " << resizedMat.cols << "x" << resizedMat.rows << std::endl;

        // Convert to dlib::cv_image
        dlib::cv_image<unsigned char> dlibImage(gray);

        // Initialize face detector
        frontal_face_detector detector = get_frontal_face_detector();

        // Detect faces in the image
        std::vector<rectangle> dets = detector(dlibImage);

        // Print the number of faces detected
        std::cout << "Number of faces detected: " << dets.size() << std::endl;
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
        std::cout << "Exception: " << e.what() << std::endl;
    }
    return {};
}

void detectFacesAsync(std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback) {
    std::thread([=]() {
        auto faces = detectFaces(imagePath, image);
        callback(faces);
    }).detach();
}