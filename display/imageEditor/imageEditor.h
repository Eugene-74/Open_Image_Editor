
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
// #include "../../functions/clickableLabel/ClickableLabel.h"

#include <QGraphicsOpacityEffect>
#include <QTimer>


class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString& i, QWidget* parent = nullptr);

signals:
    void clicked();  // Signal émis lors du clic

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;   // Override mouse press event
    void mouseReleaseEvent(QMouseEvent* event) override;  // Override mouse release event

    // Gestion de l'événement de clic de souris
    // void mousePressEvent(QMouseEvent* event) override;



private:
    QGraphicsOpacityEffect* opacityEffect;  // Déclaration de l'effet d'opacité
    QPixmap defaultPixmap;                  // Image par défaut pour le QLabel
};

class ImageEditor : public QMainWindow {
    Q_OBJECT

public:
    ImagesData& imagesData;
    // Constructor
    ImageEditor(ImagesData& a, QWidget* parent = nullptr);   // Initialize the reference member

    ~ImageEditor(){}

    void reload();
    void reloadMainImage();

    void setImage(ImageData& imageData);

    void nextImage();
    void previousImage();
    void rotateLeft();
    void rotateRight();


protected:
    // void resizeEvent(ImagesData& imagesData) override;


private:
    QLabel* imageLabel; // Pour afficher l'image

};
#endif

