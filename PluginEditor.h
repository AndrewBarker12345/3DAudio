//
//  PluginEditor.h
//
//  Created by Andrew Barker on 4/26/14.
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

#ifndef __PluginEditor__
#define __PluginEditor__

//#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"
#include "TextUtils.h"
#include "GLUT.h"
#include "SelectionBox.h"
#include "View2D.h"
#include "DrawInterpolator.h"
#include "Points.h"

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

//static constexpr float stroke_scale = 0.00035;
//static constexpr float topMax = 119.05*stroke_scale;

//void drawStringStroke(char *s, float x, float y, float z, float aspect, float scale);

class MultiFieldTextInputRestrictor : public TextInputRestrictor {
public:
    MultiFieldTextInputRestrictor(const std::vector<TextInputRestrictor*>& fieldIRs,
                                  const std::string& fieldSeparator) noexcept;
    void setFieldIRs(const std::vector<TextInputRestrictor*>& fieldIRs) noexcept;
    TextInputRestrictor* getFieldIR(std::size_t index) const noexcept;
    TextInputRestrictor* clone() const override;
    bool insert(std::string& text,
                int& cursorPosition,
                const std::string& textToInsert) const override;
    void validateInput(std::string& text) const override;
    std::string getID() const noexcept override;
    std::string getField(const std::string& text,
                         std::size_t index) const noexcept;
private:
    std::string fieldSeparator;
    std::vector<PolyPtr<TextInputRestrictor>> fieldIRs;
};

class PositionerText3D {
public:
    PositionerText3D (OpenGLWindow* window) noexcept;
    void show (Box boundary,
               const std::array<float, 3>& rae,
               TextLook* look);
    void hide() noexcept;
    bool isVisible() const noexcept;
    void releaseFocus() noexcept;
    void draw (OpenGLWindow &window,
               const Point<float>& mousePosition,
               bool mouseOverEnabled = true);
    bool mouseClicked();
    bool mouseDoubleClicked();
    bool mouseDragged (const Point<float>& mouseDownPosition,
                       const Point<float>& mouseCurrentPosition,
                       bool adjustScroll = false);
    bool keyPressed (const std::string& key);
    const OpenGLWindow* getWindow() const noexcept;
    void fitBoundaryToFontSize();
    Box getBoundary() const noexcept;
    void setBoundary(const Box& boundary) noexcept;
    void setXYZ(const std::array<float, 3>& xyz) noexcept;
    std::array<float, 3> getRAE(const std::array<float, 3>& defaultRAE) const noexcept;
    std::array<float, 3> getXYZ(const std::array<float, 3>& defaultXYZ) const noexcept;
    /** takes rae in radians from XYZtoRAE and turns it into degrees with azi [-180, 180] and ele [-90, 90] */
    static void worldToDisplayRAE(std::array<float, 3>& rae) noexcept;
    const Animation& getShowAnimation() const noexcept;
//private:
    bool mouseOverEnabled = true;
    EditableTextBox raeText, xyzText;
private:
    TextLook finalLook;
    TextLook look;
    Animation showAnimation;
    bool visible;
};

//class MultiFieldTextInputRestrictor : public  {
//public:
//    MultiFieldEditableText (int numFields,
//                            const std::string& fieldSeparator,
//                            std::unique_ptr<TextInputRestrictor> fieldIR) noexcept;
//private:
//    int numFields;
//    std::string fieldSeparator;
//    PolyPtr<TextInputRestrictor> fieldIR;
//};

class ThreeDAudioProcessorEditor  : public AudioProcessorEditor, public OpenGLRenderer, public MultiTimer
{
public:
    ThreeDAudioProcessorEditor (ThreeDAudioProcessor* ownerFilter);
    ~ThreeDAudioProcessorEditor();
    
