#include "Main.hpp"

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QGuiApplication>
#include <QIcon>
#include <QScreen>

#include "InitialWindow.hpp"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setWindowIcon(QIcon(":/icons/icon.ico"));

    InitialWindow window;
    window.showMaximized();
    QScreen* screen = QGuiApplication::primaryScreen();

    QRect screenGeometry = screen->geometry();
    window.setGeometry(0, 0, screenGeometry.width(), screenGeometry.height());
    window.setMinimumSize(screenGeometry.width() / 3, screenGeometry.height() / 3);

    return app.exec();
}

// #include <fstream>
// #include <opencv2/opencv.hpp>

// #include "Const.hpp"
// #include "FaceRecognition.hpp"

// std::vector<std::string> load_class_list() {
//     std::vector<std::string> class_list;
//     std::ifstream ifs(APP_FILES.toStdString() + "/coco.names");
//     std::string line;
//     while (getline(ifs, line)) {
//         class_list.push_back(line);
//     }
//     return class_list;
// }

// cv::dnn::Net load_net(bool is_cuda) {
//     auto result = cv::dnn::readNet(APP_FILES.toStdString() + "/yolov5n.onnx");
//     if (result.empty()) {
//         std::cerr << "Error loading network\n";
//         return result;
//     }
//     if (is_cuda) {
//         std::cout << "Attempty to use CUDA\n";
//         result.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
//         result.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA_FP16);
//     } else {
//         std::cout << "Running on CPU\n";
//         result.setPreferableBackend(cv::dnn::DNN_BACKEND_OPENCV);
//         result.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
//     }
//     return result;
// }

// const float INPUT_WIDTH = 640.0;
// const float INPUT_HEIGHT = 640.0;
// const float SCORE_THRESHOLD = 0.2;
// const float NMS_THRESHOLD = 0.4;
// const float CONFIDENCE_THRESHOLD = 0.4;

// cv::Mat format_yolov5(const cv::Mat &source) {
//     int col = source.cols;
//     int row = source.rows;
//     int _max = MAX(col, row);
//     cv::Mat result = cv::Mat::zeros(_max, _max, CV_8UC3);
//     source.copyTo(result(cv::Rect(0, 0, col, row)));
//     return result;
// }

// DetectedObjects detect(cv::Mat &image) {
//     bool is_cuda = false;
//     cv::dnn::Net net = load_net(is_cuda);
//     std::vector<std::string> className = load_class_list();

//     cv::Mat blob;

//     auto input_image = format_yolov5(image);

//     cv::dnn::blobFromImage(input_image, blob, 1. / 255., cv::Size(INPUT_WIDTH, INPUT_HEIGHT), cv::Scalar(), true, false);
//     net.setInput(blob);
//     std::vector<cv::Mat> outputs;
//     net.forward(outputs, net.getUnconnectedOutLayersNames());

//     float x_factor = input_image.cols / INPUT_WIDTH;
//     float y_factor = input_image.rows / INPUT_HEIGHT;

//     float *data = (float *)outputs[0].data;

//     const int dimensions = 85;
//     const int rows = 25200;

//     std::vector<int> class_ids;
//     std::vector<float> confidences;
//     std::vector<cv::Rect> boxes;

//     for (int i = 0; i < rows; ++i) {
//         float confidence = data[4];
//         if (confidence >= CONFIDENCE_THRESHOLD) {
//             float *classes_scores = data + 5;
//             cv::Mat scores(1, className.size(), CV_32FC1, classes_scores);
//             cv::Point class_id;
//             double max_class_score;
//             minMaxLoc(scores, 0, &max_class_score, 0, &class_id);
//             if (max_class_score > SCORE_THRESHOLD) {
//                 confidences.push_back(confidence);

//                 class_ids.push_back(class_id.x);

//                 float x = data[0];
//                 float y = data[1];
//                 float w = data[2];
//                 float h = data[3];
//                 int left = int((x - 0.5 * w) * x_factor);
//                 int top = int((y - 0.5 * h) * y_factor);
//                 int width = int(w * x_factor);
//                 int height = int(h * y_factor);
//                 boxes.push_back(cv::Rect(left, top, width, height));
//             }
//         }

//         data += 85;
//     }
//     DetectedObjects detectedObjects;
//     std::map<std::string, std::vector<std::pair<cv::Rect, float>>> detectedObjectsMap;

//     std::vector<int> nms_result;
//     cv::dnn::NMSBoxes(boxes, confidences, SCORE_THRESHOLD, NMS_THRESHOLD, nms_result);

//     for (int i = 0; i < nms_result.size(); i++) {
//         int idx = nms_result[i];
//         std::string class_name = className[class_ids[idx]];
//         if (detectedObjectsMap.find(class_name) == detectedObjectsMap.end()) {
//             detectedObjectsMap[class_name] = std::vector<std::pair<cv::Rect, float>>();
//         }
//         detectedObjectsMap[class_name].emplace_back(boxes[idx], confidences[idx]);
//     }
//     detectedObjects.setDetectedObjects(detectedObjectsMap);
//     return detectedObjects;
// }

// int main(int argc, char **argv) {
//     cv::Mat frame;
//     frame = cv::imread(APP_FILES.toStdString() + "/test.jpg");

//     auto start = std::chrono::high_resolution_clock::now();

//     DetectedObjects detectedObjects = detect(frame);

//     for (const auto &[class_name, detections] : detectedObjects.getDetectedObjects()) {
//         std::cout << "Class: " << class_name << std::endl;
//         for (const auto &[box, confidence] : detections) {
//             std::cout << "  Box: " << box << " Confidence: " << confidence << std::endl;
//         }
//     }

//     return 0;
// }