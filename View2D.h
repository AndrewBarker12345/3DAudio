//
//  View2D.h
//
//  Created by Andrew Barker on 9/21/16.
//
//

#ifndef View2D_h
#define View2D_h

#include "../JuceLibraryCode/JuceHeader.h"
#include "Box.h"
#include "BoundedValue.h"

//class View2DOld
//{
//public:
//    float width = 1, height = 1;
//    float xOffset = 0, yOffset = 0;
//    float xScale = 1, yScale = 1;
//};

/** represents a 2D, vertically/horizontally scrollable/zoomable view positioned within a holding box */
class View2D
{
public:
    /** initializes a zero-sized view with no holding box */
    View2D () noexcept;
    /** initializes a view of a specfied size and position within a holding box 
        holder:    the holding box
        boundary:  the view's boundary within the holding box in terms of the holding box's coordinates 
        width:     the horizontal width of the view including its bounds in terms of the view's coordinates
        height:    the vertical height of the view including its bounds in terms of the view's coordinates
        xPosition: the horizontal position of the view, defined as the x coordinate (view's coordinates) of the view's center
        xMin/xMax: the minimum and maximum x coordinates (view's coordinates) visible through the view
        yPosition: the vertical position of the view, defined as the y coordinate (view's coordinates) of the view's center
        yMin/yMax: the minimum and maximum y coordinates (view's coordinates) visible through the view
    */
    View2D (Box* holder,
            const Box& boundary,
            const BoundedValue<float>& width,
            const BoundedValue<float>& height,
            float xPosition, float xMin, float xMax,
            float yPosition, float yMin, float yMax) noexcept;
    
    /** returns a pointer to the holding box for this view */
    const Box* getHolder () const noexcept;
    /** sets the holding box for this view to some external box 
        NOTE: the holding box is NOT required to contain the entire boundary for the view */
    void setHolder (Box* holder) noexcept;
    
    /** returns the boundary box in terms of the holding box's coordinates for this view */
    Box getBoundary () const noexcept;
    /** sets the boundary box in terms of the holding box's coordinates for this view 
        NOTE: the holding box is NOT required to contain the entire boundary for the view */
    void setBoundary (const Box& boundary) noexcept;
    
    /** gets the width of the view (in the view's coordinates) along with the bounds for the width */
    BoundedValue<float> getWidth () const noexcept;
    /** sets the width of the view (in the view's coordinates) with bounds checking */
    void setWidth (float width) noexcept;
    /** sets the width and it's bounds for the view (in the view's coordinates) */
    void setWidth (const BoundedValue<float>& width) noexcept;
    
    /** gets the height of the view (in the view's coordinates) along with the bounds for the height */
    BoundedValue<float> getHeight () const noexcept;
    /** sets the height of the view (in the view's coordinates) with bounds checking */
    void setHeight (float height) noexcept;
    /** sets the height and it's bounds for the view (in the view's coordinates) */
    void setHeight (const BoundedValue<float>& height) noexcept;
    
    /** returns the view's horizontal position, defined as the x view coordinate of the view's center */
    float getXPosition () const noexcept;
    /** sets the view's horizontal position, defined as the x view coordinate of the view's center, with bounds checking */
    void setXPosition (float xPosition) noexcept;
    /** returns the view's vertical position, defined as the y view coordinate of the view's center */
    float getYPosition () const noexcept;
    /** sets the view's horizontal position, defined as the y view coordinate of the view's center, with bounds checking */
    void setYPosition (float yPosition) noexcept;
    
    /** returns the minimum x view coordinate visible by the view */
    float getXMin () const noexcept;
    /** sets the minimum x view coordinate visible by the view */
    void setXMin (float xMin) noexcept;
    /** returns the maximum x view coordinate visible by the view */
    float getXMax () const noexcept;
    /** sets the maximum x view coordinate visible by the view */
    void setXMax (float xMax) noexcept;
    /** returns the minimum y view coordinate visible by the view */
    float getYMin () const noexcept;
    /** sets the minimum y view coordinate visible by the view */
    void setYMin (float yMin) noexcept;
    /** returns the maximum y view coordinate visible by the view */
    float getYMax () const noexcept;
    /** sets the maximum y view coordinate visible by the view */
    void setYMax (float yMax) noexcept;
    
    /** returns the minimum x view coordinate visible by the view at the view's current position */
    float xMinCurrent () const noexcept;
    /** returns the maximum x view coordinate visible by the view at the view's current position */
    float xMaxCurrent () const noexcept;
    /** returns the minimum y view coordinate visible by the view at the view's current position */
    float yMinCurrent () const noexcept;
    /** returns the maximum y view coordinate visible by the view at the view's current position */
    float yMaxCurrent () const noexcept;
    
    /** returns a view x coordinate from a holder x coordinate */
    float holderToViewX (float holderX) const noexcept;
    /** returns a view y coordinate from a holder y coordinate */
    float holderToViewY (float holderY) const noexcept;
    /** returns a point in view coordinates from a holder coordinate point */
    Point<float> holderToView (const Point<float>& pt) const noexcept;
    /** returns a box in view coordinates from a holder coordinate box */
    Box holderToView (const Box& box) const noexcept;
    
    /** returns a holder x coordinate from a view x coordinate */
    float viewToHolderX (float viewX) const noexcept;
    /** returns a holder y coordinate from a view y coordinate */
    float viewToHolderY (float viewY) const noexcept;
    /** returns a point in holder coordinates from a view coordinate point */
    Point<float> viewToHolder (const Point<float>& pt) const noexcept;
    /** returns a box in holder coordinates from a view coordinate box */
    Box viewToHolder (const Box& box) const noexcept;
    
private:
    /** these are just a nice way to reduce code duplication for the implementation */
    template <class T>
    void setWidthInternal (const T& width) noexcept;
    template <class T>
    void setHeightInternal (const T& height) noexcept;
    
    Box *_holder, _boundary;
    BoundedValue<float> _width, _height, _xPosition, _yPosition;
};


//namespace View2DFuncs {
//
//float viewX (const View2D& view,
//             float normalizedX) noexcept;
//
//float viewY (const View2D& view,
//             float normalizedY) noexcept;
//
//Point<float> viewPoint (const View2D& view,
//                        const Point<float>& normalizedPoint) noexcept;
//
//Box viewBox (const View2D& view,
//             const Box& normalizedBox) noexcept;
//    
//float normalizedX (const View2D& view,
//                   float viewX) noexcept;
//
//float normalizedY (const View2D& view,
//                   float viewY) noexcept;
//
//Point<float> normalizedPoint (const View2D& view,
//                              const Point<float>& viewPoint) noexcept;
//
//Box normalizedBox (const View2D& view,
//                   const Box& viewBox) noexcept;
//    
//}

#endif /* View2D_h */
