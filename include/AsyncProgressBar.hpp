#pragma once
#include <QProgressBar>
#include <mutex>

class AsyncProgressBar : public QProgressBar {
    Q_OBJECT
   private:
    mutable std::mutex mutex;

   public:
    using QProgressBar::QProgressBar;

    void setValue(int value) {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::setValue(value);
    }
    int value() const {
        std::lock_guard<std::mutex> lock(mutex);
        return QProgressBar::value();
    }
    void setMinimum(int min) {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::setMinimum(min);
    }
    void setMaximum(int max) {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::setMaximum(max);
    }
    void setRange(int min, int max) {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::setRange(min, max);
    }
    void show() {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::show();
    }
    void hide() {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::hide();
    }
    void setStyleSheet(const QString& style) {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::setStyleSheet(style);
    }
    void setAlignment(Qt::Alignment alignment) {
        std::lock_guard<std::mutex> lock(mutex);
        QProgressBar::setAlignment(alignment);
    }
};
