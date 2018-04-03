//
//  Box.cpp
//
//  Created by Andrew Barker on 9/19/16.
//
//

#include "Box.h"
#include "OpenGL.h"
#include <algorithm>


Box::Box() noexcept
    : top    (0),
      bottom (0),
      left   (0),
      right  (0)
{
}

Box::Box(const float top,
         const float bottom,
         const float left,
         const float right) noexcept
    : top    (std::max(top, bottom)),
      bottom (std::min(top, bottom)),
      left   (std::min(left, right)),
      right  (std::max(left, right))
{
}

Box::Box(const Point<float>& pt,
         const float radius) noexcept
    : top    (pt.y + radius),
      bottom (pt.y - radius),
      left   (pt.x - radius),
      right  (pt.x + radius)
{
}

bool Box::operator== (const Box& box) const noexcept
{
    return top == box.getTop() &&
    bottom == box.getBottom() &&
    left == box.getLeft() &&
    right == box.getRight();
}

bool Box::operator!= (const Box& box) const noexcept
{
    return top != box.getTop() ||
    bottom != box.getBottom() ||
    left != box.getLeft() ||
    right != box.getRight();
}

float Box::getTop() const noexcept { return top; }
void Box::setTop(const float newTop) noexcept
{
    if (newTop > bottom)
        top = newTop;
    else {
        top = bottom;
        bottom = newTop;
    }
}

float Box::getBottom() const noexcept { return bottom; }
void Box::setBottom(const float newBottom) noexcept
{
    if (newBottom < top)
        bottom = newBottom;
    else {
        bottom = top;
        top = newBottom;
    }
}

float Box::getLeft() const noexcept { return left; }
void Box::setLeft(const float newLeft) noexcept
{
    if (newLeft < right)
        left = newLeft;
    else {
        left = right;
        right = newLeft;
    }
}

float Box::getRight() const noexcept { return right; }
void Box::setRight(const float newRight) noexcept
{
    if (newRight > left)
        right = newRight;
    else {
        right = left;
        left = newRight;
    }
}

float Box::width () const noexcept { return getRight() - getLeft(); }
float Box::height () const noexcept { return getTop() - getBottom(); }
float Box::area () const noexcept { return width() * height(); }
float Box::centerX() const noexcept { return getLeft() + 0.5f * width(); }
float Box::centerY() const noexcept { return getBottom() + 0.5f * height(); }

void Box::drawVerticies () const
{
    for (const auto& pt : boundaryPoints())
        glVertex2f(pt.x, pt.y);
//    glVertex2f(getLeft(),  getTop());
//    glVertex2f(getRight(), getTop());
//    glVertex2f(getRight(), getBottom());
//    glVertex2f(getLeft(),  getBottom());
}
void Box::drawOutline () const
{
    glBegin(GL_LINE_LOOP);
    drawVerticies();
    glEnd();
}
void Box::drawFill () const
{
    glBegin(GL_QUADS);
    drawVerticies();
    glEnd();
}

std::vector<Point<float>> Box::boundaryPoints () const noexcept
{
    return {{getLeft(),  getTop()},
            {getRight(), getTop()},
            {getRight(), getBottom()},
            {getLeft(),  getBottom()}};
}

bool Box::contains (const Point<float>& pt) const noexcept
{
    return getLeft() < pt.x && pt.x < getRight() &&
           getBottom() < pt.y && pt.y < getTop();
}

bool Box::overlaps (const Box& other) const noexcept
{
    return ! (getBottom() > other.getTop() || getTop() < other.getBottom() ||
              getLeft() > other.getRight() || getRight() < other.getLeft());
}

void Box::crop (const Box& crop) noexcept
{
    if (getTop() > crop.getTop())
        setTop(crop.getTop());
    if (getBottom() < crop.getBottom())
        setBottom(crop.getBottom());
    if (getLeft() < crop.getLeft())
        setLeft(crop.getLeft());
    if (getRight() > crop.getRight())
        setRight(crop.getRight());
}

Box Box::combinedWith (const Box& other) const noexcept
{
    return { std::max(getTop(), other.getTop()), std::min(getBottom(), other.getBottom()),
             std::min(getLeft(), other.getLeft()), std::max(getRight(), other.getRight()) };
}

