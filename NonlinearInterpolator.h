//
//  NonlinearInterpolator.h
//
//  Created by Andrew Barker on 9/8/16.
//
//

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
        // if (base > 1)
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