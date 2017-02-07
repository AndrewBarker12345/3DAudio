/*
 Interpolator.h
 
 N-dimensional interpolators of various type with polyporphic spline segments.

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

#ifndef __Interpolator__
#define __Interpolator__

#include "DrewLib.h"
#include "PolynomialSpline.h"
#include <list>
#include <atomic>
#include <algorithm>
#include <numeric>

// the types of actual, non-abstract interpolators
enum class InterpolatorType
{
    CLOSED_PARAMETRIC,
    OPEN_PARAMETRIC,
    FUNCTIONAL
};

template <typename T>
class SelectablePoint
{
public:
    SelectablePoint() : selected(false) {};
    SelectablePoint(const std::vector<T>& pt) : point(pt), selected(false) {};
    std::vector<T> point;
    bool selected;
};

/* Things I wish I did / didn't do:
 - don't make every single thing so damn polymorphic just for the sake of adhering to DRY, for instance i don't know if i will ever use Parametric/Functional Interpolators polymorphicly.  Open/Closed sure, but too much polymorphism can get in the way of future feature implementations and has as in the case with the moveSelectedPoints(WithReorderingInfo)()
 - don't ever use lists because Bjarne said so (poor memory locality is horrible for modern processors and caches are huge these days)
 - why do i store individual points as if each one can have a different dimensionality within the same interpolater and this can possibly represent something sensible
 - do i really need to store the selected info in two places (inside each point and also in the selected_points list), i know i did it for efficiency when looking at the selected state of just one point, but does complicate things
 ...
*/
// defines the interface for a multi spline interpolator and implements/keeps track of some common things as well
template <typename T>
class Interpolator
{
public:
    // think this was needed, but not sure, yep
    Interpolator() {};
    // copy constructor
    Interpolator(const Interpolator& interp)
    {
        // shallow copiable
        points = interp.points; // default SelectablePoint copy/assignment should do...
        selected_points = interp.selected_points;
        listeners = interp.listeners;
        //changed = interp.changed;
        spline_type = interp.spline_type;
        max_pts_per_spline = interp.max_pts_per_spline;
        //type = interp.type;
        // deep copy the pointers
        splines.resize(interp.splines.size());
        for (int i = 0; i < splines.size(); ++i)
            splines[i] = interp.splines[i]->clone();
    };
    // assignment operator
    virtual Interpolator& operator= (const Interpolator& interp)
    {
        // check for self-assignment
        if (this == &interp)
            return *this;
        // deallocate previous stuff
        splines.clear();
        // non dynamically allocated data can be shallow copied
        points = interp.points; // default SelectablePoint copy/assignment should do...
        selected_points = interp.selected_points;
        listeners = interp.listeners;
        //changed = interp.changed;
        spline_type = interp.spline_type;
        max_pts_per_spline = interp.max_pts_per_spline;
        //type = interp.type;
        // need to deep copy pointers that are not null
        splines.resize(interp.splines.size());
        for (int i = 0; i < splines.size(); ++i)
            splines[i] = interp.splines[i]->clone();
        return *this;
    };
    // C++ is so much fun
    virtual ~Interpolator() {};
    // get the input value range [begin,end] for the interpolator
    virtual std::array<T, 2> getInputRange() const = 0;
    virtual void getInputRangeQuick(T (&range)[2]) const noexcept = 0;
    // returns 1 on successful output, 0 for out of bounds input value or an empty spline segment
    virtual int pointAt(T val, std::vector<T>& point) const = 0;
    // adds a point (at the end for para interps, and in order for functional interps)
    virtual void addPoint(const std::vector<T>& point) = 0;
    // adds a point at the index specified (func interps should check if the ordering is valid)
    //virtual void addPoint(const std::vector<T>& point, int index) = 0;
    // move the selected points in each dimension specified by delta, return the number of points moved
    virtual int moveSelectedPoints(const std::vector<T>& delta) = 0;
    // copy the selected points by duplicating them at their current position, unselecting the originals, and leaving the duplicates selected, returns num copied
    virtual int copySelectedPoints() = 0;
//    // set the point at the index to the new position (functional interps need to fix potential changes in ordering)
//    virtual void setPointPosition(const std::vector<T>& new_pos, int index) = 0;
//    // ? virtual SplineShape getSplineType(int index);
//    // ? virtual std::vector<SplineShape> getSplineTypes();
    // set the splines boardered by selected points on both sides to the new spline type
    virtual int setSelectedSplinesType(SplineShape new_spline_type) = 0;
    /** what kinda interp we got here? */
    virtual InterpolatorType getType() const noexcept = 0;
    //virtual T getMaxDistanceFrom(const std::vector<T>& point);
    // delete all the selected points, returns number of points deleted
    int deleteSelectedPoints();
    // set a single point's selected state
    void setPointSelected(int index, bool new_selected);
//    // set selected state for all the points at the list of indecies specified
//    void setPointsSelected(const std::vector<int>& indecies, bool new_selected);
//    // set the points' selected state to the cooresponding specified state array
//    void setPointsSelected(const std::vector<bool>& new_points_selected);
    // select/unselect all points
    void setAllPointsSelected(bool new_selected);
    // return an array of points' selected state
    std::vector<bool> getPointsSelected() const;
    // return the selected state of one point
    bool getPointSelected(int index) const;
    // return an array of the points that are selected
    std::vector<std::vector<T>> getSelectedPoints() const;
    /** returns an ordered vector of indices of the unselected points */
    std::vector<int> getUnselectedPointIndices() const;
    // return the indecies of the selected points
    std::vector<int> getSelectedPointIndices() const;
    // same functionality as above, but more obviously returns the points indecies
    //std::vector<std::tuple<std::vector<float>, int>> getSelectedPointsWithIndecies() const;
    // get just one selected point (and its absolute index) given its relative index among only the selected points
    std::tuple<std::vector<T>, int> getSelectedPoint(int indexAmongSelecteds) const;
    // get all the interpolator's points
    std::vector<std::vector<T>> getPoints() const;
    // get the interpolator's point at specified index
    std::vector<T> getPoint(int index) const;
    const std::vector<SelectablePoint<T>>& getSelectablePoints() const;
    //std::vector<std::vector<T>>* getPointsPtr() { return &points; };
    // get just the number of interpolator's points
    int getNumPoints() const noexcept { return points.size(); };
    int getNumSelectedPoints() const noexcept { return selected_points.size(); };
    int getNumDimensions() const noexcept { return (points.size() > 0 ? points[0].point.size() : 0); }
    // used to be just a helper function for setSelectedSplinesType() in base classes
    std::vector<int> getSelectedSplines() const;
    // get the shape of i-th spline segment
    SplineShape getSplineShape(int splineIndex) const noexcept;
    // object that can be informed when an interpolator changes
    class Listener
    {
    public:
        Listener& operator= (const Listener& other)
        {
            if (this != &other)
                changed.store(other.changed.load());
            return *this;
        }
        std::atomic<bool> changed {false};
    };
    void addListener(Listener* listener) noexcept { listeners.emplace_back(listener); };
    void removeListeners() noexcept { listeners.clear(); };
   
protected:
    // recalc existing splines in specified index range [begin, end), useful for derived class specific bounds checking
    virtual void calcSplinesInRange(int begin, int end) = 0;
    // get needed boarder points to calc spline i
    virtual void calcSplineAt(std::size_t index) = 0;
    // called after edits to the interp's points for the child classes to optionally override to get a callback to update things that might need to change (OpenEndedInterp's ext points for instance)
    virtual void informChildren() {};
    // helper function for setSelectedSplinesType() in base classes
    //std::vector<int> getSelectedSplines();
    // the interactive points of the interpolator
    std::vector<SelectablePoint<T>> points;
    // ? the selected state for each corresponding point
    // ? std::vector<bool> point_selecteds;
    // sorted indecies of only the points that are selected, for quick access to just the selected points
    std::list<int> selected_points; // probably should be a vector since lists are bad for cache locality, but i didn't realize this at the time and i like not having to fix bugs so yah. doesn't seem to be a performance bottleneck anyways...
    // the splines that connect each pair of points in the interp, spline i is surrounded by points i and i+1
    std::vector<std::unique_ptr<Spline<T>>> splines;
    // what is the spline type generated in the constructor (can be modifed later on using setSelectedSplinesType())
    SplineShape spline_type = SplineShape::CUBIC;
    // max number of points that a spline might use for its shape
    unsigned char max_pts_per_spline = 4;
    // mark the listeners that the interp has changed
    void informListenersOfChange() noexcept { for (auto& l : listeners) { l->changed = true; } };
    // all the objects that want to be informed of changes to the interp
    std::vector<Listener*> listeners;
};

// an interpolator where the last point connects back to first point in a closed loop
template <typename T>
class ClosedEndedInterpolator : public virtual Interpolator<T>
{
public:
    virtual ~ClosedEndedInterpolator() {};
    // recalc existing splines in specified index range [begin, end)
    void calcSplinesInRange(int begin, int end) override;
    // recalc existing splines with polymorphism
    void calcSplineAt(std::size_t index) override;
    using Interpolator<T>::getPoints;
private:
    using Interpolator<T>::points;
    using Interpolator<T>::splines;
    using Interpolator<T>::spline_type;
    using Interpolator<T>::selected_points;
    using Interpolator<T>::max_pts_per_spline;
};

// an interpolator with different, unconnected begin and end points
template <typename T>
class OpenEndedInterpolator : public virtual Interpolator<T>
{
public:
    virtual ~OpenEndedInterpolator() {};
    // think this was needed, but not sure, yep
    OpenEndedInterpolator() {};
    // load interp with a set of points, slopes at endpoints are determined by using two imaginary linear extension points on either end
    OpenEndedInterpolator(const std::vector<std::vector<T>>& new_points);
    // calculates begin/end ext pts from current loaded points
    virtual void calcExtPts() = 0;
    // spline initialization is dependent on func/para base class's SplineBehavior
    virtual void initSplines() = 0;
    virtual void initSplines(const std::vector<SplineShape>& new_splines) = 0;
    // recalc existing splines in specified index range [begin, end)
    void calcSplinesInRange(int begin, int end) override;
    // recalc existing splines with polymorphism
    void calcSplineAt(std::size_t index) override;
    // want to update ext pts when Interpolator's pts are edited via parent class
    void informChildren() override { calcExtPts(); };
    using Interpolator<T>::getPoints;
protected:
    // the two imaginary linear extension points
    std::vector<T> begin_ext_pt;
    std::vector<T> end_ext_pt;
private:
    using Interpolator<T>::points;
    using Interpolator<T>::splines;
    using Interpolator<T>::spline_type;
    using Interpolator<T>::selected_points;
    using Interpolator<T>::max_pts_per_spline;
};

