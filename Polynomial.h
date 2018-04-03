//
//  Polynomial.h
//
//  Created by Andrew Barker on 9/25/15.
//
//
/*
     3DAudio: simulates surround sound audio for headphones
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


#ifndef __Polynomial__
#define __Polynomial__

#include <valarray> // for speed and sexy

// A class for storing a polynomial by the coefficients and exponants of its terms and providing a simple, efficient way to get its value at any input.
template <typename T>
class Polynomial
{
public:
    // need a default constructor to compile (has to do with using this class in stl containers)
    Polynomial() {};
    // outer dim should only be size two
    Polynomial(const std::valarray<std::valarray<T>>& new_terms)
    {
        terms = new_terms;
    };
    
    T operator()(const T& value) const noexcept
    {
        return (terms[0] * pow(value, terms[1])).sum();
    };
    
    void allocate(const int numTerms)
    {
        terms.resize(2);
        terms[0].resize(numTerms);
        terms[1].resize(numTerms);
    };
    
    void fill(const T* coeffs, const T* exps, const int numTerms) noexcept
    {
        for (int i = 0; i < numTerms; ++i)
        {
            terms[0][i] = coeffs[i];
            terms[1][i] = exps[i];
        }
    };
    
private:
    std::valarray<std::valarray<T>> terms; // coeffs in [0][*], exponents in [1][*]
};

#endif /* defined __Polynomial__ */
