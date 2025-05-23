
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QFile>
#include <QGuiApplication>
#include <QIcon>
#include <QMessageBox>
#include <QScreen>

#include "InitialWindow.hpp"
#include "Recover.hpp"
#include "Text.hpp"

/**
 * @brief Main function of the application
 * @param argc ...
 * @param argv ...
 * @return ...
 * @details This function initializes the application, sets the window icon, and shows the initial window.
 */
int main(int argc, char* argv[]) {
    try {
        QApplication app(argc, argv);

        app.setWindowIcon(QIcon(":/icons/icon.ico"));

        InitialWindow window;
        window.showMaximized();
        QScreen* screen = QGuiApplication::primaryScreen();

        QRect screenGeometry = screen->geometry();
        window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());
        window.setMinimumSize(screenGeometry.width() / 3, screenGeometry.height() / 3);

        return app.exec();
    } catch (const std::exception& e) {
        qFatal() << "Exception caught:" << e.what();
        recover();

    } catch (...) {
        qFatal() << "Unknown exception caught!";
    }
    return -1;
}

// #include <iostream>
// #include <opencv2/opencv.hpp>

// cv::Mat getFaceImage(const std::string& imagePath) {
//     cv::Mat img = cv::imread(imagePath);
//     if (img.empty()) {
//         std::cerr << "Erreur lors du chargement de l'image." << std::endl;
//         return cv::Mat();
//     }

//     // Convertir en niveaux de gris
//     cv::Mat gray;
//     cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);

//     // Charger le classificateur en cascade pour la détection de visages
//     cv::CascadeClassifier faceCascade;
//     if (!faceCascade.load("C:/Users/eugen/AppData/Local/OpenImageEditor/haarcascade_frontalface_alt2.xml")) {
//         std::cerr << "Erreur lors du chargement du classificateur en cascade." << std::endl;
//         return cv::Mat();
//     }

//     // Détecter les visages
//     std::vector<cv::Rect> faces;
//     faceCascade.detectMultiScale(gray, faces);

//     // Extraire et afficher les régions des visages
//     // for (size_t i = 0; i < faces.size(); ++i) {
//     if (faces.size() == 1) {
//         cv::Mat faceROI = img(faces[0]);
//         return faceROI;
//     }
//     std::cerr << "Aucun visage détecté pour : " << imagePath << std::endl;
//     return cv::Mat();
// }

// double cosineSimilarity(const cv::Mat& vec1, const cv::Mat& vec2) {
//     return vec1.dot(vec2) / (cv::norm(vec1) * cv::norm(vec2));
// }

// float distanceImage(const std::string imagePath1, const std::string imagePath2) {
//     cv::Mat face1 = getFaceImage(imagePath1);
//     cv::Mat face2 = getFaceImage(imagePath2);

//     if (face1.empty() || face2.empty()) {
//         std::cerr << "Erreur lors de l'extraction des visages." << std::endl;
//         return -1;
//     }

//     cv::resize(face1, face1, cv::Size(112, 112));
//     cv::resize(face2, face2, cv::Size(112, 112));

//     // cv::imshow("Face 1", face1);
//     // cv::imshow("Face 2", face2);
//     // cv::waitKey(0);

//     cv::Mat blob1 = cv::dnn::blobFromImage(face1, 1.0 / 128, cv::Size(112, 112), cv::Scalar(127.5, 127.5, 127.5), true, false);
//     cv::Mat blob2 = cv::dnn::blobFromImage(face2, 1.0 / 128, cv::Size(112, 112), cv::Scalar(127.5, 127.5, 127.5), true, false);

//     // Charger le modèle FaceNet
//     cv::dnn::Net net1 = cv::dnn::readNetFromONNX("C:/Users/eugen/AppData/Local/OpenImageEditor/arcface.onnx");
//     cv::dnn::Net net2 = cv::dnn::readNetFromONNX("C:/Users/eugen/AppData/Local/OpenImageEditor/arcface.onnx");

//     net1.setInput(blob1);
//     cv::Mat embedding1 = net1.forward();
//     cv::normalize(embedding1, embedding1);

//     net2.setInput(blob2);
//     cv::Mat embedding2 = net2.forward();
//     cv::normalize(embedding2, embedding2);

//     // std::cout << "Embedding 1 (premiers éléments) : ";
//     // for (int i = 0; i < std::min(10, static_cast<int>(embedding1.total())); i++) {
//     //     std::cout << embedding1.at<float>(i) << " ";
//     // }
//     // std::cout << std::endl;

//     // std::cout << "Embedding 2 (premiers éléments) : ";
//     // for (int i = 0; i < std::min(10, static_cast<int>(embedding2.total())); i++) {
//     //     std::cout << embedding2.at<float>(i) << " ";
//     // }
//     // std::cout << std::endl;

//     double similarity = cosineSimilarity(embedding1, embedding2);
//     std::cout << "Similarité cosinus : " << similarity << std::endl;
//     return similarity;
// }

// int main() {
//     std::vector<std::string> paths = {
//         "C:/Users/eugen/AppData/Local/OpenImageEditor/eugene1.jpg",
//         "C:/Users/eugen/AppData/Local/OpenImageEditor/suzie1.jpg",
//         "C:/Users/eugen/AppData/Local/OpenImageEditor/emilien1.jpg",
//         "C:/Users/eugen/AppData/Local/OpenImageEditor/emilien2.jpg",
//         "C:/Users/eugen/AppData/Local/OpenImageEditor/pauline1.jpg"};
//     for (size_t i = 0; i < paths.size(); ++i) {
//         for (size_t j = i + 1; j < paths.size(); ++j) {
//             std::cout << "" << std::endl;
//             std::cout << "Comparaison entre " << paths[i] << " et " << paths[j] << " : " << std::endl;
//             distanceImage(paths[i], paths[j]);
//         }
//     }
//     // distanceImage(path1, path2);

//     return 0;
// }
