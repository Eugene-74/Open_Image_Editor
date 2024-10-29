#pragma once


#include <QMessageBox>

void showInformationMessage(QWidget* parent, std::string text);
void showWarningMessage(QWidget* parent, std::string text);
void showErrorMessage(QWidget* parent, std::string text);
bool showQuestionMessage(QWidget* parent, std::string text);
