//
//  Animation.h
//
//  Created by Andrew Barker on 4/24/16.
//
//

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
