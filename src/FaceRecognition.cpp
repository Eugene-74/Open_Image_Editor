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

DetectedObjects detect(std::string imagePath, QImage image) {
    try {
        cv::Mat mat = QImageToCvMat(image);

        if (mat.channels() == 4) {
            cv::cvtColor(mat, mat, cv::COLOR_BGRA2BGR);
        }

        // TODO download the model if it doesn't exist
        std::string modelConfiguration = APP_FILES.toStdString() + "/" + "yolov3.cfg";
        std::string modelWeights = APP_FILES.toStdString() + "/" + "yolov3.weights";
        cv::dnn::Net net = cv::dnn::readNetFromDarknet(modelConfiguration, modelWeights);
        std::string classNamesFile = APP_FILES.toStdString() + "/" + "coco.names";
        std::vector<std::string> classNames = loadClassNames(classNamesFile);

        cv::Mat blob;
        cv::dnn::blobFromImage(mat, blob, 1 / 255.0, cv::Size(416, 416), cv::Scalar(), true, false);
        net.setInput(blob);

        std::vector<cv::Mat> outs;
        net.forward(outs, net.getUnconnectedOutLayersNames());

        std::vector<cv::Rect> boxes;
        std::vector<int> classIds;
        std::vector<float> confidences;
        float confidenceThreshold = 0.5;
        float nmsThreshold = 0.4;

        for (size_t i = 0; i < outs.size(); ++i) {
            float* data = (float*)outs[i].data;
            for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
                cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
                cv::Point classIdPoint;
                double confidence;
                cv::minMaxLoc(scores, nullptr, &confidence, nullptr, &classIdPoint);
                if (confidence > confidenceThreshold) {
                    int centerX = (int)(data[0] * mat.cols);
                    int centerY = (int)(data[1] * mat.rows);
                    int width = (int)(data[2] * mat.cols);
                    int height = (int)(data[3] * mat.rows);
                    int left = centerX - width / 2;
                    int top = centerY - height / 2;

                    classIds.push_back(classIdPoint.x);
                    confidences.push_back((float)confidence);
                    boxes.push_back(cv::Rect(left, top, width, height));
                }
            }
        }

        std::vector<int> indices;
        cv::dnn::NMSBoxes(boxes, confidences, confidenceThreshold, nmsThreshold, indices);

        std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjects;
        for (size_t i = 0; i < indices.size(); ++i) {
            int idx = indices[i];
            cv::Rect box = boxes[idx];
            std::string className = classNames[classIds[idx]];  // Use actual class names
            detectedObjects[className].emplace_back(box, confidences[idx]);
        }

        DetectedObjects detectedObjectsObj;
        detectedObjectsObj.setDetectedObjects(detectedObjects);

        return detectedObjectsObj;

    } catch (const std::exception& e) {
        qWarning() << "detect" << e.what();
        return {};
    }
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
        DetectedObjects detectedObjects = detect(imagePath, image);

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
void DetectedObjects::setDetectedObjects(const std::map<std::string, std::vector<std::pair<cv::Rect, float>>>& detectedObjects) {
    this->detectedObjects = detectedObjects;
}

void DetectedObjects::clear() {
    detectedObjects.clear();
}