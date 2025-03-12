#include "Box.hpp"

#include <QCheckBox>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QWidget>
#include <string>

/**
 * @brief
 * @param parent
 * @param icon
 * @param text
 * @param title
 * @param posX
 * @param posY
 * @param async
 */
void showModalDialog(QWidget* parent, QMessageBox::Icon icon, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    auto* msgBox = new QMessageBox(parent);
    msgBox->setIcon(icon);
    msgBox->setText(QString::fromStdString(text));
    msgBox->setWindowTitle(QString::fromStdString(title));
    msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);

    if (posX != -1 && posY != -1) {
        msgBox->move(posX, posY);
    }

    if (async) {
        msgBox->setWindowModality(Qt::NonModal);
        msgBox->show();
    } else {
        msgBox->setWindowModality(Qt::ApplicationModal);
        msgBox->exec();
    }
}

/**
 * @brief
 * @param parent
 * @param text
 * @param title
 * @param posX
 * @param posY
 * @param async
 */
void showInformationMessage(QWidget* parent, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    showModalDialog(parent, QMessageBox::Information, text, title, posX, posY, async);
}

/**
 * @brief
 * @param parent
 * @param text
 * @param title
 * @param posX
 * @param posY
 * @param async
 */
void showWarningMessage(QWidget* parent, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    showModalDialog(parent, QMessageBox::Warning, text, title, posX, posY, async);
}

/**
 * @brief
 * @param parent
 * @param text
 * @param title
 * @param posX
 * @param posY
 * @param async
 */
void showErrorMessage(QWidget* parent, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    showModalDialog(parent, QMessageBox::Critical, text, title, posX, posY, async);
}

/**
 * @brief
 * @param parent
 * @param text
 * @param callback
 * @param title
 * @param posX
 * @param posY
 * @param async
 */
void showQuestionMessage(QWidget* parent, const std::string& text, std::function<void(bool)> callback, const std::string& title, int posX, int posY, bool async) {
    auto* msgBox = new QMessageBox(parent);
    msgBox->setWindowModality(Qt::ApplicationModal);
    msgBox->setIcon(QMessageBox::Question);
    msgBox->setText(QString::fromStdString(text));
    msgBox->setWindowTitle(QString::fromStdString(title));
    msgBox->setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox->setWindowFlags(msgBox->windowFlags() | Qt::WindowStaysOnTopHint);

    if (posX != -1 && posY != -1) {
        msgBox->move(posX, posY);
    }

    QObject::connect(msgBox, &QMessageBox::buttonClicked, [msgBox, callback](QAbstractButton* button) {
        bool result = (msgBox->buttonRole(button) == QMessageBox::YesRole);
        msgBox->deleteLater();
        callback(result);
    });

    if (async) {
        msgBox->setWindowModality(Qt::NonModal);
        msgBox->show();
    } else {
        msgBox->setWindowModality(Qt::ApplicationModal);
        msgBox->exec();
    }
}

/**
 * @brief
 * @param parent
 * @param windowName
 * @param options
 * @return
 */
std::map<std::string, std::string> showOptionsDialog(QWidget* parent, const std::string& windowName, const std::map<std::string, Option>& options) {
    QDialog dialog(parent);
    dialog.setWindowTitle(QString::fromStdString(windowName));
    auto* layout = new QVBoxLayout(&dialog);
    std::map<std::string, QWidget*> widgets;

    for (const auto& [key, option] : options) {
        auto* rowLayout = new QHBoxLayout();
        auto* label = new QLabel(QString::fromStdString(key));
        rowLayout->addWidget(label);

        if (option.type == "bool") {
            auto* checkBox = new QCheckBox();
            checkBox->setChecked(option.value == "true");
            rowLayout->addWidget(checkBox);
            widgets[key] = checkBox;
        } else if (option.type == "text") {
            auto* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.value));
            rowLayout->addWidget(lineEdit);
            widgets[key] = lineEdit;
        } else if (option.type == "file") {
            auto* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.value));
            auto* browseButton = new QPushButton("Browse");
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
            auto* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.value));
            auto* browseButton = new QPushButton("Browse");
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

    auto* buttonsLayout = new QHBoxLayout();
    auto* okButton = new QPushButton("OK");
    auto* cancelButton = new QPushButton("Cancel");
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
