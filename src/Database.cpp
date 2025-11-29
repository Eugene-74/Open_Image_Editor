#include "Database.hpp"

#include <sqlite3.h>

#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QIODevice>
#include <ctime>

#include "AppConfig.hpp"
#include "Const.hpp"

/**
 * @brief databasePath
 * @return Path to the database file
 */
QString databasePath() {
    return QString::fromStdString(DB_PATH);
}

/**
 * @brief initDatabase
 * @return True on success, false on error.
 */
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

    // initialise tables if needed (from bundled SQL)
    executeSqlFile(QString::fromStdString(INIT_SQL_FILE));

    // verify app version saved in DB matches current application version
    {
        const char* select_sql = "SELECT value FROM app_meta WHERE key = 'app_version' LIMIT 1;";
        sqlite3_stmt* sel = nullptr;
        rc = sqlite3_prepare_v2(db, select_sql, -1, &sel, nullptr);
        QString storedAppVersion;
        if (rc == SQLITE_OK && sel) {
            rc = sqlite3_step(sel);
            if (rc == SQLITE_ROW) {
                const unsigned char* txt = sqlite3_column_text(sel, 0);
                if (txt) storedAppVersion = QString::fromUtf8(reinterpret_cast<const char*>(txt));
            }
            sqlite3_finalize(sel);
        } else {
            if (sel) sqlite3_finalize(sel);
        }

        const QString currentAppVersion = QString::fromUtf8(APP_VERSION);
        if (storedAppVersion != currentAppVersion) {
            qInfo() << "App version changed (DB:" << storedAppVersion << "-> App:" << currentAppVersion << "). Updating app_meta.";
            const char* upsert_sql = "INSERT OR REPLACE INTO app_meta (key, value) VALUES ('app_version', ?);";
            sqlite3_stmt* up = nullptr;
            rc = sqlite3_prepare_v2(db, upsert_sql, -1, &up, nullptr);
            if (rc == SQLITE_OK && up) {
                sqlite3_bind_text(up, 1, currentAppVersion.toUtf8().constData(), -1, SQLITE_TRANSIENT);
                rc = sqlite3_step(up);
                if (rc != SQLITE_DONE) {
                    qWarning() << "Failed to update app_version in app_meta:" << sqlite3_errmsg(db);
                }
                sqlite3_finalize(up);
            } else {
                qWarning() << "Failed to prepare statement to update app_version:" << sqlite3_errmsg(db);
                if (up) sqlite3_finalize(up);
            }
            // Here you could trigger migrations if needed based on storedAppVersion
        }
    }

    sqlite3_close(db);
    qInfo() << "Database initialized at" << dbPath;
    return true;
}

/**
 * @brief executeSqlFile
 * @param sqlFilePath Path to the SQL file
 * @return True on success, false on error.
 */
bool executeSqlFile(const QString& sqlFilePath) {
    QFile file(sqlFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open SQL file:" << sqlFilePath;
        return false;
    }

    QByteArray data = file.readAll();
    QString sql = QString::fromUtf8(data);
    file.close();

    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    QByteArray utf8 = sql.toUtf8();
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, utf8.constData(), nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "SQL error while executing file" << sqlFilePath << ":" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    qInfo() << "Executed SQL file:" << sqlFilePath;
    return true;
}

/**
 * @brief setAppMeta
 * @param key Key to set
 * @param value Value to set
 * @return True on success, false on error.
 */
