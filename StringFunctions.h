//
//  StringFunctions.h
//
//  Created by Andrew Barker on 10/5/16.
//
//

#ifndef StringFunctions_h
#define StringFunctions_h

#include "DrewLib.h"
#include <string>

namespace StrFuncs {
    /** returns a string with up to 6 decimals of precision and rounding applied from a float */
    //std::string roundedFloatString(float num, int numDecimalPlaces = 2) noexcept;
    /** Takes a floating point type and returns a string that is rounded to a number of decimal places (default is 2).
        Negative decimal places translate to rounding to digits left of the decimal ex: -1 rounds to tens, -2 rounds to hundreds, etc. */
    template <class T>
    std::string roundedFloatString (T num, int numDecimalPlaces = 2) noexcept;
    std::string hrMinSecFromSec (float sec, int numDecimalPlaces = 2);
    float secFromHrMinSec (const std::string& hrMinSec) noexcept;
    int castToInt(const std::string& str) noexcept;
}

#include <sstream>
/** Takes a floating point type and returns a string that is rounded to a number of decimal places (default is 2).
    Negative decimal places translate to rounding to digits left of the decimal ex: -1 rounds to tens, -2 rounds to hundreds, etc. */
template <class T>
std::string StrFuncs::roundedFloatString (const T num,
                                          const int numDecimalPlaces) noexcept
{
    std::stringstream stream;
    std::fixed(stream);
    const auto precision = (numDecimalPlaces >= 0) ? numDecimalPlaces : 1; // don't want rounding done by stream for numDecimalPlaces < 0
    stream.precision(precision);
    stream << num;
    if (numDecimalPlaces >= 0)
        return stream.str();
    else {
        auto str = stream.str();
        str.erase(str.end() - 2, str.end()); // get rid of ".(tens)" that was kept to prevent rounding
        if ((int)str.length() + numDecimalPlaces - 1 < 0)
            str.insert(str.begin(), '0'); // pad front with zero for rounding up to extra digit if need be
        if ((int)str.length() + numDecimalPlaces - 1 < 0)
            return "0"; // answer will be rounded down to zero
        const auto roundingDigitIndex = (int)str.length() + numDecimalPlaces;
        const auto roundingDigit = std::stoi(str.substr(roundingDigitIndex, 1));
        auto digitToRoundIndex = roundingDigitIndex - 1;
        if (roundingDigit > 4) { // need to round up
            while (digitToRoundIndex > 0 && str[digitToRoundIndex] == '9')
                --digitToRoundIndex;
            const auto roundedDigit = std::to_string(std::stoi(str.substr(digitToRoundIndex, 1)) + 1);
            str.replace(str.begin() + digitToRoundIndex, str.begin() + digitToRoundIndex + 1, roundedDigit);
            while (++digitToRoundIndex < str.length()) // fill in zeros if need be
                str[digitToRoundIndex] = '0';
        } else { // rounding down, just truncate and fill in zeros if need be
            while (++digitToRoundIndex < str.length())
                str[digitToRoundIndex] = '0';
            if (std::count(str.begin(), str.end(), '0') == str.length())
                str = "0"; // replace potential full string of zeros with just one
        }
        return str;
    }
}

#endif /* StringFunctions_h */
