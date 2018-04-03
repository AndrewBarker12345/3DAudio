//
//  OpenGL.h
//
//  Created by Andrew Barker on 9/19/16.
//
//

#ifndef OpenGL_h
#define OpenGL_h

#ifdef __APPLE__
  #include <OpenGL/gl.h>
#else // windows or linux
  #include <GL/gl.h>
#endif

//#include "../JuceLibraryCode/JuceHeader.h"
//
//void glColour(const Colour& color)
//{
//    glColor4f(color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue(), color.getFloatAlpha());
//}

#endif /* OpenGL_h */
