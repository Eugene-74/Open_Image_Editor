#pragma once

#include <QMessageBox>
#include <map>
#include <string>

#include "Option.hpp"

// Forward declarations
class QWidget;

void showModalDialog(QWidget* parent, QMessageBox::Icon icon, const std::string& text, const std::string& title, int posX, int posY, bool async);

void showInformationMessage(QWidget* parent, const std::string& text, const std::string& title = "information", int posX = -1, int posY = -1, bool async = false);
void showWarningMessage(QWidget* parent, const std::string& text, const std::string& title = "warning", int posX = -1, int posY = -1, bool async = false);
void showErrorMessage(QWidget* parent, const std::string& text, const std::string& title = "error", int posX = -1, int posY = -1, bool async = false);
void showQuestionMessage(QWidget* parent, const std::string& text, std::function<void(bool)> callback, const std::string& title = "question", int posX = -1, int posY = -1, bool async = false);

std::map<std::string, std::string> showOptionsDialog(QWidget* parent, const std::string& windowName, const std::map<std::string, Option>& options);
