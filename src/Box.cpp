#include "Box.hpp"

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
                                  QMessageBox::No | QMessageBox::Yes);
    return (reply == QMessageBox::Yes);
}

std::map<std::string, std::string> showOptionsDialog(QWidget* parent, const std::string windowName, const std::map<std::string, Option>& options) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QString::fromStdString(windowName));
    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    std::map<std::string, QWidget*> widgets;

    for (const auto& [key, option] : options) {
        QHBoxLayout* rowLayout = new QHBoxLayout();
        QLabel* label = new QLabel(QString::fromStdString(key));
        rowLayout->addWidget(label);

        if (option.type == "bool") {
            QCheckBox* checkBox = new QCheckBox();
            checkBox->setChecked(option.value == "true");
            rowLayout->addWidget(checkBox);
            widgets[key] = checkBox;
        } else if (option.type == "text") {
            QLineEdit* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.value));
            rowLayout->addWidget(lineEdit);
            widgets[key] = lineEdit;
        } else if (option.type == "file") {
            QLineEdit* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.value));
            QPushButton* browseButton = new QPushButton("Browse");
            rowLayout->addWidget(lineEdit);
            rowLayout->addWidget(browseButton);
            widgets[key] = lineEdit;

            QObject::connect(browseButton, &QPushButton::clicked, [lineEdit, parent]() {
                QString filePath = QFileDialog::getOpenFileName(parent, "Select File");
                if (!filePath.isEmpty()) {
                    lineEdit->setText(filePath);
                }
            });
        } else if (option.type == "directory") {
            QLineEdit* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.value));
            QPushButton* browseButton = new QPushButton("Browse");
            rowLayout->addWidget(lineEdit);
            rowLayout->addWidget(browseButton);
            widgets[key] = lineEdit;

            QObject::connect(browseButton, &QPushButton::clicked, [lineEdit, parent]() {
                QString dirPath = QFileDialog::getExistingDirectory(parent, "Select Directory");
                if (!dirPath.isEmpty()) {
                    lineEdit->setText(dirPath);
                }
            });
        }
        layout->addLayout(rowLayout);
    }

    QHBoxLayout* buttonsLayout = new QHBoxLayout();
    QPushButton* okButton = new QPushButton("OK");
    QPushButton* cancelButton = new QPushButton("Cancel");
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(cancelButton);
    layout->addLayout(buttonsLayout);

    QObject::connect(okButton, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(cancelButton, &QPushButton::clicked, &dialog, &QDialog::reject);

    std::map<std::string, std::string> results;

    if (dialog.exec() == QDialog::Accepted) {
        for (const auto& [key, widget] : widgets) {
            if (QCheckBox* checkBox = qobject_cast<QCheckBox*>(widget)) {
                results[key] = checkBox->isChecked() ? "true" : "false";
            } else if (QLineEdit* lineEdit = qobject_cast<QLineEdit*>(widget)) {
                results[key] = lineEdit->text().toStdString();
            }
        }
    }

    return results;
}
