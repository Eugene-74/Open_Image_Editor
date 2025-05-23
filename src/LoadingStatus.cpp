#include "LoadingStatus.hpp"

/**
 * @brief Set the status of the image
 * @param status The status to set
 */
void LoadingStatus::setStatus(LoadingStatusType status) {
    this->status = status;
}

/**
 * @brief Set the status to loading
 */
void LoadingStatus::setStatusLoading() {
    setStatus(LoadingStatusType::Loading);
}

/**
 * @brief Set the status to not loaded
 */
void LoadingStatus::setStatusNotLoaded() {
    setStatus(LoadingStatusType::NotLoaded);
}

/**
 * @brief Set the status to loaded
 */
void LoadingStatus::setStatusLoaded() {
    setStatus(LoadingStatusType::Loaded);
}

/**
 * @brief Check if the status is loading
 * @return True if the status is loading, false otherwise
 */
bool LoadingStatus::isStatusLoading() {
    return status == LoadingStatusType::Loading;
}

/**
 * @brief Check if the status is not loaded
 * @return True if the status is not loaded, false otherwise
 */
bool LoadingStatus::isStatusNotLoaded() {
    return status == LoadingStatusType::NotLoaded;
}

/**
 * @brief Check if the status is loaded
 * @return True if the status is loaded, false otherwise
 */
bool LoadingStatus::isStatusLoaded() {
    return status == LoadingStatusType::Loaded;
}

/**
 * @brief Save the status to a file
 * @param out The output file stream
 */
void LoadingStatus::save(std::ofstream& out) const {
    out.write(reinterpret_cast<const char*>(&status), sizeof(status));
}

/**
 * @brief Load the status from a file
 * @param in The input file stream
 */
void LoadingStatus::load(std::ifstream& in) {
    in.read(reinterpret_cast<char*>(&status), sizeof(status));
    if (isStatusLoading()) {
        setStatusNotLoaded();
    }
}