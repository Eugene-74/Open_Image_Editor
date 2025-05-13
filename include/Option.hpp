#pragma once

#include <string>

class Option {
   public:
    std::string getTypeConst() const;
    std::string* getTypePtr();
    std::string getValueConst() const;
    std::string* getValuePtr();

    Option() = default;
    Option(const std::string& type, const std::string& value)
        : type(type), value(value) {
    }

   private:
    std::string type;
    std::string value;
};