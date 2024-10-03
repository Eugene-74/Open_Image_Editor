#include "imageEditor.h"

#include <opencv2/opencv.hpp>
#include <iostream>
#include <QApplication>
#include <QLabel>
#include <QWidget>

#include <QPixmap>
#include <QPushButton>
#include <QVBoxLayout>
#include <QPainter>

#include "../main.h"
#include "../../structure/imagesData/imagesData.h"

// #include "../../structure/imagesData/imagesData.h"

// using ImagesData = std::vector<ImageData*>;


#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QResizeEvent>
#include <QSize>
#include <QGraphicsOpacityEffect>
#include <QTimer>


// #include "../../functions/clickableLabel/ClickableLabel.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent)
    : QLabel(parent) {
    // Load image using OpenCV with alpha channel
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (!image.empty()) {
        // Convert BGR to RGB (if needed) and keep alpha channel
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            this->setPixmap(QPixmap::fromImage(qImage).scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            this->setPixmap(QPixmap::fromImage(qImage).scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
    else {
        // Handle the case where the image is not valid (optional)
        this->setText("Erreur : Image non valide !");
    }

    this->setAlignment(Qt::AlignCenter);

    // Enable mouse tracking
    setMouseTracking(true);


    // opacityEffect->setOpacity(1.0);  // Default opacity
    setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid transparent;
            border-radius: 15px;
            background-color: #b3b3b3; 
        }
    )");
    // connect(timer, &QTimer::timeout, this, &ClickableLabel::resetStyle);
}
// Gérer l'entrée de la souris
void ClickableLabel::enterEvent(QEvent* event) {
    // opacityEffect->setOpacity(0.5);  // Réduire l'opacité à 50% au survol
    QLabel::enterEvent(event);
}

// Gérer la sortie de la souris
void ClickableLabel::leaveEvent(QEvent* event) {
    // opacityEffect->setOpacity(1.0);  // Remettre l'opacité à 100% après le survol
    QLabel::leaveEvent(event);
}

void ClickableLabel::mousePressEvent(QMouseEvent* event) {

    if (event->button() == Qt::LeftButton) {
        // Emit clicked signal
        emit clicked();
        // Change style to indicate click
        setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid #969393;
            border-radius: 15px;
            background-color: #9c9c9c; 
        }
    )");
    }
    QLabel::mousePressEvent(event);  // Call the base class implementation

}

void ClickableLabel::mouseReleaseEvent(QMouseEvent* event) {
    emit clicked();  // Émettre le signal quand on clique

    if (event->button() == Qt::LeftButton) {
        // Reset style on mouse release
        setStyleSheet(R"(
        QLabel {
            border: 2px solid transparent;
            border-radius: 15px;
        }
        QLabel:hover {
            border: 2px solid transparent;
            border-radius: 15px;
            background-color: #b3b3b3; 
        }
    )");
    }
    QLabel::mouseReleaseEvent(event);  // Call the base class implementation
}



