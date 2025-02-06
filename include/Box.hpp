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
#include <map>
#include <string>

struct Option {
    std::string type;
    std::string value;

    Option() = default;  // Default constructor
    Option(const std::string& type, const std::string& value)
        : type(type), value(value) {
    }
};

void showInformationMessage(QWidget* parent, std::string text);
void showWarningMessage(QWidget* parent, std::string text);
void showErrorMessage(QWidget* parent, std::string text);
bool showQuestionMessage(QWidget* parent, std::string text);
std::map<std::string, std::string> showOptionsDialog(QWidget* parent, const std::string windowName, const std::map<std::string, Option>& options);
