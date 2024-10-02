#include "imageEditor.h"


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


// #include "../../functions/clickableLabel/ClickableLabel.h"

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent)
    : QLabel(parent) {
    // TODO mettre dans clickableLabel
    QPixmap pixmap(imagePath);
    this->setPixmap(pixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    this->setAlignment(Qt::AlignCenter);

    // Activer l'événement de survol
    setMouseTracking(true);

    // Créer un effet d'opacité
    opacityEffect = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(opacityEffect);
    opacityEffect->setOpacity(1.0);  // Opacité par défaut
}
// Gérer l'entrée de la souris
void ClickableLabel::enterEvent(QEvent* event) {
    opacityEffect->setOpacity(0.5);  // Réduire l'opacité à 50% au survol
    QLabel::enterEvent(event);
}

// Gérer la sortie de la souris
void ClickableLabel::leaveEvent(QEvent* event) {
    opacityEffect->setOpacity(1.0);  // Remettre l'opacité à 100% après le survol
    QLabel::leaveEvent(event);
}

ImageEditor::ImageEditor(ImagesData& imagesData, QWidget* parent) : QMainWindow(parent) {

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect screenR = screen->availableGeometry();
    qreal pixelRatio = screen->devicePixelRatio();
    // screen->geometry()
        // QRect screenSize = screen.geometry();
    QSize screenGeometry = screenR.size();

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
    ClickableLabel* imageActionLabel1 = new ClickableLabel(":ressources/rotateRight.png", this);
    imageActionLabel1->setFixedSize((screenGeometry.width() * 1 / 12) / pixelRatio, (screenGeometry.height() * 1 / 12) / pixelRatio); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* imageActionLabel2 = new ClickableLabel(":ressources/rotateLeft.png", this);
    imageActionLabel2->setFixedSize((screenGeometry.width() * 1 / 12) / pixelRatio, (screenGeometry.height() * 1 / 12) / pixelRatio); // Définir la taille fixe du bouton (largeur, hauteur)

    actionButtonLayout->addWidget(imageActionLabel1);
    actionButtonLayout->addWidget(imageActionLabel2);

    actionButtonLayout->setAlignment(Qt::AlignCenter);

    mainLayout->addLayout(actionButtonLayout);



    // Créer un layout horizontal pour les boutons
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Créer les boutons avec des tailles spécifiques
    ClickableLabel* buttonImageBefore = new ClickableLabel(":ressources/before.png", this);
    buttonImageBefore->setFixedSize((screenGeometry.width() * 1 / 12) / pixelRatio, (screenGeometry.height() * 1 / 12) / pixelRatio); // Définir la taille fixe du bouton (largeur, hauteur)

    ClickableLabel* buttonImageNext = new ClickableLabel(":ressources/next.png", this);
    buttonImageNext->setFixedSize((screenGeometry.width() * 1 / 12) / pixelRatio, (screenGeometry.height() * 1 / 12) / pixelRatio); // Définir la taille fixe du bouton (largeur, hauteur)


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
    QPixmap pixmap(imagePath);
    if (!pixmap.isNull()) {
        // imageLabel->resize(event->size());
        imageLabel->setPixmap(pixmap.scaled(imageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
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