Box Box::scaled (const float hScale,
                 const float vScale) const noexcept
{
    const float dWidth  = width()  * 0.5f * (1 - hScale);
    const float dHeight = height() * 0.5f * (1 - vScale);
    const float top     = getTop()    - dHeight;
    const float bottom  = getBottom() + dHeight;
    const float left    = getLeft()   + dWidth;
    const float right   = getRight()  - dWidth;
    return {top, bottom, left, right};
}

void Box::move (const float dx,
                const float dy) noexcept
{
    *this = {getTop() + dy, getBottom() + dy, getLeft() + dx, getRight() + dx};
}


//float width (const Box& b) noexcept
//{
//    return b.getRight() - b.getLeft();
//}
//
//float height (const Box& b) noexcept
//{
//    return b.getTop() - b.getBottom();
//}
//
//float area (const Box& b) noexcept
//{
//    return width(b) * height(b);
//}
//
//void drawVerticies (const Box& b)
//{
//    glVertex2f(b.getLeft(),  b.getTop());
//    glVertex2f(b.getRight(), b.getTop());
//    glVertex2f(b.getRight(), b.getBottom());
//    glVertex2f(b.getLeft(),  b.getBottom());
//}
//
//void drawOutline (const Box& b)
//{
//    glBegin(GL_LINE_LOOP);
//    drawVerticies(b);
//    glEnd();
//}
//
//void drawFill (const Box& b)
//{
//    glBegin(GL_QUADS);
//    drawVerticies(b);
//    glEnd();
//}
//
//std::vector<Point<float>> boundaryPoints (const Box& b) noexcept
//{
//    return {{b.getLeft(),  b.getTop()},
//            {b.getRight(), b.getTop()},
//            {b.getRight(), b.getBottom()},
//            {b.getLeft(),  b.getBottom()}};
//}
//
//bool contain (const Box& b,
//              const Point<float>& pt) noexcept
//{
//    return b.getLeft() < pt.x && pt.x < b.getRight() &&
//           b.getBottom() < pt.y && pt.y < b.getTop();
//}
//
//bool overlap (const Box& b1,
//              const Box& b2) noexcept
//{
//     return ! (b1.getBottom() > b2.getTop() || b1.getTop() < b2.getBottom() ||
//               b1.getLeft() > b2.getRight() || b1.getRight() < b2.getLeft());
//}
//
//void crop (Box& b,
//           const Box& crop) noexcept
//{
//    if (b.getTop() > crop.getTop())
//        b.setTop(crop.getTop());
//    if (b.getBottom() < crop.getBottom())
//        b.setBottom(crop.getBottom());
//    if (b.getLeft() < crop.getLeft())
//        b.setLeft(crop.getLeft());
//    if (b.getRight() > crop.getRight())
//        b.setRight(crop.getRight());
//}
//
//Box combined (const Box& b1,
//              const Box& b2) noexcept
//{
//    return { std::max(b1.getTop(), b2.getTop()), std::min(b1.getBottom(), b2.getBottom()),
//             std::min(b1.getLeft(), b2.getLeft()), std::max(b1.getRight(), b2.getRight()) };
//}
//
//Box getScaled (const Box& b,
//               const float hScale,
//               const float vScale) noexcept
//{
//    const float dWidth  = width(b)  * 0.5f * (1 - hScale);
//    const float dHeight = height(b) * 0.5f * (1 - vScale);
//    const float top     = b.getTop()    - dHeight;
//    const float bottom  = b.getBottom() + dHeight;
//    const float left    = b.getLeft()   + dWidth;
//    const float right   = b.getRight()  - dWidth;
//    return {top, bottom, left, right};
//}
//    
//void move(Box& b,
//          const float dx,
//          const float dy) noexcept
//{
//    b.top += dy;
//    b.bottom += dy;
//    b.left += dx;
//    b.right += dx;
//}
//
////void placeWithin(Box& b,
////                 const Box& placeWithin,
////                 const std::vector<Box>& noOverlap) noexcept
////{
////    int dirCount = 0;
////    for (const auto & x : noOverlap) {
////        if (overlap(b, x)) {
////            move(b,
////        }
////    }
////}
