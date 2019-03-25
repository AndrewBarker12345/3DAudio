//
//  SoundSource.h
//
//  Created by Andrew Barker on 7/2/14.
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

#ifndef __SoundSource__
#define __SoundSource__

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrewLib.h"
#include "Doppler.h"
#include "Interpolator.h"
#include "Data.h"
#include "StackArray.h"
#include <array>

//#ifdef WIN32
//  static constexpr float M_PI   = 3.14159265358979323846264338327950288;
//  static constexpr float M_PI_2 = 1.57079632679489661923132169163975144;
//#endif

// for visually representing a sound source in space and editing it
class SoundSource
{
    friend class PlayableSoundSource;
public:
    // construct a source at the default location
    SoundSource();
    // construct a source at a spaital xyz location
    SoundSource(const std::array<float, 3>& xyz);
    // copy constructor
    SoundSource(const SoundSource& source);
    // copy assignment operator
    SoundSource& operator=(const SoundSource& source);
    // non-const copy assignment operator only copies the path and pathPos interps if they were modified
    //SoundSource& operator=(SoundSource& source);
    // constructor for reconstruction of source saved in XML
    SoundSource(XmlElement *sourceXML);
    // get an XML representation of this object in order to save its state in a plugin setting
    XmlElement* getXML() const;
    // get an XML for an interp to save its state
    XmlElement* getXML(const Interpolator<float>& interp) const;
    // create an interp from its saved XML state
    std::unique_ptr<Interpolator<float>> getInterpolator(const XmlElement& interpXML) const;
    // bounds checking for where the source/path pts can exist
    void boundsCheckRAE(std::array<float, 3>& rae, float& eleDirection) noexcept;
    void boundsCheckRAE(float (&rae)[3], float& eleDirection) noexcept;
    void boundsCheckXYZ(std::array<float, 3>& xyz);
    // control source position with rae coordinate
    void setPosRAE(std::array<float, 3>& rae);
    std::array<float, 3> getPosRAE() const;
    // needed tor moving the source continously in ele
    void setEleDir(float newEleDir);
    float getEleDir() const;
    // control the source position with xyz coordinate
    void setPosXYZ(const std::array<float, 3>& xyz);
    void setPosXYZ(const float* xyz);
    //void setPosXYZ(const float (&xyz)[3]);
    std::array<float, 3> getPosXYZ() const;
    // set the source position given a time, playing state, and previous pathPos index from the realtime processing thread
    bool setParametricPosition(float posSec, int& prevPathPosIndex, float parametricPositionFromDAW = -1);
    void setPositionUpdate(const std::array<float, 3>& newPosRAE, bool newMuted);
    // control if the source is selected for editing
    void setSourceSelected(bool newSourceSelected) noexcept;
    bool getSourceSelected() const noexcept;
    // control if the source is processing audio or not
    void setSourceMuted(bool newMutedState);
    bool getSourceMuted() const;
    // source path interpolator interaction
    //std::unique_ptr<ParametricInterpolator<float>> getPath() const;
    ParametricInterpolator<float>* const getPathPtrMutable();
    const ParametricInterpolator<float>* const getPathPtr() const;
    void addPathPoint();
    void addPathPoint(std::array<float, 3>& xyz);
    int deleteSelectedPathPoints();
    void setPathType(int pathType);
    std::vector<bool> getSelectedPathPoints() const;
    bool moveSelectedPathPointsXYZ(float dX, float dY, float dZ);
    bool moveSelectedPathPointsRAE(float dRad, float dAzi, float dEle);
    void setAllPathPointsSelected(bool selectedState);
    void setPathPointSelected(int ptIndex, bool selectedState);
    bool getPathPointSelected(int ptIndex) const;
    void doneUpdatingPath() noexcept;
    void setPathChanged(bool changed) noexcept;
    int copySelectedPathPoints();
    int getNumPathPoints() const;
    int getNumSelectedPathPoints() const;
    std::vector<std::vector<float>> getPathPoints() const;
    // source path position interpolator interaction    
    FunctionalInterpolator<float>* getPathPosPtr() noexcept;
    const FunctionalInterpolator<float>* const getPathPosPtr() const noexcept;
    void addPathAutomationPoint(float x, float y);
    bool deleteSelectedAutomationPoints();
    void setAllPathAutomationPointsSelected(bool selectedState);
    void setPathAutomationPointSelected(int ptIndex, bool selectedState);
    bool getPathAutomationPointSelected(int ptIndex) const;
    int moveSelectedPathAutomationPoints(float dx, float dy);
//    std::vector<int> moveSelectedPathAutomationPointsWithReorderingInfo(float dx, float dy);
    std::vector<bool> getSelectedPathAutomationPoints() const;
    bool setSelectedPathAutomationPointsSegmentType(int newSegType);
    void doneUpdatingPathPos() noexcept;
    void setPathPosChanged(bool changed) noexcept;
    int copySelectedPathAutomationPoints();
    int getNumPathAutomationPoints() const;
    int getNumSelectedPathAutomationPoints() const;
protected: // used by PlayableSoundSource
    std::array<float, 3> posRAE {1, 0, M_PI/2}; // (rad,azi,ele) position
    bool sourceMuted = false; // source can be muted to not produce sound
private:
    float eleDir = 1.0f; // for continous ele movement
    bool sourceSelected = false; // is the source currently selected for editing by the GUI?
    std::unique_ptr<ParametricInterpolator<float>> path = nullptr; // the 3D spatial parametric path that this source moves on, if it has one
    FunctionalInterpolator<float> pathPos; // the 2D interpolator that defines a source's position on the 3D path as a function of time
    Interpolator<float>::Listener pathListener, pathPosListener;
    //int what = 0;
};