    void resized() override;
    // This is just a standard Juce paint method...
    //void paint (Graphics& g);
    void newOpenGLContextCreated() override;
    void openGLContextClosing() override;
    void mouseEnter(const MouseEvent & event) override;
    void mouseExit(const MouseEvent & event) override;
    void mouseMove(const MouseEvent & event) override;
    void mouseDown(const MouseEvent & event) override;
    void mouseDrag(const MouseEvent & event) override;
    void mouseDoubleClick(const MouseEvent & event) override;
    void mouseUp(const MouseEvent & event) override;
    void mouseWheelMove	(const MouseEvent & event,
                         const MouseWheelDetails & wheel) override;
    void mouseMagnify (const MouseEvent & event,
                       float scaleFactor) override;
    //void showMouseDetails() const;
    bool keyPressed	(const KeyPress & key) override;
    //void keepGrabbedPathAutoPointUnderMouse();
    void autoAlignAutomationPoints(bool alignInX, bool alignInY, bool alignWithMouse);
    //void perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar);
    // Draw scene in here
    void renderOpenGL() override;
    void drawHelp();
    void loadHelpText();
    void drawHead() const;
    // main view for drawing a path
    void drawMain();
    void draw3DAxis();
    // view for automating a moving source
    void drawPathControl();
    void drawInterpolatedPath(int sourceIndex);
    void drawLoopingRegion();
    inline float timeValueToXPosition(float tVal) const;
    float getMouseX() const;
    float getMouseY() const;
    Point<float> getMousePosition() const;
    //void drawStringBitmap(void *font, char *s, float x, float y, float z) const;
    //void drawStringStroke(char *s, float x, float y, float z) const;
    std::string getFormattedTimeString(float timeInSec) const;
    //void getFormattedTime(float timeInSec, char* str) const;
    void timerCallback(int timerID) override;
    //void drawMouseDragging();
    //void drawCircle(float x, float y, float r, int segments) const;
    void updateEyePositionRAE(float rad, float azi, float ele);
    void updateEyePositionXYZ(float x, float y, float z);
    //void updateXYZ(float rad, float azi, float ele, float *xyz) const;
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
    void drawSelectableOrb(const PointXYZ<float>& position,
                           float radius,
                           int numSlices,
                           int numStacks,
                           Colour normalColor,
                           Colour mouseOverColor,
                           bool mouseOver,
                           bool& prevMouseOver,
                           Multi<Animation>& mouseOverAnimations,
                           bool selected,
                           bool& prevSelected,
                           Multi<Animation>& selectAnimations,
                           int orbID,
                           float alpha,
                           bool haloEnabled = true,
                           bool positionLinesEnabled = true);
    //void drawRect(float centerX, float centerY, float deltaX, float deltaY);
    //void setProcessingModeAutoDetect(bool isHostRealTime) noexcept;
private:
    TextLook axis3dTextLook;
    std::array<TextBox, 6> axis3dLabels;
    void repositionAxis3dLabels();
    
    /** adjust the boundary so that it fits inside the window and is not overlapping with the help / doppler buttons and view tabs */
    void adjustSoVisible(Box& b) noexcept;
    /** a place to do stuff when mouse is moved on the gl thread */
    void myMouseMoved();
    
    std::array<bool, maxNumSources> prevSourceMuted;
    
    TextLook pathIndexTextLook;
    TextLook pathIndexSourceSelectAnimationLook;
    TextLook pathIndexSourceDeselectAnimationLook;
    TextLook pathIndexSelectedTextLook;
    std::vector<std::vector<std::unique_ptr<TextBox>>> pathIndexTexts;
    //std::vector<std::vector<EditableTextBox>> pathIndexTexts;// make TextBox*'s ?
    /** repopulates the path point index texts with the appropriate state */
    void reindexPathIndexTexts();
    /** repositions the existing path point index texts on the screen */
    void repositionPathIndexTexts();
    
    TextLook sourcePositionerTextLook;
    TextLook sourcePathPointPositionerTextLook;
    PositionerText3D positionerText3D { &glWindow };
    struct PositionerText3DID {
		PositionerText3DID() : sourceIndex(-1), pathPtIndex(-1) {}
		PositionerText3DID(int sI, int pPtI) : sourceIndex(sI), pathPtIndex(pPtI) {}
        bool equals(int s, int pt = -1) { return s == sourceIndex && pt == pathPtIndex; }
        int sourceIndex = -1, pathPtIndex = -1;
    };
    bool isPositionerText3DIDInBounds() const noexcept;
    PositionerText3DID positionerText3DID;
    void updatePositioner3DTextValueAndPosition();
    void tryHidePositioner3D();
    
