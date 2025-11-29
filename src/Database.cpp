#include "Database.hpp"

#include <sqlite3.h>

#include <QDebug>
#include <QDir>

#include "Const.hpp"

QString databasePath() {
    return QString::fromStdString(DB_PATH);
}

bool initDatabase() {
    QDir dir(APP_FILES);
    if (!dir.exists()) {
        if (!dir.mkpath(".")) {
            qWarning() << "Failed to create app files directory:" << APP_FILES;
            return false;
        }
    }

    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* create_table_sql = "CREATE TABLE IF NOT EXISTS app_meta (key TEXT PRIMARY KEY, value TEXT);";
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, create_table_sql, nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "SQL error while creating table:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    qInfo() << "Database initialized at" << dbPath;
    return true;
}