// structure to hold previous input buffers and their lengths (to compute the convolution tails and support variable length buffer sizes)
typedef struct _Input_ {
    std::vector<float> input;  // array of input values
    int N = 0;                 // num of values actually in use
    _Input_() {};
    ~_Input_() {};
    void setSize(const int maxLength) {
        input.resize(maxLength, 0);
        N = 0;
    }
    void load(const float* newInput, const int length) noexcept {
        N = length;
        for (int n = 0; n < length; ++n)
            input[n] = newInput[n];
    }
    void clear() noexcept {
        N = 0;
    }
} Input;
//typedef struct _Input_ {
//    float* input = nullptr;  // array of input values
//    int N = 0;               // length of array
//    
//    _Input_(int maxLength) {
//        input = new float[maxLength];
//        for (int n = 0; n < (const int)maxLength; ++n)
//            input[n] = 0;
//    }
//    void load(const float* newInput, int length) {
//        N = length;
//        for (int n = 0; n < (const int)N; ++n)
//            input[n] = newInput[n];
//    }
////    _Input_(const float* array, int length)
////            : N(length) {
////        input = new float[N];
////        for (int n = 0; n < (const int)N; ++n)
////            input[n] = array[n];
////    }
//    ~_Input_() {
//        delete[] input;
//    }
//} Input;

// holds the information needed for producing audio for a SoundSource
class PlayableSoundSource
{
public:
    PlayableSoundSource();
    ~PlayableSoundSource();
    void advancePosition() noexcept;
    // update the PlayableSoundSource with the state of a SoundSource
    void updateFromSoundSource(const SoundSource& source) noexcept;
    std::array<float,3> getPosRAE() const noexcept;
    // need to know this to allocate enough temp storage for intermediate audio processing
    void allocateForMaxBufferSize(int N_max);
//    // set if the source processes audio in real time or not
//    void setRealTime(bool isRealTime) noexcept;
//    bool getRealTime() const noexcept;
    // control Doppler effect
    void setDopplerOn(bool newDopplerOn, float newSpeedOfSound);
    void setDopplerSampleRate(float sampleRate) noexcept;
    // control if the source is processing audio or not
    void setSourceMuted(bool newMutedState) noexcept;
    bool getSourceMuted() const noexcept;
    // audio/hrir processing
    void interpolateHRIR(const float* rae, float* hrir) const noexcept;
    //void processAudioRealTime(const float* dataTime, int N, float* sourceOutput);
    //void interpolateHRIR(const std::array<float,3>& rae, float* hrir) const;
    void resetProcessingState() noexcept;
    void processAudio(const float* dataIn, int N, float* dataOut, const bool realTime);
    // for efficiently remembering the last accessed index of the pathPos interp
    int prevPathPosIndex = 0;
private:
    // for the doppler effect
    bool dopplerOn = false;
    Doppler doppler[2];
    float dopplerMaxDistance = 20; // 20 meters is good to start with
    float dopplerSpeedOfSound = defaultSpeedOfSound;
    //bool dopplerMaxDistanceChanged = false;
    // to hold previous buffer(s)'s inputs for computing convolution tails
    //std::vector<Input> inputs;
    //int newInputIndex = 0;
    int Nmax = 0;
	
