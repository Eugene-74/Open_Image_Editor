#include "DetectObjectsModel.hpp"

#include <fstream>

std::vector<std::string> DetectObjectsModel::getClassNames() {
    return this->classNames;
}

void DetectObjectsModel::setClassNames(std::vector<std::string> classNames) {
    this->classNames = classNames;
}

std::string DetectObjectsModel::getModelName() {
    return this->modelName;
}

void DetectObjectsModel::setModelName(std::string modelName) {
    this->modelName = modelName;
}

std::string DetectObjectsModel::getLoadedModelName() {
    return this->loadedModelName;
}

void DetectObjectsModel::setLoadedModelName(std::string loadedModelName) {
    this->loadedModelName = loadedModelName;
}

float DetectObjectsModel::getConfidence() const {
    return this->confidence;
}

void DetectObjectsModel::setConfidence(float confidence) {
    this->confidence = confidence;
}

void DetectObjectsModel::save(std::ofstream& out) const {
    size_t modelNameLength = modelName.size();
    out.write(reinterpret_cast<const char*>(&modelNameLength), sizeof(modelNameLength));
    out.write(modelName.c_str(), modelNameLength);
}

void DetectObjectsModel::load(std::ifstream& in) {
    size_t modelNameLength;
    in.read(reinterpret_cast<char*>(&modelNameLength), sizeof(modelNameLength));
    modelName.resize(modelNameLength);
    in.read(&modelName[0], modelNameLength);
}