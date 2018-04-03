//
//  DrewLib.h
//  Andrew Barker's C++ convenience library.
//
//  Created by Andrew Barker on 11/13/16.
//
//

#ifndef DrewLib_h
#define DrewLib_h

#define cauto   const auto
#define cint    const int
#define cfloat  const float

#ifdef WIN32
  static constexpr float M_PI   = 3.14159265358979323846264338327950288;
  static constexpr float M_PI_2 = 1.57079632679489661923132169163975144;
#endif

#include "Functions.h"

#endif /* DrewLib_h */
