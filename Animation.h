/*
 Animation.h
 
 Keeps track of counting and progress for drawing animations.
 
 Copyright (C) 2017 Andrew Barker
 
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

#ifndef Animation_h
#define Animation_h

class Animation
{
public:
    Animation() noexcept;
    Animation(float durationInSeconds, bool startImmediately = false) noexcept;
    void advance(float frameRate) noexcept;
    void restart() noexcept;
    void restart(float durationInSeconds) noexcept;
    void finish() noexcept;
    float getProgress() const noexcept;
    bool isPlaying() const noexcept;
    float getDuration() const noexcept;
private:
    float durationInSeconds, currentTimeInSeconds;
};

#endif /* Animation_h */
