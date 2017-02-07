/*
 NonlinearInterpolator.h
 
 Polymorphic curves ex:  y = a^x and x = log_a(y)

 Copyright (C) 2017  Andrew Barker
 
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 
 The author can be contacted via email at andrew.barker.12345@gmail.com.
*/

#ifndef NonlinearInterpolator_h
#define NonlinearInterpolator_h

#include <cmath>

template <class FloatingPointType>
class NonlinearInterpolator
{
public:
    NonlinearInterpolator() {}
    virtual ~NonlinearInterpolator() {}
    virtual NonlinearInterpolator<FloatingPointType>* clone() const = 0;
    // given a linear value between min and max, return a nonlinearly interpolated value between min and max
    virtual FloatingPointType getValue(FloatingPointType linearValue,
                                       FloatingPointType min,
                                       FloatingPointType max) const = 0;
    virtual FloatingPointType getInverseValue(FloatingPointType linearValue,
                                              FloatingPointType min,
                                              FloatingPointType max) const = 0;
};

template <class FloatingPointType>
class LogarithmicInterpolator : public NonlinearInterpolator<FloatingPointType>
{
public:
    LogarithmicInterpolator(const FloatingPointType base = 2) noexcept : base(base) {}
    
    NonlinearInterpolator<FloatingPointType>* clone() const override
    {
        return new LogarithmicInterpolator<FloatingPointType>(*this);
    }
    
    FloatingPointType getValue(const FloatingPointType linearValue,
                               const FloatingPointType min,
                               const FloatingPointType max) const noexcept override
    {
        const auto range = max - min;
        const auto normalizedValue = (linearValue - min) / range * (base - 1) + 1; // [1, base]
        return min + std::log(normalizedValue) / std::log(base) * range; // [min, max]
    }
    
    FloatingPointType getInverseValue(const FloatingPointType linearValue,
                                      const FloatingPointType min,
                                      const FloatingPointType max) const noexcept override
    {
        const auto range = max - min;
        const auto normalizedValue = (linearValue - min) / range; // [0, 1]
        return min + (std::pow(base, normalizedValue) - 1) / (base - 1) * range; // [min, max]
    }

private:
    FloatingPointType base;
};

#endif /* NonlinearInterpolator_h */