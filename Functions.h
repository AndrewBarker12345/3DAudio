//
//  Functions.h
//
//  Created by Andrew Barker on 4/26/14.
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

#ifndef Functions_h
#define Functions_h

#include <cmath>
#include <algorithm>
#include <vector>
#include <cassert>
#include "DrewLib.h"

//inline float lagrangeInterpolate(const float y1, const float y2, const float y3, const float y4, const float x) noexcept
//{
//    const float xm1 = x-1.0;
//    const float xm2 = x-2.0;
//    const float xm3 = x-3.0;
//    const float xm4 = x-4.0;
//    
////    float a0 = (xm2)*(xm3)*(xm4)/(-6.0)*y1;
////    float a1 = (xm1)*(xm3)*(xm4)/(2.0)*y2;
////    float a2 = (xm1)*(xm2)*(xm4)/(-2.0)*y3;
////    float a3 = (xm1)*(xm2)*(xm3)/(6.0)*y4;
//    
//    // the need for speed ...
//    const float a0 = (xm2)*(xm3)*(xm4)*-0.1666666666666666667*y1;
//    const float a1 = (xm1)*(xm3)*(xm4)*0.5*y2;
//    const float a2 = (xm1)*(xm2)*(xm4)*-0.5*y3;
//    const float a3 = (xm1)*(xm2)*(xm3)*0.1666666666666666667*y4;
//    
//    return a0 + a1 + a2 + a3;
////    return ((x-2.0)*(x-3.0)*(x-4.0)/(-6.0)*y1 + (x-1.0)*(x-3.0)*(x-4.0)/(2.0)*y2
////            + (x-1.0)*(x-2.0)*(x-4.0)/(-2.0)*y3 + (x-1.0)*(x-2.0)*(x-3.0)/(6.0)*y4);
//}
//
//// the vectorized version of the one above
//inline void lagrangeInterpolate(const float* y1, const float* y2, const float* y3, const float* y4, const int N, const float x, float* output) noexcept
//{
//    const float xm1 = x-1.0;
//    const float xm2 = x-2.0;
//    const float xm3 = x-3.0;
//    const float xm4 = x-4.0;
//    const float a1 = (xm2)*(xm3)*(xm4)*-0.1666666666666666667;
//    const float a2 = (xm1)*(xm3)*(xm4)*0.5;
//    const float a3 = (xm1)*(xm2)*(xm4)*-0.5;
//    const float a4 = (xm1)*(xm2)*(xm3)*0.1666666666666666667;
//    for (int n = 0; n < N; ++n)
//        output[n] = a1*y1[n] + a2*y2[n] + a3*y3[n] + a4*y4[n];
//}

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
        if (k == indicesToRotate.size() || i != indicesToRotate[k])
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

// circular input buffer convolution
inline void convolve(const float *cBuf, const int cBufIdx, const int cBufN,
					 const float *h, const int Nh, const float hScale,
					 float* output, const int N) noexcept
{
	// for each output sample
	for (int n = 0; n < N; ++n) {
		// for each overlaping sample of the two signals
		float sum = 0;
		int i = (cBufIdx + n) % cBufN;
		for (int j = 0; j < Nh; ++j) {
			sum += cBuf[i] * h[j];
			if (--i < 0)
				i = cBufN - 1;
		}
		output[n] = sum * hScale;
	}
}

