#pragma once

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
#include <QTimer>
#include <iostream>
#include <QPushButton>
#include <QResizeEvent>
#include <QSize>
#include <QGraphicsOpacityEffect>
#include <QPointer>


#include <opencv2/opencv.hpp>

#include "../../structure/imagesData/imagesData.h"

class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString& i, QWidget* parent = nullptr, QSize size = QSize(0, 0));

signals:
    void clicked();  // Signal émis lors du clic

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;   // Override mouse press event
    void mouseReleaseEvent(QMouseEvent* event) override;  // Override mouse release event




private:
    // QGraphicsOpacityEffect* opacityEffect;  // Déclaration de l'effet d'opacité
    QPixmap defaultPixmap;                  // Image par défaut pour le QLabel
};

class ImageEditor : public QMainWindow {
    Q_OBJECT

public:

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
    void updatePreview();


protected:
    // void resizeEvent(ImagesData& imagesData) override;


private:
    ImagesData& imagesData;
    QLabel* imageLabel; // Pour afficher l'image
    // QVBoxLayout& mainLayout;
    QHBoxLayout* previewButtonLayout;
    QSize previewSize;



};



