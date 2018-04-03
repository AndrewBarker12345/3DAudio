//
//  SelectionBox.h
//
//  Created by Andrew Barker on 9/20/16.
//
//

#ifndef SelectionBox_h
#define SelectionBox_h

#include "Animation.h"
#include "Box.h"
#include "OpenGLWindow.h"

class SelectionBox : public Box
{
public:
    SelectionBox() noexcept;
    SelectionBox(float top, float bottom, float left, float right) noexcept;
    
    const Animation& getAnimation() const noexcept;
    Animation& getAnimation() noexcept;
//    void setAnimation(const Animation& newAnimation) noexcept;
    
    bool isActive() const noexcept;
    void setActive(bool active) noexcept;
    
private:
    bool active;
    Animation animation;
};

// can SelectionBox be used with contain(Box, Point<float>)?
void draw (SelectionBox& box,
           const OpenGLWindow& window);
void mouseDragged (SelectionBox& box,
                   const Point<float> mouseDown,
                   const Point<float> mouseCurrent) noexcept;
//bool isDragging (const SelectionBox& box) noexcept;

#endif /* SelectionBox_h */
