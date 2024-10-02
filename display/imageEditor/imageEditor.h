
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



class ClickableLabel : public QLabel {
    Q_OBJECT

public:
    explicit ClickableLabel(const QString& imagePath, QWidget* parent = nullptr);

signals:
    void clicked();  // Signal émis lors du clic

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    // Gestion de l'événement de clic de souris
    void mousePressEvent(QMouseEvent* event) override {
        emit clicked();  // Émettre le signal quand on clique
    }

private:
    QGraphicsOpacityEffect* opacityEffect;  // Déclaration de l'effet d'opacité
    QPixmap defaultPixmap;                  // Image par défaut pour le QLabel
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


protected:
    // void resizeEvent(ImagesData& imagesData) override;


private:
    QLabel* imageLabel; // Pour afficher l'image
};
#endif