bool setAppMeta(const QString& key, const QString& value) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "INSERT OR REPLACE INTO app_meta (key, value) VALUES (?, ?);";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_text(stmt, 2, value.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to execute statement:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

/**
 * @brief getAppMeta
 * @param key Key to retrieve
 * @param defaultValue Default value if key not found
 * @return Value associated with the key or defaultValue if not found
 */
QString getAppMeta(const QString& key, const QString& defaultValue) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return defaultValue;
    }

    const char* sql = "SELECT value FROM app_meta WHERE key = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare select statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return defaultValue;
    }

    sqlite3_bind_text(stmt, 1, key.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    QString result = defaultValue;
    if (rc == SQLITE_ROW) {
        const unsigned char* txt = sqlite3_column_text(stmt, 0);
        if (txt) result = QString::fromUtf8(reinterpret_cast<const char*>(txt));
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

/**
 * @brief removeAppMeta
 * @param key Key to remove
 * @return True on success, false on error.
 */
bool removeAppMeta(const QString& key) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "DELETE FROM app_meta WHERE key = ?;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare delete statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, key.toUtf8().constData(), -1, SQLITE_TRANSIENT);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to delete app_meta key:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return true;
}

/**
 * @brief addImageData
 * @param imagePath Path to the image
 * @param outId Output parameter for the new image ID
 * @return True on success, false on error.
 */
bool addImageData(const QString& imagePath, int& outId) {
    outId = -1;
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "INSERT INTO ImagesData (ImagePath) VALUES (?);";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare insert ImagesData statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_text(stmt, 1, imagePath.toUtf8().constData(), -1, SQLITE_TRANSIENT);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to insert ImagesData:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_finalize(stmt);

    sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
    if (rowid <= 0) {
        // In case table uses explicit Id and not rowid, try to get max(Id)
        const char* sel = "SELECT MAX(Id) FROM ImagesData;";
        sqlite3_stmt* s2 = nullptr;
        rc = sqlite3_prepare_v2(db, sel, -1, &s2, nullptr);
        if (rc == SQLITE_OK && s2) {
            rc = sqlite3_step(s2);
            if (rc == SQLITE_ROW) {
                rowid = sqlite3_column_int64(s2, 0);
            }
            sqlite3_finalize(s2);
        }
    }

    outId = static_cast<int>(rowid);

    sqlite3_close(db);
    return outId > 0;
}

/**
 * @brief removeImageData
 * @param id Image Id
 * @return True on success, false on error.
 */
bool removeImageData(int id) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    // Enable foreign keys so ON DELETE CASCADE works as defined in the schema
    char* errMsg = nullptr;
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to enable foreign_keys:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        // continue, attempt delete anyway
    }

    // Deleting from ImagesData will cascade and remove MetaData, Coords and DATES
    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to begin transaction:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    const char* del_img_sql = "DELETE FROM ImagesData WHERE Id = ?;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, del_img_sql, -1, &stmt, nullptr);
    if (rc == SQLITE_OK && stmt) {
        sqlite3_bind_int(stmt, 1, id);
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            qWarning() << "Failed to delete ImagesData for Id" << id << ":" << sqlite3_errmsg(db);
            sqlite3_finalize(stmt);
            sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
            if (errMsg) sqlite3_free(errMsg);
            sqlite3_close(db);
            return false;
        }
        sqlite3_finalize(stmt);
    } else {
        qWarning() << "Failed to prepare ImagesData delete statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to commit transaction:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    qInfo() << "Removed ImageData with Id" << id;
    return true;
}

/**
 * @brief setImageOrientation
 * @param id Image Id
 * @param orientation Orientation to set
 * @return True on success, false on error.
 */
bool setImageOrientation(int id, int orientation) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* update_sql = "UPDATE MetaData SET Orientation = ? WHERE Id = ?;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare update Orientation statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, orientation);
    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to execute update Orientation statement:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changes == 0) {
        // No existing row updated, try to insert a new MetaData row for this Id
        const char* insert_sql = "INSERT INTO MetaData (Id, Orientation) VALUES (?, ?);";
        sqlite3_stmt* ins = nullptr;
        rc = sqlite3_prepare_v2(db, insert_sql, -1, &ins, nullptr);
        if (rc != SQLITE_OK) {
            qWarning() << "Failed to prepare insert Orientation statement:" << sqlite3_errmsg(db);
            if (ins) sqlite3_finalize(ins);
            sqlite3_close(db);
            return false;
        }

        sqlite3_bind_int(ins, 1, id);
        sqlite3_bind_int(ins, 2, orientation);

        rc = sqlite3_step(ins);
        if (rc != SQLITE_DONE) {
            qWarning() << "Failed to insert MetaData Orientation:" << sqlite3_errmsg(db);
            sqlite3_finalize(ins);
            sqlite3_close(db);
            return false;
        }
        sqlite3_finalize(ins);
    }

    sqlite3_close(db);
    return true;
}

/**
 * @brief getImageOrientation
 * @param id Image Id
 * @param outOrientation Output orientation
 * @return True on success, false on error.
 */
