#include <gtest/gtest.h>
#include <sqlite3.h>

#include <QDir>
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

TEST(DatabaseTest, ExecuteSqlFileCreatesTable) {
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_TRUE(initDatabase());

    // create temporary sql file
    QString tmpPath = QDir::tempPath() + "/test_exec_sql.sql";
    QFile f(tmpPath);
    if (f.exists()) f.remove();
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    QByteArray sql = "CREATE TABLE IF NOT EXISTS tmp_test_table (id INTEGER PRIMARY KEY, name TEXT);";
    f.write(sql);
    f.close();

    EXPECT_TRUE(executeSqlFile(tmpPath));

    // verify table exists
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT name FROM sqlite_master WHERE type='table' AND name='tmp_test_table';", -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    rc = sqlite3_step(stmt);
    EXPECT_EQ(rc, SQLITE_ROW);
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    QFile::remove(tmpPath);
    QFile::remove(QString::fromStdString(DB_PATH));
}

TEST(DatabaseTest, AppMetaCRUD) {
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_TRUE(initDatabase());

    EXPECT_TRUE(setAppMeta("t_key", "t_value"));
    QString val = getAppMeta("t_key", "default");
    EXPECT_EQ(val.toStdString(), "t_value");

    EXPECT_TRUE(removeAppMeta("t_key"));
    QString def = getAppMeta("t_key", "my_default");
    EXPECT_EQ(def.toStdString(), "my_default");

    QFile::remove(QString::fromStdString(DB_PATH));
}

TEST(DatabaseTest, AddAndRemoveImageData) {
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_TRUE(initDatabase());

    // ensure ImagesData table exists
    QString createSql =
        "CREATE TABLE IF NOT EXISTS ImagesData (Id INTEGER PRIMARY KEY AUTOINCREMENT, ImagePath TEXT);"
        "CREATE TABLE IF NOT EXISTS MetaData (Id INTEGER PRIMARY KEY, CoordId INTEGER, Orientation INTEGER, \"Date\" INTEGER);"
        "CREATE TABLE IF NOT EXISTS Coords (Id INTEGER PRIMARY KEY AUTOINCREMENT, latitude REAL, longitude REAL);";
    // use executeSqlFile by writing a temp file
    QString tmp = QDir::tempPath() + "/create_images_tables.sql";
    QFile f(tmp);
    if (f.exists()) f.remove();
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(createSql.toUtf8());
    f.close();
    ASSERT_TRUE(executeSqlFile(tmp));

    int id = -1;
    EXPECT_TRUE(addImageData("/tmp/img.jpg", id));
    EXPECT_GT(id, 0);

    // verify row exists
    sqlite3* db = nullptr;
    int rc = sqlite3_open(DB_PATH.c_str(), &db);
    ASSERT_EQ(rc, SQLITE_OK);
    sqlite3_stmt* stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT ImagePath FROM ImagesData WHERE Id = ?;", -1, &stmt, nullptr);
    ASSERT_EQ(rc, SQLITE_OK);
    sqlite3_bind_int(stmt, 1, id);
    rc = sqlite3_step(stmt);
    ASSERT_EQ(rc, SQLITE_ROW);
    const unsigned char* txt = sqlite3_column_text(stmt, 0);
    ASSERT_NE(txt, nullptr);
    std::string path(reinterpret_cast<const char*>(txt));
    EXPECT_EQ(path, "/tmp/img.jpg");
    sqlite3_finalize(stmt);
    sqlite3_close(db);

    EXPECT_TRUE(removeImageData(id));

    QFile::remove(tmp);
    QFile::remove(QString::fromStdString(DB_PATH));
}

