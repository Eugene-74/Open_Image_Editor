#pragma once

#include <QHBoxLayout>
#include <QMainWindow>

#include "ClickableLabel.hpp"
#include "Data.hpp"
#include "InitialWindow.hpp"

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