#include "FaceRecognition.hpp"

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QInputDialog>
#include <opencv2/opencv.hpp>

#include "Data.hpp"

void DetectedObjects::save(std::ofstream& out) const {
    size_t mapSize = detectedObjects.size();
    out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));
    for (const auto& [key, vec] : detectedObjects) {
        size_t keySize = key.size();
        out.write(reinterpret_cast<const char*>(&keySize), sizeof(keySize));
        out.write(key.data(), keySize);

        size_t vectorSize = vec.size();
        out.write(reinterpret_cast<const char*>(&vectorSize), sizeof(vectorSize));
        for (const auto& [rect, confidence] : vec) {
            out.write(reinterpret_cast<const char*>(&rect), sizeof(rect));
            out.write(reinterpret_cast<const char*>(&confidence), sizeof(confidence));
        }
    }
}

void DetectedObjects::load(std::ifstream& in) {
    size_t mapSize;
    in.read(reinterpret_cast<char*>(&mapSize), sizeof(mapSize));
    for (size_t i = 0; i < mapSize; ++i) {
        size_t keySize;
        in.read(reinterpret_cast<char*>(&keySize), sizeof(keySize));
        std::string key(keySize, ' ');
        in.read(&key[0], keySize);

        size_t vectorSize;
        in.read(reinterpret_cast<char*>(&vectorSize), sizeof(vectorSize));
        std::vector<std::pair<cv::Rect, float>> vec(vectorSize);
        for (size_t j = 0; j < vectorSize; ++j) {
            in.read(reinterpret_cast<char*>(&vec[j]), sizeof(vec[j]));
        }
        detectedObjects[key] = vec;
    }
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

QImage CvMatToQImage(const cv::Mat& inImage) {
    switch (inImage.type()) {
        case CV_8UC4: {
            QImage image(inImage.data, inImage.cols, inImage.rows, inImage.step, QImage::Format_RGB32);
            return image.rgbSwapped();
        }
        case CV_8UC3: {
            QImage image(inImage.data, inImage.cols, inImage.rows, inImage.step, QImage::Format_RGB888);
            return image.rgbSwapped();
        }
        case CV_8UC1: {
            QImage image(inImage.data, inImage.cols, inImage.rows, inImage.step, QImage::Format_Indexed8);
            return image;
        }
        default:
            break;
    }
    return QImage();
}

std::vector<std::string> loadClassNames(const std::string& filename) {
    std::vector<std::string> classNames;
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    std::string line;
    while (std::getline(file, line)) {
        classNames.push_back(line);
    }
    return classNames;
}

std::pair<int, double> recognize_face(cv::Ptr<cv::face::LBPHFaceRecognizer> model, const cv::Mat& test_image) {
    int predicted_label = -1;
    double confidence = 0.0;

    model->predict(test_image, predicted_label, confidence);

    std::cout << "Predicted Label: " << predicted_label << std::endl;
    std::cout << "Confidence: " << confidence << std::endl;

    return std::make_pair(predicted_label, confidence);
}

void detectFacesAsync(Data* data, std::string imagePath, QImage image, std::function<void(DetectedObjects)> callback) {
    data->addHeavyThread([=]() {
        DetectedObjects detectedObjects = data->detect(imagePath, image);

        try {
            callback(detectedObjects);
        } catch (const std::exception& e) {
            qCritical() << e.what();
        }
    });
}

std::map<std::string, std::vector<std::pair<cv::Rect, float>>> DetectedObjects::getDetectedObjects() {
    return detectedObjects;
}

std::map<std::string, std::vector<std::pair<cv::Rect, float>>> DetectedObjects::getDetectedObjectsConst() const {
    return detectedObjects;
}

void DetectedObjects::setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects) {
    this->detectedObjects = detectedObjects;
}

void DetectedObjects::clear() {
    detectedObjects.clear();
}
