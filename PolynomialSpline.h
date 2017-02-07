/*
 PolynomialSpline.h
 
 Polymorphic polynomial curves.

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

#ifndef __PolynomialSpline_h__
#define __PolynomialSpline_h__

#include "Spline.h"
#include "Polynomial.h"

// virtual class that stores the N polynomials needed for an N-dimensional polynomial spline
template <typename T>
class PolynomialSpline : public virtual Spline<T>
{
public:
    virtual std::vector<T> pointAt(const T& val) override;
    virtual void pointAt(const T& val, T** point) const override;
protected:
    std::vector<Polynomial<T>> spline;
};

// Class for cubic functional splines, a.k.a. f(x) = [y, z, ...]
template <typename T>
class CubicFunctionalSpline : public PolynomialSpline<T>, NPointSpline<T,4>
{
public:
    CubicFunctionalSpline() {}
    // load a set of points like: 0 -PREVIOUS SPLINE- 1 -THIS SPLINE- 2 -NEXT SPLINE- 3
    CubicFunctionalSpline(const std::vector<T>& p0, const std::vector<T>& p1,
                          const std::vector<T>& p2, const std::vector<T>& p3);
    void calc() override;
    std::vector<T> pointAt(const T& val) override;
    //void pointAt(const T& val, T* point) override;
    virtual void pointAt(const T& val, T** point) const override;
    std::unique_ptr<Spline<T>> clone() override { return std::make_unique<CubicFunctionalSpline<T>>(*this);}
    SplineShape getShape() const noexcept override { return SplineShape::CUBIC; }
private:
    using PolynomialSpline<T>::spline;
    using Spline<T>::points;
};
// used in doppler effect to support pre-allocation for to real time use
template <typename T, const int N>
class LightweightCubicFunctionalSpline
{
public:
    void allocate()
    {
        spline.resize(N - 1);
        spline.shrink_to_fit();
        for (auto& poly : spline)
            poly.allocate(4);
    };
    void calc(const T (&points)[4][N]) noexcept
    {
        splineStart = points[1][0];      // necessary for computing pointAt()
        
        T h_k, h_km1, h_kp1;             // first dim segment lengths surrounding the kth segment
        T delta_k, delta_km1, delta_kp1; // linear slopes of surrounding segments
        T d_k, d_kp1;                    // spline slopes at segment endpoints
        T a0, a1, a2, a3;                // cubic polynomial coefficients
        
        h_k   = 1.0 / (points[2][0]-points[1][0]);
        h_km1 = 1.0 / (points[1][0]-points[0][0]);
        h_kp1 = 1.0 / (points[3][0]-points[2][0]);
        
        for (int i = 1; i < N; ++i)
        {
            delta_k   = (points[2][i]-points[1][i]) * h_k;
            delta_km1 = (points[1][i]-points[0][i]) * h_km1;
            delta_kp1 = (points[3][i]-points[2][i]) * h_kp1;
            // check for NaNs
            if (delta_k != delta_k)
                delta_k = 0;
            if (delta_km1 != delta_km1)
                delta_km1 = 0;
            if (delta_kp1 != delta_kp1)
                delta_kp1 = 0;
            // if there is a change in sign between segment slopes or either slope is zero, make the boundary point a local min/max by setting the endpoint slope to 0,
            // otherwise the slopes at the points are the geometric mean of the linear slopes of the surrounding segments
            if (delta_k == 0 || delta_km1 == 0 || (delta_k < 0 && delta_km1 > 0) || (delta_k > 0 && delta_km1 < 0))
                d_k = 0;
            else
                d_k = 2.0 / (1.0/delta_km1 + 1.0/delta_k);
            
            if (delta_kp1 == 0 || delta_k == 0 || (delta_kp1 < 0 && delta_k > 0) || (delta_kp1 > 0 && delta_k < 0))
                d_kp1 = 0;
            else
                d_kp1 = 2.0 / (1.0/delta_k + 1.0/delta_kp1);
            
            a0 = points[1][i];
            a1 = d_k;
            a2 = (3.0*delta_k - 2.0*d_k - d_kp1) * h_k;
            a3 = (d_k - 2.0*delta_k + d_kp1) * (h_k * h_k);
            
            const T coeffs[4] = {a0,a1,a2,a3};
            const T exps[4]   = {0,1,2,3};
            spline[i-1].fill(coeffs, exps, 4);
        }
    };
    void pointAt(const T& val, T* point) const noexcept
    {
        const int numDimensions = spline.size();
        const T adjustedVal = val - splineStart;
        for (int i = 0; i < numDimensions; ++i)
            point[i] = spline[i](adjustedVal);
    };
private:
    std::vector<Polynomial<T>> spline;
    T splineStart;
};

// Class for cubic parametric splines, a.k.a. f(t) = [x, y, z, ...]
template <typename T>
class CubicParametricSpline : public PolynomialSpline<T>, NPointSpline<T,4>
{
public:
    CubicParametricSpline() {}
    // load a set of points like: 0 -PREVIOUS SPLINE- 1 -THIS SPLINE- 2 -NEXT SPLINE- 3
    CubicParametricSpline(const std::vector<T>& p0, const std::vector<T>& p1,
                          const std::vector<T>& p2, const std::vector<T>& p3);
    void calc() override;
    std::unique_ptr<Spline<T>> clone() override { return std::make_unique<CubicParametricSpline<T>>(*this); }
    SplineShape getShape() const noexcept override { return SplineShape::CUBIC; }
private:
    using PolynomialSpline<T>::spline;
    using Spline<T>::points;
};

// Class for linear functional splines, a.k.a. f(x) = [y, z, ...]
template <typename T>
class LinearFunctionalSpline : public PolynomialSpline<T>, NPointSpline<T,2>
{
public:
    LinearFunctionalSpline() {}
    // spline goes from p0 to p1
    LinearFunctionalSpline(const std::vector<T>& p0, const std::vector<T>& p1);
    void calc() override;
    std::unique_ptr<Spline<T>> clone() override { return std::make_unique<LinearFunctionalSpline<T>>(*this); }
     SplineShape getShape() const noexcept override { return SplineShape::LINEAR; }
private:
    using PolynomialSpline<T>::spline;
    using Spline<T>::points;
};

// Class for linear parametric splines, a.k.a. f(t) = [x, y, z, ...]
template <typename T>
class LinearParametricSpline : public PolynomialSpline<T>, NPointSpline<T,2>
{
public:
    LinearParametricSpline() {}
    // spline goes from p0 to p1 in a parametric distance of para_intrvl
    LinearParametricSpline(const std::vector<T>& p0, const std::vector<T>& p1);
    void calc() override;
    std::unique_ptr<Spline<T>> clone() override { return std::make_unique<LinearParametricSpline<T>>(*this); }
    SplineShape getShape() const noexcept override { return SplineShape::LINEAR; }
private:
    using PolynomialSpline<T>::spline;
    using Spline<T>::points;
};

template <typename T>
std::unique_ptr<Spline<T>> SplineFactory(const SplineShape shape,
                                         const SplineBehavior behavior)
{
    if (shape == SplineShape::EMPTY)
        return std::make_unique<EmptySpline<T>>();
    else if (shape == SplineShape::CUBIC) {
        if (behavior == SplineBehavior::FUNCTIONAL)
            return std::make_unique<CubicFunctionalSpline<T>>();
        else if (behavior == SplineBehavior::PARAMETRIC)
            return std::make_unique<CubicParametricSpline<T>>();
    }
    else if (shape == SplineShape::LINEAR) {
        if (behavior == SplineBehavior::FUNCTIONAL)
            return std::make_unique<LinearFunctionalSpline<T>>();
        else if (behavior == SplineBehavior::PARAMETRIC)
            return std::make_unique<LinearParametricSpline<T>>();
    }
    return nullptr;
}

// the implementations
template <typename T>
std::vector<T> PolynomialSpline<T>::pointAt(const T& val)
{
    std::vector<T> point (spline.size());
    for (int i = 0; i < point.size(); ++i)
        point[i] = spline[i](val);
    return point;
}
template <typename T>
void PolynomialSpline<T>::pointAt(const T& val, T** point) const
{
    const int numDimensions = spline.size();
    for (int i = 0; i < numDimensions; ++i)
        (*point)[i] = spline[i](val);
}

// cubic functional spline equation uses a substitution of s = x-x_k so adjust for that here
template <typename T>
std::vector<T> CubicFunctionalSpline<T>::pointAt(const T& val)
{
    std::vector<T> point (spline.size());
    const T adjustedVal = val - points[1][0];
    for (int i = 0; i < point.size(); ++i)
        point[i] = spline[i](adjustedVal);
    return point;
}
template <typename T>
void CubicFunctionalSpline<T>::pointAt(const T& val, T** point) const
{
    const int numDimensions = spline.size();
    const T adjustedVal = val - points[1][0];
    for (int i = 0; i < numDimensions; ++i)
        (*point)[i] = spline[i](adjustedVal);
}

template <typename T>
CubicFunctionalSpline<T>::CubicFunctionalSpline(const std::vector<T>& p0,
                                                const std::vector<T>& p1,
                                                const std::vector<T>& p2,
                                                const std::vector<T>& p3)
{
    points = {p0, p1, p2, p3};
    calc();
}

template <typename T>
void CubicFunctionalSpline<T>::calc()
{
    T h_k, h_km1, h_kp1;             // first dim segment lengths surrounding the kth segment
    T delta_k, delta_km1, delta_kp1; // linear slopes of surrounding segments
    T d_k, d_kp1;                    // spline slopes at segment endpoints
    T a0, a1, a2, a3;                // cubic polynomial coefficients
    
    h_k   = 1.0 / (points[2][0]-points[1][0]);
    h_km1 = 1.0 / (points[1][0]-points[0][0]);
    h_kp1 = 1.0 / (points[3][0]-points[2][0]);
    
    // check for NaNs
    if (h_k != h_k)
        h_k = 0;
    if (h_km1 != h_km1)
        h_km1 = 0;
    if (h_kp1 != h_kp1)
        h_kp1 = 0;
    
    // request size once before loop as opposed to push_back() in each iteration to be fast
    spline.resize(points[0].size()-1);
    for (int i = 1; i < points[0].size(); ++i)
    {
        delta_k   = (points[2][i]-points[1][i]) * h_k;
        delta_km1 = (points[1][i]-points[0][i]) * h_km1;
        delta_kp1 = (points[3][i]-points[2][i]) * h_kp1;
        
        // if there is a change in sign between segment slopes or either slope is zero, make the boundary point a local min/max by setting the endpoint slope to 0,
        // otherwise the slopes at the points are the geometric mean of the linear slopes of the surrounding segments
        if (delta_k == 0 || delta_km1 == 0 || (delta_k < 0 && delta_km1 > 0) || (delta_k > 0 && delta_km1 < 0))
            d_k = 0;
        else
            d_k = 2.0 / (1.0/delta_km1 + 1.0/delta_k);
        
        if (delta_kp1 == 0 || delta_k == 0 || (delta_kp1 < 0 && delta_k > 0) || (delta_kp1 > 0 && delta_k < 0))
            d_kp1 = 0;
        else
            d_kp1 = 2.0 / (1.0/delta_k + 1.0/delta_kp1);
        
        a0 = points[1][i];
        a1 = d_k;
        a2 = (3.0*delta_k - 2.0*d_k - d_kp1) * h_k;
        a3 = (d_k - 2.0*delta_k + d_kp1) * (h_k * h_k);
        
        spline[i-1] = Polynomial<T>({{a0,a1,a2,a3},{0,1,2,3}});
    }
}


template <typename T>
CubicParametricSpline<T>::CubicParametricSpline(const std::vector<T>& p0,
                                                const std::vector<T>& p1,
                                                const std::vector<T>& p2,
                                                const std::vector<T>& p3)
{
    points = {p0, p1, p2, p3};
    calc();
}

template <typename T>
void CubicParametricSpline<T>::calc()
{
    T m0, m1, m2;       // linear slopes of surrounding segments
    T t1, t2;           // spline slopes at segment entpoints
    T a0, a1, a2, a3;   // cubic polynomial coefficients
   
    spline.resize(points[0].size());
    for (int i = 0; i < points[0].size(); ++i)
    {
        // compute segment slopes
        m2 = (points[3][i]-points[2][i]);
        m1 = (points[2][i]-points[1][i]);
        m0 = (points[1][i]-points[0][i]);
        
        // compute interp slopes at segment boundaries by the arithmetic mean of surrounding segment slopes
        t1 = 0.5*(m0+m1);
        t2 = 0.5*(m1+m2);
        
        a0 = points[1][i];
        a1 = t1;
        a2 = (3.0*m1 - 2.0*t1 - t2);
        a3 = (t1 + t2 - 2.0*m1);
        
        spline[i] = Polynomial<T>({{a0,a1,a2,a3},{0,1,2,3}});
    }
}


template <typename T>
LinearFunctionalSpline<T>::LinearFunctionalSpline(const std::vector<T>& p0,
                                                  const std::vector<T>& p1)
{
    points = {p0, p1};
    calc();
}

template <typename T>
void LinearFunctionalSpline<T>::calc()
{
    // polynomial coefficients
    T m;       // slope for each dimension between the two points
    T b;       // the b in y = mx + b
    // first dim distance between points
    T dx = 1.0 / (points[1][0]-points[0][0]);
    spline.resize(points[0].size()-1);
    for (int i = 1; i < points[0].size(); ++i) {
        m = (points[1][i]-points[0][i]) * dx;
        b = points[1][i] - m*points[1][0];
        spline[i-1] = Polynomial<T>({{b,m},{0,1}});
    }
}


template <typename T>
LinearParametricSpline<T>::LinearParametricSpline(const std::vector<T>& p0,
                                                  const std::vector<T>& p1)
{
    points = {p0, p1};
    calc();
}

template <typename T>
void LinearParametricSpline<T>::calc()
{
    // polynomial coefficients
    T m;       // slope for each dimension between the two points
    T b;       // the b in y = mx + b
    spline.resize(points[0].size());
    for (int i = 0; i < points[0].size(); i++) {
        m = (points[1][i]-points[0][i]);
        b = points[0][i];
        spline[i] = Polynomial<T>({{b,m},{0,1}});
    }
}


#endif /* defined __PolynomialSpline_h__ */
