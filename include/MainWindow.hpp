#pragma once

#include <QHBoxLayout>
#include <QMainWindow>

#include "Data.hpp"

// Forward declaration
class QHBoxLayout;

class MainWindow : public QMainWindow {
    Q_OBJECT

   public:
    MainWindow(std::shared_ptr<Data> dat, QWidget* parent = nullptr);
    ~MainWindow() {}

    std::shared_ptr<Data> data;
    QHBoxLayout* mainLayout;
    QHBoxLayout* switchLayout;

    QSize* actionSize = &data->getSizesPtr()->mainWindowSizes->actionSize;
   signals:
    void switchToImageBooth();
    void switchToImageEditor();

   protected:
    void enterEvent(QEnterEvent* event) override;
};