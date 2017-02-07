/*
 Animation.cpp
 
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

#include "Animation.h"

Animation::Animation() noexcept
    : durationInSeconds(1), currentTimeInSeconds(1)
{
}

Animation::Animation(const float durationInSeconds,
                     const bool startImmediately) noexcept
    : durationInSeconds(durationInSeconds),
      currentTimeInSeconds(startImmediately ? 0 : durationInSeconds)
{
}

void Animation::advance(const float frameRate) noexcept
{
    currentTimeInSeconds += 1.0f / frameRate;
}

void Animation::restart() noexcept
{
    currentTimeInSeconds = 0;
}

void Animation::restart(const float newDurationInSeconds) noexcept
{
    currentTimeInSeconds = 0;
    durationInSeconds = newDurationInSeconds;
}

void Animation::finish() noexcept
{
    currentTimeInSeconds = durationInSeconds;
}

float Animation::getProgress() const noexcept
{
    if (currentTimeInSeconds < durationInSeconds)
        return currentTimeInSeconds / durationInSeconds;
    else
        return 1.0f;
}

bool Animation::isPlaying() const noexcept
{
    return currentTimeInSeconds < durationInSeconds;
}

float Animation::getDuration() const noexcept
{
    return durationInSeconds;
}
    