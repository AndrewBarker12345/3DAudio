//
//  OpenGLWindow.h
//
//  Created by Andrew Barker on 9/20/16.
//
//

#ifndef OpenGLWindow_h
#define OpenGLWindow_h

#include "../JuceLibraryCode/JuceHeader.h"

class OpenGLWindow
{
public:
    OpenGLWindow(OpenGLContext* context) noexcept : context(context) {}
    void checkResized(int w, int h) noexcept
    {
        if (w != width || h != height) {
            resized = true;
            width  = w;
            height = h;
        }
    }
    void saveResized() noexcept
    {
        resized = false;
    }
    float getAspect() const noexcept
    {
        return ((float)height) / width;
    }
    OpenGLContext* context = nullptr;
    int width = 0, height = 0;
    float frameRate = 30; // lets be artsy and get that "film" look, haha
    bool resized = true;
};

#endif /* OpenGLWindow_h */