    void resizePathPtsPrevState();
    Multi<Animation> mouseOverPathPtAnimations;
    std::array<std::vector<bool>, maxNumSources> prevMouseOverPathPts;
    Multi<Animation> selectPathPtAnimations;
    std::array<std::vector<bool>, maxNumSources> prevSelectedPathPts;
    
    Multi<Animation> mouseOverSourceAnimations;
    std::array<bool, maxNumSources> prevMouseOverSources;
    Multi<Animation> selectSourceAnimations;
    std::array<bool, maxNumSources> prevSelectedSources;
    
    // amount of main window that is used for "stuff"
    float x_scale = 0.9f, y_scale = 0.9f;
    Box windowCoordinates {1, -1, -1, 1}; // coordinate system of the plugin's window
    View2D pathAutomationView { &windowCoordinates,
        {y_scale, -y_scale, -x_scale, x_scale},
        {60, 0.001f, 1000*60*60},
        {1, 0.001f, 1},
        30, 0, 1000*60*60,
        0.5f, 0, 1 };
    
    // text for the automation view time limits
    TextLook automationViewLowerTimeTextLook;
    TextBox automationViewLowerTimeText {"", Box(-y_scale - 0.1f * (1 - y_scale), -1 + 0.1f * (1 - y_scale), -x_scale, 0), &automationViewLowerTimeTextLook};
    TextLook automationViewUpperTimeTextLook;
    TextBox automationViewUpperTimeText {"", Box(-y_scale - 0.1f * (1 - y_scale), -1 + 0.1f * (1 - y_scale), 0, x_scale), &automationViewUpperTimeTextLook};
    
    TextLook automationViewTimeTextLook;
    float automationViewTimeTextWidth;
    TextBox automationViewTimeText {"", Box(-y_scale - 0.1f * (1 - y_scale), -1 + 0.15f * (1 - y_scale), -2, 2), &automationViewTimeTextLook};
    std::vector<TextBox> automationViewPercentTexts;
    // the positioner text is like a right click and edit attribute of some thing through text
    bool positionerTextVisible = false;
    bool positionerTextDrawEditable = false;
    TextLook positionerLook;
    //TextBox mousePositionText { "", Box(), &mousePositionLook };// TextBox(const std::string& text, const Box& boundary, TextLook* look) n
    EditableTextBox positionerText {{ "", Box(), &positionerLook }, &glWindow};
    void positionerTextMouseMoved(bool forceRefreshText = false);
    void boundsCheckPositionerText() noexcept;
    
    PointLook pointLook;
    std::array<DrawablePointState, maxNumSources> pointStates;
    void myResized(); // so i can do resize stuff safely on the gl thread

    TextLook volumeSliderTextLook;
    GLSlider volumeSlider;
    
    TextLook mixSliderTextLook;
    GLSlider mixSlider;
    
    TextLook dopplerSliderTextLook;
    Box dopplerTitleBox {-0.92f, -0.995f, -0.8f, 0.0f};
    Box dopplerValueBox {-0.92f, -0.995f, 0.0f, 0.8f};
    GLSlider dopplerSlider {{-0.85f, -0.95f, -0.8f, 0.8f},
        {"Speed Of Sound:", dopplerTitleBox, &dopplerSliderTextLook},
        {{"-1", dopplerValueBox, &dopplerSliderTextLook}, &glWindow}};

	OpenGLContext openGLContext;
    OpenGLWindow glWindow {&openGLContext};
    
    GLTextButton dopplerButton {"Doppler", {0.845f, 0.785f, 0.8f, 0.998f}};
    GLTextButton helpButton {"Help", {0.91f, 0.85f, 0.8f, 0.998f}};
    
    TextLook helpTextLook;
    TextBoxGroup helpText {{}, 0, {.95f, -.95f, -1.0f, 1.0f}, &helpTextLook};
    
