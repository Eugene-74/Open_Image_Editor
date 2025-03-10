#pragma once

#include <QCheckBox>
#include <QDialog>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <map>
#include <string>
struct Option {
    std::string type;
    std::string value;

    Option() = default;
    Option(const std::string& type, const std::string& value)
        : type(type), value(value) {
    }
};
void showModalDialog(QWidget* parent, QMessageBox::Icon icon, std::string text, std::string title, int posX, int posY, bool async);

void showInformationMessage(QWidget* parent, const std::string& text, const std::string& title = "information", int posX = -1, int posY = -1, bool async = false);
void showWarningMessage(QWidget* parent, const std::string& text, const std::string& title = "warning", int posX = -1, int posY = -1, bool async = false);
void showErrorMessage(QWidget* parent, const std::string& text, const std::string& title = "error", int posX = -1, int posY = -1, bool async = false);
void showQuestionMessage(QWidget* parent, const std::string& text, std::function<void(bool)> callback, const std::string& title = "question", int posX = -1, int posY = -1, bool async = false);

std::map<std::string, std::string> showOptionsDialog(QWidget* parent, const std::string& windowName, const std::map<std::string, Option>& options);
