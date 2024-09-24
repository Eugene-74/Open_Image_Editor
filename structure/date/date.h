#ifndef DATE_H
#define DATE_H

#include <iostream>
#include <vector>
class Date {
public:
    int seconds;
    int minutes;
    int hours;
    int day;
    int month;
    int year;

    Date(int s, int m, int h, int d, int m2, int y) : seconds(s), minutes(m), hours(h), day(d), month(m2), year(y) {
        if (day < 1 || day > 31) {
            std::cout << "Jour invalide" << std::endl;
            return;
        }
        if (month < 1 || month > 12) {
            std::cout << "Mois invalide" << std::endl;
            return;
        }
        if (year < 0 || year > 9999) {
            std::cout << "Année invalide" << std::endl;
            return;
        }
    }
    void print()const;

    std::string getStringJJ_MM_AAAA()const;
};


#endif