ImageEditor::ImageEditor(ImagesData& imagesData, QWidget* parent) : QMainWindow(parent) {

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenR = screen->availableGeometry();
    qreal pixelRatio = screen->devicePixelRatio();
    // screen->geometry()
        // QRect screenSize = screen.geometry();
    QSize screenGeometry = screenR.size();

    int actionButtonSize;
    if (screenGeometry.width() < screenGeometry.height()) {
        actionButtonSize = (screenGeometry.width() * 1 / 12) / pixelRatio;
    }
    else {
        actionButtonSize = (screenGeometry.height() * 1 / 12) / pixelRatio;

    }


    // QRect windowRect = screen->geometry();
    // windowRect.setSize(size());
    // std::cerr << screen->geometry().height() << std::endl;


    // std::cerr << screenGeometry.size().width() << std::endl;
    // std::cerr << screenGeometry.size().height() << std::endl;


    // Créer un widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Créer un layout vertical pour toute la fenêtre
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(10);  // Espacement entre les widgets
    mainLayout->setContentsMargins(20, 20, 20, 20); // Marges autour des bords (gauche, haut, droite, bas)

    QHBoxLayout* actionButtonLayout = new QHBoxLayout();
    // Créer les boutons avec des tailles spécifiques
    // TODO mieux deffir pour que il soit carrer
    ClickableLabel* imageActionLabel1 = new ClickableLabel("ressources/rotateRight.png", this);
    imageActionLabel1->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* imageActionLabel2 = new ClickableLabel("ressources/rotateLeft.png", this);
    imageActionLabel2->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)

    actionButtonLayout->addWidget(imageActionLabel1);
    actionButtonLayout->addWidget(imageActionLabel2);

    actionButtonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(actionButtonLayout);



    // Créer un layout horizontal pour les boutons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Créer les boutons avec des tailles spécifiques
    ClickableLabel* buttonImageBefore = new ClickableLabel("ressources/before.png", this);
    buttonImageBefore->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* buttonImageNext = new ClickableLabel("ressources/next.png", this);
    buttonImageNext->setFixedSize(actionButtonSize, actionButtonSize); // Définir la taille fixe du bouton (largeur, hauteur)


    // Créer un QLabel pour afficher l'image
    imageLabel = new QLabel(this);
    // screenGeometry.size() * 5 / 6
    imageLabel->setFixedSize((screenGeometry.width() * 4 / 6) / pixelRatio, (screenGeometry.height() * 4 / 6) / pixelRatio); // Ajuster la taille de l'image
    imageLabel->setAlignment(Qt::AlignCenter); // Centrer l'image dans le QLabel


    buttonLayout->addWidget(buttonImageBefore);
    buttonLayout->addWidget(imageLabel, 0, Qt::AlignCenter);
    buttonLayout->addWidget(buttonImageNext);
    buttonLayout->setAlignment(Qt::AlignCenter); // Centrer les boutons horizontalement

    // Ajouter le layout des boutons au layout principal
    mainLayout->addLayout(buttonLayout);

    // Connecter les images cliquables à des actions
    connect(buttonImageBefore, &ClickableLabel::clicked, [this, &imagesData]() { this->previousImage(imagesData); });
    connect(buttonImageNext, &ClickableLabel::clicked, [this, &imagesData]() { this->nextImage(imagesData); });

    // Définir le titre de la fenêtre
    setWindowTitle("Changer l'image dans QMainWindow");
}

void ImageEditor::setImage(const QString& imagePath) {
    // Load image using OpenCV
    cv::Mat image = cv::imread(imagePath.toStdString(), cv::IMREAD_UNCHANGED);
    if (!image.empty()) {
        // Convert the OpenCV image (BGR format) to QImage (RGB format)
        if (image.channels() == 4) {
            // Create QImage from OpenCV Mat with alpha channel
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_ARGB32);
            // Set the pixmap with scaling
            imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else {
            // Handle images without an alpha channel (optional)
            cv::cvtColor(image, image, cv::COLOR_BGR2RGB);
            QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);
            imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        // cv::cvtColor(image, image, cv::COLOR_BGR2RGB); // Convert BGR to RGB
        // QImage qImage(image.data, image.cols, image.rows, image.step[0], QImage::Format_RGB888);

        // // Set the QPixmap to the label with scaling
        // imageLabel->setPixmap(QPixmap::fromImage(qImage).scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    else {
        imageLabel->setText("Erreur : Image non valide !");
    }
    reload();
}


void ImageEditor::nextImage(ImagesData& imagesData){


    imagesData.setImageNumber(imagesData.getImageNumber() + 1);


    const std::string imagePath = imagesData.getImageData(imagesData.getImageNumber())->imagePath;

    QString qimagePath = QString::fromStdString(imagePath);

    setImage(qimagePath);
    std::cerr << "prochaine image : " << imagePath << std::endl;
}

void ImageEditor::previousImage(ImagesData& imagesData){


    imagesData.setImageNumber(imagesData.getImageNumber() - 1);


    const std::string imagePath = imagesData.getImageData(imagesData.getImageNumber())->imagePath;

    QString qimagePath = QString::fromStdString(imagePath);

    setImage(qimagePath);
    std::cerr << "prochaine image : " << imagePath << std::endl;
}

void ImageEditor::reload(){
    imageLabel->update();
}


