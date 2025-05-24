#include "Download.hpp"

#include <json/json.h>

#include <QApplication>
#include <QPushButton>
#include <filesystem>
#include <fstream>

#include "AppConfig.hpp"
#include "Const.hpp"
#include "Network.hpp"

namespace fs = std::filesystem;

/**
 * @brief Callback function for writing data received from the server and saving it to a file
 * @param contents Pointer to the data received from the server
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param userp Pointer to the user-defined data (in this case, an ofstream object)
 * @return The size of the data written
 */
size_t WriteCallbackAndSave(void* contents, size_t size, size_t nmemb, void* userp) {
    std::ofstream* out = static_cast<std::ofstream*>(userp);
    size_t totalSize = size * nmemb;
    out->write(static_cast<char*>(contents), totalSize);
    return totalSize;
}

/**
 * @brief Callback function for progress updates during file download (shows the progress)
 * @param ptr Pointer to the user-defined data (in this case, a QProgressDialog object)
 * @param totalToDownload Total size of the data to be downloaded
 * @param nowDownloaded Size of the data downloaded so far
 * @param totalToUpload Total size of the data to be uploaded (not used in this case)
 * @param nowUploaded Size of the data uploaded so far (not used in this case)
 * @return 0 to continue, non-zero to abort the operation
 * @details This function is used to update the progress dialog during the download process.
 */
int progressCallback(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QProgressDialog* progressDialog = static_cast<QProgressDialog*>(ptr);
    QApplication::processEvents();
    if (totalToDownload > 0) {
        double progress = (nowDownloaded / (double)totalToDownload) * 100.0;
        progressDialog->setValue((int)progress);
    }
    if (progressDialog->wasCanceled()) {
        return 1;  // Stop the download
    }
    return 0;
}

/**
 * @brief Callback function for progress updates during file download (alternative version (dosn't shows the progress))
 * @param ptr Pointer to the user-defined data (in this case, a QProgressDialog object)
 * @param totalToDownload Total size of the data to be downloaded
 * @param nowDownloaded Size of the data downloaded so far
 * @param totalToUpload Total size of the data to be uploaded (not used in this case)
 * @param nowUploaded Size of the data uploaded so far (not used in this case)
 * @return 0 to continue, non-zero to abort the operation
 * @details This function is used to update the progress dialog during the download process.
 */
int progressCallbackForContinuous(void* ptr, curl_off_t totalToDownload, curl_off_t nowDownloaded, curl_off_t totalToUpload, curl_off_t nowUploaded) {
    QApplication::processEvents();
    return 0;
}

/**
 * @brief Callback function for writing data received from the server
 * @param contents Pointer to the data received from the server
 * @param size Size of each data element
 * @param nmemb Number of data elements
 * @param userp Pointer to the user-defined data (in this case, a string)
 * @return The size of the data written
 */
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    ((std::string*)userp)->append((char*)contents, size * nmemb);
    return size * nmemb;
}

/**
 * @brief Download a model from the specified modelName. But it only download it if the file doesn't already exist.
 * @param modelName The name of the model to download (with extension)
 * @return true if the download was successful, false otherwise
 * @note This function must be called from an other thread than the main thread.
 */
bool downloadModelIfNotExists(const std::string& modelName, std::string tag) {
    qDebug() << "Downloading model: " << QString::fromStdString(modelName);
    if (!hasConnection()) {
        return false;
    }

    const std::string modeloutputPath = APP_FILES.toStdString() + "/" + modelName;
    if (fs::exists(modeloutputPath)) {
        qInfo() << "Model already exists: " << QString::fromStdString(modeloutputPath);
        return true;
    }

    return downloadModel(modelName, tag);
}

/**
 * @brief Download a model from the specified modelName.
 * @param modelName The name of the model to download (with extension)
 * @return true if the download was successful, false otherwise
 * @note This function must be called from an other thread than the main thread.
 */
bool downloadModel(const std::string& modelName, std::string tag) {
    if (!hasConnection()) {
        return false;
    }

    bool result = false;
    QMetaObject::invokeMethod(QApplication::instance(), [&]() {
        const std::string url = "https://github.com/" + std::string(REPO_OWNER) + "/" + std::string(REPO_NAME) + "/releases/download/yolov5/" + std::string(modelName);
        qInfo() << "Downloading model from URL: " << QString::fromStdString(url) << " to path: " << QString::fromStdString(modelName);
        const std::string modelOutputPath = APP_FILES.toStdString() + "/" + modelName;

        QProgressDialog progressDialog("Downloading " + QString::fromStdString(modelName), nullptr, 0, 100);
        progressDialog.setWindowModality(Qt::ApplicationModal);
        QPushButton* cancelButton = new QPushButton("Cancel", &progressDialog);
        QObject::connect(cancelButton, &QPushButton::clicked, &progressDialog, &QProgressDialog::cancel);
        progressDialog.setCancelButton(cancelButton);
        progressDialog.setValue(0);
        progressDialog.show();
        QApplication::processEvents();
        qInfo() << "Downloading model from URL 2: " << QString::fromStdString(url) << " to path: " << QString::fromStdString(modelName);
        result = downloadFile(url, modelOutputPath, &progressDialog); }, Qt::BlockingQueuedConnection);
    return result;
}