/**
 * @brief
 * @param id
 * @param outOrientation
 * @return
 */
bool getImageOrientation(int id, int& outOrientation) {
    outOrientation = 0;
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "SELECT Orientation FROM MetaData WHERE Id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare select Orientation statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        outOrientation = sqlite3_column_int(stmt, 0);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}

/**
 * @brief setImageLatLon
 * @param id Image Id
 * @param latitude Latitude to set
 * @param longitude Longitude to set
 * @return True on success, false on error.
 */
bool setImageLatLon(int id, double latitude, double longitude) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    char* errMsg = nullptr;
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to enable foreign_keys:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        // continue anyway
    }

    rc = sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to begin transaction:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    bool ok = true;
    int coordsId = -1;
    sqlite3_stmt* stmt = nullptr;

    // 1) Try to find existing Coords with same latitude and longitude
    const char* find_coords_sql = "SELECT Id FROM Coords WHERE latitude = ? AND longitude = ? LIMIT 1;";
    rc = sqlite3_prepare_v2(db, find_coords_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare select Coords by lat/lon statement:" << sqlite3_errmsg(db);
        ok = false;
    } else {
        sqlite3_bind_double(stmt, 1, latitude);
        sqlite3_bind_double(stmt, 2, longitude);
        rc = sqlite3_step(stmt);
        if (rc == SQLITE_ROW) {
            coordsId = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        stmt = nullptr;
    }

    // 2) If not found, insert new Coords row
    if (ok && coordsId < 0) {
        const char* ins_coords_sql = "INSERT INTO Coords (latitude, longitude) VALUES (?, ?);";
        rc = sqlite3_prepare_v2(db, ins_coords_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            qWarning() << "Failed to prepare insert Coords statement:" << sqlite3_errmsg(db);
            ok = false;
        } else {
            sqlite3_bind_double(stmt, 1, latitude);
            sqlite3_bind_double(stmt, 2, longitude);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                qWarning() << "Failed to insert Coords:" << sqlite3_errmsg(db);
                ok = false;
            }
            sqlite3_finalize(stmt);
            stmt = nullptr;

            if (ok) {
                sqlite3_int64 rowid = sqlite3_last_insert_rowid(db);
                coordsId = static_cast<int>(rowid);
                if (coordsId <= 0) {
                    qWarning() << "Insert Coords returned invalid id";
                    ok = false;
                }
            }
        }
    }

    // 3) If we have a coordsId, set MetaData.CoordId to it for the image Id
    if (ok && coordsId > 0) {
        const char* upd_meta_sql = "UPDATE MetaData SET CoordId = ? WHERE Id = ?;";
        rc = sqlite3_prepare_v2(db, upd_meta_sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) {
            qWarning() << "Failed to prepare update MetaData CoordId statement:" << sqlite3_errmsg(db);
            ok = false;
        } else {
            sqlite3_bind_int(stmt, 1, coordsId);
            sqlite3_bind_int(stmt, 2, id);
            rc = sqlite3_step(stmt);
            if (rc != SQLITE_DONE) {
                // It's possible no row was updated because MetaData doesn't exist yet.
                // We'll handle insertion below.
            }
            int changes = sqlite3_changes(db);
            sqlite3_finalize(stmt);
            stmt = nullptr;

            if (changes == 0) {
                const char* ins_meta_sql = "INSERT INTO MetaData (Id, CoordId) VALUES (?, ?);";
                rc = sqlite3_prepare_v2(db, ins_meta_sql, -1, &stmt, nullptr);
                if (rc != SQLITE_OK) {
                    qWarning() << "Failed to prepare insert MetaData statement:" << sqlite3_errmsg(db);
                    ok = false;
                } else {
                    sqlite3_bind_int(stmt, 1, id);
                    sqlite3_bind_int(stmt, 2, coordsId);
                    rc = sqlite3_step(stmt);
                    if (rc != SQLITE_DONE) {
                        qWarning() << "Failed to insert MetaData CoordId:" << sqlite3_errmsg(db);
                        ok = false;
                    }
                    sqlite3_finalize(stmt);
                    stmt = nullptr;
                }
            }
        }
    }

    if (!ok) {
        rc = sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK) {
            qWarning() << "Failed to rollback transaction:" << (errMsg ? errMsg : "(null)");
            if (errMsg) sqlite3_free(errMsg);
        }
        sqlite3_close(db);
        return false;
    }

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &errMsg);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to commit transaction:" << (errMsg ? errMsg : "(null)");
        if (errMsg) sqlite3_free(errMsg);
        sqlite3_close(db);
        return false;
    }

    sqlite3_close(db);
    return true;
}

