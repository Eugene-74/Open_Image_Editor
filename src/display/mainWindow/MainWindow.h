#pragma once

#include <QMainWindow>
#include <QHBoxLayout>

#include "../../structure/data/Data.h"
#include "../../display/clickableLabel/ClickableLabel.h"

#include "../../display/initialWindow/InitialWindow.h"


class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(Data* dat, QWidget* parent = nullptr);
    ~MainWindow() {}

    void clear();

    Data* data;
    QHBoxLayout* mainLayout;
    QHBoxLayout* switchLayout;
signals:
    void switchToImageBooth();
    void switchToImageEditor();


};