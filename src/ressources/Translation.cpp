#include "Translation.hpp"

#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <fstream>
#include <sstream>
#include <stdexcept>
TranslationManager::TranslationManager() {
    this->currentLanguage = "fr";
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

std::map<std::string, std::string>* openTranslation(const std::string& language) {
    std::map<std::string, std::string>* translations = new std::map<std::string, std::string>();
    QString filePath = QString(":/translations/%1.csv").arg(QString::fromStdString(language));
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
            QString key = parts[0].trimmed();
            QString value = parts[1].trimmed();
            if (!key.isEmpty() && !value.isEmpty()) {  // Vérifie que la clé et la valeur ne sont pas vides
                (*translations)[key.toStdString()] = value.toStdString();
            } else {
                qWarning() << "Invalid translation entry in file:" << filePath << "->" << line;
            }
        } else {
            qWarning() << "Malformed line in translation file:" << filePath << "->" << line;
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
        if (it->second.empty()) {
            qWarning() << "Translation is empty for key:" << QString::fromStdString(key);
        } else {
            qDebug() << "Translation found for key:" << QString::fromStdString(key) << "->" << QString::fromStdString(it->second);
            return it->second;
        }
    }

    auto defaultIt = defaultTranslations.find(key);
    if (defaultIt != defaultTranslations.end()) {
        qDebug() << "Default translation found for key:" << QString::fromStdString(key) << "->" << QString::fromStdString(defaultIt->second);
        return defaultIt->second;
    }

    qWarning() << "Translation not found for key:" << QString::fromStdString(key);
    return "[MISSING TRANSLATION: " + key + "]";  // Retourne une chaîne explicite pour les traductions manquantes
}