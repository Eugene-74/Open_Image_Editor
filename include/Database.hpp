#pragma once

#include <QString>

// Initialize the application database located in the app files folder.
// Returns true on success, false on error.
bool initDatabase();

// Return the path to the database file (QString)
QString databasePath();

// Execute SQL commands contained in the file at `sqlFilePath` against
// the application database. Returns true on success, false on error.
bool executeSqlFile(const QString& sqlFilePath);

// app_meta (key/value) helpers
// Store a string value for a key. Returns true on success.
bool setAppMeta(const QString& key, const QString& value);

// Retrieve a string value for a key. If the key is not present, returns `defaultValue`.
QString getAppMeta(const QString& key, const QString& defaultValue = QString());

// Remove a key from app_meta. Returns true on success (or if the key didn't exist).
bool removeAppMeta(const QString& key);

// Insert a new ImageData row for `imagePath` and return the generated Id via `outId`.
// Returns true on success, false on error.
bool addImageData(const QString& imagePath, int& outId);

// Remove an ImageData row (and its MetaData) by Id. Returns true on success.
bool removeImageData(int id);

// Update metadata for an existing image row (by Id).
// Currently supports updating the Orientation field stored in the MetaData table.
// Returns true on success.
bool setImageOrientation(int id, int orientation);

// Get orientation for an image by Id. Returns true on success and fills outOrientation.
bool getImageOrientation(int id, int& outOrientation);

bool setImageLatLon(int id, double latitude, double longitude);

bool getImageLatLon(int id, double& outLatitude, double& outLongitude);

bool setImageTimestamp(int id, long timestamp);

bool getImageTimestamp(int id, long& outTimestamp);
