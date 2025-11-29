#pragma once

#include <QString>

// Initialize the application database located in the app files folder.
// Returns true on success, false on error.
bool initDatabase();

// Return the path to the database file (QString)
QString databasePath();
