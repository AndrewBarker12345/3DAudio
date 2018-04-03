/*
 BoundedValue.h
 
 Abstracts the concept of an ordered value that must be in the range of [min, max]
 
 Copyright (C) 2016  Andrew Barker
 
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

#ifndef BoundedValue_h
#define BoundedValue_h

#include <cmath>

template <class T>
class BoundedValue
{
public:
    /** sets up a value of zero with a bounds of [0, 0] */
    BoundedValue () noexcept;
    /** sets up with a specified value and bounds of [min, max] */
    BoundedValue (T value, T min, T max) noexcept;
    
    /** returns the value */
    T getValue () const noexcept;
    /** sets the value and makes sure it is inside the specified bounds */
    void setValue (T value) noexcept;
    
    /** returns the minimum allowed value for the bounded value */
    T getMin () const noexcept;
    /** sets the minimum allowed value for the bounded value */
    void setMin (T min) noexcept;
    
    /** returns the maximum allowed value for the bounded value */
    T getMax () const noexcept;
    /** sets the maximum allowed value for the bounded value */
    void setMax (T max) noexcept;
    
    /** returns the difference between the min and max for the value */
    T range () const noexcept;
    
    /** returns the value normalized to it's range, between [0, 1] */
    T normalized () const noexcept;
    
    /** allows implicit conversion to the type of the bounded value */
    operator T () const noexcept;
    
    /** sets the value and makes sure it is inside the specified bounds */
    BoundedValue& operator = (T value) noexcept;
    /** adds the the specified amount to the value with bounds checking */
    BoundedValue& operator += (T addValue) noexcept;
    /** subtracts the specified amount from the value with bounds checking */
    BoundedValue& operator -= (T subValue) noexcept;
    /** multiplies the value by the specified amount with bounds checking */
    BoundedValue& operator *= (T multValue) noexcept;
    /** divides the value by the specified amount with bounds checking */
    BoundedValue& operator /= (T divValue) noexcept;
    /** sets the value to the remainder of value / divValue with bounds checking */
    BoundedValue& operator %= (T divValue) noexcept;
    
private:
    T _value, _min, _max;
};

// implementation

template <class T>
BoundedValue<T>::BoundedValue () noexcept
    : _value (0),
      _min   (0),
      _max   (0)
{}

template <class T>
BoundedValue<T>::BoundedValue (const T value,
                               const T min,
                               const T max) noexcept
    : _value (value),
      _min   (min),
      _max   (max)
{
    if (_min > _max) {
        _min = max;
        _max = min;
    }
}

template <class T>
T BoundedValue<T>::getValue () const noexcept { return _value; }

template <class T>
void BoundedValue<T>::setValue (const T value) noexcept
{
    if (getMin() <= value && value <= getMax())
        _value = value;
    else if (value < getMin())
        _value = getMin();
    else
        _value = getMax();
}

template <class T>
T BoundedValue<T>::getMin () const noexcept { return _min; }

template <class T>
void BoundedValue<T>::setMin (const T min) noexcept
{
    if (min <= getMax()) {
        _min = min;
        setValue(getValue());
    } else {
        _min = min;
        _max = min;
        _value = min;
    }
}
    
template <class T>
T BoundedValue<T>::getMax () const noexcept { return _max; }

template <class T>
void BoundedValue<T>::setMax (const T max) noexcept
{
    if (max >= getMin()) {
        _max = max;
        setValue(getValue());
    } else {
        _min = max;
        _max = max;
        _value = max;
    }
}

template <class T>
BoundedValue<T>::operator T () const noexcept { return _value; }

template <class T>
BoundedValue<T>& BoundedValue<T>::operator = (const T value) noexcept
{
    setValue(value);
    return *this;
}

template <class T>
BoundedValue<T>& BoundedValue<T>::operator += (const T value) noexcept
{
    setValue(getValue() + value);
    return *this;
}

template <class T>
BoundedValue<T>& BoundedValue<T>::operator -= (const T value) noexcept
{
    setValue(getValue() - value);
    return *this;
}

template <class T>
BoundedValue<T>& BoundedValue<T>::operator *= (const T value) noexcept
{
    setValue(getValue() * value);
    return *this;
}

template <class T>
BoundedValue<T>& BoundedValue<T>::operator /= (T value) noexcept
{
    setValue(getValue() / value);
    return *this;
}

template <class T>
BoundedValue<T>& BoundedValue<T>::operator %= (T value) noexcept
{
    setValue(std::fmod(getValue(), value));
    return *this;
}

template <class T>
T BoundedValue<T>::range () const noexcept
{ return getValue().getMax() - getValue().getMin(); }

template <class T>
T BoundedValue<T>::normalized () const noexcept
{ return (getValue() - getMin()) * range(); }

#endif /* BoundedValue_h */