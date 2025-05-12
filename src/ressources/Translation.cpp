#include "Translation.hpp"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include <sstream>
#include <stdexcept>
TranslationManager::TranslationManager() {
    this->currentLanguage = "en";
    this->defaultTranslations = *openTranslation("en");
}

void TranslationManager::setLanguage(const std::string& language) {
    this->currentLanguage = language;
    this->translations = *openTranslation(language);
}

std::string TranslationManager::getLanguage() const {
    return this->currentLanguage;
}

std::string TranslationManager::translate(const std::string& key) const {
    return getTranslation(this->translations, this->defaultTranslations, key);
}

QString TranslationManager::translateQ(const std::string& key) const {
    return QString::fromStdString(getTranslation(translations, defaultTranslations, key));
}

std::map<std::string, std::string>* openTranslation(const std::string& langage) {
    std::map<std::string, std::string>* translations = new std::map<std::string, std::string>();
    QString filePath = QString(":/translations/%1.csv").arg(QString::fromStdString(langage));
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Unable to open file:" << filePath;
        return nullptr;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(':');
        if (parts.size() == 2) {
            (*translations)[parts[0].toStdString()] = parts[1].toStdString();
            // qDebug() << "Loaded translation:" << parts[0] << "->" << parts[1];
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

    qWarning() << "Translation not found for key:" << QString::fromStdString(key);

    return "";
}