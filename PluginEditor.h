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

#ifndef __PluginEditor__
#define __PluginEditor__

//#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include <GLUT/glut.h>

// selectable objects
#define SOURCE  0
#define SOURCE_PTS 8

// buffer to hold selected object data
#define SELECT_BUF_SIZE 256//512

//class Menu
//{
//    using Pt = std::array<float, 2>;
//    using Rect = std::array<Pt, 2>;
//    using ItemList = std::vector<std::string>;
//public:
//    Menu(ItemList&& newItems);
//    Menu(ItemList&& newItems, bool newOpensDown)
//        : items(std::move(newItems)), opensDown(newOpensDown) {};
//    void draw() const noexcept;
//    void updateFromMouse(const Pt& mousePosition, bool mouseClicked);
//    void setIsOpen(bool newIsOpen) noexcept;
//    //void getOpensDown(bool newOpensDown) noexcept { return opensDown;
//private:
//    ItemList items;
//    Rect mainBox;
//    float openedHeight;
//    bool isOpen = false;
//    bool opensDown = true; // or false to open upwards
//};

static constexpr float stroke_scale = 0.00035;
static constexpr float topMax = 119.05*stroke_scale;

class Tabs
{
public:
    void draw(float windowAspectRatio) const noexcept;
    int updateFromMouse(float mouseX, float mouseY, bool mouseClicked) noexcept;
    //void updateWindowSize(float width, float height) noexcept;
    void setSelectedTab(int newSelectedTab) noexcept;
    float getBottom() const noexcept;
private:
    const std::vector<char*> tabs {(char*)"Edit Sound Sources",
                                   (char*)"Automate Source Movement",
                                   (char*)"Settings" };
    static constexpr float top = 0.99;
    static constexpr float bottom = 0.92;
    static constexpr float left = -1.0;
    static constexpr float right = 1.0;
    static constexpr float textTop = top - (top - bottom)*0.15;
    static constexpr float textBottom = bottom + (top - bottom)*0.25;
//    static constexpr float stroke_scale = 0.00035;
//    static constexpr float topMax = 119.05*stroke_scale;
    const float gap = (right-left)*0.1/(tabs.size()*3.0+1.0);
    const float tab = (right-left)*0.9/tabs.size();
    const int longestStrIndex = std::distance(tabs.begin(), std::max_element(tabs.begin(), tabs.end(),
                                            [](auto& s1, auto& s2){return strlen(s1) < strlen(s2);}));
    int selectedTab = 0;
    int mouseOverTab = -1;
    //float windowAspectRatio = 1.0;
};

class RadioOption
{
public:
    void draw(float windowAspectRatio) const noexcept;
    int updateFromMouse(float mouseX, float mouseY, bool mouseClicked, float windowAspectRatio) noexcept;
    void setAutoDetectOption(bool isHostRealTime) noexcept;
    void setSelectedOption(int newSelectedOption) noexcept;
    std::string getHelpText() const noexcept;
    //void updateWindowSize(float width, float height) noexcept;
private:
    float getStrLength() const noexcept
    {
        float length = 0;
        for (auto opt : options)
        {
            length += glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)opt);
            //length += glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)spacing);
        }
        return length;
    };
    const std::vector<char*> options {(char*)"ProcessingMode:  ",
                                      (char*)"RealTime ",
                                      (char*)"HighQuality ",
                                      (char*)"AutoDetect"};
    const std::array<std::string, 3> helpText
    {"The real-time processing mode is intended to be used when you are editing the tracks that use this plugin. It puts the least strain on your CPU so that you can listen to your tracks in real-time while you edit them. However, for moving sound sources, the audio quality will be less than ideal so don't use this setting when you are doing the final export of your tracks that have moving sound sources.",
     "The high quality processing mode is intended to be used when you are done editing your tracks that use this plugin and want the highest audio quality possible for moving sound sources. When using this processing mode, high demand is placed on your CPU so you may not be able to listen to your tracks in real-time. Select this mode before you lock any tracks that you are done editing or before you do the final export of your tracks to get the highest possible audio quality for your moving sound sources.",
     "The auto-detect processing mode chooses the appropriate processing mode for this plugin based on if your DAW is running in a real-time capacity or not. Some DAWs do not provide the means to explicitly specify this, but you can do so for this plugin by choosing either real-time or high quality above. Note that these modes only affect the audio quality and CPU demand for moving sound sources. If you only have stationary sound sources, you don't need to worry about this setting."};
        
    //const char* spacing = "  ";
    static constexpr float top = 0.85;
    static constexpr float bottom = 0.75;
    static constexpr float left = -0.9;
    static constexpr float right = 0.9;
    const float strLength = getStrLength();
    int selectedOption = 3;
    int autoDetectOption = 1;
    int mouseOverOption = -1;
    //float windowAspectRatio = 1.0;
};

