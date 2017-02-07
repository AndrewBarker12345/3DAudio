/*
 Box.h
 
 A 2D rectangle enforcing that top > bottom and left < right.
 
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
 
#ifndef Box_h
#define Box_h

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>

class Box
{
public:
    /** creates an empty box */
    Box () noexcept;
    /** creates a box from the specified coordinates of its edges */
    Box (float top, float bottom, float left, float right) noexcept;
    /** creates a square box centered around a point and width and height of 2 * radius */
    Box (const Point<float>& pt, float radius) noexcept;
    
    /** returns true if both boxes have the same edge coordinates */
    bool operator== (const Box& box) const noexcept;
    /** returns true if both boxes don't have the same edge coordinates */
    bool operator!= (const Box& box) const noexcept;
    
    /** returns the y coordinate of the box's top edge */
    float getTop () const noexcept;
    /** sets the y coordinate of the box's bottom edge */
    void setTop (float top) noexcept;
    /** returns the y coordinate of the box's bottom edge */
    float getBottom () const noexcept;
    /** sets the Y coordinate of the box's bottom edge */
    void setBottom (float bottom) noexcept;
    /** returns the x coordinate of the box's left edge */
    float getLeft () const noexcept;
    /** sets the x coordinate of the box's left edge */
    void setLeft (float left) noexcept;
    /** returns the x coordinate of the box's right edge */
    float getRight () const noexcept;
    /** sets the x coordinate of the box's right edge */
    void setRight (float right) noexcept;
    
    /** returns the width of the box */
    float width () const noexcept;
    /** returns the height of the box */
    float height () const noexcept;
    /** returns the area of the box */
    float area () const noexcept;
    /** returns the x coordinate horizontal center of the box */
    float centerX() const noexcept;
    /** returns the y coordinate vertical center of the box */
    float centerY() const noexcept;
    
    /** calls glVertex2f() for all the corner points of the box */
    void drawVerticies () const;
    /** draws the outline of the box using OpenGL */
    void drawOutline () const;
    /** draws a fill of the box using OpenGL */
    void drawFill () const;
    
    /** returns all the corner points of the box */
    std::vector<Point<float>> boundaryPoints () const noexcept;
    
    /** returns true if the specified point is inside the box */
    bool contains (const Point<float>& pt) const noexcept;
    /** returns true if this box overlaps at all with another */
    bool overlaps (const Box& other) const noexcept;
    /** crops this box by another */
    void crop (const Box& crop) noexcept;
    /** returns a box that is the minimum needed to contain this box and another */
    Box combinedWith (const Box& other) const noexcept;
    /** returns a box that is this box horizontally and vertically scaled as specified */
    Box scaled (float hScale, float vScale) const noexcept;
    /** moves this box by a given x/y distance */
    void move (float dx, float dy) noexcept;
    
private:
    float top, bottom, left, right;
};

#endif /* Box_h */