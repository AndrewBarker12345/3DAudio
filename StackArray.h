//
//  StackArray.h
//
//  Created by Andrew Barker on 9/14/16.
//
//

#ifndef StackArray_h
#define StackArray_h

// windows compatible stack array with size determined at runtime
#ifdef WIN32
  #define STACK_ARRAY(type, name, size)  type *name = static_cast<type *>(alloca((size) * sizeof(type)));
#else
  #define STACK_ARRAY(type, name, size)  type name[size];
#endif

#endif /* StackArray_h */
