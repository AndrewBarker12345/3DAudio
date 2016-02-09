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

// time domain convolution
inline void convolve(const float *x, int Nx, const float *h, int Nh, float *output) noexcept
{
    const int Nxm1 = Nx-1;
    const int Nhm1 = Nh-1;
    const int L = Nx+Nhm1;
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
        if (xf < Nxm1)
            ++xf;
        else if (hi < Nhm1)
            ++hi;
        
        if (i > Nhm1)
            ++xi;
    
        output[i] = sum;
    }
}

// only compute convolution for the output samples from beginIndex to endIndex inclusive
inline void convolve(const float *x, const int Nx, const float *h, const int Nh, float *output, const int beginIndex, const int endIndex) noexcept
{
    const int Nxm1 = Nx-1;
    const int Nhm1 = Nh-1;
    int xi, xf, hi;
    
    // find xi xf and hi starting at begin index
    if (beginIndex > Nhm1)
        xi = beginIndex - Nh;
    else
        xi = 0;
    
    if (beginIndex <= Nxm1)
    {
        xf = beginIndex;
        hi = 0;
    }
    else
    {
        xf = Nxm1;
        hi = beginIndex - Nxm1;
        if (hi > Nhm1)
            hi = Nhm1;
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
        if (xf < Nxm1)
            ++xf;
        else if (hi < Nhm1)
            ++hi;
        
        if (i > Nhm1)
            ++xi;
        
        output[i] = sum;
    }
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
    else  // avoid divide by zero
        rae[1] = 0;
    
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

#endif  // Functions_h
