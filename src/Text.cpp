#include "Text.hpp"

#include <fstream>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>

#include "Const.hpp"

std::map<std::string, std::string> openTranslation(const std::string& langage) {
    std::map<std::string, std::string> translations;
    std::string filePath = ":/" + langage + ".csv";
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Unable to open file: " + filePath);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream lineStream(line);
        std::string key, value;

        if (std::getline(lineStream, key, ':') && std::getline(lineStream, value)) {
            translations[key] = value;
        }
    }

    file.close();
    return translations;
}

std::string getTranslation(const std::map<std::string, std::string>& translations,
                           const std::map<std::string, std::string>& defaultTranslations,
                           const std::string& key) {
    auto it = translations.find(key);
    if (it != translations.end()) {
        return it->second;
    }

    auto defaultIt = defaultTranslations.find(key);
    if (defaultIt != defaultTranslations.end()) {
        return defaultIt->second;
    }

    return "";
}