/*
 PluginProcessor.h
 
 The meat of 3DAudio's audio processing and plugin related code.

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

#ifndef __PluginProcessor__
#define __PluginProcessor__

#include "../JuceLibraryCode/JuceHeader.h"
#include "DrewLib.h"

#include "SoundSource.h"
#include "Resampler.h"
#include "ConcurrentResource.h"

// keeps track of the number of plugin instances so we can only use one copy of the HRIR data
static int numRefs = 0;
// possible states for GUI display
enum class DisplayState { MAIN, PATH_AUTOMATION, SETTINGS, NUM_DISPLAY_STATES };
// realtime is lightest on cpu and will not glitch, offline is expensive on cpu and may glitch, auto-detect assumes the processing mode from the host
enum class ProcessingMode { REALTIME, OFFLINE, AUTO_DETECT };
// max number of sound sources
static constexpr auto maxNumSources = 8;
// making life easier
using Sources = std::vector<SoundSource>;
using Locker = std::lock_guard<Mutex>;

class ThreeDAudioProcessorEditor;

class ThreeDAudioProcessor : public AudioProcessor, public UndoManager
{
public:
    ThreeDAudioProcessor();
    ~ThreeDAudioProcessor();
    // the methods to for JUCE's AudioProcessor interface
    void prepareToPlay (double sampleRate, int samplesPerBlock);
    void releaseResources();
    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
    AudioProcessorEditor* createEditor();
    bool hasEditor() const;
    const String getName() const;
    // no need to override the ones in AudioProcessor
//    int getNumParameters();
//    float getParameter (int index);
//    void setParameter (int index, float newValue);
//    const String getParameterName (int index);
//    const String getParameterText (int index);
    const String getInputChannelName (int channelIndex) const;
    const String getOutputChannelName (int channelIndex) const;
    bool isInputChannelStereoPair (int index) const;
    bool isOutputChannelStereoPair (int index) const;
    bool acceptsMidi() const;
    bool producesMidi() const;
    bool silenceInProducesSilenceOut() const;
    double getTailLengthSeconds() const;
    int getNumPrograms();
    int getCurrentProgram();
    void setCurrentProgram (int index);
    const String getProgramName (int index);
    void changeProgramName (int index, const String& newName);
    void getStateInformation (MemoryBlock& destData);
    void setStateInformation (const void* data, int sizeInBytes);
    // methods for source interaction
    void saveCurrentState(int beforeOrAfter);
    void getSourcePosXYZ(int sourceIndex, float (&xyz)[3]) const;
    bool addSourceAtXYZ(const float (&xyz)[3]);
    void copySelectedSources();
    bool getSourceSelected(std::size_t sourceIndex) const;
    void setSourceSelected(int sourceIndex, bool newSelectedState);
    void selectAllSources(bool newSelectedState);
    void deleteSelectedSources();
    void toggleLockSourcesToPaths();
    bool getLockSourcesToPaths() const;
    void toggleDoppler();
    int moveSelectedSourcesXYZ(float dx, float dy, float dz, bool moveSource = false);
    int moveSelectedSourcesRAE(float dr, float da, float de, bool moveSource = false);
    void toggleSelectedSourcesPathType();
    Array<SoundSource*, CriticalSection>* getSources();
    //void setSources(const Lockable<Sources>& newSources);
    void setSources(const Sources& newSources);
    // path point interaction
    void dropPathPoint();
    bool dropPathPoint(const float (&xyz)[3]);
    void togglePathPointSelected(int sourceIndex, int ptIndex);
    void setPathPointSelectedState(int sourceIndex, int ptIndex, bool newSelectedState);
    //void markPathAsUpdated(int sourceIndex);
    std::vector<std::vector<float>> getPathPoints(int sourceIndex) const;
    std::vector<bool> getPathPointsSelected(int sourceIndex) const;
    bool getPathPointSelected(std::size_t sourceIndex,
                              std::size_t pathPointIndex) const;
    void setSelectedPathPointIndecies(int sourceIndex,
                                      int pathPointIndex,
                                      int newIndex);
    // path automation point interaction
    void togglePathAutomationPointSelected(int sourceIndex, int ptIndex);
    void selectAllPathAutomationView(bool newSelectedState);
    void setPathAutomationPointSelectedState(int sourceIndex, int ptIndex, bool newSelectedState);
    void deselectAllPathAutomationPoints();
    int moveSelectedPathAutomationPoints(float dx, float dy);
    //std::vector<int> moveSelectedPathAutomationPointsWithReorderingInfo(float dx, float dy, int sourceIndexOfInterest);
    void moveSelectedPathAutomationPointsTo(int referencePtSourceIndex,
                                            int& referencePtIndex,
                                            int referencePtIndexAmongSelecteds,
                                            float x, float y);
    void addPathAutomationPtAtXY(const float (&xy)[2]);
    void deleteSelectedAutomationPoints();
    void setSelectedPathAutomationPointsSegmentType(int newSegType);
    //void markPathPosAsUpdated(int sourceIndex);
    void copySelectedPathAutomationPoints();
    std::vector<std::vector<float>> getPathAutomationPoints(int sourceIndex) const;
    //std::vector<std::vector<float>> getSelectedPathAutomationPoints(int sourceIndex);
    //std::vector<bool> getPathAutomationPointsSelected(int sourceIndex);
    int getPathAutomationPointIndexAmongSelectedPoints(int sourceIndex, int pointIndex) const;
    bool areAnySelectedSourcesPathAutomationPointsSelected() const;
    void makeSourcesVisibleForPathAutomationView();
    // resets the playing state if processBlock() has not been called in a while, needed because of the logic for moving selected sources
    void resetPlaying(float frameRate) noexcept;
    // for looping
    void toggleLooping(float defaultBegin, float defaultEnd);
    void defineLoopingRegionUsingSelectedPathAutomationPoints();
    
    std::atomic<bool> presetJustLoaded {true}; // to communicate to the editor when preset is loaded, editor resets to false after doing what it needs
    std::atomic<float> loopRegionBegin {-1};
    std::atomic<float> loopRegionEnd {-1};
    std::atomic<bool> loopingEnabled {false};
    // for the doppler effect
    void setSpeedOfSound(float newSpeedOfSound);
    bool dopplerOn = false;
    float speedOfSound = defaultSpeedOfSound;
    float maxSpeedOfSound = 500.0f;
    float minSpeedOfSound = 0.1f;
    // plugin window size
    int lastUIWidth = 700;
    int lastUIHeight = 600;
    // current time position, buffer size, sample rate, bpm, and time signature
    std::atomic<float> posSEC {0};
    std::atomic<int> N;
    std::atomic<float> fs;
    std::atomic<float> bpm {120};
    std::atomic<float> timeSigNum {4};
    std::atomic<float> timeSigDen {4};
    std::string getCurrentTimeString(int opt) const;
    std::tuple<int, int, float> getMeasuresBeatsFrac(float sec) const;
    // eye position
    float upDir = 1.0f;  // y component of eyeUp
    float eyePos[3]; // x,y,z
    float eyeUp[3] = {0.0f, 1.0f, 0.0f};
    float eyeRad = 3.3f;
    float eyeAzi = 9*M_PI/8;
    float eyeEle = M_PI/2.2f;
    // layout for the path automation view
    float automationViewWidth = 60.0f;
    float automationViewOffset = automationViewWidth/2.0f;
    // which view is displayed in the plugin window
    std::atomic<DisplayState> displayState {DisplayState::MAIN};
    // determines audio rendering quality and realtime processing performance
    void setProcessingMode(ProcessingMode newMode) noexcept;
    std::atomic<ProcessingMode> processingMode {ProcessingMode::AUTO_DETECT};
    std::atomic<bool> realTime {true};
    std::atomic<bool> isHostRealTime {false};
    // show the controls for that view
    //bool showHelp = false;
    // for letting the GL know when its display lists for drawing the path and pathPos interps for each source are updated
    std::atomic<bool> pathChanged {false};
    std::atomic<bool> pathPosChanged {false};
    //std::array<std::atomic<bool>, maxNumSources> pathChangeds;
    //std::array<std::atomic<bool>, maxNumSources> pathPosChangeds;
    // the visual representation of sound sources along with temporary copies to support undo/redos
    RealtimeConcurrent<Sources, 3> sources;
    //AudioPlayHead::CurrentPositionInfo gPositionInfo;
    std::array<std::atomic<AudioParameterFloat*>, maxNumSources> sourcePathPositionsFromDAW; // for source position automation from DAW
    std::atomic<float> wetOutputVolume {1.0f};
    std::atomic<float> dryOutputVolume {0.0f};
    
private:
    // version of sources that can be used to process audio, only updated in processBlock() and is therefore thread-safe to use for processing
    std::vector<PlayableSoundSource> playableSources;
    int prevSourcesSize = 0; // see processBlock() for useage
    // temporary SoundSource copies to support undo/redos
    Sources beforeUndo;
    Sources currentUndo;
//    Lockable<Sources> beforeUndo;
//    Lockable<Sources> currentUndo;
    // objects for sample rate conversion
    Resampler resampler;
    Resampler unsamplerCh1;
    Resampler unsamplerCh2;
    // previous buffer's time position from this plugin's perspective
    float posSECprev = 0;
    // prev buf time position from host's perspective
    float posSECPrevHost = 0;
    // are we ready to process audio with sound sources?
    std::atomic<bool> inited {false};
    // during playback sources can be locked to move on their paths, or moved about freely by the user
    std::atomic<bool> lockSourcesToPaths {true};
    // are we playing back audio now?
    std::atomic<bool> playing {false};
    std::atomic<int> resetPlayingCount {0};
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeDAudioProcessor)
};

class EditSources : public UndoableAction
{
public:
//    EditSources(const Lockable<Sources>& prevSourcesIn, const Lockable<Sources>& nextSourcesIn, ThreeDAudioProcessor* ownerIn)
//    {
//        const Locker lockPrev   (prevSources.getLock());
//        const Locker lockPrevIn (prevSourcesIn.getLock());
//        const Locker lockNext   (nextSources.getLock());
//        const Locker lockNextIn (nextSourcesIn.getLock());
//        Sources& prev = prevSources.getResource();
//        for (const auto& source : prevSourcesIn.getResource())
//            prev.emplace_back(source);
//        Sources& next = nextSources.getResource();
//        for (const auto& source : nextSourcesIn.getResource())
//            next.emplace_back(source);
//        owner = ownerIn;
//    }
    EditSources(const Sources& prevSourcesIn, const Sources& nextSourcesIn, ThreeDAudioProcessor* ownerIn)
    {
        for (const auto& source : prevSourcesIn)
            prevSources.emplace_back(source);
        for (const auto& source : nextSourcesIn)
            nextSources.emplace_back(source);
        owner = ownerIn;
    }
    bool perform() override
    {
        owner->setSources(nextSources);
        return true;
    }
    bool undo() override
    {
        owner->setSources(prevSources);
        return true;
    }
    int getSizeInUnits() override
    {
        return 10;
    }
    UndoableAction* createCoalescedAction (UndoableAction* nextAction) override
    {
        UndoableAction* coalescedAction = new EditSources(prevSources, ((EditSources*)nextAction)->nextSources, ((EditSources*)nextAction)->owner);
        return coalescedAction;
    }
private:
//    Lockable<Sources> prevSources;
//    Lockable<Sources> nextSources;
    Sources prevSources;
    Sources nextSources;
    ThreeDAudioProcessor* owner;
};

#endif /* defined(__PluginProcessor__) */


