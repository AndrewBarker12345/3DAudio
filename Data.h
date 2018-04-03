//
//  Data.h
//
//  Created by Andrew Barker on 6/15/14.
//
//
/*
    3DAudio: simulates surround sound audio for headphones
    Copyright (C) 2016  Andrew Barker

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


//#include <iostream>
//#include <fstream>
//
//// note:  these dimensions determine the size of the dvf/hrir data.  if these are
////          too large then the application may run out of RAM.
////          (especially for 32-bit @ about 2GB)
//#define numAzimuthSteps  180
//#define numElevationSteps  90
//#define numTimeSteps  128
//#define numDistanceSteps 20
//#define distanceBegin  0.1
//#define distanceEnd  3.25
//#define sampleRate_HRTF  44100
//#define sphereRad  0.09
//#define threshold  0.00000001
//#define earElevation  90
//#define earAzimuth  85
//
//// a singleton class to hold the hrir and dvf data so that multiple plugin instances may run without having to consume unecessary RAM
//class Data
//{
//public:
//    static Data& getSingleton() //Return our instance
//    {
//        return mInstance;
//    }
//    
//    void load()
//    {
//        
//    }
//    
//    void clear()
//    {
//        
//    }
//    
//    float***** HRIR;
//    //float HRIR[numDistanceSteps][numAzimuthSteps][numElevationSteps][2][numTimeSteps];
//    
//private:
//    static Data mInstance;
//    
//    Data() // read in data on construction
//    {
//////        // binary hrtf file name
//////        File f;
//////        f.getSpecialLocation(File::currentApplicationFile);
//////        //juce::File f = juce::File::getSpecialLocation(juce::File::currentApplicationFile);
//////        String path = f.getFullPathName();
//////// probably not necessary...
////////#ifdef _WIN32
////////        path += "\";
////////#else
////////        path += "/";
////////#endif
//////        path += "/Contents/3dAudioData.bin";
////////        char fullPath[300];
////////        strcpy(fullPath, path.toRawUTF8());
////        char* fileName = (char*)"/Users/AndrewBarker/Documents/Programming/3dAudio/SphericalHRIR2.bin"/*"SphericalHRIR.bin"*/;
////        // basic read (should be cross platform)
////        // open the stream
////        std::ifstream is(/*path.getCharPointer()*/fileName);
////        if (is.good()) {
////            // determine the file length
////            is.seekg(0, is.end);
////            size_t size = is.tellg();
////            is.seekg(0, is.beg);
////            // create a temp array to store the data
////            char* dataBuffer = new char[size];
////            // load the data
////            is.read(dataBuffer, size);
////            // close the file
////            is.close();
////            // pack data into HRIR array
////            int index = 0;
////            for (int d = 0; d < numDistanceSteps; d++) {
////                for (int a = 0; a < numAzimuthSteps; a++) {
////                    for (int e = 0; e < numElevationSteps; e++) {
////                        for (int ch = 0; ch < 2; ch++) {
////                            for (int t = 0; t < numTimeSteps; t++) {
////                                HRIR[d][a][e][ch][t] = *((float*) &dataBuffer[index*sizeof(float)]);
////                                //HRIR[d][a][e][ch][t] = 0;
////                                index++;
////                            }
////                        }
////                    }
////                }
////            }
////            // cleanup the temp array
////            delete[] dataBuffer;
////        } else {
////            // failed to open hrtf binary file, load up zeros
////            int index = 0;
////            for (int d = 0; d < numDistanceSteps; d++) {
////                for (int a = 0; a < numAzimuthSteps; a++) {
////                    for (int e = 0; e < numElevationSteps; e++) {
////                        for (int ch = 0; ch < 2; ch++) {
////                            for (int t = 0; t < numTimeSteps; t++) {
////                                HRIR[d][a][e][ch][t] = 0;
////                                index++;
////                            }
////                        }
////                    }
////                }
////            }
////        }
//    }
//
//    ~Data() {}
//    
//    // Dont forget to declare these two. You want to make sure they
//    // are unaccessable otherwise you may accidently get copies of
//    // your singleton appearing.
//    Data(Data const&);           // Don't Implement
//    void operator=(Data const&); // Don't implement
//};
//
//Data Data::mInstance;

#endif
