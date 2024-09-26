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

ClickableLabel::ClickableLabel(const QString& imagePath, QWidget* parent)
    : QLabel(parent) {
    QPixmap pixmap(imagePath);
    this->setPixmap(pixmap.scaled(this->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    this->setAlignment(Qt::AlignCenter);
}

ImageEditor::ImageEditor(ImagesData& imagesData, QWidget* parent) : QMainWindow(parent) {
    // Créer un widget central
    QWidget* centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Créer un layout vertical
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);
    layout->setSpacing(10);  // Espacement entre les widgets
    layout->setContentsMargins(20, 20, 20, 20);// Marges autour des bords (gauche, haut, droite, bas)
    // Créer un QLabel pour afficher l'image
    imageLabel = new QLabel(this);

    QSize imageLabelSize = QGuiApplication::primaryScreen()->size(); // taille de l'ecran
    imageLabelSize.setWidth(imageLabelSize.width() * 2 / 3);
    imageLabelSize.setHeight(imageLabelSize.height() * 2 / 3);
    // std::cerr << imageLabelSize.height() << "\n" << std::endl;
    // std::cerr << imageLabelSize.width() << std::endl;
    imageLabel->setFixedSize(imageLabelSize);
    // TODO enlevre ça :
    imageLabel->setFixedSize(500, 300);


    imageLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(imageLabel);

    // Créer un bouton pour changer l'image
    QPushButton* nextImageButton = new QPushButton("Changer l'image", this);
    layout->addWidget(nextImageButton);
    connect(nextImageButton, &QPushButton::clicked, [this, &imagesData]() { this->nextImage(imagesData); });

    QWidget* upWidget = new QWidget(this);

    // Créer un layout horizental
    QHBoxLayout* buttonLayout = new QHBoxLayout(upWidget);
    buttonLayout->setSpacing(10);

    // Créer et ajouter des images cliquables
    ClickableLabel* imageLabel1 = new ClickableLabel(":ressources/next.JPG", this);
    buttonLayout->addWidget(imageLabel1);

    ClickableLabel* imageLabel2 = new ClickableLabel(":ressources/before.JPG", this);
    buttonLayout->addWidget(imageLabel2);

    // Connecter les images cliquables à des actions
    connect(imageLabel1, &ClickableLabel::clicked, [this, &imagesData]() { this->nextImage(imagesData); });
    connect(imageLabel2, &ClickableLabel::clicked, [this, &imagesData]() { this->previousImage(imagesData); });



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

    std::cerr << "prochaine image : " << std::endl;

    imagesData.setImageNumber(imagesData.getImageNumber() + 1);


    const std::string imagePath = imagesData.getImageData(imagesData.getImageNumber())->imagePath;

    QString qimagePath = QString::fromStdString(imagePath);

    setImage(qimagePath);
    std::cerr << "prochaine image : " << imagePath << std::endl;
}

void ImageEditor::previousImage(ImagesData& imagesData){

    std::cerr << "prochaine image : " << std::endl;

    imagesData.setImageNumber(imagesData.getImageNumber() - 1);


    const std::string imagePath = imagesData.getImageData(imagesData.getImageNumber())->imagePath;

    QString qimagePath = QString::fromStdString(imagePath);

    setImage(qimagePath);
    std::cerr << "prochaine image : " << imagePath << std::endl;
}

void ImageEditor::reload(){
    imageLabel->update();
}


