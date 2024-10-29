#pragma once
#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QVBoxLayout>

#include "../../structure/imagesData/ImagesData.h"
#include "../../structure/data/Data.h"

// Assurez-vous que le chemin d'accès à imageEditor.h est correct
#include "../imageEditor/ImageEditor.h"

class InitialWindow : public QMainWindow {
    Q_OBJECT

public:
    InitialWindow();
    qreal pixelRatio;
    QSize screenGeometry;

    void createImageEditor(Data& data);
    void clearImageEditor();

protected:
    // void resizeEvent(ImagesData& imagesData) override;

private:
    // Data* data;
    // ImageEditor* imageEditor; // Déclarer un pointeur vers ImageEditor
};