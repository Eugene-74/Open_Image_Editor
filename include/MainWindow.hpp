#pragma once

#include <QHBoxLayout>
#include <QMainWindow>

#include "Data.hpp"

// Forward declaration
class QHBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(Data* dat, QWidget* parent = nullptr);
    ~MainWindow() {}

    Data* data;
    QHBoxLayout* mainLayout;
    QHBoxLayout* switchLayout;

    QSize* actionSize = &data->sizes->mainWindowSizes->actionSize;
   signals:
    void switchToImageBooth();
    void switchToImageEditor();

   protected:
    void enterEvent(QEnterEvent* event) override;
};