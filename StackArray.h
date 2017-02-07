/*
 StackArray.h
 
 Cross-platform (windows compatible) stack array with size determined at runtime.

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

#ifndef StackArray_h
#define StackArray_h

#ifdef WIN32
  #define STACK_ARRAY(type, name, size)  type *name = static_cast<type *>(alloca((size) * sizeof(type)));
#else
  #define STACK_ARRAY(type, name, size)  type name[size];
#endif

#endif /* StackArray_h */