    GLTextButton websiteButton {"made by Freedom Audio", {-.85f, -.95f, -.5f, .5f}};

//    TextLook etbLook;
//    EditableTextBox etb {{"Dear Dasvidania,    I love you.  Regards,    Bob", {-.1f, -.7f, -.5f, .9f}, &etbLook}, &glWindow};
    // display time as sec (=0) or in measures|beats|frac (=1)
    int timeMode = 0;
    // to sync reads/writes to various variables used by the GL rendering JUCE message thread threads
	std::recursive_mutex /*std::mutex*/ glLock;
    std::atomic<float> windowAspectRatio {1.0f};
    GLTextTabs tabs {{"Sound Sources", "Automate Movement", "Audio Settings"}, {0.995f, 0.92f, -1.0f, 1.0f}, GLTextTabs::ABOVE};
    TextLook processingModeNormalLook;
    TextLook processingModeSelectedLook;
    TextLook processingModeSelectAnimationBeginLook;
    TextLook processingModeMouseOverLook;
    TextLook processingModeMouseOverAutoDetectLook;
    GLTextRadioButton processingModeOptions {{{"Realtime", "HighQuality", "AutoDetect"}, 1, {0.8f, 0.7f, -.9f, 0.9f}, &processingModeNormalLook, true}, 2};
//    GLTitledRadioButton processingModeOptions {
//        {"Processing Mode:", {0.8f, 0.7f, -0.9f, -.25f}, &processingModeNormalLook},
//        {{{"Realtime", "HighQuality", "AutoDetect"}, 1, {0.8f, 0.7f, -.2f, 0.9f}, &processingModeNormalLook, true}, 2}
//    };
//    TitledRadioButton processingModeOptions {"Processing Mode:", {0.8, 0.7, -0.9, -0.35},
//        {"Realtime", "HighQuality", "AutoDetect"}, {0.8, 0.7, -0.35, 0.9}, 1, 3, 2};
    
//    RadioOptionWithAutoDetect processingModeOptions {{{"Processing Mode:", {0.8, 0.7, -0.9, -0.35}},
//        {{"Realtime", "HighQuality", "AutoDetect"}, {0.8, 0.7, -0.35, 0.9}, 1, 3}}, 2, 0};
    TextLook processingModeHelpLook;
    TextBox processingModeHelp {"", {0.65f, websiteButton.getBoundary().getTop(), -0.85f, 0.85f}, &processingModeHelpLook};
    //MultiLineTextBox processingModeHelp {"", {0.65, 0, -0.85, 0.85}};
    int currentProcessingModeHelpIndex = -1;
    // *** stuff that the plugin instance should own ***
    // eye position
    float upDir = 1;  // y component of eyeUp
    float eyePos[3]; // x,y,z
    float eyeUp[3] = {0, 1, 0};
    float eyeRad = 3.1f;
    float eyeAzi = 9*M_PI/8;
    float eyeEle = M_PI/2.2f;
//    float automationViewWidth = 30.0;
//    float automationViewOffset = automationViewWidth/2.0;
    //DisplayState displayState = DisplayState::MAIN;
    //bool showHelp = false;
    // *************************************************
//    float prevAutomationViewWidth = automationViewWidth;
//    float prevAutomationViewOffset = automationViewOffset;
    ScopedPointer<ResizableCornerComponent> resizerCorner;
    //ScopedPointer<ResizableCornerComponent> resizerCorner2;
    //ScopedPointer<ResizableBorderComponent> resizerBorder;
    ComponentBoundsConstrainer resizeLimits;
    //CriticalSection resizerLock;
    // buffer to hold selected object data
    GLuint objSelectBuf[SELECT_BUF_SIZE];
    // display list to draw lots of glVertices at once for the path automation curve, one for each of the 8 possible sources
    std::array<GLuint, maxNumSources> pathDisplayList = {0};
    std::array<GLuint, maxNumSources> pathAutomationDisplayList = {0};
    int mouseOverSourceIndex = -1;
    int mouseOverPathPointSourceIndex = -1;
    int mouseOverPathPointIndex = -1;
    bool mouseOverLoopRegionBegin = false;
    bool mouseOverLoopRegionEnd = false;
    bool loopRegionBeginSelected = false;
    bool loopRegionEndSelected = false;
    bool loopRegionBeginHasPositionerTextFocus = false;
    bool loopRegionEndHasPositionerTextFocus = false;
    Animation loopRegionToggleAnimation {0.25f};
    Animation loopRegionBeginAnimation;
    Animation loopRegionEndAnimation;
    Animation loopRegionSelectAnimation;
    //bool loopRegionJustDeselected = false;
    std::array<int, 3> pathAutomationPointEditableTextIndex {-1,-1,-1};
    std::array<int, 3> mouseOverPathAutomationPointIndex {-1,-1,-1}; // [sourceIndex, pathAutoPtIndex, pathAutoPtIndexAmongSelectedPoints]
    bool pathAutomationPointsGrabbedWithMouse = false; // might get rid of this since we can just know by the 3rd dim of the index array being ==/!= -1
    bool pathAutomationPointIndexValid(const std::array<int, 3>& index) const noexcept;
    //bool mouseOverDopplerSlider = false;
    //float newDopplerSpeedOfSound = defaultSpeedOfSound;
    Sources* sources = nullptr;
    float secPos = 0;
    // GL is not initialized until newOpenGLContextCreated() is called
    bool glInited = false;
    ThreeDAudioProcessor* processor = nullptr;
    
