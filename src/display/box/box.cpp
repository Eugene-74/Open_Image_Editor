#include "box.h"


// Function to display an information message box
void showInformationMessage(QWidget* parent, std::string text) {
    QMessageBox::information(parent, "Information", QString::fromStdString(text));
}

// Function to display a warning message box
void showWarningMessage(QWidget* parent, std::string text) {
    QMessageBox::warning(parent, "Warning", QString::fromStdString(text));
}

// Function to display an error message box
void showErrorMessage(QWidget* parent, std::string text) {
    QMessageBox::critical(parent, "Error", QString::fromStdString(text));
}

// Function to display a question message box with Yes/No options
bool showQuestionMessage(QWidget* parent, std::string text) {
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(parent, "Question", QString::fromStdString(text),
        QMessageBox::Yes | QMessageBox::No);
    return (reply == QMessageBox::Yes);
}
