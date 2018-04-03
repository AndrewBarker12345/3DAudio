//
//  Animation.cpp
//
//  Created by Andrew Barker on 4/24/16.
//
//

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
    