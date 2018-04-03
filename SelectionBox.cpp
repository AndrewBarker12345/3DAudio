//
//  SelectionBox.cpp
//
//  Created by Andrew Barker on 9/20/16.
//
//

#include "SelectionBox.h"
#include "OpenGL.h"

SelectionBox::SelectionBox() noexcept
    : animation(0.5f)
{
}

SelectionBox::SelectionBox(const float top,
                           const float bottom,
                           const float left,
                           const float right) noexcept
    : Box(top, bottom, left, right),
      animation(0.5f)
{
}

const Animation& SelectionBox::getAnimation() const noexcept
{
    return animation;
}

Animation& SelectionBox::getAnimation() noexcept
{
    return animation;
}
//void SelectionBox::setAnimation(const Animation& newAnimation) noexcept
//{
//    animation = newAnimation;
//}

bool SelectionBox::isActive() const noexcept
{
    return active;
}

void SelectionBox::setActive(const bool newActive) noexcept
{
    active = newActive;
}



void draw (SelectionBox& box,
           const OpenGLWindow& window)
{
    Animation& animation = box.getAnimation();
    const bool active = box.isActive();
    if (active || animation.isPlaying()) {
        const float alphaFactor = active ? 1 : 1 - animation.getProgress();
        animation.advance(window.frameRate);
        glColor4f(1, 1, 1, 1 * alphaFactor);
        box.drawOutline();// drawOutline(box);
        glColor4f(0.6f, 0.6f, 0.6f, 0.2f * alphaFactor);
        box.drawFill();// drawFill(box);
    }
}

void mouseDragged (SelectionBox& box,
                   const Point<float> mouseDown,
                   const Point<float> mouseCurrent) noexcept
{
    box = {mouseCurrent.y, mouseDown.y, mouseCurrent.x, mouseDown.x};
    box.setActive(true);
}

//bool isDragging (const SelectionBox& box) noexcept
//{
//    return area(box) != 0 && box.getAnimation().getProgress() == 0;
//}

