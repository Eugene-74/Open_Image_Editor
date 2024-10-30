#pragma once

#include <QDialog>
#include <QQuickWidget>

class MapWindow : public QDialog {
    Q_OBJECT

public:
    explicit MapWindow(QWidget* parent = nullptr);
    ~MapWindow();
    void setCoordinates(double latitude, double longitude);

private:
    QQuickWidget* quickWidget;
};
