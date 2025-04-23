#include "Box.hpp"

#include <QCheckBox>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <cstdlib>
#include <ctime>
#include <string>

/**
 * @brief Create a modal dialog with the given parameters
 * @param parent Parent widget
 * @param icon Qt icon type (ex : QMessageBox::Information)
 * @param text Text to display in the dialog
 * @param title Title of the dialog
 * @param posX X position of the dialog (-1 to center)
 * @param posY Y position of the dialog (-1 to center)
 * @param async If true, the dialog will be non-modal and will not block the parent widget
 * @details The dialog will be deleted automatically when closed
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
 * @brief Create a modal information dialog with the given parameters
 * @param parent Parent widget
 * @param text Text to display in the dialog
 * @param title Title of the dialog
 * @param posX X position of the dialog (-1 to center)
 * @param posY Y position of the dialog (-1 to center)
 * @param async If true, the dialog will be non-modal and will not block the parent widget
 * @details The dialog will be deleted automatically when closed
 */
void showInformationMessage(QWidget* parent, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    showModalDialog(parent, QMessageBox::Information, text, title, posX, posY, async);
}

/**
 * @brief Create a modal warning dialog with the given parameters
 * @param parent Parent widget
 * @param text Text to display in the dialog
 * @param title Title of the dialog
 * @param posX X position of the dialog (-1 to center)
 * @param posY Y position of the dialog (-1 to center)
 * @param async If true, the dialog will be non-modal and will not block the parent widget
 * @details The dialog will be deleted automatically when closed
 */
void showWarningMessage(QWidget* parent, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    showModalDialog(parent, QMessageBox::Warning, text, title, posX, posY, async);
}

/**
 * @brief Create a modal error dialog with the given parameters
 * @param parent Parent widget
 * @param text Text to display in the dialog
 * @param title Title of the dialog
 * @param posX X position of the dialog (-1 to center)
 * @param posY Y position of the dialog (-1 to center)
 * @param async If true, the dialog will be non-modal and will not block the parent widget
 * @details The dialog will be deleted automatically when closed
 */
void showErrorMessage(QWidget* parent, const std::string& text, const std::string& title, int posX, int posY, bool async) {
    showModalDialog(parent, QMessageBox::Critical, text, title, posX, posY, async);
}

/**
 * @brief Create a modal question dialog with the given parameters
 * @param parent Parent widget
 * @param text Text to display in the dialog
 * @param callback Callback function to call when the user clicks on the buttons
 * @param title Title of the dialog
 * @param posX X position of the dialog (-1 to center)
 * @param posY Y position of the dialog (-1 to center)
 * @param async If true, the dialog will be non-modal and will not block the parent widget
 * @details The dialog will be deleted automatically when closed
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
 * @brief Show a dialog to select options
 * @param parent Parent widget
 * @param windowName Name of the window
 * @param options Map of options to display
 * @details The dialog will be deleted automatically when closed
 * @return Map of selected options
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

/**
 * @brief Open a label pop up to show an error message
 * @param parent Parent of the label
 * @param message Error message to display
 * @param timeout Time before removing the label
 */
void showErrorInfo(QWidget* parent, const QString& message, int timeout) {
    if (timeout < 1000) {
        timeout = 1000;
    }
    QLabel* errorLabel = new QLabel(message, parent);
    errorLabel->setStyleSheet(
        "QLabel { "
        "background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, "
        "stop: 0 red, stop: 1 darkred); "
        "color : white; "
        "padding: 10px; "
        "border-radius: 10px; "
        "border: 2px solid transparent; "
        "}");
    errorLabel->setAlignment(Qt::AlignCenter);
    errorLabel->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    errorLabel->setAttribute(Qt::WA_DeleteOnClose);

    errorLabel->setGeometry(parent->width() - errorLabel->sizeHint().width() - 10,
                            10,
                            errorLabel->sizeHint().width(),
                            errorLabel->sizeHint().height());

    QGraphicsOpacityEffect* opacityEffect = new QGraphicsOpacityEffect(errorLabel);
    errorLabel->setGraphicsEffect(opacityEffect);

    QPropertyAnimation* fadeInAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeInAnimation->setDuration(500);  // 500 ms for fade-in
    fadeInAnimation->setStartValue(0.0);
    fadeInAnimation->setEndValue(1.0);

    QPropertyAnimation* fadeOutAnimation = new QPropertyAnimation(opacityEffect, "opacity");
    fadeOutAnimation->setDuration(500);  // 500 ms for fade-out
    fadeOutAnimation->setStartValue(1.0);
    fadeOutAnimation->setEndValue(0.0);

    fadeInAnimation->start(QAbstractAnimation::DeleteWhenStopped);

    QTimer* gradientTimer = new QTimer(errorLabel);

    QObject::connect(gradientTimer, &QTimer::timeout, [errorLabel]() {
        static int step = 0;
        static bool forward = true;
        double position = (step % 100) / 100.0;
        errorLabel->setStyleSheet(
            QString("QLabel { "
                    "background: qlineargradient(x1: %1, y1: 0, x2: %2, y2: 0, "
                    "stop: 0 red, stop: 1 darkred); "
                    "color : white; "
                    "padding: 10px; "
                    "border-radius: 10px; "
                    "border: 2px solid transparent; "
                    "}")
                .arg(position)
                .arg(position + 1.0));
        step++;
    });
    gradientTimer->start((timeout - 1000) / 100);

    QTimer::singleShot(timeout - 500, [fadeOutAnimation]() {
        fadeOutAnimation->start(QAbstractAnimation::DeleteWhenStopped);
    });
    QObject::connect(fadeOutAnimation, &QPropertyAnimation::finished, errorLabel, &QLabel::close);

    errorLabel->show();
}