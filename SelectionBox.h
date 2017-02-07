/*
 SelectionBox.h
 
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

    bool isActive() const noexcept;
    void setActive(bool active) noexcept;
    
private:
    bool active;
    Animation animation;
};

void draw (SelectionBox& box,
           const OpenGLWindow& window);
void mouseDragged (SelectionBox& box,
                   const Point<float> mouseDown,
                   const Point<float> mouseCurrent) noexcept;

#endif /* SelectionBox_h */
