#include "DetectObjectsModel.hpp"

#include <fstream>

/**
 * @brief Get the class names
 * @return The class names
 */
std::vector<std::string> DetectObjectsModel::getClassNames() {
    return this->classNames;
}

/**
 * @brief Set the class names
 * @param classNames The class names
 */
void DetectObjectsModel::setClassNames(std::vector<std::string> classNames) {
    this->classNames = classNames;
}

/**
 * @brief Get the model name
 * @return The model name
 */
std::string DetectObjectsModel::getModelName() {
    return this->modelName;
}

/**
 * @brief Set the model name
 * @param modelName The model name
 */
void DetectObjectsModel::setModelName(std::string modelName) {
    this->modelName = modelName;
}

/**
 * @brief Get the confidence
 * @return The confidence
 */
float DetectObjectsModel::getConfidence() const {
    return this->confidence;
}

/**
 * @brief Set the confidence
 * @param confidence
 */
void DetectObjectsModel::setConfidence(float confidence) {
    this->confidence = confidence;
}

/**
 * @brief Save the model name to a file
 * @param out The output file stream
 */
void DetectObjectsModel::save(std::ofstream& out) const {
    size_t modelNameLength = modelName.size();
    out.write(reinterpret_cast<const char*>(&modelNameLength), sizeof(modelNameLength));
    out.write(modelName.c_str(), modelNameLength);
}

/**
 * @brief Load the model name from a file
 * @param in The input file stream
 */
void DetectObjectsModel::load(std::ifstream& in) {
    size_t modelNameLength;
    in.read(reinterpret_cast<char*>(&modelNameLength), sizeof(modelNameLength));
    modelName.resize(modelNameLength);
    in.read(&modelName[0], modelNameLength);
}