TEST(DatabaseTest, OrientationLatLonTimestamp) {
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_TRUE(initDatabase());

    // create tables
    QString createSql =
        "CREATE TABLE IF NOT EXISTS ImagesData (Id INTEGER PRIMARY KEY AUTOINCREMENT, ImagePath TEXT);"
        "CREATE TABLE IF NOT EXISTS MetaData (Id INTEGER PRIMARY KEY, CoordId INTEGER, Orientation INTEGER, \"Date\" INTEGER);"
        "CREATE TABLE IF NOT EXISTS Coords (Id INTEGER PRIMARY KEY AUTOINCREMENT, latitude REAL, longitude REAL);";
    QString tmp = QDir::tempPath() + "/create_meta_tables.sql";
    QFile f(tmp);
    if (f.exists()) f.remove();
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(createSql.toUtf8());
    f.close();
    ASSERT_TRUE(executeSqlFile(tmp));

    int id = 9999;  // arbitrary id for MetaData row operations

    // Orientation
    EXPECT_TRUE(setImageOrientation(id, 6));
    int outOri = 0;
    EXPECT_TRUE(getImageOrientation(id, outOri));
    EXPECT_EQ(outOri, 6);

    // Timestamp: set then get
    long ts = 1600000000L;
    EXPECT_TRUE(setImageTimestamp(id, ts));
    long outTs = 0;
    EXPECT_TRUE(getImageTimestamp(id, outTs));
    EXPECT_EQ(outTs, ts);

    // Clear timestamp
    EXPECT_TRUE(setImageTimestamp(id, 0));
    EXPECT_TRUE(getImageTimestamp(id, outTs));
    EXPECT_EQ(outTs, 0);

    // Lat/Lon: set specific coords
    double lat = 12.345678;
    double lon = 98.7654321;
    EXPECT_TRUE(setImageLatLon(id, lat, lon));
    double outLat = 0.0, outLon = 0.0;
    EXPECT_TRUE(getImageLatLon(id, outLat, outLon));
    EXPECT_DOUBLE_EQ(outLat, lat);
    EXPECT_DOUBLE_EQ(outLon, lon);

    // Set to 0,0 should remove coord association
    EXPECT_TRUE(setImageLatLon(id, 0.0, 0.0));
    EXPECT_FALSE(getImageLatLon(id, outLat, outLon));

    QFile::remove(tmp);
    QFile::remove(QString::fromStdString(DB_PATH));
}
TEST(DatabaseTest, GetImageIdByPath) {
    QFile::remove(QString::fromStdString(DB_PATH));
    ASSERT_TRUE(initDatabase());

    // create ImagesData table
    QString createSql =
        "CREATE TABLE IF NOT EXISTS ImagesData (Id INTEGER PRIMARY KEY AUTOINCREMENT, ImagePath TEXT);";
    QString tmp = QDir::tempPath() + "/create_images_table_for_getid.sql";
    QFile f(tmp);
    if (f.exists()) f.remove();
    ASSERT_TRUE(f.open(QIODevice::WriteOnly | QIODevice::Text));
    f.write(createSql.toUtf8());
    f.close();
    ASSERT_TRUE(executeSqlFile(tmp));

    // When no row exists, getImageIdByPath should indicate not found
    int outId = -42;
    EXPECT_FALSE(getImageIdByPath("/no/such/path.jpg", outId));

    // Insert an image and verify getImageIdByPath finds it
    int insertedId = -1;
    EXPECT_TRUE(addImageData("/tmp/get_image_id.jpg", insertedId));
    EXPECT_GT(insertedId, 0);

    int foundId = -1;
    EXPECT_TRUE(getImageIdByPath("/tmp/get_image_id.jpg", foundId));
    EXPECT_EQ(foundId, insertedId);

    // Removing the image should make getImageIdByPath return false again
    EXPECT_TRUE(removeImageData(insertedId));
    int afterRemoveId = -1;
    EXPECT_FALSE(getImageIdByPath("/tmp/get_image_id.jpg", afterRemoveId));

    QFile::remove(tmp);
    QFile::remove(QString::fromStdString(DB_PATH));
}