// parametric interps should treat the input value as a parametric value (the range is assumed to be from 0 to 1)
template <typename T>
class ParametricInterpolator : public virtual Interpolator<T>
{
public:
//    ParametricInterpolator() {};
    virtual ~ParametricInterpolator()  {};
//    ParametricInterpolator(const ParametricInterpolator& interp)
//        : Interpolator<T>(interp) {};
    ParametricInterpolator& operator= (const ParametricInterpolator& interp)
    {
        return Interpolator<T>::operator= (interp);
    }
    // for polymorphic copying of Closed/OpenParaInterps
    virtual std::unique_ptr<ParametricInterpolator<T>> clone() = 0;
    // get the input value range [begin,end] for the interpolator
    virtual void getInputRangeQuick(T (&range)[2]) const noexcept override { range[0] = 0; range[1] = splines.size(); };
    virtual std::array<T, 2> getInputRange() const override { return {0, static_cast<T>(splines.size())}; };
    // get the point at input of val
    int pointAt(T val, std::vector<T>& point) const override;
    int pointAt(T val, T* point) const;
    // set the splines boardered by selected points on both sides to the new spline type
    int setSelectedSplinesType(SplineShape new_spline_type) override;
    // adds a point (at the end for para interps)
    void addPoint(const std::vector<T>& point) override;
    // inserts a point at the specified index, shifting those after back by one
    virtual void addPoint(const std::vector<T>& point, int index) = 0;
    // moves the only selected point to index and shifts points after index by one spot
    //void setSelectedPointIndex(int index);
    /** moves the point at pointIndex to newIndex and rotates all the other selected points in index with the same delta */
    virtual void setSelectedPointIndices(int pointIndex, int newIndex) = 0;
    // move the selected points in each dimension specified by delta, return the number of points moved
    int moveSelectedPoints(const std::vector<T>& delta) override;
    // set the point at the index to the new position
    void setPointPosition(const std::vector<T>& new_pos, int index)
    {   //bool pt_selected = points[index].selected;
        points[index].point = new_pos;
        informListenersOfChange();
        //points[index].selected = pt_selected;
    };
    // quick and dirty way to have an interpolator recomputed by the user, added to support setPointPosition()
    void recalcSplines() { informChildren(); calcSplinesInRange(0, splines.size()); informListenersOfChange(); /*changed = true;*/ };
//    // another quick and dirty...
//    std::vector<int> getSelectedPointIndicies() { return std::vector<int> (selected_points.begin(), selected_points.end()); };
    
    //using Interpolator<T>::changed;
    //using Interpolator<T>::type;
    using Interpolator<T>::getSelectedSplines;
private:
    using Interpolator<T>::splines;
    using Interpolator<T>::points;
    using Interpolator<T>::selected_points;
    using Interpolator<T>::max_pts_per_spline;
    using Interpolator<T>::spline_type;
    //using Interpolator<T>::getSelectedSplines;
    using Interpolator<T>::calcSplineAt;
    using Interpolator<T>::calcSplinesInRange;
    using Interpolator<T>::informChildren;
    using Interpolator<T>::informListenersOfChange;
};

// see below
template <typename T>
class OpenParametricInterpolator;

// a.k.a.  f(t) = [x, y, z, ...]
template <typename T>                   // i don't know why i have to put a public for each, if not there were some issues
class ClosedParametricInterpolator : public ParametricInterpolator<T>, public ClosedEndedInterpolator<T>
{
    friend class OpenParametricInterpolator<T>;
public:
    ClosedParametricInterpolator()
        : Interpolator<T>(InterpolatorType::CLOSED_PARAMETRIC) {};
    // new interpolator constructed in order from list of points
    ClosedParametricInterpolator(const std::vector<std::vector<T>>& new_points);
    //                                                           ... or points and spline shapes
    ClosedParametricInterpolator(const std::vector<std::vector<T>>& new_points, const std::vector<SplineShape>& new_splines);
    // construct a closed para interp from an open para interp
    ClosedParametricInterpolator<T>(const OpenParametricInterpolator<T>& open_interp);
//        :  ClosedParametricInterpolator<T>(open_interp.getPoints())
//    {   // copy over the selected state stuff as well
//        const std::vector<int> selected_indecies = open_interp.getSelectedPointIndicies();
//        selected_points = std::list<int>(selected_indecies.begin(), selected_indecies.end());
//        for (auto i : selected_points)
//            points[i].selected = true;
//        //bool hey_dingus = true;
//    };
    // clone ClosedParaInterp as a ParaInterp
    std::unique_ptr<ParametricInterpolator<T>> clone() override { return std::unique_ptr<ParametricInterpolator<T>>(new ClosedParametricInterpolator<T>(*this)); };
    // inserts a point at the specified index, shifting those after back by one
    void addPoint(const std::vector<T>& point, int index) override;
    /** moves the point at pointIndex to newIndex and rotates all the other selected points in index with the same delta */
    void setSelectedPointIndices(int pointIndex, int newIndex) override;
    // copy the selected points by duplicating them at their current position, appending the coppied points to the end, unselecting the originals, and leaving the duplicates selected, returns num copied
    int copySelectedPoints() override;
    InterpolatorType getType() const noexcept override { return InterpolatorType::CLOSED_PARAMETRIC; }
    //using Interpolator<T>::changed;
private:
    using Interpolator<T>::points;
    using Interpolator<T>::selected_points;
    using Interpolator<T>::max_pts_per_spline;
    using Interpolator<T>::splines;
    using Interpolator<T>::spline_type;
    using Interpolator<T>::informListenersOfChange;
    using ClosedEndedInterpolator<T>::calcSplineAt;
    using ClosedEndedInterpolator<T>::calcSplinesInRange;
};

// a.k.a.  f(t) = [x, y, z, ...]
template <typename T>
class OpenParametricInterpolator : public ParametricInterpolator<T>, public OpenEndedInterpolator<T>
{
    friend class ClosedParametricInterpolator<T>;
public:
    OpenParametricInterpolator() {}
        //: Interpolator<T>(InterpolatorType::OPEN_PARAMETRIC) { };
    // new interpolator constructed in order from list of points
    OpenParametricInterpolator(const std::vector<std::vector<T>>& new_points)
        :/*Interpolator<T>(InterpolatorType::OPEN_PARAMETRIC),*/
          OpenEndedInterpolator<T>(new_points) { calcExtPts(); initSplines(); };
    OpenParametricInterpolator(const std::vector<std::vector<T>>& new_points,
                               const std::vector<SplineShape>& new_splines)
        :/*Interpolator<T>(InterpolatorType::OPEN_PARAMETRIC),*/
          OpenEndedInterpolator<T>(new_points) { calcExtPts(); initSplines(new_splines); };
    // construct an open para interp from a closed para interp
    OpenParametricInterpolator(const ClosedParametricInterpolator<T>& closed_interp);
//        : OpenParametricInterpolator<T>(closed_interp.getPoints()) {};
    // clone OpenParaInterp as a ParaInterp
    std::unique_ptr<ParametricInterpolator<T>> clone() override { return std::unique_ptr<ParametricInterpolator<T>>(new OpenParametricInterpolator<T>(*this)); };
    // initialize the splines from current points
    void initSplines() override;
    // ... or with splines
    void initSplines(const std::vector<SplineShape>& new_splines) override;
    // inserts a point at the specified index, shifting those after back by one
    void addPoint(const std::vector<T>& point, int index) override;
    /** moves the point at pointIndex to newIndex and rotates all the other selected points in index with the same delta */
    void setSelectedPointIndices(int pointIndex, int newIndex) override;
    // copy the selected points by duplicating them at their current position, appending the coppied points to the end, unselecting the originals, and leaving the duplicates selected, returns num copied
    int copySelectedPoints() override;
    void calcExtPts() override;
    InterpolatorType getType() const noexcept override { return InterpolatorType::OPEN_PARAMETRIC; }
    //using Interpolator<T>::changed;
private:
    using Interpolator<T>::points;
    using Interpolator<T>::selected_points;
    using Interpolator<T>::max_pts_per_spline;
    using Interpolator<T>::splines;
    using Interpolator<T>::spline_type;
    using Interpolator<T>::informListenersOfChange;
    using OpenEndedInterpolator<T>::calcExtPts;
    using OpenEndedInterpolator<T>::calcSplineAt;
    using OpenEndedInterpolator<T>::calcSplinesInRange;
    using OpenEndedInterpolator<T>::begin_ext_pt;
    using OpenEndedInterpolator<T>::end_ext_pt;
};

// a.k.a.  f(x) = [y, z, a, ...]; different beg/end pts and the higher dimension points must be a function of the first dimension.
template <typename T>
class FunctionalInterpolator : public OpenEndedInterpolator<T>
{
public:
    FunctionalInterpolator() {}// : Interpolator<T>(InterpolatorType::FUNCTIONAL) {};
    // the higher dimension points must be a function of the first dimension
    FunctionalInterpolator(const std::vector<std::vector<T>>& new_points)
        :/* Interpolator<T>(InterpolatorType::FUNCTIONAL),*/
          OpenEndedInterpolator<T>(new_points) { calcExtPts(); initSplines(); }
    FunctionalInterpolator(const std::vector<std::vector<T>>& new_points,
                           const std::vector<SplineShape>& new_splines)
        : /*Interpolator<T>(InterpolatorType::FUNCTIONAL),*/
          OpenEndedInterpolator<T>(new_points) { calcExtPts(); initSplines(new_splines); }
    // initialize the splines from current points
    void initSplines() override;
    // ... or with splines
    void initSplines(const std::vector<SplineShape>& new_splines) override;
    // get the input value range [begin,end] for the interpolator
    void getInputRangeQuick(T (&range)[2]) const noexcept override
    {
        if (points.size() > 0)
        {
            range[0] = points.front().point[0];
            range[1] = points.back().point[0];
        }
        else
        {
            range[0] = 0;
            range[1] = 0;
        }
    };
    std::array<T, 2> getInputRange() const override
    {
        if (points.size() > 0)
            return {points.front().point[0], points.back().point[0]};
        else
            return {0, 0};
    };
    // fills point with the interp point at a specified value. returns 1 on successful output, 0 for out of bounds input value or an empty spline segment
    int pointAt(T val, std::vector<T>& point) const override;
    // gets a point using a specified index for efficiency in searching through the points, modifies the index passed in to the correct if the input val is at a different index than specified
    //int pointAtSmart(T val, std::vector<T>& point, int& index) const;
    int pointAtSmart(T val, T* point, int& spline_index) const;
    // set the splines boardered by selected points on both sides to the new spline type
    int setSelectedSplinesType(SplineShape new_spline_type) override;
    // adds a point (in order for functional interps)
    void addPoint(const std::vector<T>& point) override;
    // ? add a point at the specified index, for copy() to use ?
    //void addPoint(const std::vector<T>& point, int index) override {};
    // move the selected points in each dimension specified by delta, return the number of points moved
    int moveSelectedPoints(const std::vector<T>& delta) override;
    // this one is specific to FunctionalInterpolators due to it returning reordering info of the points
    //std::vector<int> moveSelectedPointsWithReorderingInfo(const std::vector<T>& delta);
    // sort those odd, but ingenious SelectablePoints
    void sort(std::vector<SelectablePoint<T>>& points);
    // copy the selected points by duplicating them at their current position, unselecting the originals, and leaving the duplicates selected, returns num copied
    int copySelectedPoints() override;
    void calcExtPts() override;
    InterpolatorType getType() const noexcept override { return InterpolatorType::FUNCTIONAL; }
    // used when drawing to skip over an empty segmentvv
//    float getNextSplineBegin()
//    {
//        if (points.size() > prev_spline_index+1)
//            return points[prev_spline_index+1].point[0];
//        else
//            return 0;
//    };
//    int prev_spline_index = 0;
    //using Interpolator<T>::changed;
    using Interpolator<T>::getSelectedSplines;
private:
    using Interpolator<T>::points;
    using Interpolator<T>::selected_points;
    using Interpolator<T>::splines;
    using Interpolator<T>::spline_type;
    using Interpolator<T>::max_pts_per_spline;
    using Interpolator<T>::informListenersOfChange;
    //using Interpolator<T>::getSelectedSplines;
    using OpenEndedInterpolator<T>::calcSplinesInRange;
    using OpenEndedInterpolator<T>::calcSplineAt;
    using OpenEndedInterpolator<T>::calcExtPts;
    using OpenEndedInterpolator<T>::begin_ext_pt;
    using OpenEndedInterpolator<T>::end_ext_pt;
};