	std::vector<float> inputBuffer;
	int inputBufferInPos = 0;
	int inputBufferOutPos = 0;

    //Array<Input*> inputs;
    std::array<float,3> posRAE {1, 0, M_PI/2};
    // prev's needed for hrir blending
    std::array<float,3> prevRAE {1, 0, M_PI/2};
    std::array<float,3> pprevRAE {1, 0, M_PI/2};
    // for blending async position updates
    //std::array<float,3> nextRAE {1, M_PI/5, M_PI/3};
//    float transitionTime = 0;
//    float currentTransitionTime = 0;
//    float nextTransitionTime = 0;
//    constexpr static const float maxTransitionTime = 0.05; // in seconds
    // temp's to keep track up updates until setParametricPosition() sychronizes the update
    //std::array<float, 3> tempRAE {1, M_PI/5, M_PI/3};
    // the parametric position (in sec) of the source on the path if it has one
    //float paraPos = 0.0;
    bool sourceMuted = false;
    // source is in motion or not
    //bool sourceMoving = true;
    // hrir data and blending stuff
    bool prevHRIRChange = false;
    bool HRIRChange = false; // indicates if there was change in position since the last processesing buffer
    int numHRIRs = 2;
    std::array<float, 2*numTimeSteps> HRIR {0};
    std::array<float, 4*numTimeSteps> HRIRs {0};
    float HRIRScaling[4] {1.0};
	std::vector<float> hqHRIRs;
	std::vector<float> hqHRIRScaling;
    /*float* hqHRIRs = nullptr;
    float* hqHRIRScaling = nullptr;
    float* temp = nullptr;*/
    int prevTempSize = 0;
//    // affects the degree to which the processing routine can smoothly blend between different hrirs at different positions
//    bool realTime = true;
};

#endif /* defined(__SoundSource__) */


