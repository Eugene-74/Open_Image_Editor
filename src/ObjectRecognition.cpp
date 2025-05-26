#include "ObjectRecognition.hpp"

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QInputDialog>
#include <opencv2/opencv.hpp>

#include "Data.hpp"
#include "Download.hpp"
#include "FacesRecognition.hpp"
#include "ImageData.hpp"

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
    // size_t facesSize = detectedFaces.size();
    // out.write(reinterpret_cast<const char*>(&facesSize), sizeof(facesSize));
    // for (const auto& face : detectedFaces) {
    //     cv::Rect faceRect = face.getFaceRect();
    //     out.write(reinterpret_cast<const char*>(&faceRect), sizeof(faceRect));
    //     float confidence = face.getConfidence();
    //     out.write(reinterpret_cast<const char*>(&confidence), sizeof(confidence));
    //     int personId = *face.getPersonIdPtr();
    //     out.write(reinterpret_cast<const char*>(&personId), sizeof(personId));
    //     int rows = face.getEmbedding().rows;
    //     int cols = face.getEmbedding().cols;
    //     int type = face.getEmbedding().type();
    //     out.write(reinterpret_cast<const char*>(&rows), sizeof(rows));
    //     out.write(reinterpret_cast<const char*>(&cols), sizeof(cols));
    //     out.write(reinterpret_cast<const char*>(&type), sizeof(type));
    //     if (rows > 0 && cols > 0) {
    //         size_t dataSize = face.getEmbedding().total() * face.getEmbedding().elemSize();
    //         out.write(reinterpret_cast<const char*>(face.getEmbedding().data), dataSize);
    //     }
    // }
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
    // size_t facesSize;
    // in.read(reinterpret_cast<char*>(&facesSize), sizeof(facesSize));
    // detectedFaces.clear();
    // for (size_t i = 0; i < facesSize; ++i) {
    //     cv::Rect rect;
    //     float confidence;
    //     int personId;
    //     in.read(reinterpret_cast<char*>(&rect), sizeof(rect));
    //     in.read(reinterpret_cast<char*>(&confidence), sizeof(confidence));
    //     in.read(reinterpret_cast<char*>(&personId), sizeof(personId));
    //     int rows, cols, type;
    //     in.read(reinterpret_cast<char*>(&rows), sizeof(rows));
    //     in.read(reinterpret_cast<char*>(&cols), sizeof(cols));
    //     in.read(reinterpret_cast<char*>(&type), sizeof(type));
    //     cv::Mat embedding;
    //     if (rows > 0 && cols > 0) {
    //         embedding.create(rows, cols, type);
    //         size_t dataSize = embedding.total() * embedding.elemSize();
    //         in.read(reinterpret_cast<char*>(embedding.data), dataSize);
    //     }
    //     DetectedFaces face(rect, confidence, embedding, personId);
    //     detectedFaces.push_back(face);
    // }
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
void detectObjectsAsync(std::shared_ptr<Data> data, std::string imagePath, QImage image, std::function<void(DetectedObjects)> callback, bool toFront) {
    auto detectionTask = [data, imagePath, image, callback]() {
        std::string modelName = data->getModelConst().getModelName();
        DetectedObjects* detectedObjects = data->detect(imagePath, image, modelName);
        if (detectedObjects) {
            try {
                callback(*detectedObjects);
            } catch (const std::exception& e) {
                qCritical() << e.what();
            }
        }
    };
    if (toFront) {
        data->addHeavyThreadToFront(detectionTask);
    } else {
        data->addHeavyThread(detectionTask);
    }
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

void DetectedObjects::detectFaces(ImageData imageData) {
    std::string imagePath = imageData.getImagePathConst();
    QImage qImage = loadAnImageWithRotation(imageData, 0);
    cv::Mat image = QImageToCvMat(qImage);

    if (image.empty()) {
        qWarning() << "Failed to load image for face detection" << QString::fromStdString(imagePath);
        return;
    }

    // auto it = detectedObjects.find("person");
    // if (it == detectedObjects.end()) {
    //     qWarning() << "No 'person' objects detected";
    //     return;
    // }
    // const auto& persons = it->second;
    // for (const auto& [rect, confidence] : persons) {
    //     cv::Rect validRect = rect & cv::Rect(0, 0, image.cols, image.rows);
    // if (validRect.width > 0 && validRect.height > 0) {
    // cv::Mat personROI = image(validRect).clone();
    cv::Mat personROI = image;

    auto faceResults = getFaceRect(personROI);
    for (const auto& faceResult : faceResults) {
        if (faceResult.second > 0) {
            cv::Rect adjustedRect = faceResult.first;
            // adjustedRect.x += validRect.x;
            // adjustedRect.y += validRect.y;
            detectedFaces.emplace_back(DetectedFaces(adjustedRect, faceResult.second));
        }
    }
    //     }
    // }
}

/**
 * @brief Get the detected faces
 * @return A map of detected faces with their bounding boxes and confidence scores
 */
std::vector<DetectedFaces>* DetectedObjects::getDetectedFacesPtr() {
    return &this->detectedFaces;
}

/**
 * @brief Get the detected faces
 * @return A map of detected faces with their bounding boxes and confidence scores
 */
std::vector<DetectedFaces> DetectedObjects::getDetectedFacesConst() const {
    return this->detectedFaces;
}

/**
 * @brief Get the face rectangle
 * @return The rectangle representing the detected face
 */
cv::Rect DetectedFaces::getFaceRect() const {
    return faceRect;
}

/**
 * @brief Get the confidence score of the detected face
 * @return The confidence score
 */
float DetectedFaces::getConfidence() const {
    return confidence;
}

/**
 * @brief Get the embedding matrix pointer
 * @return Pointer to the embedding matrix
 */
cv::Mat* DetectedFaces::getEmbeddingPtr() {
    return &embedding;
}

/**
 * @brief Get the person ID pointer
 * @return Pointer to the person ID
 */
int* DetectedFaces::getPersonIdPtr() {
    return &personId;
}

/**
 * @brief Get the person ID (const version)
 * @return The person ID
 */
int DetectedFaces::getPersonIdConst() const {
    return personId;
}

/**
 * @brief Get the model used for detection
 * @return The name of the model used
 */
std::string DetectedObjects::getModelUsed() const {
    return this->modelUsed;
}

/**
 * @brief Set the model used for detection
 * @param modelName The name of the model to set
 */
void DetectedObjects::setModelUsed(const std::string& modelName) {
    this->modelUsed = modelName;
}