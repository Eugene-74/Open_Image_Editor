#include <iostream>
#include "date.h"

void Date::print() const {
    std::cout << "Jour : " << day << " Mois : " << month << " Année : " << year << std::endl;
}

std::string Date::getStringJJ_MM_AAAA() const {
    return std::to_string(day) + "/" + std::to_string(month) + "/" + std::to_string(year);
}
