#include "FaceRecognition.hpp"

#include <dlib/cuda/cuda_dlib.h>
#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/opencv.h>

#include <QDebug>
#include <QFile>
#include <QInputDialog>

#include <QImage>
#include <opencv2/opencv.hpp>

#include "Data.hpp"

    using namespace dlib;
using namespace std;

double averageDistance(const std::vector<cv::Point2f>& landmarks1, const std::vector<cv::Point2f>& landmarks2) {
    if (landmarks1.size() != landmarks2.size()) {
        throw std::runtime_error("Les ensembles de points caractéristiques doivent avoir la même taille.");
    }

    double sommeDistances = 0.0;
    for (size_t i = 0; i < landmarks1.size(); ++i) {
        double distance = cv::norm(landmarks1[i] - landmarks2[i]);
        sommeDistances += distance;
    }
    return sommeDistances / landmarks1.size();
}

bool isSamePerson(const std::vector<cv::Point2f>& landmarks1, const std::vector<cv::Point2f>& landmarks2, double seuil) {
    double distanceMoyenne = averageDistance(landmarks1, landmarks2);
    return distanceMoyenne < seuil;
}

void Person::save(std::ofstream& out) const {
    size_t nameSize = name.size();
    out.write(reinterpret_cast<const char*>(&nameSize), sizeof(nameSize));
    out.write(name.c_str(), nameSize);
    out.write(reinterpret_cast<const char*>(&face.x), sizeof(face.x));
    out.write(reinterpret_cast<const char*>(&face.y), sizeof(face.y));
    out.write(reinterpret_cast<const char*>(&face.width), sizeof(face.width));
    out.write(reinterpret_cast<const char*>(&face.height), sizeof(face.height));

    size_t landmarksSize = landmarks.size();
    out.write(reinterpret_cast<const char*>(&landmarksSize), sizeof(landmarksSize));
    for (const auto& landmark : landmarks) {
        out.write(reinterpret_cast<const char*>(&landmark.x), sizeof(landmark.x));
        out.write(reinterpret_cast<const char*>(&landmark.y), sizeof(landmark.y));
    }
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

    size_t landmarksSize;
    in.read(reinterpret_cast<char*>(&landmarksSize), sizeof(landmarksSize));
    landmarks.resize(landmarksSize);
    for (auto& landmark : landmarks) {
        in.read(reinterpret_cast<char*>(&landmark.x), sizeof(landmark.x));
        in.read(reinterpret_cast<char*>(&landmark.y), sizeof(landmark.y));
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

bool isCudaAvailable() {
    try {
        dlib::cuda::get_num_devices();
        return true;
    } catch (dlib::cuda_error& e) {
        return false;
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

std::vector<Person> detectFacesCPU(std::string imagePath, QImage image) {
    try {
        // Conversion de QImage en cv::Mat
        cv::Mat mat = QImageToCvMat(image);

        // Conversion de l'image en niveaux de gris
        cv::Mat gray;
        cv::cvtColor(mat, gray, cv::COLOR_BGR2GRAY);

        // Chargement du classificateur en cascade pour la détection des visages
        cv::CascadeClassifier face_cascade;
        if (!face_cascade.load(":/models/haarcascade_frontalface_default.xml")) {
            qDebug() << "Erreur lors du chargement du classificateur en cascade";
            return {};
        }

        // Chargement du modèle de détection des points de repère faciaux
        // cv::Ptr<cv::face::Facemark> facemark = cv::face::FacemarkLBF::create();
        // facemark->loadModel(":/models/lbfmodel.yaml");
        // if (facemark->empty()) {
        //     qDebug() << "Erreur lors du chargement du modèle de points de repère faciaux";
        //     return {};
        // }

        // Détection des visages
        std::vector<cv::Rect> faces;
        face_cascade.detectMultiScale(gray, faces, 1.3, 5, cv::CASCADE_SCALE_IMAGE, cv::Size(image.width() / 20, image.height() / 20));

        qDebug() << "Nombre de visages détectés : " << faces.size();

        // Détection des points de repère faciaux
        std::vector<std::vector<cv::Point2f>> landmarks;
        // bool success = facemark->fit(mat, faces, landmarks);

        // qDebug() << "Facial landmark detection success: " << success;

        // Création de la liste des personnes détectées
        std::vector<Person> persons;
        // if (success) {
        for (size_t i = 0; i < faces.size(); ++i) {
            Person person;
            for (size_t j = 0; j < i; ++j) {
                if (isSamePerson(landmarks[i], persons[j].getLandmarks(), 0.6)) {
                    person.setName(persons[j].getName());
                    qDebug() << "Personne " << j << " reconnue comme la même personne que la personne " << i;
                    break;
                }
            }
            person.setFace(faces[i]);
            person.setName("Inconnu");
            // person.setLandmarks(landmarks[i]);
            persons.push_back(person);
            qDebug() << "Visage " << i << " : " << faces[i].x << ", " << faces[i].y << ", " << faces[i].width << ", " << faces[i].height;
            // qDebug() << "Landmarks " << i << " : " << landmarks[i].size();
        }
        // } else {
        //     qDebug() << "La détection des points de repère faciaux a échoué";
        // }

        return persons;
    } catch (const std::exception& e) {
        qDebug() << "Exception : " << e.what();
        return {};
    }
}

std::vector<Person> detectFacesCUDA(std::string imagePath, QImage image) {
    qDebug() << "not working";

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

    std::vector<cv::Rect> faces;
    for (const auto& d : dets) {
        faces.push_back(cv::Rect(cv::Point(d.left(), d.top()), cv::Point(d.right(), d.bottom())));
    }

    qDebug() << "Nombre de visages détectés : " << faces.size();

    // cv::Ptr<cv::face::Facemark> facemark = cv::face::FacemarkLBF::create();
    // facemark->loadModel(":/models/lbfmodel.yaml");
    // if (facemark->empty()) {
    //     qDebug() << "Erreur lors du chargement du modèle de points de repère faciaux";
    //     return {};
    // }
    for (auto& face : faces) {
        int newWidth = static_cast<int>(face.width * 1.1);
        int newHeight = static_cast<int>(face.height * 1.2);
        int newX = face.x - (newWidth - face.width) / 2;
        int newY = face.y - (newHeight - face.height) / 2;

        // Ensure the new dimensions do not go out of bounds
        if (newX >= 0 && newX + newWidth <= resizedMat.cols) {
            face.x = newX;
            face.width = newWidth;
        }
        if (newY >= 0 && newY + newHeight <= resizedMat.rows) {
            face.y = newY;
            face.height = newHeight;
        }

        face.x *= newSize;
        face.y *= newSize;
        face.width *= newSize;
        face.height *= newSize;
    }

    std::vector<Person> persons;

    for (size_t i = 0; i < faces.size(); ++i) {
        Person person;
        person.setFace(faces[i]);
        person.setName("Inconnu");
        persons.push_back(person);
        qDebug() << "Visage " << i << " : " << faces[i].x << ", " << faces[i].y << ", " << faces[i].width << ", " << faces[i].height;
    }

    return persons;
}

std::pair<int, double> recognize_face(cv::Ptr<cv::face::LBPHFaceRecognizer> model, const cv::Mat& test_image) {
    int predicted_label = -1;
    double confidence = 0.0;

    model->predict(test_image, predicted_label, confidence);

    std::cout << "Predicted Label: " << predicted_label << std::endl;
    std::cout << "Confidence: " << confidence << std::endl;

    return std::make_pair(predicted_label, confidence);
}

void detectFacesAsync(Data* data, std::string imagePath, QImage image, std::function<void(std::vector<Person>)> callback) {
    data->addHeavyThread([=]() {
        std::vector<Person> persons;
        // if (isCudaAvailable()) {
        //     qDebug() << "launch CUDA";
        persons = detectFacesCUDA(imagePath, image);

        try {
            callback(persons);
        } catch (const std::exception& e) {
            qCritical() << e.what();
        }
    });
}

std::string Person::getName() const {
    return name;
}

cv::Rect Person::getFace() const {
    return face;
}

std::vector<cv::Point2f> Person::getLandmarks() const {
    return this->landmarks;
}

void Person::setName(std::string name) {
    this->name = name;
}

void Person::setFace(cv::Rect face) {
    this->face = face;
}

void Person::setLandmarks(std::vector<cv::Point2f> landmarks) {
    this->landmarks = landmarks;
}

// void load_images(std::vector<cv::Mat>& images, std::vector<int>& labels, std::vector<std::string>& file_names, const std::string& path) {
//         std::vector<cv::String> file_paths;
//         cv::glob(path, file_paths);  // Get all the file paths from the directory

//         for (size_t i = 0; i < file_paths.size(); ++i) {
//             cv::Mat img = cv::imread(file_paths[i], cv::IMREAD_GRAYSCALE);  // Read image as grayscale
//             if (!img.empty()) {
//                 images.push_back(img);
//                 labels.push_back(i);                  // You can use the index as the label or create custom labels
//                 file_names.push_back(file_paths[i]);  // Store the file name
//             }
//         }
//     }

void computeFaces(Data* data, std::string imagePath) {
    QImage image = data->imageCache->at(imagePath).image;

    // TODO convertie
    cv::Mat matImage = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);

    cv::Ptr<cv::face::LBPHFaceRecognizer> model = data->model;
    // qDebug() << self->data->getImagesData()->getImageData(self->data->getImagesData()->getImageNumberInTotal(recognize_face(self->data->model, matImage)))->getImagePath();
    std::vector<Person> persons = data->getImagesData()->getImageData(imagePath)->getpersons();
    for (auto& person : persons) {
        std::vector<cv::Mat> images;
        std::vector<int> labels;
        cv::Rect faceRect = person.getFace();
        cv::Mat matImage = cv::imread(imagePath, cv::IMREAD_GRAYSCALE);
        cv::Mat face = matImage(faceRect).clone();
        images.push_back(face);
        if (model->empty()) {
            qDebug() << "Le modèle n'a pas encore été entraîné. Entraînement en cours...";
            qDebug() << "start train";
            labels.push_back(1);
            model->train(images, labels);
            qDebug() << "stop train";

            person.setName("Person_" + std::to_string(labels.back()));
            qDebug()
                << "Entraînement terminé.";
        } else {
            std::pair<int, double> personValue = recognize_face(model, face);
            // TODO if confident < 30 : alors ajouter la personne
            if (personValue.second < 30) {
                labels.push_back(personValue.first);
            } else if (personValue.second < 80) {
                // Create a list of existing person names
                QStringList personNames;
                for (const auto& existingPerson : persons) {
                    personNames << QString::fromStdString(existingPerson.getName());
                }

                // Show a popup to choose among existing persons
                bool ok;
                QString selectedPerson = QInputDialog::getItem(nullptr, "Select Person", "Choose the person:", personNames, 0, false, &ok);

                if (ok && !selectedPerson.isEmpty()) {
                    // Find the selected person and use their label
                    auto it = std::find_if(persons.begin(), persons.end(), [&](const Person& p) {
                        return p.getName() == selectedPerson.toStdString();
                    });
                    if (it != persons.end()) {
                        labels.push_back(std::distance(persons.begin(), it));
                    }
                } else {
                    labels.push_back(model->getLabels().rows + 1);
                }
                qDebug() << "may be";
            } else {
                labels.push_back(model->getLabels().rows + 1);
            }
            person.setName("Person_" + std::to_string(labels.back()));
            model->update(images, labels);

            qDebug() << "person number" << personValue.first;

            qDebug() << "Le modèle a déjà été entraîné.";
        }
    }
    data->getImagesData()->getImageData(imagePath)->setpersons(persons);
    // model->train(images, labels);

    data->save_model(model, APPDATA_PATH.toStdString() + "/lbph_face_recognizer.yml");
    // } else {
    // qDebug() << "launch CPU";

    // persons = detectFacesCPU(imagePath, image);
    // }
}