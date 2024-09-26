
#ifndef IMAGEEDITOR_H
#define IMAGEEDITOR_H

#include <iostream>
#include "../../structure/imagesData/imagesData.h"


#include <QApplication>
#include <QMainWindow>
#include <QPushButton>
#include <QLabel>
#include <QPixmap>
#include <QVBoxLayout>
#include <QWidget>
#include <QFileDialog>
#include <QScreen>
#include <QMessageBox>

// Subclass QLabel to make it clickable
class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString& imagePath, QWidget* parent = nullptr);


signals:
    void clicked();  // Signal émis lors du clic

protected:
    // Gestion de l'événement de clic de souris
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked();  // Émettre le signal quand on clique
    }
};

class ImageEditor : public QMainWindow {
    Q_OBJECT

public:
    ImageEditor(ImagesData& imagesData, QWidget* parent = nullptr);
    ~ImageEditor(){}

    void reload();

    void setImage(const QString& imagePath);

    void nextImage(ImagesData& imagesData);
    void previousImage(ImagesData& imagesData);


    void onImage1Clicked() {
        QMessageBox::information(this, "Image 1", "Vous avez cliqué sur l'image 1 !");
    }

    void onImage2Clicked() {
        QMessageBox::information(this, "Image 2", "Vous avez cliqué sur l'image 2 !");
    }

protected:
    // void resizeEvent(ImagesData& imagesData) override;


private:
    QLabel* imageLabel; // Pour afficher l'image
};
#endif

