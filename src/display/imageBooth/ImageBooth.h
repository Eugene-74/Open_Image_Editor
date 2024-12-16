#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <vector>
#include <QTimer>

#include "../../structure/data/Data.h"
#include "../../display/clickableLabel/ClickableLabel.h"
#include "../../display/initialWindow/InitialWindow.h"

class ImageBooth : public QMainWindow
{
    Q_OBJECT

public:
    ImageBooth(Data *dat, QWidget *parent = nullptr);
    ~ImageBooth() {}

    void clear();

private:
    Data *data;
    int imageNumber;
    int loadedImageNumber = 0;
    int done = 0;

    QScrollArea *scrollArea;

    QHBoxLayout *mainLayout;
    QVBoxLayout *linesLayout;

    QSize imageSize = data->sizes.imagesBoothSizes->imageSize;
    QSize realImageSize = data->sizes.imagesBoothSizes->realImageSize;

    void createLine();
    ClickableLabel *createImage(std::string imagePath, int nbr);
    void setImageNumber(int nbr);

signals:
    void switchToImageEditor();
};
