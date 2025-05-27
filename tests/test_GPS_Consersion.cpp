#include <gtest/gtest.h>

#include <string>

#include "GPS_Conversion.hpp"

TEST(GPS_ConversionTest, RoundTripConversion) {
    double original = 48.858333;
    std::string coord = convertDecimalToGpsCoordinate(original);
    double result = convertGpsCoordinateToDecimal(coord);
    EXPECT_NEAR(result, original, 1e-4);
}

TEST(GPS_ConversionTest, ReverseRoundTripConversion) {
    std::string originalCoord = "48/1 51/1 30000/10000";
    double decimal = convertGpsCoordinateToDecimal(originalCoord);
    std::string resultCoord = convertDecimalToGpsCoordinate(decimal);
    EXPECT_EQ(resultCoord, originalCoord);
}