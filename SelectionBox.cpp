/*
 SelectionBox.cpp
 
 Represents a rectangle that can select stuff, aka click and drag.

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
        box.drawOutline();
        glColor4f(0.6f, 0.6f, 0.6f, 0.2f * alphaFactor);
        box.drawFill();
    }
}

void mouseDragged (SelectionBox& box,
                   const Point<float> mouseDown,
                   const Point<float> mouseCurrent) noexcept
{
    box = {mouseCurrent.y, mouseDown.y, mouseCurrent.x, mouseDown.x};
    box.setActive(true);
}