inline void convolve(const float *cBuf, const int cBufIdx, const int cBufN,
					 const float *hs, const int Nh, const int numHs, const float *hScales, const int ch, 
					 float *output, const int N) noexcept
{
	// for each output sample
	for (int n = 0; n < N; ++n) {
		const float L = N / float(numHs - 1);
		const float ndL = n / L;
		const int hi = ndL;
		const int hIdx1 = 2 * hi + ch;
		const int hIdx2 = 2 * (hi + 1) + ch;
		const float *h1 = &hs[hIdx1 * Nh],
			        *h2 = &hs[hIdx2 * Nh];
		int i = (cBufIdx + n) % cBufN;
		float sum1 = 0, sum2 = 0;
		// for each overlaping sample of the two signals
		for (int j = 0; j < Nh; ++j) {
			sum1 += cBuf[i] * h1[j];
			sum2 += cBuf[i] * h2[j];
			if (--i < 0)
				i = cBufN - 1;
		}
		const float hBlend = ndL - hi; //std::fmod(n, L);
		output[n] = sum1 * hScales[hIdx1] * (1-hBlend) + sum2 * hScales[hIdx2] * hBlend;
	}
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

//template<class InputIt1, class InputIt2, class T>
//T inner_product(InputIt1 first1, InputIt1 last1,
//                InputIt2 first2, T value)
//{
//    while (first1 != last1) {
//        value += (*first1) * (*first2);
//        --first1;
//        ++first2;
//    }
//    return value;
//}
//
//inline void convolve2(float *x, const int Nx, float *h, const int Nh, float *output, const int beginIndex, const int endIndex)
//{
//    const int L = Nx+Nh-1;
//    int xi, xf, hi;
//    
//    // find xi xf and hi starting at begin index
//    if (beginIndex > Nh-1)
//        xi = beginIndex - Nh;
//    else
//        xi = 0;
//    
//    if (beginIndex <= Nx-1)
//    {
//        xf = beginIndex;
//        hi = 0;
//    }
//    else
//    {
//        xf = Nx-1;
//        hi = beginIndex - (Nx-1);
//        if (hi > Nh-1)
//            hi = Nh-1;
//    }
//    
//    // for each output sample
//    for (int i = beginIndex; i <= endIndex; ++i)
//    {
//        // for each overlaping sample of two signals
//        const int M = xf-xi+1;
//        output[i] = inner_product(&x[xf], &x[xf-M], &h[hi], 0);
//        
//        // shifting
//        if (xf < Nx-1)
//            ++xf;
//        else if (hi < Nh-1)
//            ++hi;
//        
//        if (i > Nh-1)
//            ++xi;
//    }
//}

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

//inline void XYZtoRAE(const float (&xyz)[3], float (&rae)[3])
//{
//    rae[0] = std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
//    if (rae[0] != 0)
//        rae[2] = std::acos(xyz[1]/rae[0]);
//    else // avoid divide by zero
//        rae[2] = 0;
//    
//    if (xyz[0] < 0)
//        rae[1] = std::atan(xyz[2]/xyz[0]) + M_PI;
//    else if (xyz[0] > 0)
//        rae[1] = std::atan(xyz[2]/xyz[0]);
//    else  // avoid divide by zero
//        rae[1] = 0;
//    
//    while (rae[1] > 2.0*M_PI)
//        rae[1] -= 2.0*M_PI;
//    
//    while (rae[1] < 0)
//        rae[1] += 2.0*M_PI;
//}
//
//inline void RAEtoXYZ(const float (&rae)[3], float (&xyz)[3])
//{
//    xyz[0] = rae[0]*std::sin(rae[2])*std::cos(rae[1]);
//    xyz[1] = rae[0]*std::cos(rae[2]);
//    xyz[2] = rae[0]*std::sin(rae[2])*std::sin(rae[1]);
//}
//
//// more C++ er
//inline void XYZtoRAE(const std::array<float,3>& xyz, std::array<float,3>& rae)
//{
//    rae[0] = std::sqrt(xyz[0]*xyz[0] + xyz[1]*xyz[1] + xyz[2]*xyz[2]);
//    if (rae[0] != 0)
//        rae[2] = std::acos(xyz[1]/rae[0]);
//    else // avoid divide by zero
//        rae[2] = 0;
//    
//    if (xyz[0] < 0)
//        rae[1] = std::atan(xyz[2]/xyz[0]) + M_PI;
//    else if (xyz[0] > 0)
//        rae[1] = std::atan(xyz[2]/xyz[0]);
//    else  // avoid divide by zero
//        rae[1] = 0;
//    
//    while (rae[1] > 2.0*M_PI)
//        rae[1] -= 2.0*M_PI;
//    
//    while (rae[1] < 0)
//        rae[1] += 2.0*M_PI;
//}
//
//inline void RAEtoXYZ(const std::array<float,3>& rae, std::array<float,3>& xyz)
//{
//    xyz[0] = rae[0]*std::sin(rae[2])*std::cos(rae[1]);
//    xyz[1] = rae[0]*std::cos(rae[2]);
//    xyz[2] = rae[0]*std::sin(rae[2])*std::sin(rae[1]);
//}



//// x assumed to be between -1.0 and +1.0
//static double raisedCosine(double x)
//{
//	double y;
//
//	// B between 0 (square pulse) and 1 (sinc-like pulse)
//    //	double B = 0.3;
//
//	//length = 1/T = 1.0 -> T = 1.0
//
//    //	if (fabs(x) <= (1-B)/2.0) {
//    //		y = 1;
//    //	} if (fabs(x) <= (1+B)/2.0) {
//    //		y = 0.5*(1.0 + cos(M_PI/B * (fabs(x) - (1.0-B)/2.0)));
//    //	} else {
//    //		y = 0;
//    //	}
//
//    //	y = cos(M_PI/2.0 * x);
//	y = 0.5*(1.0 + cos(M_PI * x));
//
//	return y;
//}

//// cubic interpolation of 4 equally spaced data points
////   mu is the parametric variable between 0 to 1 and gives the interpolated value between values y1 and y2
//static float cubicInterpolate(float y0, float y1, float y2, float y3, float mu)
//{
//    float a0,a1,a2,a3,mu2;
//
//    mu2 = mu*mu;
//    a0 = y3 - y2 - y0 + y1;
//    a1 = y0 - y1 - a0;
//    a2 = y2 - y0;
//    a3 = y1;
//
//    return (a0*mu*mu2 + a1*mu2 + a2*mu + a3);
//}

#endif  // Functions_h
