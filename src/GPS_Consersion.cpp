#include <sstream>
#include <string>

#include "GPS_Conversion.hpp"

double convertGpsCoordinateToDecimal(const std::string& coordinate) {
    std::istringstream iss(coordinate);
    std::string degrees, minutes, seconds;

    std::getline(iss, degrees, ' ');
    std::getline(iss, minutes, ' ');
    std::getline(iss, seconds, ' ');

    auto parseFraction = [](const std::string& fraction) -> double {
        size_t slashPos = fraction.find('/');
        if (slashPos != std::string::npos) {
            double numerator = std::stod(fraction.substr(0, slashPos));
            double denominator = std::stod(fraction.substr(slashPos + 1));
            return numerator / denominator;
        }
        return std::stod(fraction);
    };

    double deg = parseFraction(degrees);
    double min = parseFraction(minutes);
    double sec = parseFraction(seconds);

    return deg + (min / 60.0) + (sec / 3600.0);
};

std::string convertDecimalToGpsCoordinate(double decimal) {
    int degrees = static_cast<int>(decimal);
    double fractional = (decimal - degrees) * 60.0;
    int minutes = static_cast<int>(fractional);
    double seconds = (fractional - minutes) * 60.0;

    auto formatFraction = [](double value) -> std::string {
        int numerator = static_cast<int>(value * 10000);
        int denominator = 10000;
        return std::to_string(numerator) + "/" + std::to_string(denominator);
    };

    std::string degreesStr = std::to_string(degrees) + "/1";
    std::string minutesStr = std::to_string(minutes) + "/1";
    std::string secondsStr = formatFraction(seconds);

    return degreesStr + " " + minutesStr + " " + secondsStr;
};