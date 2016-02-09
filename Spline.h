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

#ifndef __Spline_h__
#define __Spline_h__

#include <vector>

enum SplineShape
{
    LINEAR,     // linearly interpolates
    CUBIC,      // cubicly interpolates
    EMPTY       // a non existant spline
};

enum SplineBehavior
{
    FUNCTIONAL,
    PARAMETRIC
};

// the virtual base class to define the interface for an N-dimensional interpolating spline using N (a different N) surrounding points to determine its shape
template <typename T>
class Spline
{
public:
    // for polymorphic copying of splines, default copy constructors should be fine for splines as we have no pointers...
    virtual std::unique_ptr<Spline<T>> clone() = 0;
    virtual ~Spline() {};
    // a spline is pretty much an N-dimensional function f(val) = [y,z,a,...]
    virtual std::vector<T> pointAt(const T& val) = 0;
    //virtual void pointAt(const T& val, T* point) = 0;
    virtual void pointAt(const T& val, T** point) const = 0; // need this wackiness to be able to set the external pointer to nullptr for an empty spline, could just check spline type though...
    // calc spline from loaded points
    virtual void calc() = 0;
    // load from new interpolator point range centered about the spline and recalc spline
    virtual void calc(const std::vector<std::vector<T>>& new_points) = 0;
    SplineShape type;
protected:
    // points that are used to construct it
    std::vector<std::vector<T>> points;
};

// class to represent a nonexistent spline
template <typename T>
class EmptySpline : public Spline<T>
{
public:
    EmptySpline() { type = EMPTY; };
    std::unique_ptr<Spline<T>> clone() override { return std::unique_ptr<Spline<T>>(new EmptySpline<T>(*this)); };
    // return an empty vector to signal an open spline segment
    std::vector<T> pointAt(const T& val) override
    {
        std::vector<T> empty;
        return empty;
    };
    //void pointAt(const T& val, T* point) { /*point = nullptr;*/ }; // this don't set the external pointer
    void pointAt(const T& val, T** point) const override { *point = nullptr; }; // this do
    void calc() override {};
    void calc(const std::vector<std::vector<T>>& new_points) override {};
    using Spline<T>::type;
};

// virtual class for recomputing an N-point spline from a set of N points from an interpolator
template <typename T, const int N>
class NPointSpline : public virtual Spline<T>
{
public:
    void calc(const std::vector<std::vector<T>>& new_points) override
    {
        // example case for N = 4 pts:
        //points = {new_points[middle-2], new_points[middle-1], new_points[middle], new_points[middle+1]};
        const int dim = new_points[0].size();
        const int middle = (new_points.size()>>1);
        points.resize(N);
        int i = -1*(N>>1);
        for (int j = 0; j < N; ++j)
        {
            if (0 <= middle+i && middle+i < new_points.size())
                points[j] = new_points[middle+i];
            else // need to fill in some not yet needed points so that calc() below doesn't have a bad access crash when the interpolator (and spline) only has one specified point
                points[j] = std::vector<T>(dim, 0);
            ++i;
        }
        calc();
    }
private:
    using Spline<T>::calc;
    using Spline<T>::points;
};

#endif /* defined __Spline_h__ */
