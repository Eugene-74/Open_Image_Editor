#pragma once

#include <QApplication>
#include <QMainWindow>

#include "../../structure/data/Data.h"


class PhotoBooth : public QMainWindow {
    Q_OBJECT

public:

    PhotoBooth(Data* dat, QWidget* parent = nullptr);
    ~PhotoBooth() {}

private:
    Data* data;
};
