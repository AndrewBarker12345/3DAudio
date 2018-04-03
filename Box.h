//
//  Box.h
//
//  Created by Andrew Barker on 9/19/16.
//
//

#ifndef Box_h
#define Box_h

#include "../JuceLibraryCode/JuceHeader.h"
#include <vector>

//template <class T>
//struct Point { T x, y; };

/** A 2D rectangle enforcing that top > bottom and left < right */
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
//    friend void move(Box& b,
//                     float dx,
//                     float dy) noexcept;
    
private:
    float top, bottom, left, right;
};

//float width (const Box& b) noexcept;
//float height (const Box& b) noexcept;
//float area (const Box& b) noexcept;
//void drawVerticies (const Box& b);
//void drawOutline (const Box& b);
//void drawFill (const Box& b);
//std::vector<Point<float>> boundaryPoints(const Box& b) noexcept;
//bool contain (const Box& b,
//              const Point<float>& pt) noexcept;
//bool overlap (const Box& b1,
//              const Box& b2) noexcept;
//void crop (Box& b,
//           const Box& crop) noexcept;
//Box combined (const Box& b1,
//              const Box& b2) noexcept;
//Box getScaled (const Box& b,
//               float hScale,
//               float vScale) noexcept;
//void move(Box& b,
//          float dx,
//          float dy) noexcept;
////void placeWithin(Box& b,
////                 const Box& placeWithin,
////                 const std::vector<Box>& noOverlap) noexcept;

#endif /* Box_h */