// can be used to correctly sort some points for creating a FunctionalInterpolator
template <typename T>
void functionalSort(std::vector<std::vector<T>>& new_points)
{
    std::sort(new_points.begin(), new_points.end(), [](const std::vector<T>& p1,
                                                       const std::vector<T>& p2)
                                                      {return p1[0] < p2[0];} );
}

template <typename T>
std::unique_ptr<Interpolator<T>> InterpolatorFactory(InterpolatorType type,
                                                     const std::vector<std::vector<T>>& points)
{
    switch (type)
    {
//        case InterpolatorType::CLOSED_PARAMETRIC:
//            return std::unique_ptr<Interpolator<T>>(new ClosedParametricInterpolator<T>(points));
//        case InterpolatorType::OPEN_PARAMETRIC:
//            return std::unique_ptr<Interpolator<T>>(new OpenParametricInterpolator<T>(points));
//        case InterpolatorType::FUNCTIONAL:
//            return std::unique_ptr<Interpolator<T>>(new FunctionalInterpolator<T>(points));
            
        case InterpolatorType::CLOSED_PARAMETRIC:
            return std::make_unique<ClosedParametricInterpolator<T>>(points); //std::unique_ptr<Interpolator<T>>(new ClosedParametricInterpolator<T>(points));
        case InterpolatorType::OPEN_PARAMETRIC:
            return std::make_unique<OpenParametricInterpolator<T>>(points); //std::unique_ptr<Interpolator<T>>(new OpenParametricInterpolator<T>(points));
        case InterpolatorType::FUNCTIONAL:
            return std::make_unique<FunctionalInterpolator<T>>(points); //std::unique_ptr<Interpolator<T>>(new FunctionalInterpolator<T>(points));
    }
}

template <typename T>
std::unique_ptr<Interpolator<T>> InterpolatorFactory(InterpolatorType type,
                                                     const std::vector<std::vector<T>>& points,
                                                     const std::vector<SplineShape>& splines)
{
    switch (type)
    {
        case InterpolatorType::CLOSED_PARAMETRIC:
            return std::make_unique<ClosedParametricInterpolator<T>>(points, splines);
        case InterpolatorType::OPEN_PARAMETRIC:
            return std::make_unique<OpenParametricInterpolator<T>>(points, splines);
        case InterpolatorType::FUNCTIONAL:
            return std::make_unique<FunctionalInterpolator<T>>(points, splines);
    }
}

// the implementation
//#include "Interpolator_impl.h"

template <typename T>
void Interpolator<T>::setPointSelected(int index, bool new_selected)
{
    points[index].selected = new_selected;
    if (new_selected)
    {
        selected_points.emplace_back(index);
        selected_points.sort();
        selected_points.unique();
    }
    else
        selected_points.remove(index);
    informListenersOfChange();
}

//template <typename T>
//void Interpolator<T>::setPointsSelected(const std::vector<int>& indecies, bool new_selected)
//{
//    bool need_sorting = false;
//    for (int index : indecies)
//    {
//        points[index].selected = new_selected;
//        if (new_selected)
//        {
//            selected_points.emplace_back(index);
//            need_sorting = true;
//        }
//        else
//            selected_points.remove(index);
//    }
//    if (need_sorting)
//    {
//        selected_points.sort();
//        selected_points.unique();
//    }
//}
//
//template <typename T>
//void Interpolator<T>::setPointsSelected(const std::vector<bool>& new_points_selected)
//{
//    int N = std::min(points.size(), new_points_selected.size());
//    selected_points.clear();
//    for (int i = 0; i < N; ++i)
//    {
//        points[i].selected = new_points_selected[i];
//        if (points[i].selected)
//            selected_points.emplace_back(i);
//    }
//}

template <typename T>
void Interpolator<T>::setAllPointsSelected(const bool new_selected)
{
    if (new_selected)
    {
        selected_points.clear();
        for (int i = 0; i < points.size(); ++i)
        {
            points[i].selected = new_selected;
            selected_points.emplace_back(i);
        }
    }
    else
    {
        for (int i = 0; i < points.size(); ++i)
            points[i].selected = new_selected;
        selected_points.clear();
    }
    informListenersOfChange();
}

template <typename T>
std::vector<bool> Interpolator<T>::getPointsSelected() const
{
    std::vector<bool> points_selected (points.size());
    for (int i = 0; i < points.size(); ++i)
        points_selected[i] = points[i].selected;
    return points_selected;
}

template <typename T>
bool Interpolator<T>::getPointSelected(const int index) const
{
    return points[index].selected;
}

template <typename T>
std::vector<std::vector<T>> Interpolator<T>::getSelectedPoints() const
{
    std::vector<std::vector<T>> sel_pts (selected_points.size());
    int j = 0;
    for (auto i : selected_points)
    {
        sel_pts[j++] = points[i].point;
        //sel_pts[j++].emplace_back(i); // return the index, but in a sneaky way...
        //++j;
    }
    return sel_pts;
}

template <class T>
std::vector<int> Interpolator<T>::getUnselectedPointIndices() const
{
    std::vector<int> x (getNumPoints() - getNumSelectedPoints());
    for (int i = 0, j = 0; i < getNumPoints(); ++i)
        if (! getPointSelected(i))
            x[j++] = i;
    return x;
}

template <typename T>
std::vector<int> Interpolator<T>::getSelectedPointIndices() const
{
    return std::vector<int> (selected_points.begin(), selected_points.end());
}

template <typename T>
std::tuple<std::vector<T>, int> Interpolator<T>::getSelectedPoint(const int indexAmongSelecteds) const
{
    //return { points[selected_points[indexAmongSelecteds]].point, selected_points[indexAmongSelecteds] }; // why did i use a list!?!
    int j = 0;
    for (auto i : selected_points)
        if (j++ == indexAmongSelecteds)
            return std::make_tuple(points[i].point, i);//{ points[i].point, i };
    return std::make_tuple(std::vector<T>(), 0); // return a meaningless something;
}

//template <typename T>
//std::vector<std::tuple<std::vector<float>, int>> Interpolator<T>::getSelectedPointsWithIndecies() const
//{
//    std::vector<std::tuple<std::vector<T>, int>> sel_pts (selected_points.size());
//    int j = 0;
//    for (auto i : selected_points)
//    {
//        sel_pts[j++] = std::make_tuple(points[i].point, i);//{ points[i].point, i };
//    }
//    return sel_pts;
//}

template <typename T>
std::vector<std::vector<T>> Interpolator<T>::getPoints() const
{
    std::vector<std::vector<T>> pts (points.size());
    for (int i = 0; i < pts.size(); ++i)
        pts[i] = points[i].point;
    return pts;
}

template <typename T>
std::vector<T> Interpolator<T>::getPoint(const int index) const
{
    return points[index].point;
}

template <typename T>
const std::vector<SelectablePoint<T>>& Interpolator<T>::getSelectablePoints() const
{
    return points;
}

template <typename T>
std::vector<int> Interpolator<T>::getSelectedSplines() const
{
    std::vector<int> sel_splines;
    sel_splines.reserve(selected_points.size());
    int prev_selected_index;
    for (auto i : selected_points)
    {
        // if we haven't found the beginning of a new selected chunk
        if (i != selected_points.front() && i == prev_selected_index + 1)
            sel_splines.emplace_back(prev_selected_index);
        prev_selected_index = i;
    }
    return sel_splines;
}

template <typename T>
SplineShape Interpolator<T>::getSplineShape(const int splineIndex) const noexcept
{
    if (0 <= splineIndex && splineIndex < splines.size())
        return splines[splineIndex]->getShape();
    else
        return static_cast<SplineShape>(-1);
}

template <typename T>
int Interpolator<T>::deleteSelectedPoints()
{
    // all points are selected and deleted
    int num_deleted = selected_points.size();
    if (num_deleted == points.size())
    {
        points.clear();
        selected_points.clear();
        splines.clear();
        return num_deleted;
    }
    int prev_selected_index;
    std::list<int> delete_chunks; // beginning indecies of each group of consecutive selected points, adjusted to be the corresponding index in the points/splines arrays after all the deletions have been made
    int deletes = 0; // # deletes thus far in loop below
    int relative_index; // relative index taking into account # deletes thus far
    for (auto i : selected_points)
    {
        relative_index = i - deletes;
        if (i == selected_points.front())
            delete_chunks.emplace_back(i);   // add first delete chunk start index
        else if (i != prev_selected_index + 1)    // found beginning of a new delete chunk
            delete_chunks.emplace_back(relative_index);
        // do deletions
        points.erase(points.begin() + relative_index); // crashed here before when deleting pathPos pts after copying, moving, and auto aligning them. selected_points was out of order and had more points than points in the interpolator... (relative_index was negative), think this was fixed to by handling FunctionalInterpolator path points of the same x value correctly...
        if (relative_index < splines.size())
            splines.erase(splines.begin() + relative_index);
        // advance stuff
        prev_selected_index = i;
        ++deletes;
    }
    if (num_deleted > 0)
    {
        // gotta deal with this somehow otherwise if the last point is among the deleted we have a number of splines that is equal to points which gets things screwy
        if (getType()/*type*/ == InterpolatorType::OPEN_PARAMETRIC && points.size() > 0 && splines.size() != points.size()-1)
            splines.pop_back();
        selected_points.clear();
        // used for a callback to OpenEndedInterpolator so it can recalculate its extended points, which are needed for calcSplineAt() below
        informChildren();
        // recalculate each spline that needs it
        int start, end; // spline indecies
        int d = (max_pts_per_spline>>1);
        for (auto i : delete_chunks)
        {
            if (i == delete_chunks.front())
            {
                start = i - d;
                end = i + d;
            }
            else
            {
                // spline indecies needing update from two delete blocks might overlap
                start = std::max(end, i - d);
                end = i + d;
            }
            // bounds checking included as per subclass needs
            calcSplinesInRange(start, end);
        }
        informListenersOfChange();
        //changed = true;
    }
    return num_deleted;
}

