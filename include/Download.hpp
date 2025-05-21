#pragma once

#include <CURL/curl.h>

#include <QProgressDialog>
#include <string>

bool downloadModelIfNotExists(const std::string& modelName);
bool downloadModel(const std::string& modelName);

bool downloadFileIfNotExists(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog);
bool downloadFile(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog);

std::string getLatestGitHubTag();
int progressCallbackForContinuous(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded);
int progressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded);
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp);