//// PRE CONCURRENTRESOURCE
//#ifndef __PluginProcessor__
//#define __PluginProcessor__
//
//#include <iostream>
//#include <fstream>
//
//#include "../JuceLibraryCode/JuceHeader.h"
//
//#include "SoundSource.h"
//#include "Resampler.h"
////#include "RealTimeConcurrency.h"
//
//// keeps track of the number of plugin instances so we can only use one copy of the HRIR data
//static int numRefs = 0;
//// possible states for GUI display
//enum class DisplayState { MAIN, PATH_AUTOMATION, SETTINGS, NUM_DISPLAY_STATES};
//// realtime is lightest on cpu and will not glitch, offline is expensive on cpu and may glitch, auto-detect assumes the processing mode from the host
//enum class ProcessingMode { REALTIME, OFFLINE, AUTO_DETECT };
//
////class ThreeDAudioProcessorEditor; // forward declare this so we can have a pointer to the editor in the processor
//
//class ThreeDAudioProcessor : public AudioProcessor, public UndoManager
//{
//public:
//    ThreeDAudioProcessor();
//    ~ThreeDAudioProcessor();
//    // the methods to for JUCE's AudioProcessor interface
//    void prepareToPlay (double sampleRate, int samplesPerBlock);
//    void releaseResources();
//    void processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages);
//    AudioProcessorEditor* createEditor();
//    bool hasEditor() const;
//    const String getName() const;
//    int getNumParameters();
//    float getParameter (int index);
//    void setParameter (int index, float newValue);
//    const String getParameterName (int index);
//    const String getParameterText (int index);
//    const String getInputChannelName (int channelIndex) const;
//    const String getOutputChannelName (int channelIndex) const;
//    bool isInputChannelStereoPair (int index) const;
//    bool isOutputChannelStereoPair (int index) const;
//    bool acceptsMidi() const;
//    bool producesMidi() const;
//    bool silenceInProducesSilenceOut() const;
//    double getTailLengthSeconds() const;
//    int getNumPrograms();
//    int getCurrentProgram();
//    void setCurrentProgram (int index);
//    const String getProgramName (int index);
//    void changeProgramName (int index, const String& newName);
//    void getStateInformation (MemoryBlock& destData);
//    void setStateInformation (const void* data, int sizeInBytes);
//    // methods for source interaction
//    void saveCurrentState(int beforeOrAfter);
//    void getSourcePosXYZ(int sourceIndex, float (&xyz)[3]) const;
//    void addSourceAtXYZ(const float (&xyz)[3]);
//    void copySelectedSources();
//    bool getSourceSelected(int sourceIndex);
//    void setSourceSelected(int sourceIndex, bool newSelectedState);
//    //void selectAllSourcesIfNoneSelected();
//    void selectAllSources(bool newSelectedState);
//    void deleteSelectedSources();
//    void toggleLockSourcesToPaths();
//    bool getLockSourcesToPaths() const;
//    void toggleDoppler();
//    int moveSelectedSourcesXYZ(float dx, float dy, float dz);
//    int moveSelectedSourcesRAE(float dr, float da, float de);
//    void toggleSelectedSourcesPathType();
//    //Array<SoundSource> getSourcesCopy();
//    //Array<SoundSource*, CriticalSection> getSources();
//    Array<SoundSource*, CriticalSection>* getSourcesPtr();
//    void setSources(const Array<SoundSource*, CriticalSection> & newSources);
//    // path point interaction
//    void dropPathPoint();
//    bool dropPathPoint(const float (&xyz)[3]);
//    void togglePathPointSelected(int sourceIndex, int ptIndex);
//    void setPathPointSelectedState(int sourceIndex, int ptIndex, bool newSelectedState);
//    void markPathAsUpdated(int sourceIndex);
//    std::vector<std::vector<float>> getPathPoints(int sourceIndex) const;
//    std::vector<bool> getPathPointsSelected(int sourceIndex) const;
//    // path automation point interaction
//    void togglePathAutomationPointSelected(int sourceIndex, int ptIndex);
//    void selectAllPathAutomationView(bool newSelectedState);
//    void setPathAutomationPointSelectedState(int sourceIndex, int ptIndex, bool newSelectedState);
//    void deselectAllPathAutomationPoints();
//    int moveSelectedPathAutomationPoints(float dx, float dy);
//    //std::vector<int> moveSelectedPathAutomationPointsWithReorderingInfo(float dx, float dy, int sourceIndexOfInterest);
//    void moveSelectedPathAutomationPointsTo(int referencePtSourceIndex,
//                                            int& referencePtIndex,
//                                            int referencePtIndexAmongSelecteds,
//                                            float x, float y);
//    void addPathAutomationPtAtXY(const float (&xy)[2]);
//    void deleteSelectedAutomationPoints();
//    void setSelectedPathAutomationPointsSegmentType(int newSegType);
//    void markPathPosAsUpdated(int sourceIndex);
//    void copySelectedPathAutomationPoints();
//    std::vector<std::vector<float>> getPathAutomationPoints(int sourceIndex);
//    //std::vector<std::vector<float>> getSelectedPathAutomationPoints(int sourceIndex);
//    //std::vector<bool> getPathAutomationPointsSelected(int sourceIndex);
//    int getPathAutomationPointIndexAmongSelectedPoints(int sourceIndex,
//                                                       int pointIndex);
//    bool areAnySelectedSourcesPathAutomationPointsSelected();
//    void makeSourcesVisibleForPathAutomationView() noexcept;
//    // for looping
//    void defineLoopingRegionUsingSelectedPathAutomationPoints();
//    std::atomic<float> loopRegionBegin {-1};//= ATOMIC_VAR_INIT(-1.0);
//    std::atomic<float> loopRegionEnd {-1};//= ATOMIC_VAR_INIT(-1.0);
//    // for the doppler effect
//    void setSpeedOfSound(float newSpeedOfSound);
//    bool dopplerOn = false;
//    float speedOfSound = DEFAULT_SPEED_OF_SOUND;
//    float maxSpeedOfSound = 400;
//    float minSpeedOfSound = 1;
//    // plugin window size
//    int lastUIWidth = 700;
//    int lastUIHeight = 600;
//    // current time position, buffer size, and sample rate
//    std::atomic<float> posSEC {0};// = ATOMIC_VAR_INIT(0.0);
//    std::atomic<int> N;
//    std::atomic<float> fs;
//    // eye position
//    float upDir = 1.0;  // y component of eyeUp
//    float eyePos[3]; // x,y,z
//    float eyeUp[3] = {0.0, 1.0, 0.0};
//    float eyeRad = 3.2;
//    float eyeAzi = 9*M_PI/8;
//    float eyeEle = M_PI/2.2;
//    // layout for the path automation view
//    float automationViewWidth = 60.0;
//    float automationViewOffset = automationViewWidth/2.0;
//    // which view is displayed in the plugin window
//    std::atomic<DisplayState> displayState {DisplayState::MAIN};
//    // determines audio rendering quality and realtime processing performance
//    void setProcessingMode(ProcessingMode newMode) noexcept;
//    std::atomic<ProcessingMode> processingMode {ProcessingMode::AUTO_DETECT};
//    std::atomic<bool> realTime {true};
//    std::atomic<bool> isHostRealTime {false};
//    // show the controls for that view
//    bool showHelp = true;
//    
//private:
//    //ThreeDAudioProcessorEditor* editor;
//    // version of sources that can be used to process audio, only updated in processBlock() and is therefore thread-safe to use for processing
//    std::vector<PlayableSoundSource> playableSources;
//    int prevSourcesSize = 0; // see processBlock() for useage
//    // the visual representation of sound sources along with temporary copies to support undo/redos
//    Array<SoundSource*, CriticalSection> sources;
//    Array<SoundSource*, CriticalSection> beforeUndo;
//    Array<SoundSource*, CriticalSection> currentUndo;
//    // objects for sample rate conversion
//    Resampler resampler;
//    Resampler unsamplerCh1;
//    Resampler unsamplerCh2;
//    // previous buffer's time position
//    float posSECprev = 0;
//    // are we ready to process audio with sound sources?
//    std::atomic<bool> inited {false};// = ATOMIC_VAR_INIT(false);
//    // during playback sources can be locked to move on their paths, or moved about freely by the user
//    std::atomic<bool> lockSourcesToPaths {true};// = ATOMIC_VAR_INIT(true);
//    // are we playing back audio now?
//    std::atomic<bool> playing {false};// = ATOMIC_VAR_INIT(false);
//    // for debugging
//    //FILE* outputfp = NULL;
//    //std::ofstream logFile1;
//    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ThreeDAudioProcessor)
//};
//
//class EditSources : public UndoableAction
//{
//public:
//    EditSources(Array<SoundSource*, CriticalSection> & prevSourcesIn, Array<SoundSource*, CriticalSection> & nextSourcesIn, ThreeDAudioProcessor* ownerIn)
//    {
//        const ScopedLock lockPrev (prevSources.getLock());
//        const ScopedLock lockPrevIn (prevSourcesIn.getLock());
//        const ScopedLock lockNext (nextSources.getLock());
//        const ScopedLock lockNextIn (nextSourcesIn.getLock());
//        for (int s = 0; s < prevSourcesIn.size(); ++s)
//            prevSources.add(new SoundSource(*prevSourcesIn[s]));
//        for (int s = 0; s < nextSourcesIn.size(); ++s)
//            nextSources.add(new SoundSource(*nextSourcesIn[s]));
//        owner = ownerIn;
//    }
//    ~EditSources() override
//    {
//        const ScopedLock lockPrev (prevSources.getLock());
//        const ScopedLock lockNext (nextSources.getLock());
//        for (int s = 0; s < prevSources.size(); ++s)
//            delete prevSources[s];
//        for (int s = 0; s < nextSources.size(); ++s)
//            delete nextSources[s];
//    }
//    bool perform() override
//    {
//        owner->setSources(nextSources);
//        return true;
//    }
//    bool undo() override
//    {
//        owner->setSources(prevSources);
//        return true;
//    }
//    int getSizeInUnits() override
//    {
//        return 10;
//    }
//    UndoableAction* createCoalescedAction (UndoableAction* nextAction) override
//    {
//        UndoableAction* coalescedAction = new EditSources(prevSources, ((EditSources*)nextAction)->nextSources, ((EditSources*)nextAction)->owner);
//        return coalescedAction;
//    }
//    
//private:
//    Array<SoundSource*, CriticalSection> prevSources;
//    Array<SoundSource*, CriticalSection> nextSources;
//    ThreeDAudioProcessor* owner;
//};
//
//#endif /* defined(__PluginProcessor__) */