template <typename T>
void ClosedEndedInterpolator<T>::calcSplinesInRange(int begin, int end)
{
    if (begin < 0 && end > splines.size())
    {
        begin = 0;
        end = splines.size();
    }
    else if (begin < 0)
    {
        for (int i = begin + splines.size(); i < splines.size(); ++i)
            calcSplineAt(i);
        begin = 0;
        if (end > begin + splines.size())
            end = begin + splines.size();
    }
    else if (end > splines.size())
    {
        for (int i = 0; i < end - splines.size(); ++i)
            calcSplineAt(i);
        end = splines.size();
        if (begin < end - splines.size())
            begin = end - splines.size();
    }

    for (int i = begin; i < end; ++i)
        calcSplineAt(i);

//    bool wrapped = false;
//    while (begin < 0)
//    {
//        begin += splines.size();
//        wrapped = true;
//    }
//    while (end > splines.size())
//    {
//        end -= splines.size();
//        wrapped = true;
//    }
//    if (wrapped)
//    {
//        for (int i = begin; i < splines.size(); ++i)
//            calcSplineAt(i);
//        for (int i = 0; i < end; ++i)
//            calcSplineAt(i);
//    }
//    else
//    {
//        for (int i = begin; i < end; ++i)
//            calcSplineAt(i);
//    }
}

template <typename T>
void OpenEndedInterpolator<T>::calcSplinesInRange(int begin, int end)
{
    if (begin < 0)
        begin = 0;
    if (end > splines.size())
        end = splines.size();
    for (int i = begin; i < end; ++i)
        calcSplineAt(i);
}

template <typename T>
void ClosedEndedInterpolator<T>::calcSplineAt(const std::size_t index)
{
    if (index >= splines.size()) // there was a crash below when deleting all but one of many points, this should fix that
        return;
    int b = index - (max_pts_per_spline>>1) + 1;
    int e = index + (max_pts_per_spline>>1) + 1;
    bool wrapped = false;
    while (b < 0)
    {
        b += points.size();
        wrapped = true;
    }
    while (e > points.size())
    {
        e -= points.size();
        wrapped = true;
    }
    std::vector<std::vector<T>> points_a;
    const std::vector<std::vector<T>> pts = getPoints();
    if (wrapped)
    {
        points_a = std::vector<std::vector<T>> (pts.begin() + b, pts.end());
        std::vector<std::vector<T>> points_b (pts.begin(), pts.begin() + e);
        points_a.insert(points_a.end(), points_b.begin(), points_b.end());
        // quick fix b/c points_a.size() might be = 2, rather than 4 (or max_pts_per_spline)
        if (points_a.size() == 2)
        {
            points_a.push_back(points_a[0]);
            points_a.push_back(points_a[1]);
        }
    }
    else
    {
        points_a = std::vector<std::vector<T>> (pts.begin() + b, pts.begin() + e);
    }
    splines[index]->calc(points_a);
}

template <typename T>
void OpenEndedInterpolator<T>::calcSplineAt(const std::size_t index)
{
    if (index >= splines.size())
        return;
    int b = index - (max_pts_per_spline>>1) + 1;
    int e = index + (max_pts_per_spline>>1) + 1;
    bool begin_wrapped = false, end_wrapped = false;
    while (b < 0)
    {
        b += points.size();
        begin_wrapped = true;
    }
    while (e > points.size())
    {
        e -= points.size();
        end_wrapped = true;
    }
    std::vector<std::vector<T>> temp_pts;
    const std::vector<std::vector<T>> pts = getPoints();
    if (begin_wrapped)
    {
        if (end_wrapped)
        {
            temp_pts.emplace_back(begin_ext_pt);
            temp_pts.insert(temp_pts.end(), pts.begin(), pts.end());
            temp_pts.emplace_back(end_ext_pt);
        }
        else
        {
            temp_pts.emplace_back(begin_ext_pt);
            temp_pts.insert(temp_pts.end(), pts.begin(), pts.begin() + e);
        }
    }
    else
    {
        if (end_wrapped)
        {
            temp_pts.insert(temp_pts.begin(), pts.begin() + b, pts.end());
            temp_pts.emplace_back(end_ext_pt);
        }
        else
        {
            temp_pts.insert(temp_pts.begin(), pts.begin() + b, pts.begin() + e);
        }
    }
    splines[index]->calc(temp_pts);
}

template <typename T>
void OpenParametricInterpolator<T>::calcExtPts()
{
    if (points.size() > 0)
    {
        const int dim = points[0].point.size();
        if (dim == 0)
            return; // avoid bad mem access
        if (points.size() > 2)
        {
            std::vector<T> t (dim);
            T m0, m1, mExt;
            for (int j = 0; j < dim; ++j)
            {
                m0 = (points[1].point[j] - points[0].point[j]);
                m1 = (points[2].point[j] - points[1].point[j]);
                mExt = m0 - 2 * (m1 - m0);
                t[j] = points[0].point[j] - mExt;
            }
            begin_ext_pt = t;
            for (int j = 0; j < dim; ++j)
            {
                m0 = ((points.end()-2)->point[j] - (points.end()-3)->point[j]);
                m1 = (points.back().point[j] - (points.end()-2)->point[j]);
                mExt = m1 + 2 * (m1 - m0);
                t[j] = points.back().point[j] + mExt;
            }
            end_ext_pt = t;
        }
        else if (points.size() == 2)
        {
            std::vector<T> t (dim);
            for (int j = 0; j < dim; ++j)
                t[j] = points[0].point[j] - (points[1].point[j] - points[0].point[j]);
            begin_ext_pt = t;
            for (int j = 0; j < dim; ++j)
                t[j] = points.back().point[j] + (points.back().point[j] - (points.end()-2)->point[j]);
            end_ext_pt = t;
        }
        else // points.size() == 1
        {
            begin_ext_pt = std::vector<T>(dim, 0);
            end_ext_pt = std::vector<T>(dim, 0);
        }
    }
}

template <typename T>
void FunctionalInterpolator<T>::calcExtPts()
{
    if (points.size() > 0)
    {
        const int dim = points[0].point.size();
        if (dim == 0)
            return; // avoid bad mem access
        if (points.size() > 2)
        {
            std::vector<T> t (dim);
            T x0 = points[0].point[0];
            T x1 = points[1].point[0];
            T x2 = points[2].point[0];
            t[0] = x0 - (x1 - x0);
            T m0, m1, mExt;
            for (int j = 1; j < dim; ++j)
            {
                m0 = (points[1].point[j] - points[0].point[j]) / (x1 - x0);
                m1 = (points[2].point[j] - points[1].point[j]) / (x2 - x1);
                mExt = m0 - 1.1 * (m1 - m0);
                t[j] = points[0].point[j] - mExt * (x1 - x0);
            }
            begin_ext_pt = t;
            
            x0 = (points.end()-3)->point[0];
            x1 = (points.end()-2)->point[0];
            x2 = points.back().point[0];
            t[0] = x2 + (x2 - x1);
            for (int j = 1; j < dim; ++j)
            {
                m0 = ((points.end()-2)->point[j] - (points.end()-3)->point[j]) / (x1 - x0);
                m1 = (points.back().point[j] - (points.end()-2)->point[j]) / (x2 - x1);
                mExt = m1 + 1.1 * (m1 - m0);
                t[j] = points.back().point[j] + mExt * (x2 - x1);
            }
            end_ext_pt = t;
        }
        else if (points.size() == 2)
        {
            std::vector<T> t (dim);
            for (int j = 0; j < dim; ++j)
                t[j] = points[0].point[j] - (points[1].point[j] - points[0].point[j]);
            begin_ext_pt = t;
            for (int j = 0; j < dim; ++j)
                t[j] = points.back().point[j] + (points.back().point[j] - (points.end()-2)->point[j]);
            end_ext_pt = t;
        }
        else // points.size() == 1
        {
            begin_ext_pt = std::vector<T>(dim, 0);
            end_ext_pt = std::vector<T>(dim, 0);
        }
    }
}
//template <typename T>
//void OpenEndedInterpolator<T>::calcExtPts()
//{
//    if (points.size() > 0)
//    {
//        const int dim = points[0].point.size();
//        if (points.size() > 2)
//        {
//            std::vector<T> t (dim);
//            for (int j = 0; j < dim-1; ++j)
//            {
//                t[j] = points[0].point[j] - (points[1].point[j] - points[0].point[j]);
//            }
//            bool flattenOut = false;
//            if (
//            if (con)
//            t[dim-1] = points[0].point[dim-1] - 100 * (points[1].point[dim-1] - points[0].point[dim-1]);
//            begin_ext_pt = t;
//            for (int j = 0; j < dim; ++j)
//            {
//                t[j] = points.back().point[j] + (points.back().point[j] - (points.end()-2)->point[j]);
//            }
//            t[dim-1] = points.back().point[dim-1] + 100 * (points.back().point[dim-1] - (points.end()-2)->point[dim-1]);
//            end_ext_pt = t;
//        }
//        else if (points.size() == 2/*> 1*/)
//        {
//            std::vector<T> t (dim);
//            for (int j = 0; j < dim; ++j)
//            {
//                t[j] = points[0].point[j] - (points[1].point[j] - points[0].point[j]);
//            }
//            begin_ext_pt = t;
//            for (int j = 0; j < dim; ++j)
//            {
//                t[j] = points.back().point[j] + (points.back().point[j] - (points.end()-2)->point[j]);
//            }
//            end_ext_pt = t;
//        }
//        else // points.size() == 1
//        {
//            begin_ext_pt = std::vector<T>(dim, 0);
//            end_ext_pt = std::vector<T>(dim, 0);
//        }
//    }
//}

template <typename T>
ClosedParametricInterpolator<T>::ClosedParametricInterpolator(const std::vector<std::vector<T>>& new_points)
    //: Interpolator<T>(InterpolatorType::CLOSED_PARAMETRIC)
{
    const int N = new_points.size();
    points.resize(N);
    splines.resize(N);
    for (int i = 0; i < N; ++i)
    {
        points[i] = SelectablePoint<T>(new_points[i]);
        splines[i] = SplineFactory<T>(spline_type, SplineBehavior::PARAMETRIC);
    }
    for (int i = 0; i < N; ++i)
        calcSplineAt(i);
}

template <typename T>
ClosedParametricInterpolator<T>::ClosedParametricInterpolator(const std::vector<std::vector<T>>& new_points,
                                                              const std::vector<SplineShape>& new_splines)
    //: Interpolator<T>(InterpolatorType::CLOSED_PARAMETRIC)
{
    const int N = new_points.size();
    points.resize(N);
    splines.resize(N);
    for (int i = 0; i < N; ++i)
    {
        points[i] = SelectablePoint<T>(new_points[i]);
        splines[i] = SplineFactory<T>(new_splines[i], SplineBehavior::PARAMETRIC);
    }
    for (int i = 0; i < N; ++i)
        calcSplineAt(i);
}

template <typename T>
ClosedParametricInterpolator<T>::ClosedParametricInterpolator(const OpenParametricInterpolator<T>& open_interp)
    //: Interpolator<T>(InterpolatorType::CLOSED_PARAMETRIC)
{
    
    points = open_interp.points;
    selected_points = open_interp.selected_points;
    const int N = open_interp.points.size();
    splines.resize(N);
    for (int i = 0; i < N; ++i)
    {
        splines[i] = SplineFactory<T>(spline_type, SplineBehavior::PARAMETRIC);
        calcSplineAt(i);
    }
}

