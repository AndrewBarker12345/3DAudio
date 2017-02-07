/*
 Functions.h
 
 For math stuff.

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

#ifndef Functions_h
#define Functions_h

#include <cmath>
#include <algorithm>
#include <vector>
#include <cassert>
#include "DrewLib.h"

/** true if Set s, contains value v */
template<class Set, class Value>
bool contains(const Set& s, const Value& v) noexcept
{
    return std::find(s.begin(), s.end(), v) != s.end();
}
    
/** rotates the specified indices (must be sorted) by delta. the non-rotated indices may be displaced in the same direction but their relative ordering is preserved. */
template <class T>
void partial_rotate(std::vector<T>& v, const std::vector<int>& indicesToRotate, const int delta) {
    cauto copy = v;
    std::vector<std::pair<int, T>> other (copy.size() - indicesToRotate.size());
    std::vector<bool> inPlace (copy.size(), false);
    for (int i = 0, j = 0, k = 0; i < copy.size(); ++i) {
        if (k >= indicesToRotate.size() || i != indicesToRotate[k])
            other[j++] = {i, copy[i]};
        else
            ++k;
    }
    for (int i = 0, j; i < indicesToRotate.size(); ++i) {
        j = indicesToRotate[i] + delta;
        while (j < 0)
            j += copy.size();
        while (j >= copy.size())
            j -= copy.size();
        v[j] = copy[indicesToRotate[i]];
        inPlace[j] = true;
    }
    for (int i = 0, j; i < other.size(); ++i) {
        j = other[i].first;
        while (inPlace[j])
            j = (j+1) % copy.size();
        v[j] = other[i].second;
        inPlace[j] = true;
    }
}

// should be in C++17
namespace std {
    template<class T>
    constexpr T hypot(const T x, const T y, const T z) noexcept
    {
        return std::sqrt(x*x + y*y + z*z);
    }

    template<class T, class Compare>
    constexpr const T& clamp( const T& v, const T& lo, const T& hi, Compare comp )
    {
        return assert( !comp(hi, lo) ),
            comp(v, lo) ? lo : comp(hi, v) ? hi : v;
    }

    template<class T>
    constexpr const T& clamp( const T& v, const T& lo, const T& hi )
    {
        return clamp( v, lo, hi, std::less<>() );
    }
}

template <class T>
T boundsCheck(const T value, const T min, const T max)
{
    return std::max(std::min(value, max), min);
}

// time domain convolution
/*static*/
inline void convolve(const float *x, int Nx, const float *h, int Nh, float *output) noexcept
{
    const int Nxm1 = Nx-1;
    const int Nhm1 = Nh-1;
    const int L = Nx+Nhm1;//Nx+Nh-1;
    int xi = 0;
    int xf = 0;
    int hi = 0;
    
    // for each output sample
    for (int i = 0; i < L; ++i)
    {
        // for each overlaping sample of two signals
        const int M = xf-xi+1;
        float sum = 0;
        for (int j = 0; j < M; ++j)
            sum += x[xf-j] * h[hi+j];
        
        // shifting
        if (xf < Nxm1/*Nx-1*/)
            ++xf;
        else if (hi < Nhm1/*Nh-1*/)
            ++hi;
        
        if (i > Nhm1/*Nh-1*/)
            ++xi;
    
        output[i] = sum;
    }
    
}

// only compute convolution for the output samples from beginIndex to endIndex inclusive
/*static*/
inline void convolve(const float *x, const int Nx, const float *h, const int Nh, float *output, const int beginIndex, const int endIndex) noexcept
{
    const int Nxm1 = Nx-1;
    const int Nhm1 = Nh-1;
    //const int L = Nx+Nhm1;//Nx+Nh-1;
    int xi, xf, hi;
    
    // find xi xf and hi starting at begin index
    if (beginIndex > Nhm1/*Nh-1*/)
        xi = beginIndex - Nh;
    else
        xi = 0;
    
    if (beginIndex <= Nxm1/*Nx-1*/)
    {
        xf = beginIndex;
        hi = 0;
    }
    else
    {
        xf = Nxm1/*Nx-1*/;
        hi = beginIndex - Nxm1/*(Nx-1)*/;
        if (hi > Nhm1/*Nh-1*/)
            hi = Nhm1/*Nh-1*/;
    }
    
    // for each output sample
    for (int i = beginIndex; i <= endIndex; ++i)
    {
        // for each overlaping sample of two signals
        const int M = xf-xi+1;
        float sum = 0;
        for (int j = 0; j < M; ++j)
            sum += x[xf-j] * h[hi+j];
        
        // shifting
        if (xf < Nxm1/*Nx-1*/)
            ++xf;
        else if (hi < Nhm1/*Nh-1*/)
            ++hi;
        
        if (i > Nhm1/*Nh-1*/)
            ++xi;
        
        output[i] = sum;
    }
}

inline float toDegrees(float radians) noexcept
{
    return radians * 180 / M_PI;
}

inline float toRadians(float degrees) noexcept
{
    return degrees * M_PI / 180;
}

inline void XYZtoRAE(const float* xyz, float* rae) noexcept
{
    rae[0] = std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
    if (rae[0] != 0)
        rae[2] = std::acos(xyz[1]/rae[0]);
    else // avoid divide by zero
        rae[2] = 0;
    
    if (xyz[0] < 0)
        rae[1] = std::atan(xyz[2]/xyz[0]) + M_PI;
    else if (xyz[0] > 0)
        rae[1] = std::atan(xyz[2]/xyz[0]);
    else // avoid divide by zero
        if (xyz[2] < 0)
            rae[1] = -M_PI_2;
        else
            rae[1] = M_PI_2;
    
    while (rae[1] > 2.0*M_PI)
        rae[1] -= 2.0*M_PI;
    
    while (rae[1] < 0)
        rae[1] += 2.0*M_PI;
}

inline void RAEtoXYZ(const float* rae, float* xyz) noexcept
{
    xyz[0] = rae[0]*std::sin(rae[2])*std::cos(rae[1]);
    xyz[1] = rae[0]*std::cos(rae[2]);
    xyz[2] = rae[0]*std::sin(rae[2])*std::sin(rae[1]);
}

inline float angleBetween(const std::vector<float>& a,
                          const std::vector<float>& b) noexcept
{
    if (a.size() != b.size())
        return -1;
    float dotProduct = 0, magA = 0, magB = 0;
    for (int i = 0; i < a.size(); ++i) {
        dotProduct += a[i] * b[i];
        magA += a[i] * a[i];
        magB += b[i] * b[i];
    }
    return std::acos(dotProduct / (std::sqrt(magA) * std::sqrt(magB)));
}

template <class T>
std::vector<T> toVector(const float* array, const std::size_t N)
{
    std::vector<T> vector (N);
    for (int i = 0; i < N; ++i)
        vector[i] = array[i];
    return vector;
}

#endif  // Functions_h
