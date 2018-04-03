//
//  Points.h
//  Represents all sorts of point types and associated convenience methods.
//
//  Created by Andrew Barker on 11/13/16.
//
//

#ifndef Points_h
#define Points_h

#include "OpenGL.h"

template <class T>
class PointXYZ {
public:
    T x, y, z;
    
    PointXYZ<T> addX (const T add) const noexcept { return {x + add, y, z}; }
    PointXYZ<T> addY (const T add) const noexcept { return {x, y + add, z}; }
    PointXYZ<T> addZ (const T add) const noexcept { return {x, y, z + add}; }
    PointXYZ<T> subX (const T sub) const noexcept { return {x - sub, y, z}; }
    PointXYZ<T> subY (const T sub) const noexcept { return {x, y - sub, z}; }
    PointXYZ<T> subZ (const T sub) const noexcept { return {x, y, z - sub}; }
    //void normalize() noexcept {
    
};

template <class T>
PointXYZ<T> operator+ (const PointXYZ<T>& l, const PointXYZ<T>& r) noexcept { return {l.x + r.x, l.y + r.y, l.z + r.z}; }

template <class T>
PointXYZ<T> operator- (const PointXYZ<T>& l, const PointXYZ<T>& r) noexcept { return {l.x - r.x, l.y - r.y, l.z - r.z}; }

template <class T>
PointXYZ<T> operator* (const PointXYZ<T>& l, const T r) noexcept            { return {r * l.x, r * l.y, r * l.z}; }

template <class T>
PointXYZ<T> operator* (const T l, const PointXYZ<T>& r) noexcept            { return operator* (r, l); }

template <class T>
PointXYZ<T> operator/ (const PointXYZ<T>& l, const T r) noexcept            { return {l.x / r, l.y / r, l.z / r}; }

template <class T>
PointXYZ<T> normalized (const PointXYZ<T>& p) noexcept
{
    return p / std::hypot(p.x, p.y, p.z);
}

// to perform cross product between 2 vectors in myGluLookAt
template <class T>
PointXYZ<T> crossProduct(const PointXYZ<T>& a, const PointXYZ<T>& b)
{
    return {a.y * b.z - b.y * a.z,
            b.x * a.z - a.x * b.z,
            a.x * b.y - b.x * a.y};
}

template <class T>
T dotProduct(const PointXYZ<T>& a, const PointXYZ<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

static void glVertex(const PointXYZ<float>& pt)
{
    glVertex3f(pt.x, pt.y, pt.z);
}

#endif /* Points_h */