template <typename T>
OpenEndedInterpolator<T>::OpenEndedInterpolator(const std::vector<std::vector<T>>& new_points)
{
    const int N = new_points.size();
    points.resize(N);
    for (int i = 0; i < N; ++i)
        points[i].point = new_points[i];
    //calcExtPts(); // now specific to Functional/Parametric subclasses
    // initSplines() should be called in child class constructor after this
}

template <typename T>
OpenParametricInterpolator<T>::OpenParametricInterpolator(const ClosedParametricInterpolator<T>& closed_interp)
    //: Interpolator<T>(InterpolatorType::OPEN_PARAMETRIC)
{
    points = closed_interp.points;
    calcExtPts();
    selected_points = closed_interp.selected_points;
    const int N = std::max(0, (int)(closed_interp.points.size()-1));
    splines.resize(N);
    for (int i = 0; i < N; ++i)
    {
        splines[i] = SplineFactory<T>(spline_type, SplineBehavior::PARAMETRIC);
        calcSplineAt(i);
    }
}

template <typename T>
void OpenParametricInterpolator<T>::initSplines()
{
    if (points.size() > 0)
    {
        splines.resize(points.size()-1);
        for (int i = 0; i < splines.size(); ++i)
        {
            splines[i] = SplineFactory<T>(spline_type, SplineBehavior::PARAMETRIC);
            calcSplineAt(i);
        }
    }
}

template <typename T>
void OpenParametricInterpolator<T>::initSplines(const std::vector<SplineShape>& new_splines)
{
    if (points.size() > 0)
    {
        splines.resize(new_splines.size());
        for (int i = 0; i < splines.size(); ++i)
        {
            splines[i] = SplineFactory<T>(new_splines[i], SplineBehavior::PARAMETRIC);
            calcSplineAt(i);
        }
    }
}

template <typename T>
void FunctionalInterpolator<T>::initSplines()
{
    if (points.size() > 0)
    {
        splines.resize(points.size()-1);
        for (int i = 0; i < splines.size(); ++i)
        {
            splines[i] = SplineFactory<T>(spline_type, SplineBehavior::FUNCTIONAL);
            calcSplineAt(i);
        }
    }
}

template <typename T>
void FunctionalInterpolator<T>::initSplines(const std::vector<SplineShape>& new_splines)
{
    if (points.size() > 0)
    {
        splines.resize(new_splines.size());
        for (int i = 0; i < splines.size(); ++i)
        {
            splines[i] = SplineFactory<T>(new_splines[i], SplineBehavior::FUNCTIONAL);
            calcSplineAt(i);
        }
    }
}

template <typename T>
void FunctionalInterpolator<T>::addPoint(const std::vector<T>& point)
{
    int inserted_index = -1;
    for (int i = 0; i < ((int)points.size())-1; ++i)
    {
        if (points[i].point[0] <= point[0] && point[0] < points[i+1].point[0])
        {
            points.insert(points.begin()+i+1, point);
            splines.insert(splines.begin()+i+1, SplineFactory<T>(splines[i]->getShape(), SplineBehavior::FUNCTIONAL));
            inserted_index = i+1;
            break;
        }
    }
    if (inserted_index == -1)
    {
        if (points.size() > 0 && point[0] < points[0].point[0])
        {
            inserted_index = 0;
            points.insert(points.begin(), point);
            SplineShape type;
            if (splines.size() > 0)
                type = splines.front()->getShape();
            else
                type = spline_type;
            splines.insert(splines.begin(), SplineFactory<T>(type, SplineBehavior::FUNCTIONAL));
        }
        else
        {
            inserted_index = points.size();
            points.emplace_back(point);
            if (points.size() >= 2)
            {
                SplineShape type;
                if (splines.size() > 0)
                    type = splines.back()->getShape();
                else
                    type = spline_type;
                splines.insert(splines.end(), SplineFactory<T>(type, SplineBehavior::FUNCTIONAL));
            }
        }
    }
    if (inserted_index < 2 || inserted_index > points.size()-3)
        calcExtPts();
    int b = inserted_index - (max_pts_per_spline>>1);
    int e = inserted_index + (max_pts_per_spline>>1);
    calcSplinesInRange(b, e);
    informListenersOfChange();
    //changed = true;
}

template <typename T>
void ParametricInterpolator<T>::addPoint(const std::vector<T>& point)
{
    // add the point at the end
    addPoint(point, points.size());
}

//template <typename T>
//void ParametricInterpolator<T>::addPoint(const std::vector<T>& point, int index)
//{
//    points.insert(points.begin()+index, SelectablePoint<T>(point));
//    SplineShape prev_spline_type;  // avoiding an addPoint(pt,i) impl for open/closed subclasses
//    if (0 <= index-1 && index-1 < splines.size())
//        prev_spline_type = (*(splines.begin()+index-1))->type;
//    else
//        prev_spline_type = spline_type;
//    splines.insert(splines.begin() + std::min(index,(int)splines.size()), SplineFactory<T>(prev_spline_type, PARAMETRIC));
//    // unselect all points ?
//    selected_points.clear();
//    for (auto pt : points)
//        pt.selected = false;
//    // to update OpenParametricInterpolator's ext pts which are also needed before calcSplinesInRange() below
//    //if (index < 2 || index > points.size()-3)
//    informChildren();
//    int b = index - (max_pts_per_spline>>1);
//    int e = index + (max_pts_per_spline>>1);
//    calcSplinesInRange(b, e);
//    changed = true;
//}

template <typename T>
void OpenParametricInterpolator<T>::addPoint(const std::vector<T>& point, int index)
{
    points.insert(points.begin()+index, SelectablePoint<T>(point));
    SplineShape new_spline_type;
    if (0 <= index-1 && index-1 < splines.size())
        new_spline_type = splines[index-1]->getShape();
    else if (index == 0 && splines.size() > 0)
        new_spline_type = splines[0]->getShape();
    else if (index == points.size()-1 && (0 <= index-2 && index-2 < splines.size()))
        new_spline_type = splines[index-2]->getShape();
    else
        new_spline_type = spline_type;
    int safe_index = std::min(index, (int)splines.size());
    splines.insert(splines.begin() + safe_index, SplineFactory<T>(new_spline_type, SplineBehavior::PARAMETRIC));
    // unselect all points ?
    selected_points.clear();
    for (int i = 0; i < points.size(); ++i)
        points[i].selected = false;
    // to update OpenParametricInterpolator's ext pts which are also needed before calcSplinesInRange() below
    if (index < 2 || index > points.size()-3)
        calcExtPts();
    int b = index - (max_pts_per_spline>>1);
    int e = index + (max_pts_per_spline>>1);
    calcSplinesInRange(b, e);
    informListenersOfChange();
    //changed = true;
}

template <typename T>
void ClosedParametricInterpolator<T>::addPoint(const std::vector<T>& point, int index)
{
    points.insert(points.begin()+index, SelectablePoint<T>(point));
    SplineShape new_spline_type;
    if (0 <= index-1 && index-1 < splines.size())
        new_spline_type = splines[index-1]->getShape();
    else if (index == 0 && splines.size() > 0)
        new_spline_type = splines.back()->getShape();
    else
        new_spline_type = spline_type;
    splines.insert(splines.begin() + index, SplineFactory<T>(new_spline_type, SplineBehavior::PARAMETRIC));
    // unselect all points ?
    selected_points.clear();
    for (int i = 0; i < points.size(); ++i)
        points[i].selected = false;
    int b = index - (max_pts_per_spline>>1);
    int e = index + (max_pts_per_spline>>1);
    calcSplinesInRange(b, e);
    informListenersOfChange();
    //changed = true;
}

template <typename T>
void OpenParametricInterpolator<T>::setSelectedPointIndices(cint pointIndex,
                                                            cint newIndex)
{
    cint numPoints = points.size();
   /* assert(0 <= pointIndex && pointIndex < numPoints);
    assert(0 <= newIndex && newIndex < numPoints);*/
    
    if (!Interpolator<T>::getPointSelected(pointIndex))
        Interpolator<T>::setPointSelected(pointIndex, true);
    
    cint deltaIndex = newIndex - pointIndex;
    cint numSelectedPoints = Interpolator<T>::getNumSelectedPoints();
    if (deltaIndex == 0 || numSelectedPoints == numPoints)
        return; // nothing to do here folks
    
    cauto selectedPointIndices = Interpolator<T>::getSelectedPointIndices();
    auto newSelectedPointIndices = selectedPointIndices;
    if (deltaIndex > 0) {
        int max = numPoints - numSelectedPoints;
        for (auto& i : newSelectedPointIndices)
            i = std::min(i + deltaIndex, max++);
    } else {
        int min = 0;
        for (auto& i : newSelectedPointIndices)
            i = std::max(i + deltaIndex, min++);
    }
    //cauto newSelectedPointIndicesRef = const_cast<const std::vector<int>&>(newSelectedPointIndices);
    cauto unselectedPointIndices = Interpolator<T>::getUnselectedPointIndices();
    
//    cauto toString = [](const std::vector<int>& v) {
//        std::string str;
//        for (auto i : v)
//            str += std::to_string(i) + "  ";
//        str += "\n";
//        return str;
//    };
//    debug({toString(selectedPointIndices), toString(unselectedPointIndices)});
    cauto copy = points;
    for (int i = 0, j = 0, k = 0; i < numPoints; ++i) {
        if (i == newSelectedPointIndices[j])
            points[i] = copy[selectedPointIndices[j++]];
        else
            points[i] = copy[unselectedPointIndices[k++]];
    }
    
    selected_points.clear();
    for (int i = 0; i < numPoints; ++i)
        if (points[i].selected)
            selected_points.emplace_back(i);
    
    calcExtPts();
    calcSplinesInRange(0, splines.size());
    informListenersOfChange();
}

