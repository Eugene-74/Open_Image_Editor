#include "ObjectRecognition.hpp"

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QInputDialog>
#include <opencv2/opencv.hpp>

#include "Data.hpp"
#include "Download.hpp"

/**
 * @brief Save the detected objects to a file
 * @param out The output file stream
 */
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

/**
 * @brief Load the detected objects from a file
 * @param in The input file stream
 */
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

/**
 * @brief Convert a QImage to a cv::Mat
 * @param inImage The input QImage
 * @return The converted cv::Mat
 */
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

/**
 * @brief Convert a cv::Mat to a QImage
 * @param inImage The input cv::Mat
 * @return The converted QImage
 */
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

/**
 * @brief Detect objects asynchronously
 * @param data Pointer to the Data object
 * @param imagePath Path to the image file
 * @param image The image to process
 * @param callback Callback function to call after detection
 * @details This function runs the detection in a separate thread and calls the callback with the detected objects
 */
void detectObjectsAsync(std::shared_ptr<Data> data, std::string imagePath, QImage image, std::function<void(DetectedObjects)> callback) {
    data->addHeavyThreadToFront([data, imagePath, image, callback]() {
        std::string modelName = data->model.getModelName();
        DetectedObjects* detectedObjects = data->detect(imagePath, image, modelName);
        if (detectedObjects) {
            try {
                callback(*detectedObjects);
            } catch (const std::exception& e) {
                qCritical() << e.what();
            }
        }
    });
}

/**
 * @brief Get the detected objects
 * @return A map of detected objects with their bounding boxes and confidence scores
 */
std::map<std::string, std::vector<std::pair<cv::Rect, float>>> DetectedObjects::getDetectedObjects() {
    return detectedObjects;
}

/**
 * @brief Get the detected objects (const version)
 * @return A const map of detected objects with their bounding boxes and confidence scores
 */
std::map<std::string, std::vector<std::pair<cv::Rect, float>>> DetectedObjects::getDetectedObjectsConst() const {
    return detectedObjects;
}

/**
 * @brief Set the detected objects
 * @param detectedObjects The detected objects to set
 * @details This function replaces the current detected objects with the new ones
 */
void DetectedObjects::setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects) {
    this->detectedObjects = detectedObjects;
}

/**
 * @brief Clear the detected objects
 * @details This function removes all detected objects from the current instance
 */
void DetectedObjects::clear() {
    detectedObjects.clear();
}
