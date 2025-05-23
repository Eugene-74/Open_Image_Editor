#include "FacesRecognition.hpp"

#include <QDebug>
#include <iostream>

std::vector<std::pair<cv::Rect, double>> getFaceRect(cv::Mat img) {
    if (img.empty()) {
        std::cerr << "Erreur lors du chargement de l'image." << std::endl;
        return {std::make_pair(cv::Rect(), 0.0)};
    }

    cv::Mat gray;
    cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

    cv::CascadeClassifier faceCascade;
    if (!faceCascade.load("C:/Users/eugen/AppData/Local/OpenImageEditor/haarcascade_frontalface_alt2.xml")) {
        std::cerr << "Erreur lors du chargement du classificateur en cascade." << std::endl;
        return {std::make_pair(cv::Rect(), 0.0)};
    }

    std::vector<cv::Rect> faces;
    std::vector<int> rejectLevels;
    std::vector<double> levelWeights;
    faceCascade.detectMultiScale(
        gray, faces, rejectLevels, levelWeights, 1.1, 3, 0, cv::Size(), cv::Size(), true);

    std::vector<std::pair<cv::Rect, double>> results;
    for (size_t i = 0; i < faces.size(); ++i) {
        double confidencePercent = (i < levelWeights.size()) ? levelWeights[i] / 100.0 : 0.0;
        results.emplace_back(faces[i], confidencePercent);
    }
    // If you want to keep the function signature, you can return only the first face or handle as needed.
    // Otherwise, change the function return type to std::vector<std::pair<cv::Rect, double>>
    if (!results.empty()) {
        return results;  // Or adapt the function to return all faces
    }
    return {std::make_pair(cv::Rect(), 0.0)};
}

double cosineSimilarity(const cv::Mat& embedding1, const cv::Mat& embedding2) {
    return embedding1.dot(embedding2) / (cv::norm(embedding1) * cv::norm(embedding2));
}

cv::Mat detectEmbedding(cv::Mat face) {
    if (face.empty()) {
        qWarning() << "Error detecting embedding faces";
        return cv::Mat();
    }

    cv::resize(face, face, cv::Size(112, 112));

    cv::Mat blob = cv::dnn::blobFromImage(face, 1.0 / 128, cv::Size(112, 112), cv::Scalar(127.5, 127.5, 127.5), true, false);

    cv::dnn::Net net = cv::dnn::readNetFromONNX("C:/Users/eugen/AppData/Local/OpenImageEditor/arcface.onnx");

    net.setInput(blob);
    cv::Mat embedding = net.forward();
    cv::normalize(embedding, embedding);

    return embedding;
}