template <typename T>
void ClosedParametricInterpolator<T>::setSelectedPointIndices(cint pointIndex,
                                                              cint newIndex)
{
    cint numPoints = points.size();
    /*assert(0 <= pointIndex && pointIndex < numPoints);
    assert(0 <= newIndex && newIndex < numPoints);*/
    
    if (!Interpolator<T>::getPointSelected(pointIndex))
        Interpolator<T>::setPointSelected(pointIndex, true);
    
    cint deltaIndex = (newIndex - pointIndex) % numPoints;
    //cint numSelectedPoints = Interpolator<T>::getNumSelectedPoints();
    if (deltaIndex == 0 /*|| numSelectedPoints == numPoints*/)
        return; // nothing to do here folks
    
//    cauto selectedPointIndices = Interpolator<T>::getSelectedPointIndices();
//    auto newSelectedPointIndices = selectedPointIndices;
//    if (deltaIndex > 0) {
//        for (auto& i : newSelectedPointIndices) {
//            i += deltaIndex;
//            if (i >= numPoints)
//                i -= numPoints;
//        }
//    } else {
//        for (auto& i : newSelectedPointIndices) {
//            i += deltaIndex;
//            if (i < 0)
//                i += numPoints;
//        }
//    }
//    cauto unselectedPointIndices = Interpolator<T>::getUnselectedPointIndices();
    
//    cauto toString = [](const std::vector<int>& v) {
//        std::string str;
//        for (auto i : v)
//            str += std::to_string(i) + "  ";
//        str += "\n";
//        return str;
//    };
//    debug({"selectedPointIndices:     " + toString(selectedPointIndices),
//           "newSelectedPointIndices:  " + toString(newSelectedPointIndices),
//           "unselectedPointIndicies:  " + toString(unselectedPointIndices)});
    
    partial_rotate(points, Interpolator<T>::getSelectedPointIndices(), deltaIndex);
//    //const std::vector<SelectablePoint<T>>
//    cauto copy = points;
//    std::vector<int> copiedIndices;
//    std::vector<int> newCopiedIndices;
//    copiedIndices.reserve(numPoints);
//    newCopiedIndices.reserve(numPoints);
//    for (int i = 0, j = 0, k = 0; copiedIndices.size() < numPoints
//    /*i < numPoints && j < unselectedPointIndices.size() && k < unselectedPointIndices.size()*/; ++i) {
//        if (i == newSelectedPointIndices[j]) {
//            copiedIndices.emplace_back(selectedPointIndices[j]);
//            points[i] = copy[selectedPointIndices[j++]];
//        } else /*if (k < unselectedPointIndices.size())*/ {
//            int l = unselectedPointIndices[k];
//            k = (k+1) % unselectedPointIndices.size();
//            while (std::find(copiedIndices.begin(), copiedIndices.end(), l) != copiedIndices.end())
//                ++l;
//            copiedIndices.emplace_back(l);
//            points[i] = copy[l];
//            //points[i] = copy[unselectedPointIndices[k++]];
//        }
////        else // ?
////            points[i] = copy[i];
//        if (i == numPoints - 1 && copiedIndices.size() < numPoints/* (j < selectedPointIndices.size() || k < unselectedPointIndices.size())*/)
//            i = -1;
//    }
//    
//    // copy over selected indices
//    for (int i = 0, j = 0; j < selectedPointIndices.size(); ++i) {
//        if (i == newSelectedPointIndices[j]) {
//            copiedIndices.emplace_back(selectedPointIndices[j]);
//            newCopiedIndices.emplace_back(i);
//            points[i] = copy[selectedPointIndices[j++]];
//        }
//        if (i == numPoints - 1)
//            i = -1;
//    }
//    
//    // fit in the unselected indices where they may go because they may be displaced by the selected ones
//    for (int k = 0; copiedIndices.size() < numPoints; ) {
//        int l = unselectedPointIndices[k];
//        k = (k + 1) % unselectedPointIndices.size();
//        while (contains(copiedIndices, l)/* std::find(copiedIndices.begin(), copiedIndices.end(), l) != copiedIndices.end()*/)
//            l = (l + 1) % numPoints;
//        copiedIndices.emplace_back(l);
//        points[i] = copy[l];
//    }
    
    selected_points.clear();
    for (int i = 0; i < numPoints; ++i)
        if (points[i].selected)
            selected_points.emplace_back(i);
    
    calcSplinesInRange(0, splines.size());
    informListenersOfChange();
}
//template <typename T>
//void ParametricInterpolator<T>::setSelectedPointIndex(int index)
//{
//    if (selected_points.size() == 1) // implement shifting a block of consecutive selected points ?
//    {
//        int current_index = selected_points.front();
//        points.insert(points.begin()+index, SelectablePoint<T>(points[current_index].point));
//        splines.insert(splines.begin()+index, SplineFactory<T>(splines[current_index]->getShape(), SplineBehavior::PARAMETRIC));
//        if (current_index < index)
//        {
//            points.erase(points.begin()+current_index);
//            splines.erase(splines.begin()+current_index);
//        }
//        else
//        {
//            points.erase(points.begin()+current_index+1);
//            splines.erase(splines.begin()+current_index+1);
//        }
//        selected_points.front() = index;
//        
//        // to update OpenParametricInterpolator's ext pts which are also needed before calcSplinesInRange() below
//        //if ((index < 2 || index > points.size()-3) || (current_index < 2 || current_index > points.size()-3))
//        informChildren();
//        
//        int b = std::min(current_index, index) - (max_pts_per_spline>>1);
//        int e = std::max(current_index, index) + (max_pts_per_spline>>1);
//        calcSplinesInRange(b, e);
//    }
//}

template <typename T>
int ParametricInterpolator<T>::setSelectedSplinesType(SplineShape new_spline_type)
{
    bool changed = false;
    std::vector<int> sel_splines = getSelectedSplines();
    for (int i = 0; i < sel_splines.size(); ++i)
    {
        if (splines[sel_splines[i]]->getShape() != new_spline_type)
        {
            changed = true;
            splines[sel_splines[i]].release();
            splines[sel_splines[i]] = SplineFactory<T>(new_spline_type, SplineBehavior::PARAMETRIC);
            calcSplineAt(sel_splines[i]);
        }
    }
    if (changed)
    {
        informListenersOfChange();
        return sel_splines.size();
    }
    else
        return 0;
}

template <typename T>
int FunctionalInterpolator<T>::setSelectedSplinesType(SplineShape new_spline_type)
{
    bool changed = false;
    std::vector<int> sel_splines = getSelectedSplines();
    for (int i = 0; i < sel_splines.size(); ++i)
    {
        if (splines[sel_splines[i]]->getShape() != new_spline_type)
        {
            changed = true;
            splines[sel_splines[i]].release();
            splines[sel_splines[i]] = SplineFactory<T>(new_spline_type, SplineBehavior::FUNCTIONAL);
            calcSplineAt(sel_splines[i]);
        }
    }
    if (changed)
    {
        informListenersOfChange();
        return sel_splines.size();
    }
    else
        return 0;
}

template <typename T>
int ParametricInterpolator<T>::moveSelectedPoints(const std::vector<T>& delta)
{
    const int num_moved = selected_points.size();
	if (num_moved == 0) // can't get a front() ref below if no pts selected
		return num_moved;
    const int d = max_pts_per_spline >> 1;
    std::vector<int> update_chunks {selected_points.front() - d};
    int prev_selected = selected_points.front();
    for (const auto i : selected_points)
    {
        // found the beginning of a new spline chunk that needs recalcing
        if (i - d > prev_selected + d)
        {
            update_chunks.emplace_back(prev_selected + d);
            update_chunks.emplace_back(i - d);
        }
        // move this selected point
        for (int j = 0; j < delta.size(); ++j)
            points[i].point[j] += delta[j];
        // remember this index as the prev
        prev_selected = i;
    }
    if (num_moved > 0)
    {
        update_chunks.emplace_back(selected_points.back() + d);
        // inform OpenEndedInterpolator to update its ext pts
        informChildren();
        // recalculate each spline that needs it
        for (int i = 0; i < update_chunks.size(); i += 2)
            calcSplinesInRange(update_chunks[i], update_chunks[i+1]);
        informListenersOfChange();
    }
    return num_moved;
}