/**
 * @brief Download a file from the specified URL and save it to the specified output path. But it only download it if the file doesn't already exist.
 * @param url The URL of the file to download
 * @param outputPath The path where the downloaded file will be saved
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return true if the download was successful, false otherwise
 */
bool downloadFileIfNotExists(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog) {
    if (!hasConnection()) {
        return false;
    }

    if (fs::exists(outputPath)) {
        qInfo() << "File already exists: " << QString::fromStdString(outputPath);
        return true;
    }

    return downloadFile(url, outputPath, progressDialog);
}

/**
 * @brief Download a file from the specified URL and save it to the specified output path
 * @param url The URL of the file to download
 * @param outputPath The path where the downloaded file will be saved
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return true if the download was successful, false otherwise
 */
bool downloadFile(const std::string& url, const std::string& outputPath, QProgressDialog* progressDialog) {
    if (!hasConnection()) {
        return false;
    }

    qInfo() << "Downloading file from URL: " << QString::fromStdString(url) << " to path: " << QString::fromStdString(outputPath);
    CURL* curl;
    CURLcode res;
    std::ofstream outFile(outputPath, std::ios::binary);

    curl = curl_easy_init();
    if (curl) {
        // curl_easy_setopt(curl, CURLOPT_CAINFO, APP_FILES + "\\cacert.pem");

        // TODO reactiver la validation :
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 10L);

        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackAndSave);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &outFile);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

        curl_easy_setopt(curl, CURLOPT_XFERINFOFUNCTION, progressCallback);
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
        curl_easy_setopt(curl, CURLOPT_XFERINFODATA, progressDialog);

        res = curl_easy_perform(curl);
        if (res == CURLE_COULDNT_RESOLVE_HOST || res == CURLE_COULDNT_CONNECT) {
            showWarningMessage(nullptr, "Pas de connexion Internet", "Téléchargement");
            curl_easy_cleanup(curl);
            outFile.close();
            return false;
        }
        if (res != CURLE_OK) {
            qWarning() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
        }
        curl_easy_cleanup(curl);
    }

    outFile.close();

    return res == CURLE_OK;
}

/**
 * @brief Get the latest GitHub tag from the repository (without pre-release)
 * @param progressDialog Pointer to the progress dialog for showing download progress
 * @return The latest GitHub tag as a string
 * @details This function uses the GitHub API to get the latest release tag from the specified repository.
 */
std::string getLatestGitHubTag() {
    if (!hasConnection()) {
        return "0.0.0";
    }
    CURL* curl;
    CURLcode res;
    std::string readBuffer;

    const std::string& repoOwner = REPO_OWNER;
    const std::string& repoName = REPO_NAME;

    curl = curl_easy_init();
    if (curl) {
        std::string url = "https://api.github.com/repos/" + repoOwner + "/" + repoName + "/releases";

        // Desactive la validation SSL
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        // end validation

        // TODO reactiver la validation :
        // qDebug() << "path to .pem" << APP_FILES + "/cacert.pem";
        // curl_easy_setopt(curl, CURLOPT_CAINFO, APP_FILES + "/cacert.pem");
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
        // curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);
        curl_easy_setopt(curl, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_3);

        // Set timeout time to avoir bug
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);

        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        res = curl_easy_perform(curl);
        if (res == CURLE_OPERATION_TIMEDOUT) {
            showWarningMessage(nullptr, "Could not check for update (low connexion)", "Checking for updates");
            qWarning() << "Error : Could not check for update (low connexion) : " << curl_easy_strerror(res);
            curl_easy_cleanup(curl);
            return "";
        }
        if (res != CURLE_OK) {
            qWarning() << "curl_easy_perform() failed: " << curl_easy_strerror(res);
            showWarningMessage(nullptr, "Something went wrong", "Checking for updates");

            curl_easy_cleanup(curl);
            return "";
        }
        curl_easy_cleanup(curl);
    }

    // Parse JSON response
    Json::CharReaderBuilder readerBuilder;
    Json::Value root;
    std::string errs;

    std::istringstream s(readBuffer);
    if (Json::parseFromStream(readerBuilder, s, &root, &errs)) {
        if (!root.empty() && root.isArray()) {
            for (const auto& release : root) {
                const std::string tagName = release["tag_name"].asString();
                if (!release["prerelease"].asBool() && tagName.rfind("v", 0) == 0 && std::count(tagName.begin(), tagName.end(), '.') == 2) {
                    return tagName;
                }
            }
        }
    } else {
        qWarning() << "Failed to parse JSON: " << errs;
    }

    return "";
}