class GlutTextBox
{
public:
    GlutTextBox() noexcept {};
    GlutTextBox(float topIn, float bottomIn, float leftIn, float rightIn, float desiredTextHeightIn) noexcept
        : top(topIn), bottom(bottomIn), left(leftIn), right(rightIn), desiredTextHeight(desiredTextHeightIn) {};
    ~GlutTextBox() {};
    void loadText(const std::string& text) noexcept;
    void resize(float windowAspectRatio) noexcept;
    void draw() const noexcept;
private:
    std::vector<std::string> words;
    float top = 0, bottom = 0, left = 0, right = 0, desiredTextHeight = 0, aspectRatio = 1, textScale = 1;
    
};

void drawStringStroke(char *s, float x, float y, float z, float aspect, float scale);


class ThreeDAudioProcessorEditor  : public AudioProcessorEditor, public OpenGLRenderer, public MultiTimer
{
public:
    ThreeDAudioProcessorEditor (ThreeDAudioProcessor* ownerFilter);
    ~ThreeDAudioProcessorEditor();
    
    void updateSize();
    void resized();
    // This is just a standard Juce paint method...
    void paint (Graphics& g);
    void newOpenGLContextCreated();
    void openGLContextClosing();
    void mouseMove(const MouseEvent & event);
    void mouseDown(const MouseEvent & event);
    void mouseDrag(const MouseEvent & event);
    void mouseDoubleClick(const MouseEvent & event);
    void mouseUp(const MouseEvent & event);
    void mouseWheelMove	(const MouseEvent & event,
                         const MouseWheelDetails & wheel);
    void mouseMagnify (const MouseEvent & event,
                       float scaleFactor);
    void showMouseDetails() const;
    bool keyPressed	(const KeyPress & key);
    void autoAlignAutomationPoints(bool alignInX, bool alignInY, bool alignWithMouse);
    void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);
    // Draw scene in here
    void renderOpenGL();
    // main view for drawing a path
    void drawMain();
    // view for automating a moving source
    void drawPathControl();
    void drawInterpolatedPath(int sourceIndex);
    void drawLoopingRegion();
    inline float timeValueToXPosition(float tVal) const;
    inline float getMouseX() const;
    inline float getMouseY() const;
    void drawStringBitmap(void *font, char *s, float x, float y, float z) const;
    void drawStringStroke(char *s, float x, float y, float z) const;
    void getFormattedTime(float timeInSec, char* str) const;
    void timerCallback(int timerID);
    void drawMouseDragging();
    void drawCircle(float x, float y, float r, int segments) const;
    void updateXYZ(float rad, float azi, float ele, float *xyz) const;
    bool to2D(const float (&xyz)[3], float (&xy)[2]) const;
    void to3D(float x, float y, float* xyz) const;
    bool pointInsideSelectRegion(const float (&xyz)[3]) const;
    //bool pointInsideSelectRegion(const std::array<float,3>& xyz) const;
    bool pointInsideSelectRegion2D(const float (&xy)[2]) const;
    void getOnScreenDirection(int dir, float (&xyz)[3]) const;
//    // eh not really necessary...
//    void getOnScreenDirHelper(int dir, float (&xyz)[3],
//                              const float (&zp)[2], const float (&zm)[2],
//                              const float (&yp)[2], const float ...)
    void drawArray(float* array, int L);
    //void drawRect(float centerX, float centerY, float deltaX, float deltaY);
    //void setProcessingModeAutoDetect(bool isHostRealTime) noexcept;
