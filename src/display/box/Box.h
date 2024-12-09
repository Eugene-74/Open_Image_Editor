#pragma once

#include <QMessageBox>
#include <map>
#include <string>

struct Option {
    std::string type;
    std::string defaultValue;

    Option(const std::string& type, const std::string& defaultValue)
        : type(type), defaultValue(defaultValue) {
    }
};

void showInformationMessage(QWidget* parent, std::string text);
void showWarningMessage(QWidget* parent, std::string text);
void showErrorMessage(QWidget* parent, std::string text);
bool showQuestionMessage(QWidget* parent, std::string text);
std::map<std::string, std::string> showOptionsDialog(QWidget* parent, const std::string windowName, const std::map<std::string, Option>& options);