    SelectionBox selectionBox;
//    Animation mouseDragAnimation {0.5};
//    bool mouseDragging = false;
//    float mouseDragDownX = 0;
//    float mouseDragDownY = 0;
//    float mouseDragCurrentX = 0;
//    float mouseDragCurrentY = 0;
    float mouseX = 0, mouseY = 0;
    bool mouseIsDown = false;
    bool mouseDoubleClicked = false;
    bool mouseJustDown = false;
    //bool mouseClickedOnSomething = false; // cannot remember why i used this, it seems to only make you need to click twice in empty space to deselect all...
    float mouseZoomFactor = 0;
    float mouseWheeldX = 0;
    float mouseWheeldY = 0;
    bool wasCtrlCmdDownPrev = false;
    // stuff for autoaligning path auto pts
    Animation pathPtAutoAlignAnimationX {1}, pathPtAutoAlignAnimationY {1};
//    float animationTime[2] = {0, 0};
//    float autoAlignAnimationDuration = 1.0f;
//    bool animationOn[2] = {false, false};
    float pathPtAutoAlignX = 1.1f, pathPtAutoAlignY = 1.1f;
    // **********************************************
    float arrowKeySpeedFactor = initialArrowKeySpeedFactor;
    static constexpr float initialArrowKeySpeedFactor = 1.0;
    static constexpr float baseArrowKeySpeed = 0.01;
    float displayScale = 1.0; // scale for monitor display needed to display properly on retina mac screens
    //static constexpr float openGLFrameRate = 30.0; // in frames per second
    ThreeDAudioProcessor* getProcessor() const
    {
        return static_cast <ThreeDAudioProcessor*> (getAudioProcessor());
    }
    JUCE_LEAK_DETECTOR(ThreeDAudioProcessorEditor);
};

class HoursMinSecInputRestrictor : public TextInputRestrictor
{
public:
    HoursMinSecInputRestrictor(int numDecimalPlacesForSec = 2);
    TextInputRestrictor* clone() const override;
    bool insert(std::string& text, int& cursorPosition, const std::string& textToInsert) const override;
    void validateInput(std::string& text) const override;
private:
    DecimalNumberRestrictor secIR;// = makePolyPtr<DecimalNumberRestrictor>(0, 60, 2);
    DecimalNumberRestrictor minIR;
    DecimalNumberRestrictor hrIR;
};

class MeasureBeatFractionInputRestrictor : public TextInputRestrictor
{
public:
    MeasureBeatFractionInputRestrictor();
    TextInputRestrictor* clone() const override;
    bool insert(std::string& text, int& cursorPosition, const std::string& textToInsert) const override;
    void validateInput(std::string& text) const override;
private:
    DecimalNumberRestrictor fracIR, measAndBeatIR;
};

