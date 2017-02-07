/*
 View2D.cpp
 
 Represents a 2D, vertically/horizontally scrollable/zoomable view positioned within a holding box.

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

#include "View2D.h"

#include <algorithm>

View2D::View2D () noexcept
    : _holder    (nullptr),
      _boundary  (),
      _width     (),
      _height    (),
      _xPosition (),
      _yPosition ()
{}

View2D::View2D (Box* holder,
                const Box& boundary,
                const BoundedValue<float>& width,
                const BoundedValue<float>& height,
                float xPosition, float xMin, float xMax,
                float yPosition, float yMin, float yMax) noexcept
    : _holder    (holder),
      _boundary  (boundary),
      _width     (width),
      _height    (height),
      _xPosition (xPosition, std::min(xMin, xMax) + width  * 0.5f, std::max(xMin, xMax) - width  * 0.5f),
      _yPosition (yPosition, std::min(yMin, yMax) + height * 0.5f, std::max(yMin, yMax) - height * 0.5f)
{}

const Box* View2D::getHolder () const noexcept { return _holder; }
void View2D::setHolder (Box* holder) noexcept { _holder = holder; }
    
Box View2D::getBoundary () const noexcept { return _boundary; }
void View2D::setBoundary (const Box& boundary) noexcept { _boundary = boundary; }
    
BoundedValue<float> View2D::getWidth () const noexcept { return _width; }
void View2D::setWidth (const float width) noexcept { setWidthInternal(width); }
void View2D::setWidth (const BoundedValue<float>& width) noexcept { setWidthInternal(width); }
template <class T>
void View2D::setWidthInternal (const T& width) noexcept
{
    const auto absoluteMinX = getXMin();
    const auto absoluteMaxX = getXMax();
    _width = width;
    _xPosition.setMin(absoluteMinX + _width * 0.5f);
    _xPosition.setMax(absoluteMaxX - _width * 0.5f);
}

BoundedValue<float> View2D::getHeight () const noexcept { return _height; }
void View2D::setHeight (const float height) noexcept { setHeightInternal(height); }
void View2D::setHeight (const BoundedValue<float>& height) noexcept { setHeightInternal(height); }
template <class T>
void View2D::setHeightInternal (const T& height) noexcept
{
    const auto absoluteMinY = getYMin();
    const auto absoluteMaxY = getYMax();
    _height = height;
    _yPosition.setMin(absoluteMinY + _height * 0.5f);
    _yPosition.setMax(absoluteMaxY - _height * 0.5f);
}

float View2D::getXPosition () const noexcept { return _xPosition; }
void View2D::setXPosition (const float xPosition) noexcept { _xPosition = xPosition; }

float View2D::getYPosition () const noexcept { return _yPosition; }
void View2D::setYPosition (const float yPosition) noexcept { _yPosition = yPosition; }

float View2D::getXMin () const noexcept { return _xPosition.getMin() - getWidth() * 0.5f; }
void View2D::setXMin (const float xMin) noexcept { _xPosition.setMin(xMin + getWidth() * 0.5f); }

float View2D::getXMax () const noexcept { return _xPosition.getMax() + getWidth() * 0.5f; }
void View2D::setXMax (const float xMax) noexcept { return _xPosition.setMax(xMax - getWidth() * 0.5f); }

float View2D::getYMin () const noexcept { return _yPosition.getMin() - getHeight() * 0.5f; }
void View2D::setYMin (const float yMin) noexcept { _yPosition.setMin(yMin + getHeight() * 0.5f); }

float View2D::getYMax () const noexcept { return _yPosition.getMax() + getHeight() * 0.5f; }
void View2D::setYMax (const float yMax) noexcept { _yPosition.setMax(yMax - getHeight() * 0.5f); }

// getters/setters interface over.  only use the getters/setters, not the impl variables from here on!!!
float View2D::xMinCurrent () const noexcept { return getXPosition() - getWidth() * 0.5f; }
float View2D::xMaxCurrent () const noexcept { return getXPosition() + getWidth() * 0.5f; }
float View2D::yMinCurrent () const noexcept { return getYPosition() - getHeight() * 0.5f; }
float View2D::yMaxCurrent () const noexcept { return getYPosition() + getHeight() * 0.5f; }

float View2D::holderToViewX (float holderX) const noexcept { return (holderX - getBoundary().getLeft()) / getBoundary().width() * getWidth() + xMinCurrent(); }
float View2D::holderToViewY (float holderY) const noexcept { return (holderY - getBoundary().getBottom()) / getBoundary().height() * getHeight() + yMinCurrent(); }
Point<float> View2D::holderToView(const Point<float>& pt) const noexcept { return { holderToViewX(pt.x), holderToViewY(pt.y) }; }
Box View2D::holderToView(const Box& b) const noexcept
{
    return { holderToViewY(b.getTop()),  holderToViewY(b.getBottom()),
             holderToViewX(b.getLeft()), holderToViewX(b.getRight()) };
}

float View2D::viewToHolderX (float viewX) const noexcept { return (viewX - xMinCurrent()) / getWidth() * getBoundary().width() + getBoundary().getLeft(); }
float View2D::viewToHolderY (float viewY) const noexcept { return (viewY - yMinCurrent()) / getHeight() * getBoundary().height() + getBoundary().getBottom(); }
Point<float> View2D::viewToHolder(const Point<float>& pt) const noexcept { return { viewToHolderX(pt.x), viewToHolderY(pt.y) }; }
Box View2D::viewToHolder(const Box& b) const noexcept
{
    return { viewToHolderY(b.getTop()),  viewToHolderY(b.getBottom()),
             viewToHolderX(b.getLeft()), viewToHolderX(b.getRight()) };
}
