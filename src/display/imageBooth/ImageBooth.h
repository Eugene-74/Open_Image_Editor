#pragma once

#include <QApplication>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QScrollArea>
#include <vector>

#include "../../structure/data/Data.h"
#include "../../display/clickableLabel/ClickableLabel.h"



class ImageBooth : public QMainWindow {
    Q_OBJECT

public:

    ImageBooth(Data* dat, QWidget* parent = nullptr);
    ~ImageBooth() {}

private:
    Data* data;
    QSize screenGeometry;
    int imageNumber;
    QScrollArea* scrollArea;

    QHBoxLayout* mainLayout;
    QVBoxLayout* linesLayout;

    QSize imageSize;



    ClickableLabel* createImage(std::string imagePath);
    void createLine();
    void setImageNumber(int nbr);


};
