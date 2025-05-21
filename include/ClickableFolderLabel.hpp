#pragma once

#include "ClickableLabel.hpp"

class ClickableFolderLabel : public ClickableLabel {
    Q_OBJECT
   public:
    ClickableFolderLabel(std::shared_ptr<Data> dat, const QString& imagePath, QString toolTip = "", QWidget* parent = nullptr, QSize* sizePtr = nullptr, bool setSize = false, int thumbnail = 0, bool square = false, bool force = false);

    void setText(const std::string& text);

   protected:
    void paintEvent(QPaintEvent* event) override;

   private:
    std::shared_ptr<Data> data = nullptr;
    std::string text = "";
};