//// couldn't find the one that should be in namespace std...
//template<class ForwardIterator, class T>
//void iota(ForwardIterator first, ForwardIterator last, T value)
//{
//    while (first != last) {
//        *first++ = value;
//        ++value;
//    }
//}
// where would we be without one another?  http://stackoverflow.com/questions/17074324/how-can-i-sort-two-vectors-in-the-same-way-with-criteria-that-uses-only-one-of
template <typename T, typename Compare>
std::vector<int> sort_permutation(std::vector<T> const& vec,
                                  Compare compare)
{
    std::vector<int> p (vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::stable_sort(p.begin(), p.end(), // you know i love you C++, right?
                     [&](int i, int j){ return compare(vec[i], vec[j]); });
    return p;
}
template <typename T>
std::vector<T> apply_permutation(std::vector<T> const& vec,
                                 std::vector<int> const& p)
{
    std::vector<T> sorted_vec (p.size());
    std::transform(p.begin(), p.end(), sorted_vec.begin(),
                   [&](int i){ return vec[i]; });
    return sorted_vec;
}

// this one is polymorphic with all Interpolators
template <typename T>
//std::tuple<int, std::vector<int>> FunctionalInterpolator<T>::moveSelectedPoints(const std::vector<T>& delta)
int FunctionalInterpolator<T>::moveSelectedPoints(const std::vector<T>& delta)
{
    const int num_moved = selected_points.size();
    bool need_sorting = false;
    //std::vector<int> new_points_order;
    T prev, next;
    bool need_prev = false;
    bool need_next = false;
    // move the selected points
    for (auto i : selected_points)
    {
        if (!need_sorting)
        {
            if (i-1 >= 0)
            {
                prev = points[i-1].point[0];
                need_prev = true;
            }
            if (i+1 < points.size())
            {
                next = points[i+1].point[0];
                need_next = true;
            }
        }
        // move this selected point
        for (int j = 0; j < delta.size(); ++j)
            points[i].point[j] += delta[j];
        if (!need_sorting && ((need_prev && points[i].point[0] < prev) || (need_next && points[i].point[0] > next)))
            need_sorting = true;
    }
    if (num_moved > 0)
    {
        if (need_sorting)
        {
            // sort points, splines, and selected pts
            auto new_points_order = sort_permutation(points, [](const SelectablePoint<T>& p1, const SelectablePoint<T>& p2)
                                                               {return p1.point[0] < p2.point[0];});
            points = apply_permutation<SelectablePoint<float>>(points, new_points_order);
            std::vector<Spline<T>*> spline_ptrs (splines.size());
            for (int i = 0; i < splines.size(); ++i)
                spline_ptrs[i] = splines[i].release();
            for (int i = 0; i < new_points_order.size(); ++i)
                if (new_points_order[i] >= splines.size())
                {
                    new_points_order.erase(new_points_order.begin()+i);
                    --i;
                }
            spline_ptrs = apply_permutation<Spline<T>*>(spline_ptrs, new_points_order);
            for (int i = 0; i < splines.size(); ++i)
                splines[i].reset(spline_ptrs[i]);
            // the selected indecies got jumbled, but their order is preserved in the just sorted points array
            selected_points.clear();
            for (int i = 0; i < points.size(); ++i)
                if (points[i].selected)
                    selected_points.emplace_back(i);
            // recalc extpts/splines, might as well just just do it since this sorting thing is already somewhat expensive
            calcExtPts();
//            // this fancy implementation was only really needed when pts were moved frequently through the mouseWheelMove() callback, however unupdated segments still pop up if the selected points are moved by more than one none-selected segment per moveSelectedPoints() call
//            std::vector<bool> changedd (splines.size(), false);
//            int prev_selected_index, start_selected_chunk, start, end;
//            auto d = (max_pts_per_spline >> 1) + 1;
//            for (auto i : selected_points)
//            {
//                // first chunk start index
//                if (i == selected_points.front())
//                    start_selected_chunk = i;
//                if (i == selected_points.back())
//                {
//                    if (start_selected_chunk == selected_points.front())
//                    {
//                        start = start_selected_chunk - d;
//                        end = i + d;
//                    }
//                    else // spline indecies needing update from two selected blocks might overlap
//                    {
//                        start = std::max(end, start_selected_chunk - d);
//                        end = i + d;
//                    }
//                    calcSplinesInRange(start, end);
//                    if (start < 0)
//                        start = 0;
//                    if (end > changedd.size())
//                        end = changedd.size();
//                    for (int j = start; j < end; ++j)
//                        changedd[j] = true;
//                    break;
//                }
//                // found beginning of a new selected chunk
//                if (i != selected_points.front() && i != prev_selected_index + 1)
//                {
//                    if (start_selected_chunk == selected_points.front())
//                    {
//                        start = start_selected_chunk - d;
//                        end = prev_selected_index + d;
//                    }
//                    else // spline indecies needing update from two selected blocks might overlap
//                    {
//                        start = std::max(end, start_selected_chunk - d);
//                        end = prev_selected_index + d;
//                    }
//                    calcSplinesInRange(start, end);
//                    if (start < 0)
//                        start = 0;
//                    if (end > changedd.size())
//                        end = changedd.size();
//                    for (int j = start; j < end; ++j)
//                        changedd[j] = true;
//                    start_selected_chunk = prev_selected_index;
//                }
//                prev_selected_index = i;
//            }
//            // we calc'd the splines surrounding the selected points, but there might be "holes" that need updating from where the selected points used to be
//            for (int i = 0; i < new_points_order.size()-1; ++i)
//            {
//                if (new_points_order[i]+1 != new_points_order[i+1] && !changedd[i])
//                    calcSplineAt(i);
//            }
            // this is expensive for lots of points, but not too much of a burden when moving points through the mouseMoved() callback, plus it is reliable
            for (int i = 0; i < splines.size(); ++i)
                calcSplineAt(i);
        }
        else
        {
            // recalc extpts, if needed
            if (selected_points.front() < 3 || selected_points.back() > splines.size() - 3)
                calcExtPts();
            // recalc affected splines
            int prev_selected_index, start_selected_chunk, start, end;
            auto d = max_pts_per_spline >> 1;
            for (auto i : selected_points)
            {
                // first chunk start index
                if (i == selected_points.front())
                    start_selected_chunk = i;
                if (i == selected_points.back())
                {
                    if (start_selected_chunk == selected_points.front())
                    {
                        start = start_selected_chunk - d;
                        end = i + d;
                    }
                    else // spline indecies needing update from two selected blocks might overlap
                    {
                        start = std::max(end, start_selected_chunk - d);
                        end = i + d;
                    }
                    calcSplinesInRange(start, end);
                    break;
                }
                // found beginning of a new selected chunk
                if (i != selected_points.front() && i != prev_selected_index + 1)
                {
                    if (start_selected_chunk == selected_points.front())
                    {
                        start = start_selected_chunk - d;
                        end = prev_selected_index + d;
                    }
                    else // spline indecies needing update from two selected blocks might overlap
                    {
                        start = std::max(end, start_selected_chunk - d);
                        end = prev_selected_index + d;
                    }
                    calcSplinesInRange(start, end);
                    start_selected_chunk = prev_selected_index;
                }
                prev_selected_index = i;
            }
        }
        informListenersOfChange();
        //changed = true;
    }
    return num_moved;
}

//// this one is specific to FunctionalInterpolators due to it returning any reordering info of the points
//template <typename T>
//std::vector<int> FunctionalInterpolator<T>::moveSelectedPointsWithReorderingInfo(const std::vector<T>& delta)
//{
//    auto num_moved = selected_points.size();
//    auto need_sorting = false;
//    std::vector<int> new_points_order;
//    T prev, next;
//    auto need_prev = false;
//    auto need_next = false;
//    // move the selected points
//    for (auto i : selected_points)
//    {
//        if (!need_sorting)
//        {
//            if (i-1 >= 0)
//            {
//                prev = points[i-1].point[0];
//                need_prev = true;
//            }
//            if (i+1 < points.size())
//            {
//                next = points[i+1].point[0];
//                need_next = true;
//            }
//        }
//        // move this selected point
//        for (int j = 0; j < delta.size(); ++j)
//            points[i].point[j] += delta[j];
//        if (!need_sorting && ((need_prev && points[i].point[0] < prev) || (need_next && points[i].point[0] > next)))
//            need_sorting = true;
//    }
//    if (num_moved > 0)
//    {
//        if (need_sorting)
//        {
//            // sort points, splines, and selected pts
//            /*auto*/ new_points_order = sort_permutation(points, [](const SelectablePoint<T>& p1, const SelectablePoint<T>& p2)
//                                                     {return p1.point[0] < p2.point[0];});
//            points = apply_permutation<SelectablePoint<float>>(points, new_points_order);
//            std::vector<Spline<T>*> spline_ptrs (splines.size());
//            for (int i = 0; i < splines.size(); ++i)
//                spline_ptrs[i] = splines[i].release();
//            for (int i = 0; i < new_points_order.size(); ++i)
//                if (new_points_order[i] >= splines.size())
//                {
//                    new_points_order.erase(new_points_order.begin()+i);
//                    --i;
//                }
//            spline_ptrs = apply_permutation<Spline<T>*>(spline_ptrs, new_points_order);
//            for (int i = 0; i < splines.size(); ++i)
//                splines[i].reset(spline_ptrs[i]);
//            // the selected indecies got jumbled, but their order is preserved in the just sorted points array
//            selected_points.clear();
//            for (int i = 0; i < points.size(); ++i)
//                if (points[i].selected)
//                    selected_points.emplace_back(i);
//            // recalc extpts/splines, might as well just just do it since this sorting thing is already somewhat expensive
//            calcExtPts();
//            std::vector<bool> changedd (splines.size(), false);
//            int prev_selected_index, start_selected_chunk, start, end;
//            auto d = (max_pts_per_spline >> 1) + 1;
//            for (auto i : selected_points)
//            {
//                // first chunk start index
//                if (i == selected_points.front())
//                    start_selected_chunk = i;
//                if (i == selected_points.back())
//                {
//                    if (start_selected_chunk == selected_points.front())
//                    {
//                        start = start_selected_chunk - d;
//                        end = i + d;
//                    }
//                    else // spline indecies needing update from two selected blocks might overlap
//                    {
//                        start = std::max(end, start_selected_chunk - d);
//                        end = i + d;
//                    }
//                    calcSplinesInRange(start, end);
//                    if (start < 0)
//                        start = 0;
//                    if (end > changedd.size())
//                        end = changedd.size();
//                    for (int j = start; j < end; ++j)
//                        changedd[j] = true;
//                    break;
//                }
//                // found beginning of a new selected chunk
//                if (i != selected_points.front() && i != prev_selected_index + 1)
//                {
//                    if (start_selected_chunk == selected_points.front())
//                    {
//                        start = start_selected_chunk - d;
//                        end = prev_selected_index + d;
//                    }
//                    else // spline indecies needing update from two selected blocks might overlap
//                    {
//                        start = std::max(end, start_selected_chunk - d);
//                        end = prev_selected_index + d;
//                    }
//                    calcSplinesInRange(start, end);
//                    if (start < 0)
//                        start = 0;
//                    if (end > changedd.size())
//                        end = changedd.size();
//                    for (int j = start; j < end; ++j)
//                        changedd[j] = true;
//                    start_selected_chunk = prev_selected_index;
//                }
//                prev_selected_index = i;
//            }
//            // we calc'd the splines surrounding the selected points, but there might be "holes" that need updating from where the selected points used to be
//            for (int i = 0; i < new_points_order.size()-1; ++i)
//            {
//                if (new_points_order[i]+1 != new_points_order[i+1] && !changedd[i])
//                    calcSplineAt(i);
//            }
//            // this is just ugly b/c we don't know what ordering the delta put the points in ... or do we ?
//            //for (int i = 0; i < splines.size(); ++i)
//            //    calcSplineAt(i);
//        }
//        else
//        {
//            // recalc extpts, if needed
//            if (selected_points.front() < 2 || selected_points.back() > splines.size()-2)
//                calcExtPts();
//            // recalc affected splines
//            int prev_selected_index, start_selected_chunk, start, end;
//            auto d = max_pts_per_spline >> 1;
//            for (auto i : selected_points)
//            {
//                // first chunk start index
//                if (i == selected_points.front())
//                    start_selected_chunk = i;
//                if (i == selected_points.back())
//                {
//                    if (start_selected_chunk == selected_points.front())
//                    {
//                        start = start_selected_chunk - d;
//                        end = i + d;
//                    }
//                    else // spline indecies needing update from two selected blocks might overlap
//                    {
//                        start = std::max(end, start_selected_chunk - d);
//                        end = i + d;
//                    }
//                    calcSplinesInRange(start, end);
//                    break;
//                }
//                // found beginning of a new selected chunk
//                if (i != selected_points.front() && i != prev_selected_index + 1)
//                {
//                    if (start_selected_chunk == selected_points.front())
//                    {
//                        start = start_selected_chunk - d;
//                        end = prev_selected_index + d;
//                    }
//                    else // spline indecies needing update from two selected blocks might overlap
//                    {
//                        start = std::max(end, start_selected_chunk - d);
//                        end = prev_selected_index + d;
//                    }
//                    calcSplinesInRange(start, end);
//                    start_selected_chunk = prev_selected_index;
//                }
//                prev_selected_index = i;
//            }
//        }
//        changed = true;
//    }
//    return new_points_order;
//}

//template <typename T>
//void FunctionalInterpolator<T>::sort(std::vector<SelectablePoint<T>>& points)
//{
//    //std::sort(begin(splines), end(splines), [=, &points](const std::unique_ptr<Spline<T>>& s1, const std::unique_ptr<Spline<T>>& s2)
//    //                                                    {return points[(&s1)-&splines[0]].point[0] < points[(&s2)-&splines[0]].point[0];});
//    
//    std::sort(begin(points), end(points), [](const SelectablePoint<T>& p1, const SelectablePoint<T>& p2)
//                                            {return p1.point[0] < p2.point[0];});
////    if (splines.size() > 0)
////    {
//    
////    }
//}

template <typename T>
int OpenParametricInterpolator<T>::copySelectedPoints()
{
    int start_new_selecteds = points.size();
    SplineShape prev_selected_spline_type;
    points.reserve(points.size() + selected_points.size()); // don't want pointers to get modified mid loop below with a reallocation when using emplace_back()
    splines.reserve(splines.size() + selected_points.size());
    for (auto i : selected_points)
    {
        points.emplace_back(SelectablePoint<T>(points[i].point));
        if (i == selected_points.front())
            splines.emplace_back(SplineFactory<T>(splines.back()->getShape(), SplineBehavior::PARAMETRIC));
        else
            splines.emplace_back(SplineFactory<T>(prev_selected_spline_type, SplineBehavior::PARAMETRIC));
        prev_selected_spline_type = splines[i]->getShape();
    }
//    if (selected_points.size() > 0)
//        changed = true;
    selected_points.clear();
    for (int i = 0; i < start_new_selecteds; ++i)
        points[i].selected = false;
    for (int i = start_new_selecteds; i < points.size(); ++i)
    {
        selected_points.emplace_back(i);
        points[i].selected = true;
    }
    calcExtPts();
    calcSplinesInRange(start_new_selecteds - (max_pts_per_spline>>1), splines.size() + (max_pts_per_spline>>1));
    if (selected_points.size() > 0)
        informListenersOfChange();
    return selected_points.size();
}

template <typename T>
int ClosedParametricInterpolator<T>::copySelectedPoints()
{
    int start_new_selecteds = points.size();
    points.reserve(points.size() + selected_points.size()); // don't want pointers to get modified mid loop below with a reallocation when using emplace_back()
    splines.reserve(splines.size() + selected_points.size());
    for (auto i : selected_points)
    {
        points.emplace_back(SelectablePoint<T>(points[i].point));
        splines.emplace_back(SplineFactory<T>(splines[i]->getShape(), SplineBehavior::PARAMETRIC));
    }
//    if (selected_points.size() > 0)
//        changed = true;
    selected_points.clear();
    for (int i = 0; i < start_new_selecteds; ++i)
        points[i].selected = false;
    for (int i = start_new_selecteds; i < points.size(); ++i)
    {
        selected_points.emplace_back(i);
        points[i].selected = true;
    }
    calcSplinesInRange(start_new_selecteds - (max_pts_per_spline>>1), splines.size() + (max_pts_per_spline>>1));
    if (selected_points.size() > 0)
        informListenersOfChange();
    return selected_points.size();
}

template <typename T>
int FunctionalInterpolator<T>::copySelectedPoints()
{
//    if (selected_points.size() > 0)
//        changed = true;
    SplineShape type;
    std::list<int> new_selected_points;
    int num_inserted = 0;
    int relative_index;
    for (auto i : selected_points)
    {
        relative_index = i + num_inserted;
        std::vector<T> pt = points[relative_index].point;
        //pt[0] -= 0.001;
        points.insert(points.begin() + relative_index, SelectablePoint<T>(pt));
        points[relative_index+1].selected = true;//false;
        points[relative_index].selected = false;//true;
        // bounds check b/c splines size = points size - 1
        if (relative_index < splines.size())
            type = splines[relative_index]->getShape();
        else
            type = splines.back()->getShape();
        splines.insert(splines.begin() + relative_index, SplineFactory<T>(type, SplineBehavior::FUNCTIONAL));
        new_selected_points.emplace_back(relative_index+1/*relative_index*/);
        ++num_inserted;
    }
    //selected_points.clear();
    selected_points = new_selected_points;
    // might be able to get away with not calcing any splines until the selected points are moved since the points were just copied at their current position, otherwise need to do that here...
    calcExtPts();
    calcSplinesInRange(0, splines.size());
//    for (int i = 0; i < splines.size(); ++i)
//        calcSplineAt(i);
    if (selected_points.size() > 0)
        informListenersOfChange();
    return selected_points.size();
}

template <typename T>
int ParametricInterpolator<T>::pointAt(T val, std::vector<T>& point) const
{
    // number of spline segments
    int N = splines.size();
    if (N >= 1 && points.size() > 1)//(N >= 1)
    {
        // bounds check and force input val to be periodic by parametric range = N
        while (val < 0)
            val += N;
        while (val >= N)
            val -= N;
        // figure out the piecewise polynomial section we need and where within it
        int sec_index = floor(val);
        T sec_val = val - sec_index;
        // get the point
        point = splines[sec_index]->pointAt(sec_val);
        if (point.empty())
            return 0;
        else
            return 1;
    }
    else if (points.size() == 1)
    {
        point = points[0].point;
        return 1;
    }
    else // no points loaded
    {
        return 0;
    }
}

template <typename T>
int ParametricInterpolator<T>::pointAt(T val, T* point) const
{
    // number of spline segments
    int N = splines.size();
    if (N >= 1 && points.size() > 1)//(N >= 1)
    {
        // bounds check and force input val to be periodic by parametric range = N
        while (val < 0)
            val += N;
        while (val >= N)
            val -= N;
        // figure out the piecewise polynomial section we need and where within it
        int sec_index = val;//floor(val);
        T sec_val = val - sec_index;
        // get the point
//        T temp [points[0].point.size()];
//        splines[sec_index]->pointAt(sec_val, &temp[0]);
//        for (int i = 0; i < points[0].point.size(); ++i)
//            point[i] = 0;
//        point[0] = 0.5;
//        point[1] = 0.5;
//        point[2] = 0.5;
        splines[sec_index]->pointAt(sec_val, &point);
        if (point)
            return 1;
        else
            return 0;
    }
    else if (points.size() == 1)
    {
        const int D = points[0].point.size();
        for (int j = 0; j < D; ++j)
            point[j] = points[0].point[j];
        return 1;
    }
    else // no points loaded
    {
        return 0;
    }
}

// non-smart version
template <typename T>
int FunctionalInterpolator<T>::pointAt(const T val, std::vector<T>& point) const
{
    // number of points
    int N = points.size();
    if (N > 1)
    {
        // if the val is less than our first point's x value
        if (val < points[0].point[0])
            return 0; // indicating point didn't get a valid value stored to it
        // if the xVal is greater than our last point's x value
        if (val > points.back().point[0])
            return 0; // indicating point didn't get a valid value stored to it
        // figure out the piecewise polynomial section we need
        for (int i = 0; i < N-1; ++i)
        {
            // the section index we are interested in is indicated by the xVal being between the neighboring points' xVals
            if (points[i].point[0] < val && val < points[i+1].point[0])
            {
                point = splines[i]->pointAt(val);
                if (point.empty())
                    return 0;
                else
                    return 1;
            }
            // don't want valid points getting confused with an empty spline
            else if (val == points[i].point[0])
            {
                int D = points[i].point.size();
                point = std::vector<T>(D-1);
                for (int j = 1; j < D; ++j)
                    point[j-1] = points[i].point[j];
                return 1;
            }
            else if (val == points[i+1].point[0])
            {
                int D = points[i+1].point.size();
                point = std::vector<T>(D-1);
                for (int j = 1; j < D; ++j)
                    point[j-1] = points[i+1].point[j];
                return 1;
            }
        }
        // shouldn't really get here, but if so we obviously failed to find the correct spline segment
        return 0;
    }
    else if (N == 1)
    {
        int D = points[0].point.size();
        point = std::vector<T>(D-1);
        for (int i = 1; i < D; ++i)
            point[i-1] = points[0].point[i];
        return 1;
    }
    else // N <= 0
    {
        return 0;
    }
}

template <typename T>
int FunctionalInterpolator<T>::pointAtSmart(const T val, T* point, int& spline_index) const
{
    // number of points
    int N = points.size();
    if (N > 1)
    {
        // if the val is less than our first point's x value
        if (val < points[0].point[0])
        {
            spline_index = 0;
            return 0; // indicating point didn't get a valid value stored to it
        }
        // if the xVal is greater than our last point's x value
        if (val > points.back().point[0])
        {
            spline_index = splines.size()-1;
            return 0; // indicating point didn't get a valid value stored to it
        }
        // figure out the piecewise polynomial section we need
        int begin = spline_index;
        int end = N-1;
        bool first_check = true;
    LOOK_AGAIN:
        //        // prev_spline_index can (did) screw this up when points are deleted ...
        //        if (begin < 0)
        //            begin = 0;
        //        if (end > N-1)
        //            end = N-1;
        for (int i = begin; i < end; ++i)
        {
            // the section index we are interested in is indicated by the xVal being between the neighboring points' xVals
            if (points[i].point[0] < val && val < points[i+1].point[0])
            {
                splines[i]->pointAt(val, &point);
                spline_index = i;
                if (point)
                    return 1;
                else
                    return 0;
            }
            // don't want valid points getting confused with an empty spline
            else if (val == points[i].point[0])
            {
                const int D = points[i].point.size();
                for (int j = 1; j < D; ++j)
                    point[j-1] = points[i].point[j];
                spline_index = i;
                return 1;
            }
            else if (val == points[i+1].point[0])
            {
                const int D = points[i+1].point.size();
                for (int j = 1; j < D; ++j)
                    point[j-1] = points[i+1].point[j];
                spline_index = i+1;
                return 1;
            }
        }
        if (first_check)
        {
            begin = 0;
            end = spline_index;
            first_check = false;
            goto LOOK_AGAIN;
        }
        // shouldn't really get here, but if so we obviously failed to find the correct spline segment
        return 0;
    }
    else if (N == 1)
    {
        const int D = points[0].point.size();
        for (int i = 1; i < D; ++i)
            point[i-1] = points[0].point[i];
        return 1;
    }
    else // N <= 0
    {
        return 0;
    }
}

//template <typename T>
//int FunctionalInterpolator<T>::pointAtSmart(T val, std::vector<T>& point, int& spline_index) const
//{
//    // number of points
//    int N = points.size();
//    if (N > 1)
//    {
//        // if the val is less than our first point's x value
//        if (val < points[0].point[0])
//        {
//            spline_index = 0;
//            return 0; // indicating point didn't get a valid value stored to it
//        }
//        // if the xVal is greater than our last point's x value
//        if (val > points.back().point[0])
//        {
//            spline_index = splines.size()-1;
//            return 0; // indicating point didn't get a valid value stored to it
//        }
//        // figure out the piecewise polynomial section we need
//        int begin = spline_index;
//        int end = N-1;
//        bool first_check = true;
//    LOOK_AGAIN:
////        // prev_spline_index can fuck this up when points are deleted ...
////        if (begin < 0)
////            begin = 0;
////        if (end > N-1)
////            end = N-1;
//        for (int i = begin; i < end; ++i)
//        {
//            // the section index we are interested in is indicated by the xVal being between the neighboring points' xVals
//            if (points[i].point[0] < val && val < points[i+1].point[0])
//            {
//                point = splines[i]->pointAt(val);
//                spline_index = i;
//                if (point.empty())
//                    return 0;
//                else
//                    return 1;
//            }
//            // don't want valid points getting confused with an empty spline
//            else if (val == points[i].point[0])
//            {
//                int D = points[i].point.size();
//                point = std::vector<T>(D-1);
//                for (int j = 1; j < D; ++j)
//                    point[j-1] = points[i].point[j];
//                spline_index = i;
//                return 1;
//            }
//            else if (val == points[i+1].point[0])
//            {
//                int D = points[i+1].point.size();
//                point = std::vector<T>(D-1);
//                for (int j = 1; j < D; ++j)
//                    point[j-1] = points[i+1].point[j];
//                spline_index = i+1;
//                return 1;
//            }
//        }
//        if (first_check)
//        {
//            begin = 0;
//            end = spline_index;
//            first_check = false;
//            goto LOOK_AGAIN;
//        }
//        // shouldn't really get here, but if so we obviously failed to find the correct spline segment
//        return 0;
//    }
//    else if (N == 1)
//    {
//        int D = points[0].point.size();
//        point = std::vector<T>(D-1);
//        for (int i = 1; i < D; ++i)
//            point[i-1] = points[0].point[i];
//        return 1;
//    }
//    else // N <= 0
//    {
//        return 0;
//    }
//}


#endif /* defined __Interpolator__ */
