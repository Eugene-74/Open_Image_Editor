#include "Date.hpp"

QString getCurrentFormattedDate() {
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_time_t);

    return QString("%1:%2 - %3/%4/%5")
        .arg(now_tm->tm_hour, 2, 10, QChar('0'))
        .arg(now_tm->tm_min, 2, 10, QChar('0'))
        .arg(now_tm->tm_mday, 2, 10, QChar('0'))
        .arg(now_tm->tm_mon + 1, 2, 10, QChar('0'))
        .arg(now_tm->tm_year + 1900);
}
