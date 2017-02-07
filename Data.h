/*
 Data.h
 
 Holds the size of each dimension for the HRTF data in 3DAudioData.bin.
 
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

#ifndef Data_h
#define Data_h

#define numAzimuthSteps  180
#define numElevationSteps  90
#define numTimeSteps  128
#define numDistanceSteps 20
#define distanceBegin  0.1
#define distanceEnd  3.25
#define sampleRate_HRTF  44100.0
#define sphereRad  0.09
#define threshold  0.00000001
#define earElevation  90
#define earAzimuth  85

#endif