class PathAutomationInputRestrictor : public TextInputRestrictor
{
public:
    enum TimeMode { HR_MIN_SEC, MEAS_BEAT_FRAC };
    TextInputRestrictor* clone() const override;
    bool insert(std::string& text, int& cursorPosition, const std::string& textToInsert) const override;
    void validateInput(std::string& text) const override;
    std::string getID() const noexcept override { return "PathAutomation"; }
    void setTimeMode(TimeMode mode);
    std::string divString = ", ";
private:
    PolyPtr<TextInputRestrictor> timePosition = makePolyPtr<HoursMinSecInputRestrictor>();//= makePolyPtr<DecimalNumberRestrictor>(0, std::numeric_limits<float>::infinity(), 2);
    DecimalNumberRestrictor pathPosition {0, 100, 1, "%"};
    //TimeMode mode = HR_MIN_SEC;
    //std::string units;
};

// the help texts
//static const std::string mainHelp =
//    "FOR TRACKPAD MOUSE USERS:\n\
//    mouse pinch, x/y scroll to move view / selected stuff\n\
//    FOR TRADITIONAL MOUSE USERS:\n\
//    [arrow keys / (mouse scroll +/- shift)] +/- alt to move view / selected stuff";
////    \n
////    "FOR ALL USERS:\n
////    "d-click in space to add sound source / path point for selected source(s)\n
////    "click to toggle selected state of source / path point\n
////    "click + drag to select source(s) / path point(s) in a region\n
////    "hold shift to add to selection w/o deselecting\n
////    "hold ctrl/cmd with things selected to move the view instead\n
////    "ctrl/cmd + 'a' to select all sources / path points\n
////    "ctrl/cmd + 'c' to copy all selected stuff\n
////    "ctrl/cmd + 'z' to undo a source edit\n
////    "ctrl/cmd + shift + 'z' to redo a source edit\n
////    "'x' / d-click to add path points for selected sources\n
////    "'p' to toggle selected source(s) path type (circular/pt-pt)\n
////    "'d' to toggle doppler effect on/off\n
////    "backspace to delete selected stuff\n
////    "'m' to toggle if source(s) are moving on path(s)\n
////    "'v' to switch views\n
////    "'h' to toggle help"
//
////static const StringArray automationHelp
//static std::string automationHelp =
//    "FOR TRACKPAD MOUSE USERS:\
//     mouse pinch, x/y scroll to move view\
//     FOR TRADITIONAL MOUSE USERS:\
//     arrow keys / (mouse scroll +/- shift) to move view";
////    "FOR ALL USERS:",
////    "d-click in space to add point",
////    "click on point to toggle selected",
////    "click + drag to select points in region",
////    "hold shift to add to selection w/o deselecting",
////    "d-click on point to pick up and move with mouse",
////    "arrow keys to move all selected points",
////    "hold alt when moving points to auto align w/ mouse + other points",
////    "hold ctrl/cmd to move the 3d background view instead",
////    "ctrl/cmd + 'a' to select all points",
////    "ctrl/cmd + 'c' to copy all selected points",
////    "ctrl/cmd + 'z' to undo a source edit",
////    "ctrl/cmd + shift + 'z' to redo a source edit",
////    "'c'/'s'/'o' to make selected segments curvy/straight/open",
////    "'d' to toggle doppler effect on/off",
////    "backspace to delete selected stuff",
////    "'m' to toggle if source(s) are moving on path(s)",
////    "'l' to define a looping region defined by first/last selected points",
////    "'t' to toggle time display format (hr:min:sec / measure|beat|frac)",
////    "'v' to switch views",
////    "'h' to toggle help"

