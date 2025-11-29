#include <gtest/gtest.h>
#include <sqlite3.h>

#include <QFile>

#include "Const.hpp"
#include "Database.hpp"

TEST(DatabaseTest, InitCreatesDbAndTable) {
    // Ensure a clean state
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_FALSE(QFile::exists(QString::fromStdString(DB_PATH)));

    // Initialize DB
    EXPECT_TRUE(initDatabase());
    ASSERT_TRUE(QFile::exists(QString::fromStdString(DB_PATH)));

    // Open DB and check table exists
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type='table' AND name='app_meta';", -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    rc = sqlite3_step(stmt);
    EXPECT_EQ(rc, SQLITE_ROW);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    // Cleanup
    QFile::remove(QString::fromStdString(DB_PATH));
}

TEST(DatabaseTest, InsertAndSelect) {
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_TRUE(initDatabase());

    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);

    char* err = nullptr;
    rc = sqlite3_exec(db, "INSERT OR REPLACE INTO app_meta (key, value) VALUES ('unit_test_key','unit_test_value');", nullptr, nullptr, &err);
    if (rc != SQLITE_OK) {
        if (err) sqlite3_free(err);
    }
    ASSERT_EQ(rc, SQLITE_OK);

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT value FROM app_meta WHERE key='unit_test_key';", -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    rc = sqlite3_step(stmt);
    ASSERT_EQ(rc, SQLITE_ROW);
    const unsigned char* text = sqlite3_column_text(stmt, 0);
    ASSERT_NE(text, nullptr);
    std::string val(reinterpret_cast<const char*>(text));
    EXPECT_EQ(val, "unit_test_value");

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    QFile::remove(QString::fromStdString(DB_PATH));
}
