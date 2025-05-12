#pragma once
#include <QString>
#include <map>
#include <string>

std::map<std::string, std::string>* openTranslation(const std::string& filePath);

std::string getTranslation(const std::map<std::string, std::string>& translations,
                           const std::map<std::string, std::string>& defaultTranslations,
                           const std::string& key);

class TranslationManager {
   public:
    TranslationManager();

    void setLanguage(const std::string& language);
    std::string getLanguage() const;

    std::string translate(const std::string& key) const;
    QString translateQ(const std::string& key) const;

   private:
    std::map<std::string, std::string> translations;
    std::map<std::string, std::string> defaultTranslations;
    std::string currentLanguage;
};