//// PRE CONCURRENTRESOURCE
//#ifndef __SoundSource__
//#define __SoundSource__
//
//#include "../JuceLibraryCode/JuceHeader.h"
//#include "Doppler.h"
//#include "Interpolator.h"
//#include "Data.h"
//#include <array>
//
//// for visually representing a sound source in space and editing it
//class SoundSource
//{
//    friend class PlayableSoundSource;
//public:
//    // construct a source at the default location
//    SoundSource() {};
//    // construct a source at a spaital xyz location
//    SoundSource(const std::array<float,3>& xyz);
//    // copy constructor
//    SoundSource(const SoundSource& source);
//    // overloaded assignment operator
//    SoundSource& operator=(const SoundSource& source);
//    // checks if two SoundSources are identical for the purposes of undo/redo actions
//    //bool isSameAs(const SoundSource& source);
//    // constructor for reconstruction of source saved in XML
//    SoundSource(XmlElement *sourceXML);
//    // get an XML representation of this object in order to save its state in a plugin setting
//    XmlElement* getXML() const;
//    // get an XML for an interp to save its state
//    XmlElement* getXML(const Interpolator<float>& interp) const;
//    // create an interp from its saved XML state
//    std::unique_ptr<Interpolator<float>> getInterpolator(const XmlElement& interpXML) const;
//    // bounds checking for where the source/path pts can exist
//    void boundsCheckRAE(std::array<float,3>& rae, float& eleDirection);
//    void boundsCheckRAE(float (&rae)[3], float& eleDirection);
//    void boundsCheckXYZ(std::array<float,3>& xyz);
//    // control source position with rae coordinate
//    void setPosRAE(std::array<float,3>& rae);
//    std::array<float,3> getPosRAE() const;
//    // needed tor moving the source continously in ele
//    void setEleDir(float newEleDir);
//    float getEleDir() const;
//    // control the source position with xyz coordinate
//    void setPosXYZ(const std::array<float,3>& xyz);
//    void setPosXYZ(const float* xyz);
//    //void setPosXYZ(const float (&xyz)[3]);
//    std::array<float,3> getPosXYZ() const;
//    // set the source position given a time, playing state, and previous pathPos index from the realtime processing thread
//    bool setParametricPosition(float posSec, int& prevPathPosIndex);
//    void setPositionUpdate(const std::array<float,3>& newPosRAE, bool newMuted);
//    // control if the source is selected for editing
//    void setSourceSelected(bool newSourceSelected) noexcept;
//    bool getSourceSelected() const noexcept;
//    // control if the source is processing audio or not
//    void setSourceMuted(bool newMutedState);
//    bool getSourceMuted() const;
//    // source path interpolator interaction
//    std::unique_ptr<ParametricInterpolator<float>> getPath() const;
//    void addPathPoint();
//    void addPathPoint(std::array<float,3>& xyz);
//    int deleteSelectedPathPoints();
//    void setPathType(int pathType);
//    std::vector<bool> getSelectedPathPoints() const;
//    bool moveSelectedPathPointsXYZ(float dX, float dY, float dZ);
//    bool moveSelectedPathPointsRAE(float dRad, float dAzi, float dEle);
//    void setAllPathPointsSelected(bool selectedState);
//    void setPathPointSelected(int ptIndex, bool selectedState);
//    bool getPathPointSelected(int ptIndex) const;
//    void setPathChangedState(bool changed);
//    int copySelectedPathPoints();
//    int getNumPathPoints() const;
//    std::vector<std::vector<float>> getPathPoints() const;
//    // source path position interpolator interaction
//    FunctionalInterpolator<float>* getPathPosPtr();
//    void addPathAutomationPoint(float x, float y);
//    bool deleteSelectedAutomationPoints();
//    void setAllPathAutomationPointsSelected(bool selectedState);
//    void setPathAutomationPointSelected(int ptIndex, bool selectedState);
//    bool getPathAutomationPointSelected(int ptIndex) const;
//    int moveSelectedPathAutomationPoints(float dx, float dy);
//    //    std::vector<int> moveSelectedPathAutomationPointsWithReorderingInfo(float dx, float dy);
//    std::vector<bool> getSelectedPathAutomationPoints() const;
//    bool setSelectedPathAutomationPointsSegmentType(int newSegType);
//    void setPathPosChangedState(bool changed);
//    int copySelectedPathAutomationPoints();
//    int getNumPathAutomationPoints() const;
//protected: // used by PlayableSoundSource
//    std::array<float,3> posRAE {1, 0, M_PI/2}; // (rad,azi,ele) position
//    bool sourceMuted = false; // source can be muted to not produce sound
//private:
//    float eleDir = 1.0; // for continous ele movement
//    bool sourceSelected = false; // is the source currently selected for editing by the GUI?
//    std::unique_ptr<ParametricInterpolator<float>> path = nullptr; // the 3D spatial parametric path that this source moves on, if it has one
//    FunctionalInterpolator<float> pathPos; // the 2D interpolator that defines a source's position on the 3D path as a function of time
//    //    float nextPosSec = 0.0;
//    //    std::array<float,3> nextRAE {1, M_PI/5, M_PI/3};
//    //    bool nextSourceMuted = false;
//};
//
//// structure to hold previous input buffers and their lengths (to compute the convolution tails and support variable length buffer sizes)
//typedef struct _Input_ {
//    std::vector<float> input;  // array of input values
//    int N = 0;                 // num of values actually in use
//    _Input_() {};
//    ~_Input_() {};
//    void setSize(const int maxLength) {
//        input.resize(maxLength, 0);
//        N = 0;
//    }
//    void load(const float* newInput, const int length) noexcept {
//        N = length;
//        for (int n = 0; n < length; ++n)
//            input[n] = newInput[n];
//    }
//    void clear() noexcept {
//        N = 0;
//    }
//} Input;
////typedef struct _Input_ {
////    float* input = nullptr;  // array of input values
////    int N = 0;               // length of array
////
////    _Input_(int maxLength) {
////        input = new float[maxLength];
////        for (int n = 0; n < (const int)maxLength; ++n)
////            input[n] = 0;
////    }
////    void load(const float* newInput, int length) {
////        N = length;
////        for (int n = 0; n < (const int)N; ++n)
////            input[n] = newInput[n];
////    }
//////    _Input_(const float* array, int length)
//////            : N(length) {
//////        input = new float[N];
//////        for (int n = 0; n < (const int)N; ++n)
//////            input[n] = array[n];
//////    }
////    ~_Input_() {
////        delete[] input;
////    }
////} Input;
//
//// holds the information needed for producing audio for a SoundSource
//class PlayableSoundSource
//{
//public:
//    PlayableSoundSource();
//    ~PlayableSoundSource();
//    void advancePosition() noexcept;
//    // update the PlayableSoundSource with the state of a SoundSource
//    void updateFromSoundSource(const SoundSource& source) noexcept;
//    std::array<float,3> getPosRAE() const noexcept;
//    // need to know this to allocate enough temp storage for intermediate audio processing
//    void allocateForMaxBufferSize(int N_max);
//    //    // set if the source processes audio in real time or not
//    //    void setRealTime(bool isRealTime) noexcept;
//    //    bool getRealTime() const noexcept;
//    // control Doppler effect
//    void setDopplerOn(bool newDopplerOn, float newSpeedOfSound);
//    void setDopplerSampleRate(float sampleRate) noexcept;
//    // control if the source is processing audio or not
//    void setSourceMuted(bool newMutedState) noexcept;
//    bool getSourceMuted() const noexcept;
//    // audio/hrir processing
//    void interpolateHRIR(const float* rae, float* hrir) const noexcept;
//    //void processAudioRealTime(const float* dataTime, int N, float* sourceOutput);
//    //void interpolateHRIR(const std::array<float,3>& rae, float* hrir) const;
//    void resetProcessingState() noexcept;
//    void processAudio(const float* dataIn, int N, float* dataOut, const bool realTime);
//    // for efficiently remembering the last accessed index of the pathPos interp
//    int prevPathPosIndex = 0;
//private:
//    // for the doppler effect
//    bool dopplerOn = false;
//    Doppler doppler[2];
//    float dopplerMaxDistance = 20; // 20 meters is good to start with
//    float dopplerSpeedOfSound = DEFAULT_SPEED_OF_SOUND;
//    //bool dopplerMaxDistanceChanged = false;
//    // to hold previous buffer(s)'s inputs for computing convolution tails
//    std::vector<Input> inputs;
//    int newInputIndex = 0;
//    int Nmax = 0;
//    //Array<Input*> inputs;
//    std::array<float,3> posRAE {1, 0, M_PI/2};
//    // prev's needed for hrir blending
//    std::array<float,3> prevRAE {1, 0, M_PI/2};
//    std::array<float,3> pprevRAE {1, 0, M_PI/2};
//    // for blending async position updates
//    //std::array<float,3> nextRAE {1, M_PI/5, M_PI/3};
//    //    float transitionTime = 0;
//    //    float currentTransitionTime = 0;
//    //    float nextTransitionTime = 0;
//    //    constexpr static const float maxTransitionTime = 0.05; // in seconds
//    // temp's to keep track up updates until setParametricPosition() sychronizes the update
//    //std::array<float, 3> tempRAE {1, M_PI/5, M_PI/3};
//    // the parametric position (in sec) of the source on the path if it has one
//    //float paraPos = 0.0;
//    bool sourceMuted = false;
//    // source is in motion or not
//    //bool sourceMoving = true;
//    // hrir data and blending stuff
//    bool prevHRIRChange = false;
//    bool HRIRChange = false; // indicates if there was change in position since the last processesing buffer
//    int numHRIRs = 2;
//    std::array<float, 2*numTimeSteps> HRIR {0};
//    std::array<float, 4*numTimeSteps> HRIRs {0};
//    float HRIRScaling[4] {1.0};
//    float* hqHRIRs = nullptr;
//    float* hqHRIRScaling = nullptr;
//    float* temp = nullptr;
//    int prevTempSize = 0;
//    //    // affects the degree to which the processing routine can smoothly blend between different hrirs at different positions
//    //    bool realTime = true;
//};
//
//#endif /* defined(__SoundSource__) */
