#include "Option.hpp"

/**
 * @brief Get the type of the option as a constant string.
 * @return The type of the option.
 */
std::string Option::getTypeConst() const {
    return type;
}

/**
 * @brief Get a pointer to the type string of the option.
 * @return Pointer to the type string.
 */
std::string* Option::getTypePtr() {
    return &type;
}

/**
 * @brief Get the value of the option as a constant string.
 * @return The value of the option.
 */
std::string Option::getValueConst() const {
    return value;
}

/**
 * @brief Get a pointer to the value string of the option.
 * @return Pointer to the value string.
 */
std::string* Option::getValuePtr() {
    return &value;
}

/**
 * @brief Get the value as an integer if the type is "int".
 * @return The integer value, or 0 if the type is not "int".
 */
int Option::getIntValue() const {
    if (type != "int") {
        return 0;
    }
    return std::stoi(value);
}

/**
 * @brief Get the value as a float if the type is "float".
 * @return The float value, or 0.0 if the type is not "float".
 */
double Option::getFloatValue() const {
    if (type != "float") {
        return 0.0;
    }
    return std::stod(value);
}

/**
 * @brief Get the value as a boolean if the type is "bool".
 * @return True if the value is "true" and type is "bool", otherwise false.
 */
bool Option::getBoolValue() const {
    if (type != "bool") {
        return false;
    }
    return value == "true";
}

/**
 * @brief Get the first value from a list if the type is "list".
 * @return The first value in the list, or an empty string if not a list.
 */
std::string Option::getValueFromList() const {
    if (type != "list") {
        return "";
    }
    size_t pos = value.find('|');
    if (pos == std::string::npos) {
        return value;
    }
    return value.substr(0, pos);
}