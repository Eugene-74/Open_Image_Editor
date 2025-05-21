#include "Box.hpp"

#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPropertyAnimation>
#include <QPushButton>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>
#include <QWindow>
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

        if (option.getTypeConst() == "bool") {
            auto* checkBox = new QCheckBox();
            checkBox->setChecked(option.getValueConst() == "true");
            rowLayout->addWidget(checkBox);
            widgets[key] = checkBox;
        } else if (option.getTypeConst() == "text") {
            auto* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.getValueConst()));
            rowLayout->addWidget(lineEdit);
            widgets[key] = lineEdit;
        } else if (option.getTypeConst() == "file") {
            auto* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.getValueConst()));
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
        } else if (option.getTypeConst() == "directory") {
            auto* lineEdit = new QLineEdit();
            lineEdit->setText(QString::fromStdString(option.getValueConst()));
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
        } else if (option.getTypeConst() == "list") {
            auto* comboBox = new QComboBox();
            std::string valueStr = option.getValueConst();
            size_t pos = valueStr.find('|');
            std::string currentValue = valueStr.substr(0, pos);
            std::vector<std::string> items;
            if (pos != std::string::npos) {
                std::string rest = valueStr.substr(pos + 1);
                size_t start = 0, end;
                while ((end = rest.find('|', start)) != std::string::npos) {
                    items.push_back(rest.substr(start, end - start));
                    start = end + 1;
                }
                items.push_back(rest.substr(start));
            }
            for (const auto& item : items) {
                comboBox->addItem(QString::fromStdString(item));
            }
            int idx = comboBox->findText(QString::fromStdString(currentValue));
            if (idx >= 0) comboBox->setCurrentIndex(idx);
            rowLayout->addWidget(comboBox);
            widgets[key] = comboBox;
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
            } else if (QComboBox* comboBox = qobject_cast<QComboBox*>(widget)) {
                std::string selected = comboBox->currentText().toStdString();
                std::string valueStr = options.at(key).getValueConst();
                size_t pos = valueStr.find('|');
                std::vector<std::string> items;
                if (pos != std::string::npos) {
                    std::string rest = valueStr.substr(pos + 1);
                    size_t start = 0, end;
                    while ((end = rest.find('|', start)) != std::string::npos) {
                        items.push_back(rest.substr(start, end - start));
                        start = end + 1;
                    }
                    items.push_back(rest.substr(start));
                }
                std::string newValue = selected;
                for (const auto& item : items) {
                    newValue += "|" + item;
                }
                results[key] = newValue;
            }
        }
    }

    return results;
}