/**
 * @brief getImageLatLon
 * @param id Image Id
 * @param outLatitude Output latitude
 * @param outLongitude Output longitude
 * @return True on success, false on error.
 */
bool getImageLatLon(int id, double& outLatitude, double& outLongitude) {
    outLatitude = 0.0;
    outLongitude = 0.0;

    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql =
        "SELECT C.latitude, C.longitude "
        "FROM MetaData M "
        "JOIN Coords C ON M.CoordId = C.Id "
        "WHERE M.Id = ? LIMIT 1;";

    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare select lat/lon statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        outLatitude = sqlite3_column_double(stmt, 0);
        outLongitude = sqlite3_column_double(stmt, 1);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    }

    // No row found
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}

/**
 * @brief setImageTimestamp
 * @param id Image Id
 * @param timestamp Timestamp to set (seconds since epoch). 0 to clear.
 * @return True on success, false on error.
 */
bool setImageTimestamp(int id, long timestamp) {
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* update_sql = "UPDATE MetaData SET \"Date\" = ? WHERE Id = ?;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, update_sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare update Date statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    if (timestamp == 0) {
        sqlite3_bind_null(stmt, 1);
    } else {
        sqlite3_bind_int64(stmt, 1, static_cast<sqlite3_int64>(timestamp));
    }
    sqlite3_bind_int(stmt, 2, id);

    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        qWarning() << "Failed to execute update Date statement:" << sqlite3_errmsg(db);
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    int changes = sqlite3_changes(db);
    sqlite3_finalize(stmt);

    if (changes == 0) {
        const char* insert_sql = "INSERT INTO MetaData (Id, \"Date\") VALUES (?, ?);";
        sqlite3_stmt* ins = nullptr;
        rc = sqlite3_prepare_v2(db, insert_sql, -1, &ins, nullptr);
        if (rc != SQLITE_OK) {
            qWarning() << "Failed to prepare insert Date statement:" << sqlite3_errmsg(db);
            if (ins) sqlite3_finalize(ins);
            sqlite3_close(db);
            return false;
        }

        sqlite3_bind_int(ins, 1, id);
        if (timestamp == 0) {
            sqlite3_bind_null(ins, 2);
        } else {
            sqlite3_bind_int64(ins, 2, static_cast<sqlite3_int64>(timestamp));
        }

        rc = sqlite3_step(ins);
        if (rc != SQLITE_DONE) {
            qWarning() << "Failed to insert MetaData Date:" << sqlite3_errmsg(db);
            sqlite3_finalize(ins);
            sqlite3_close(db);
            return false;
        }
        sqlite3_finalize(ins);
    }

    sqlite3_close(db);
    return true;
}

/**
 * @brief getImageTimestamp
 * @param id Image Id
 * @param outTimestamp Output timestamp (seconds since epoch). 0 if not set.
 * @return True on success, false on error.
 */
bool getImageTimestamp(int id, long& outTimestamp) {
    outTimestamp = 0;
    QString dbPath = databasePath();
    sqlite3* db = nullptr;
    int rc = sqlite3_open(dbPath.toStdString().c_str(), &db);
    if (rc != SQLITE_OK) {
        qWarning() << "Can't open database:" << sqlite3_errmsg(db);
        if (db) sqlite3_close(db);
        return false;
    }

    const char* sql = "SELECT \"Date\" FROM MetaData WHERE Id = ? LIMIT 1;";
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        qWarning() << "Failed to prepare select Date statement:" << sqlite3_errmsg(db);
        if (stmt) sqlite3_finalize(stmt);
        sqlite3_close(db);
        return false;
    }

    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        if (sqlite3_column_type(stmt, 0) == SQLITE_NULL) {
            outTimestamp = 0;
        } else {
            outTimestamp = static_cast<long>(sqlite3_column_int64(stmt, 0));
        }
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return true;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return false;
}
