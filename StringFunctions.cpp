//
//  StringFunctions.cpp
//
//  Created by Andrew Barker on 10/6/16.
//
//

#include "StringFunctions.h"

#include <cmath>
#include <cstdio>

//std::string StrFuncs::roundedFloatString (const float num,
//                                          const int numDecimalPlaces) noexcept
//{
//    auto str = std::to_string(num);
//    if (numDecimalPlaces > 5 || numDecimalPlaces < 0)
//        return str;
//    const int decimalIndex = (int)(std::find(str.begin(), str.end(), '.') - str.begin());
//    const int truncIndex = numDecimalPlaces > 0 ? decimalIndex + numDecimalPlaces + 2 : decimalIndex + 2;
//    str = str.substr(0, truncIndex);
//    const auto roundingDigit = std::stoi(str.substr(str.length() - 1));
//    str.pop_back();
//    if (roundingDigit > 4) { // need to round up
//        auto digitToRoundIndex = str.length() - 1;
//        while (digitToRoundIndex > 0 && (str[digitToRoundIndex] == '9' || str[digitToRoundIndex] == '.'))
//            --digitToRoundIndex;
//        const auto roundedDigit = std::to_string(std::stoi(str.substr(digitToRoundIndex, 1)) + 1);
//        str.replace(str.begin() + digitToRoundIndex, str.begin() + digitToRoundIndex + 1, roundedDigit);
//        ++digitToRoundIndex;
//        while (digitToRoundIndex < str.length()) { // fill in zeros if need be
//            if (str[digitToRoundIndex] != '.')
//                str[digitToRoundIndex] = '0';
//            ++digitToRoundIndex;
//        }
//    } //else // round down, just truncate
//    if (numDecimalPlaces == 0) // get rid of decimal point if no decimal places
//        str.pop_back();
//    return str;
//}

std::string StrFuncs::hrMinSecFromSec (cfloat sec,
                                       cint numDecimalPlaces)
{
    std::string str;
    cint secs = std::floor(sec);
    cint mins = secs / 60 % 60;
    cint hours = secs / 3600;
    cauto secStr = roundedFloatString(std::fmod(sec, 60), numDecimalPlaces);
//    std::string secStr = std::to_string(std::fmod(sec, 60)); // 6 decimal places
//    // was trying to fix rounding, but said screw it.
////    const int roundIndex = secStr.length() - std::max(6 - numDecimalPlaces, 0);
////    int roundDigit = -1;
////    int digitToRound = -1;
////    try {
////        roundDigit = stoi(secStr.substr(roundIndex, roundIndex + 1));
////        digitToRound = stoi(secStr.substr(roundIndex - 1, roundIndex));
////    } catch (...) {}
////    if (roundDigit > 4 && digitToRound > -1) {
////        secStr = secStr.substr(0, roundIndex - 1);
////        secStr += std::to_string(digitToRound + 1).substr(0, 1);
////    } else 
////        secStr = secStr.substr(0, roundIndex);
//    secStr = secStr.substr(0, secStr.length()
//                              - std::max(6 - numDecimalPlaces, 0));
    if (hours != 0)
        str = std::to_string(hours) + ":"
            + std::to_string(mins) + ":" + secStr;
    //        if (std::fmod(sec, 60) < 10)
    //            str = std::to_string(hours) + ":" +
    //                  std::to_string(mins) + ":" +
    //                  secStr;//std::sprintf(str, "%d:%.2d:0%.2f", hours, mins, std::fmod(sec, 60));
    //        else
    //            std::sprintf(str, "%d:%.2d:%.2f", hours, mins, std::fmod(sec, 60));
    else if (mins != 0)
        str = std::to_string(mins) + ":" + secStr;
    //            if (std::fmod(sec, 60) < 10)
    //                std::sprintf(str, "%d:0%.2f", mins, std::fmod(sec, 60));
    //            else
    //                std::sprintf(str, "%d:%.2f", mins, std::fmod(sec, 60));
    else
        str = secStr;
    //std::sprintf(str, "%.2f", sec);
    return str;
}

// returns the time value in seconds of a HR:MIN::SEC formatted time string
float StrFuncs::secFromHrMinSec (const std::string& hrMinSec) noexcept
{
    float seconds = 0;
    auto txt = hrMinSec;
    std::string hr, min, sec;
    int i, count = 0;
    while ((i = txt.rfind(':')) >= 0) {
        count++;
        if (count == 1) {
            sec = txt.substr(i + 1, hrMinSec.npos);
            min = txt.substr(0, i);
            txt = min;
        } else if (count == 2) {
            min = txt.substr(i + 1, txt.npos);
            hr = txt.substr(0, i);
            break;
        }
    }
    if (count == 0)
        sec = txt;
    try {
        if (! hr.empty())
            seconds += std::stoi(hr) * 3600;
        if (! min.empty())
            seconds += std::stoi(min) * 60;
        if (! sec.empty())
            seconds += std::stof(sec);
    } catch (...) { seconds = -1; }
    return seconds;
}

int StrFuncs::castToInt(const std::string& str) noexcept
{
    int i = 0;
    if (str.empty())
        i = -1;
    else {
        try {
            int k = str.length() - 1;
            for (unsigned char c : str)
                i += c + 256 * k--;
        } catch (...) { i = -2; }
    }
    return i;
}
