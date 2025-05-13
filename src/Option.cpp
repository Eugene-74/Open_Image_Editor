#include "Option.hpp"

std::string Option::getTypeConst() const {
    return type;
}

std::string* Option::getTypePtr() {
    return &type;
}

std::string Option::getValueConst() const {
    return value;
}

std::string* Option::getValuePtr() {
    return &value;
}