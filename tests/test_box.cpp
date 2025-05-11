#include <gtest/gtest.h>

#include <QApplication>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

#include "Box.hpp"

class BoxTest : public ::testing::Test {
   protected:
    QApplication* app;

    void SetUp() override {
        int argc = 0;
        char* argv[] = {nullptr};
        app = new QApplication(argc, argv);
    }

    void TearDown() override {
        delete app;
    }
};

TEST_F(BoxTest, TestShowModalDialog) {
    QWidget parent;
    QTimer::singleShot(500, [&]() {
        auto* dialog = parent.findChild<QMessageBox*>();
        ASSERT_NE(dialog, nullptr);
        dialog->close();
    });
    showModalDialog(&parent, QMessageBox::Information, "Test Message", "Test Title", -1, -1, false);
}

TEST_F(BoxTest, TestShowInformationMessage) {
    QWidget parent;
    QTimer::singleShot(500, [&]() {
        auto* dialog = parent.findChild<QMessageBox*>();
        ASSERT_NE(dialog, nullptr);
        dialog->close();
    });
    showInformationMessage(&parent, "Info Message", "Info Title", -1, -1, true);
}

TEST_F(BoxTest, TestShowWarningMessage) {
    QWidget parent;
    QTimer::singleShot(500, [&]() {
        auto* dialog = parent.findChild<QMessageBox*>();
        ASSERT_NE(dialog, nullptr);
        dialog->close();
    });
    showWarningMessage(&parent, "Warning Message", "Warning Title", -1, -1, false);
}

TEST_F(BoxTest, TestShowErrorMessage) {
    QWidget parent;
    QTimer::singleShot(500, [&]() {
        auto* dialog = parent.findChild<QMessageBox*>();
        ASSERT_NE(dialog, nullptr);
        dialog->close();
    });
    showErrorMessage(&parent, "Error Message", "Error Title", -1, -1, true);
}

TEST_F(BoxTest, TestShowQuestionMessage) {
    QWidget parent;
    bool callbackResult = false;
    showQuestionMessage(&parent, "Question Message", [&](bool result) { callbackResult = result; }, "Question Title", -1, -1, true);
    QTimer::singleShot(500, [&]() {
        auto* dialog = parent.findChild<QMessageBox*>();
        ASSERT_NE(dialog, nullptr);
        dialog->close();
    });
}

TEST_F(BoxTest, TestShowOptionsDialog) {
    QWidget parent;
    std::map<std::string, Option> options = {
        {"Option1", {"bool", "true"}},
        {"Option2", {"text", "default"}},
        {"Option3", {"file", ""}}};
    QTimer::singleShot(500, [&]() {
        auto* dialog = parent.findChild<QDialog*>();
        ASSERT_NE(dialog, nullptr);
        dialog->accept();
    });
    auto result = showOptionsDialog(&parent, "Options Dialog", options);

    ASSERT_EQ(result["Option1"], "true");
    ASSERT_EQ(result["Option2"], "default");
    ASSERT_EQ(result["Option3"], "");
}