// the help texts
//static const StringArray mainHelp =
static const std::vector<std::string> mainHelp =
{
    "FOR TRACKPAD MOUSE USERS:",
    "mouse pinch, x/y scroll to move view / selected stuff",
    "FOR TRADITIONAL MOUSE USERS:",
    "[arrow keys / (mouse scroll +/- shift)] +/- alt to move view / selected stuff",
    "FOR ALL USERS:",
    "d-click in space to add sound source / path point for selected source(s)",
    "click to toggle selected state of source / path point",
    "right click to edit position of source / path point",
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

//static const StringArray automationHelp
static const std::vector<std::string> automationHelp =
{
    "FOR TRACKPAD MOUSE USERS:",
    "mouse pinch, x/y scroll to move view",// / selected points",
    "FOR TRADITIONAL MOUSE USERS:",
    "arrow keys / (mouse scroll +/- shift) to move view",// / selected points",
    "FOR ALL USERS:",
    "d-click in space to add point",
    "click on point to toggle selected",
    "click + drag to select points in region",
    "right click to edit position of point / looping region",
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
    "'l' to toggle looping",
    //"'t' to toggle time display format (hr:min:sec / measure|beat|frac)", // decided to abandon this feature since I can't find a good way to deal with tempo / bpm changes, but leaving the partially working code intact for now
    "'v' to switch views",
    "'h' to toggle help"
};

static const std::array<std::string, 3> processingModeHelpText
    {"    The realtime processing mode is intended to be used when you are editing the tracks that use this plugin.  It puts the least strain on your CPU so that you can listen to your tracks in realtime while you edit them.  However, for moving sound sources, the audio quality will be less than ideal so don't use this setting when you are doing the final export of your tracks that have moving sound sources.",
     "    The high quality processing mode is intended to be used when you are done editing your tracks that use this plugin and want the highest audio quality possible for moving sound sources.  When using this processing mode, high demand is placed on your CPU so you may not be able to listen to your tracks in realtime.  Select this mode before you lock any tracks that you are done editing or before you do the final export of your tracks to get the highest possible audio quality for your moving sound sources.",
     "    The auto-detect processing mode chooses the appropriate processing mode for this plugin based on if your DAW is running in a realtime capacity or not.  Some DAWs may not provide the means to explicitly specify this, but you can do so for this plugin by choosing either realtime or high quality above.  Note that these modes only affect the audio quality and CPU demand for moving sound sources.  If you have only stationary sound sources, you don't need to worry about this setting."};

//// the help texts
//static const String mainHelp[] =//{""};
//{
//  "FOR TRACKPAD MOUSE USERS:",
//  "mouse pinch, x/y scroll to move view / selected stuff",
//  "FOR TRADITIONAL MOUSE USERS:",
//  "[arrow keys / (mouse scroll +/- shift)] +/- alt to move view / selected stuff",
//  "FOR ALL USERS:",
//  "d-click in space to add sound source / path point for selected source(s)",
//  "click to toggle selected state of source / path point",
//  "click + drag to select source(s) / path point(s) in a region",
//  "hold shift to add to selection w/o deselecting",
//  "hold ctrl/cmd with things selected to move the view instead",
//  "ctrl/cmd + 'a' to select all sources / path points",
//  "ctrl/cmd + 'c' to copy all selected stuff",
//  "ctrl/cmd + 'z' to undo a source edit",
//  "ctrl/cmd + shift + 'z' to redo a source edit",
//  "'x' / d-click to add path points for selected sources",
//  "'p' to toggle selected source(s) path type (circular/pt-pt)",
//  "'d' to toggle doppler effect on/off",
//  "backspace to delete selected stuff",
//  "'m' to toggle if source(s) are moving on path(s)",
//  "'v' to switch views",
//  "'h' to toggle help"
//};
//
//static const String automationHelp[] =//{""};
//{
//  "FOR TRACKPAD MOUSE USERS:",
//  "mouse pinch, x/y scroll to move view",// / selected points",
//  "FOR TRADITIONAL MOUSE USERS:",
//  "arrow keys / (mouse scroll +/- shift) to move view",// / selected points",
//  "FOR ALL USERS:",
//  "d-click in space to add point",
//  "click on point to toggle selected",
//  "click + drag to select points in region",
//  "hold shift to add to selection w/o deselecting",
//  "d-click on point to pick up and move with mouse",
//  "arrow keys to move all selected points",
//  "hold alt when moving points to auto align w/ mouse + other points",
//  "hold ctrl/cmd to move the 3d background view instead",
//  "ctrl/cmd + 'a' to select all points",
//  "ctrl/cmd + 'c' to copy all selected points",
//  "ctrl/cmd + 'z' to undo a source edit",
//  "ctrl/cmd + shift + 'z' to redo a source edit",
//  "'c'/'s'/'o' to make selected segments curvy/straight/open",
//  "'d' to toggle doppler effect on/off",
//  "backspace to delete selected stuff",
//  "'m' to toggle if source(s) are moving on path(s)",
//  "'l' to define a looping region defined by first/last selected points",
//  "'v' to switch views",
//  "'h' to toggle help"
//};

#endif /* defined(__PluginEditor__) */