private:
    float windowAspectRatio = 1.0;
    Tabs tabs;
    RadioOption processingModeOptions;
    GlutTextBox processingModeText {0.7, 0.3, -0.9, 0.9, 0.1};
    // *** stuff that the plugin instance should own ***
    // eye position
    float upDir = 1.0;  // y component of eyeUp
    float eyePos[3]; // x,y,z
    float eyeUp[3] = {0.0, 1.0, 0.0};
    float eyeRad = 3.1;
    float eyeAzi = 9*M_PI/8;
    float eyeEle = M_PI/2.2;
    float automationViewWidth = 30.0;
    float automationViewOffset = automationViewWidth/2.0;
    //DisplayState displayState = DisplayState::MAIN;
    bool showHelp = true;
    // *************************************************
    float prevAutomationViewWidth = automationViewWidth;
    float prevAutomationViewOffset = automationViewOffset;
    ScopedPointer<ResizableCornerComponent> resizerCorner;
    //ScopedPointer<ResizableCornerComponent> resizerCorner2;
    //ScopedPointer<ResizableBorderComponent> resizerBorder;
    ComponentBoundsConstrainer resizeLimits;
    //CriticalSection resizerLock;
    // buffer to hold selected object data
    GLuint objSelectBuf[SELECT_BUF_SIZE];
    // display list to draw lots of glVertices at once for the path automation curve, one for each of the 8 possible sources
    GLuint pathDisplayList[8] = {0};
    GLuint pathPosDisplayList[8] = {0};
    //std::vector<GLuint> pathPosDisplayList[8];
    int mouseOverSourceIndex = -1;
    int mouseOverPathPointSourceIndex = -1;
    int mouseOverPathPointIndex = -1;
    bool mouseOverLoopRegionBegin = false;
    bool mouseOverLoopRegionEnd = false;
    bool loopRegionBeginSelected = false;
    bool loopRegionEndSelected = false;
    //bool loopRegionJustDeselected = false;
    std::array<int,3> mouseOverPathAutomationPointIndex {-1,-1,-1}; // [sourceIndex, pathAutoPtIndex, pathAutoPtIndexAmongSelectedPoints]
    bool pathAutomationPointsGrabbedWithMouse = false; // might get rid of this since we can just know by the 3rd dim of the index array being ==/!= -1
    bool mouseOverDopplerSlider = false;
    float newDopplerSpeedOfSound = DEFAULT_SPEED_OF_SOUND;
    Sources* sources = nullptr;
    float secPos = 0;
    // GL is not initialized until newOpenGLContextCreated() is called
    bool glInited = false;
    ThreeDAudioProcessor* processor = nullptr;
    OpenGLContext openGLContext;
    bool mouseDragging = false;
    float mouseDragDownX = 0;
    float mouseDragDownY = 0;
    float mouseDragCurrentX = 0;
    float mouseDragCurrentY = 0;
    bool mouseIsDown = false;
    bool mouseDoubleClicked = false;
    bool mouseJustDown = false;
    //bool mouseClickedOnSomething = false; // cannot remember why i used this, it seems to only make you need to click twice in empty space to deselect all...
    float mouseZoomFactor = 0;
    float mouseWheeldX = 0;
    float mouseWheeldY = 0;
    bool wasCtrlCmdDownPrev = false;
    // stuff for autoaligning path auto pts
    float animationTime[2] = {0,0};
    float animationDuration = 1.0;
    bool animationOn[2] = {false,false};
    float autoAlignPos[2] = {1.1, 1.1};
    // **********************************************
    float arrowKeySpeedFactor = initialArrowKeySpeedFactor;
    static constexpr float initialArrowKeySpeedFactor = 1.0;
    static constexpr float baseArrowKeySpeed = 0.01;
    static constexpr float openGLFrameRate = 30.0; // in frames per second
    ThreeDAudioProcessor* getProcessor() const
    {
        return static_cast <ThreeDAudioProcessor*> (getAudioProcessor());
    }
};

// the help texts
static const String mainHelp[] =//{""};
{
  "FOR TRACKPAD MOUSE USERS:",
  "mouse pinch, x/y scroll to move view / selected stuff",
  "FOR TRADITIONAL MOUSE USERS:",
  "[arrow keys / (mouse scroll +/- shift)] +/- alt to move view / selected stuff",
  "FOR ALL USERS:",
  "d-click in space to add sound source / path point for selected source(s)",
  "click to toggle selected state of source / path point",
  "click + drag to select source(s) / path point(s) in a region",
  "hold shift to add to selection w/o deselecting",
  "hold ctrl/cmd with things selected to move the view instead",
  "ctrl/cmd + 'a' to select all sources / path points",
  "ctrl/cmd + 'c' to copy all selected stuff",
  "ctrl/cmd + 'z' to undo a source edit",
  "ctrl/cmd + shift + 'z' to redo a source edit",
  "'x' / d-click to add path points for selected sources",
  "'p' to toggle selected source(s) path type (circular/pt-pt)",
  "'d' to toggle doppler effect on/off",
  "backspace to delete selected stuff",
  "'m' to toggle if source(s) are moving on path(s)",
  "'v' to switch views",
  "'h' to toggle help"
};

static const String automationHelp[] =//{""};
{
  "FOR TRACKPAD MOUSE USERS:",
  "mouse pinch, x/y scroll to move view",// / selected points",
  "FOR TRADITIONAL MOUSE USERS:",
  "arrow keys / (mouse scroll +/- shift) to move view",// / selected points",
  "FOR ALL USERS:",
  "d-click in space to add point",
  "click on point to toggle selected",
  "click + drag to select points in region",
  "hold shift to add to selection w/o deselecting",
  "d-click on point to pick up and move with mouse",
  "arrow keys to move all selected points",
  "hold alt when moving points to auto align w/ mouse + other points",
  "hold ctrl/cmd to move the 3d background view instead",
  "ctrl/cmd + 'a' to select all points",
  "ctrl/cmd + 'c' to copy all selected points",
  "ctrl/cmd + 'z' to undo a source edit",
  "ctrl/cmd + shift + 'z' to redo a source edit",
  "'c'/'s'/'o' to make selected segments curvy/straight/open",
  "'d' to toggle doppler effect on/off",
  "backspace to delete selected stuff",
  "'m' to toggle if source(s) are moving on path(s)",
  "'l' to define a looping region defined by first/last selected points",
  "'v' to switch views",
  "'h' to toggle help"
};

#endif /* defined(__PluginEditor__) */
