/*
 PluginEditor.cpp
 
 The shitshow of code that is 3DAudio's GUI.

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

#include "PluginEditor.h"
#include "Functions.h"
#include "DrawInterpolator.h"
#include <algorithm>
//#include <functional>
//#include <iostream>
//#include <fstream>

// the global hrir data that gets one instance across multiple plugin instances, this just references the one instance defined in PluginProcessor.cpp
//extern float***** HRIRdata;
//extern float**** HRIRdataPoles;
#ifdef WIN32
	static bool glutInited = false;
#endif
//==============================================================================
ThreeDAudioProcessorEditor::ThreeDAudioProcessorEditor (ThreeDAudioProcessor* ownerFilter)
    : AudioProcessorEditor (ownerFilter)
{
//#ifdef WIN32
//	// this ctor is getting called twice somehow on windows, and glutInit only likes being called once
//	if (!glutInited) {
//		char *myargv[1];
//		int myargc = 1;
//		myargv[0] = _strdup("");
//		glutInit(&myargc, myargv);
//		glutInited = true;
//	}
//#endif
    
    // keep a pointer to the processor (no longer necessary b/c getProcessor() is available in this class)
    processor = ownerFilter;
    
    // *** update view state stuff from the plugin instance ***
    upDir = processor->upDir;
    eyePos[0] = processor->eyePos[0];
    eyePos[1] = processor->eyePos[1];
    eyePos[2] = processor->eyePos[2];
    eyeUp[0] = processor->eyeUp[0];
    eyeUp[1] = processor->eyeUp[1];
    eyeUp[2] = processor->eyeUp[2];
    eyeRad = processor->eyeRad;
    eyeAzi = processor->eyeAzi;
    eyeEle = processor->eyeEle;
    pathAutomationView.setWidth(processor->automationViewWidth);
    pathAutomationView.setXPosition(processor->automationViewOffset);
//    automationViewWidth = processor->automationViewWidth;
//    automationViewOffset = processor->automationViewOffset;
    //displayState = processor->displayState;
    //showHelp = processor->showHelp;
    // ********************************************************
    
    processingModeOptions.setSelected(static_cast<int>(processor->processingMode.load()), false);
    const auto popsicleGreen = Colour::fromFloatRGBA(102.0f/255, 1, 102.0f/255, 1);
//    processingModeOptions.setColor(popsicleGreen);
//    processingModeOptions.options.setBoundaryPadding(0.8, 1);
    
    //processingModeSelectedLook.fontStyle = Font::FontStyleFlags::bold;//(Font::FontStyleFlags)(Font::FontStyleFlags::underlined | Font::FontStyleFlags::bold);
    processingModeSelectedLook.color = popsicleGreen;
    processingModeSelectedLook.horizontalPad = 0.85f;
    //processingModeSelectedLook.just = Justification:: centredLeft;
    
    const auto salmonPink = Colour::fromFloatRGBA(1, 1, 1, 1);// Colour::fromFloatRGBA(1, 0, 149/255.0f, 1);
    const MyGlowEffect glow (salmonPink/*Colours::purple.withAlpha(1.0f)*/, 12, 1);
    processingModeSelectAnimationBeginLook = processingModeSelectedLook;
    processingModeSelectAnimationBeginLook.effects.emplace_back(glow);
    processingModeSelectAnimationBeginLook.horizontalPad = 0.85f;
    //processingModeSelectAnimationBeginLook.just = Justification:: centredLeft;
    processingModeOptions.setSelectedLook(&processingModeSelectedLook,
                                          &processingModeSelectAnimationBeginLook);
    
    const MyGlowEffect glow2 (salmonPink.withAlpha(0.9f)/*Colours::purple.withAlpha(0.9f)*/, 2, 1);
    processingModeMouseOverLook.effects.emplace_back(glow2);
    processingModeMouseOverLook.color = popsicleGreen;
    processingModeMouseOverLook.horizontalPad = 0.85f;
    //processingModeMouseOverLook.just = Justification::centredLeft;
    processingModeOptions.setMouseOverLook(&processingModeMouseOverLook);
    
    processingModeMouseOverAutoDetectLook.effects.emplace_back(glow2);
    processingModeMouseOverAutoDetectLook.color = popsicleGreen;
    //processingModeMouseOverAutoDetectLook.fontStyle = Font::FontStyleFlags::bold;//Font::FontStyleFlags::underlined;
    processingModeMouseOverAutoDetectLook.horizontalPad = 0.85f;
    //processingModeMouseOverAutoDetectLook.just = Justification::centredLeft;
    processingModeOptions.setMouseOverAutoDetectLook(&processingModeMouseOverAutoDetectLook);
    
    processingModeNormalLook.color = popsicleGreen;
    processingModeNormalLook.horizontalPad = 0.85f;
    //processingModeNormalLook.just = Justification::centredLeft;
    processingModeOptions.setNormalLook(&processingModeNormalLook);
    
    processingModeHelpLook.multiLine = true;
    processingModeHelpLook.wrap = true;
    processingModeHelpLook.color = popsicleGreen;
    processingModeHelpLook.just = Justification::topLeft;
    processingModeHelp.setLook(&processingModeHelpLook);
    
    tabs.setSelected(static_cast<int>(processor->displayState.load()), false);
    loadHelpText();
    
    // set the eye's initial xyz position
    axis3dTextLook.fontSize = 16;
    axis3dLabels[0] = TextBox("F", Box(), &axis3dTextLook);
    axis3dLabels[1] = TextBox("B", Box(), &axis3dTextLook);
    axis3dLabels[2] = TextBox("L", Box(), &axis3dTextLook);
    axis3dLabels[3] = TextBox("R", Box(), &axis3dTextLook);
    axis3dLabels[4] = TextBox("U", Box(), &axis3dTextLook);
    axis3dLabels[5] = TextBox("D", Box(), &axis3dTextLook);
    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
    //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
    
    //openGLContext.setMultisamplingEnabled(true); // antialiasing, this works, but looks kinda shit...
    // improves performance since not using Component::paint()
    openGLContext.setComponentPaintingEnabled(false);
    // make this class the one with the OpenGL rendering callbacks
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    
    // start 30fps timer for OpenGL rendering
    startTimer(0, 1000.0f / glWindow.frameRate);
    
    // tells this class it wants to be able to receive key events
    setWantsKeyboardFocus(true);
    
    // chooses whether a click on this component automatically grabs the focus
    setMouseClickGrabsKeyboardFocus(true);
    
    // add the resizer stuff and set its limits (much smaller than 400x400 and host window minimum size is exceeded and potentially causes crashes, ahhem AULab...)
    addAndMakeVisible (resizerCorner = new ResizableCornerComponent (this, &resizeLimits));
    resizeLimits.setSizeLimits (550, 500, 5000, 5000);
    //addAndMakeVisible (resizerCorner2 = new ResizableCornerComponent (this, &resizeLimits));
    //addAndMakeVisible (resizerBorder = new ResizableBorderComponent (this, &resizeLimits));
    resizerCorner->setAlwaysOnTop(true);
    //resizerBorder->setAlwaysOnTop(true);
    
    // set the plugin's editor window's inital size
    setSize (processor->lastUIWidth, processor->lastUIHeight);
    
//    etbLook.fontName = "Copperplate Gothic Light";
//    etbLook.multiLine = true;
//    //etbLook.wrap = false;
//    //etbLook.linePad = 0.7f;
//    etbLook.fontSize = 20;
//    etbLook.just = Justification::centred;
//    etb.setLook(&etbLook);
//    //etb.setTextInputRestrictor(std::make_unique<DecimalNumberRestrictor>(-10.032, 1001, 4));
    
    dopplerSliderTextLook.color = Colours::red;
    dopplerSliderTextLook.fontSize = 16;
    dopplerSlider.setTextInputRestrictor(std::make_unique<DecimalNumberRestrictor>(processor->minSpeedOfSound, processor->maxSpeedOfSound, 2, " m/s"));
    dopplerSlider.setInterpolator(std::make_unique<LogarithmicInterpolator<float>>(20.0f));
    //dopplerSlider.setUnits("m/s");
    dopplerSlider.setValue(processor->speedOfSound);
   
    websiteButton.showsState = false;
    websiteButton.drawBoundary = false;
//    websiteMessageLook.multiLine = true;
//    websiteMessage.setLook(&websiteMessageLook);
    //websiteMessage.setDrawMultiLine(true);
    
    TextLook dopplerAndHelpTextLook;
    dopplerAndHelpTextLook.fontSize = 16;
    dopplerAndHelpTextLook.horizontalPad = 0.9f;
    dopplerButton.setTextLook(dopplerAndHelpTextLook);
    dopplerButton.setColor(Colours::red);
    if (processor->dopplerOn)
        dopplerButton.press();
    helpButton.setTextLook(dopplerAndHelpTextLook);
    helpButton.setColor(popsicleGreen);
    //helpButton.color = popsicleGreen;
    //helpButton.textBox.setColor(popsicleGreen);
    //PointLook pointLook;
    pointLook.radius = 3;
    pointLook.mouseOverRadius = pointLook.radius * 1.2f;
    pointLook.color = Colour::fromFloatRGBA(0, 0, 1, 1);
    pointLook.selectedColor = Colour::fromFloatRGBA(1, 0, 0, 1);
    pointLook.animationDuration = 0.2f;
    //mousePositionLook.fontName = "Good Times";
    //mousePositionLook.fontSize = 12;
    //positionerLook.just = Justification::centredLeft;
    positionerLook.fontSize = 16;
    positionerLook.horizontalPad = 1;
    positionerText.setLook(positionerLook);
    positionerText.setTextInputRestrictor(std::make_unique<PathAutomationInputRestrictor>());
    positionerText.extendToFitSameSizeFont = true;
    positionerText.doubleClickSeparator = ", ";
//    processingModeHelp.setColor(popsicleGreen);
//    processingModeHelp.setDrawMultiLine(true);
//    processingModeHelp.setJustification(Justification::centredLeft);

    automationViewLowerTimeTextLook.just = Justification::centredLeft;
    automationViewLowerTimeTextLook.horizontalPad = 1;
    automationViewLowerTimeTextLook.fontSize = 18;
    automationViewLowerTimeText.setLook(automationViewLowerTimeTextLook);
    
    automationViewUpperTimeTextLook.just = Justification::centredRight;
    automationViewUpperTimeTextLook.horizontalPad = 1;
    automationViewUpperTimeTextLook.fontSize = 18;
    automationViewUpperTimeText.setLook(automationViewUpperTimeTextLook);
    
    automationViewTimeTextLook.fontSize = 18;
    automationViewTimeTextLook.color = Colour::fromFloatRGBA(0, 1, 0, 1);
    automationViewTimeText.setLook(automationViewTimeTextLook);
    
    sourcePositionerTextLook.color = Colour::fromFloatRGBA(0, 1, 0, 1);
    sourcePositionerTextLook.fontSize = 16;
    sourcePathPointPositionerTextLook.color = Colour::fromFloatRGBA(1, 1, 1, 1);
    sourcePathPointPositionerTextLook.fontSize = 16;
    
    pathIndexTextLook.fontSize = 16;
    //pathIndexTextLook.fontStyle = Font::bold;
    pathIndexTextLook.horizontalPad = 1;
    pathIndexSourceSelectAnimationLook = pathIndexSourceDeselectAnimationLook = pathIndexSelectedTextLook = pathIndexTextLook;
//    pathIndexSourceSelectAnimationLook.fontSize = 15;
//    pathIndexSourceDeselectAnimationLook.fontSize = 15;
//    pathIndexSelectedTextLook.fontSize = 15;
    pathIndexSelectedTextLook.color = Colour::fromFloatRGBA(0, 1, 0, 1);
    
    resizePathPtsPrevState(); // gotta make space for this otherwise CRASH!!!
    
    volumeSliderTextLook.fontSize = 16;
    volumeSlider.setup(Box(), TextBox("Volume", Box(), &volumeSliderTextLook),
                       EditableTextBox(TextBox("", Box(), &volumeSliderTextLook), &glWindow),
                       volumeSliderTextLook.color);
    volumeSlider.setTextInputRestrictor(std::make_unique<DecimalNumberRestrictor>(0.0f, 10.0f, 2, "x"));
    volumeSlider.setInterpolator(std::make_unique<LogarithmicInterpolator<float>>(100.0f));
    volumeSlider.setValue(processor->wetOutputVolume + processor->dryOutputVolume);
    
    mixSliderTextLook.fontSize = 16;
    mixSlider.setup(Box(), TextBox("Mix", Box(), &mixSliderTextLook),
                    EditableTextBox(TextBox("", Box(), &mixSliderTextLook), &glWindow),
                    mixSliderTextLook.color);
    mixSlider.setTextInputRestrictor(std::make_unique<DecimalNumberRestrictor>(0.0f, 100.0f, 0, "%"));
    mixSlider.setValue(100.0f * processor->wetOutputVolume / volumeSlider.getValue());

    //setOpaque(true);
}

ThreeDAudioProcessorEditor::~ThreeDAudioProcessorEditor()
{
//#ifdef WIN32
//    glutExit();
//#endif
    stopTimer(0); // need in order to fix crash caused by timer when gl context/window is closed
    stopTimer(1);
    openGLContext.detach();
    // *** update view state stuff to the plugin instance ***
    if (processor != nullptr) {
        processor->upDir = upDir;
        processor->eyePos[0] = eyePos[0];
        processor->eyePos[1] = eyePos[1];
        processor->eyePos[2] = eyePos[2];
        processor->eyeUp[0] = eyeUp[0];
        processor->eyeUp[1] = eyeUp[1];
        processor->eyeUp[2] = eyeUp[2];
        processor->eyeRad = eyeRad;
        processor->eyeAzi = eyeAzi;
        processor->eyeEle = eyeEle;
        processor->automationViewWidth = pathAutomationView.getWidth();
        processor->automationViewOffset = pathAutomationView.getXPosition();
//        processor->automationViewWidth = automationViewWidth;
//        processor->automationViewOffset = automationViewOffset;
        //processor->displayState = displayState;
        //processor->showHelp = showHelp;
    }
    // ******************************************************
}

void ThreeDAudioProcessorEditor::resized()
{
    // get the new window width and height
    const int w = getWidth();
    const int h = getHeight();
    
    windowAspectRatio = static_cast<float>(h) / w;
    
    // NOTE: there is a leak somewhere in JUCE when the window is resized
    {   // need to get a lock for the message manager here to make the (component).setBounds() call thread-safe
        const MessageManagerLock mmLock;
        // do the resizing of the corner resizer component
        resizerCorner->setBounds(w - 15, h - 15, 15, 15);
        //resizerCorner2->setBounds(0, h - 15, 15, 15);
        //resizerBorder->setBounds(this->getBounds()/* 0, 0, w, h*/);
    }   // mmLock goes out of scope here and is released
    
    // update processor with new width and height
    processor->lastUIWidth = w;
    processor->lastUIHeight = h;
}

////==============================================================================
//void ThreeDAudioProcessorEditor::paint (Graphics& g)
//{
////    g.fillAll(Colours::white);
////    g.setColour(Colours::black);
////    //Graphics g (image);
////    //g.setColour(color);
////    const int w = getWidth()/100.0;
////    const int h = getHeight()/100.0;
////    int x = 0, y = 0;
////    //Font font (fontName, cellHeight, fontStyle);
////    Font font = g.getCurrentFont();
////    const String str ("billy, bob, joe!!!");
////    float len = 0;
////    for (int i = 0; i < 100; ++i) {
////        len += font.getStringWidthFloat(str);
//////        g.drawText(str, 10, y, 100, 40, Justification::centredLeft);
//////        y += 10;
//////        g.drawEllipse(x, y, w, h, 1);
//////        x += w;
//////        y += h;
////    }
//    
////    // dimensions of window
////    const int w = getWidth();
////    const int h = getHeight();
////
//////    if (processor->dopplerOn && displayState == MAIN)
//////    {
//////        String dopplerText ("Doppler On");
//////        g.setColour(Colours::red);
//////        int textBoarderX = std::ceil(0.4*w);
//////        int textBoarderY = std::ceil(0.04*h);
//////        int maximumNumberOfLines = 1;
//////        int fontSize = std::floor( std::min(textBoarderY*1.0, 2.5*(w-2.0*textBoarderX)/dopplerText.length()) );
//////        g.setFont(fontSize);
//////        float minimumHorizontalScale = 0.7f; // allow a bit of text compression so no "..." shows up instead of text
//////        const Rectangle<int> area (textBoarderX, h-2.5*textBoarderY, // x/y postion of top left corner
//////                                   w - 2*textBoarderX, fontSize);    // width/height of rectangle (height set to font size)
//////        g.drawFittedText (dopplerText,
//////                          area,
//////                          Justification::centredTop,
//////                          maximumNumberOfLines,
//////                          minimumHorizontalScale);
//////    }
////    
////    // only show the help text if that is the current GUI state
////    if (showHelp && processor->displayState != DisplayState::SETTINGS)
////    {
////        g.fillAll(Colour::fromFloatRGBA(0.1, 0.1, 0.3, 0.3));
////        
////        // how much empty space around text is there
////        int textBoarderX = std::ceil(0.05*w);
////        int textBoarderY = std::ceil(0.05*h);
////        
////        // other stuff for position of each text line in a rectangle area
////        Justification justificationFlags = Justification::centred; // horizontally centered text in area rectangle
////        int maximumNumberOfLines = 1; // one line limit per line of text (no wrapping to another line)
////        float minimumHorizontalScale = 0.7f; // allow a bit of text compression so no "..." shows up instead of text
////        
////        // white as snow
////        g.setColour(Colours::white);
////        //g.setOpacity(0.95); // well maybe not quite...
////        
////        // the help text to draw depending on the view state
////        /*const*/ String** text = nullptr;
////        int numLines = 0; // how many lines of text?
////        switch (processor->displayState) {
////            case DisplayState::MAIN:
////                numLines = sizeof(mainHelp)/sizeof(String);
////                text = new /*const*/ String*[numLines];
////                for (int i = 0; i < numLines; ++i)
////                    text[i] = (String*) & mainHelp[i];
////                break;
////            case DisplayState::PATH_AUTOMATION:
////                numLines = sizeof(automationHelp)/sizeof(String);
////                text = new /*const*/ String*[numLines];
////                for (int i = 0; i < numLines; ++i)
////                    text[i] = (String*) & automationHelp[i];
////                break;
////        }
////        
////        // find the longest text string in the text block so the font size can be chosen to fit the window nicely
////        int maxTextLength = 0;
////        for (int i = 0; i < numLines; ++i)
////        {
////            if (text[i]->length() > maxTextLength)
////                maxTextLength = text[i]->length();
////        }
////        
////        // choose the font size based on what will fit the minimum window dimension (text width is about 2.5x as narrow as text height for the font of a given size)
////        int fontSize = std::floor( std::min((h-2.0*textBoarderY)/numLines, 2.5*(w-2.0*textBoarderX)/maxTextLength) );//20;
////        g.setFont (fontSize);
////        
////        // displacement so text block is vertically centered
////        int roomY = std::floor( (h - 2*textBoarderY - fontSize*numLines)/2.0 );
////        
////        // draw the text (each area rectangle defines the position/bounds of each line of text)
////        for (int i = 0; i < numLines; ++i)
////        {
////            const juce::Rectangle<int> area (textBoarderX, textBoarderY + i*fontSize + roomY, // x/y postion of top left corner
////                                             w - 2*textBoarderX, fontSize);                   // width/height of rectangle (height set to font size)
////            g.drawFittedText (*text[i],
////                              area,
////                              justificationFlags,
////                              maximumNumberOfLines,
////                              minimumHorizontalScale);
////        }
////        
////        // has crashed here before... when text was const
////        delete[] text;
////    }
//}

void ThreeDAudioProcessorEditor::drawHelp()
{
    if (helpButton.isDown() || helpButton.getPressAnimation().isPlaying()) {
        // darken background
        const float alpha = helpButton.getPressAnimation().getProgress();
        const float factor = helpButton.isDown() ? alpha : std::max(1 - 2*alpha, 0.0f);
        glColor4f(0.0, 0.0, 0.0, 0.5*factor);
        glBegin(GL_QUADS);
        glVertex2f(-1.0,  1.0);
        glVertex2f( 1.0,  1.0);
        glVertex2f( 1.0, -1.0);
        glVertex2f(-1.0, -1.0);
        glEnd();
        if (factor != 1) {
            helpTextLook.color = helpTextLook.color.withAlpha(factor);
            helpText.setLook(&helpTextLook, false); // not necessary to resize font if just changing color
        }
        helpText.draw(glWindow);
    }
    // if nothing is picked up or mouse is not dragging
    helpButton.mouseOverEnabled =  !selectionBox.isActive() /*mouseDragging*/
                                && !loopRegionBeginSelected
                                && !loopRegionEndSelected
                                && !pathAutomationPointsGrabbedWithMouse;
    helpButton.draw(glWindow, {getMouseX(), getMouseY()});
}

void ThreeDAudioProcessorEditor::loadHelpText()
{
    //const Array<String> * whichText = nullptr;
    //const std::vector<std::string> * whichText = nullptr;
    switch (processor->displayState) {
        case DisplayState::MAIN:
            //whichText = &mainHelp;
            helpText = TextBoxGroup(mainHelp, mainHelp.size(), helpText.getBoundary(), &helpTextLook);
            //helpText = GLTextBoxGroup(mainHelp, std::ceil(mainHelp.size()/2.0), helpText.getBoundary(), &helpTextLook);
            //helpText.setTexts(mainHelp, mainHelp.size(), 1);
            break;
        case DisplayState::PATH_AUTOMATION:
            //whichText = &automationHelp;
            helpText = TextBoxGroup(automationHelp, automationHelp.size(), helpText.getBoundary(), &helpTextLook);
            //helpText = GLTextBoxGroup(automationHelp, automationHelp.size(), helpText.getBoundary(), &helpTextLook);
            //helpText.setTexts(automationHelp, automationHelp.size(), 1);
            break;
        case DisplayState::SETTINGS:
            break;
        case DisplayState::NUM_DISPLAY_STATES:
            break;
    }
//    if (whichText != nullptr) {
////        const float top = 0.95, bottom = -0.95, left = -1, right = 1;
//        helpText.setTexts(mainHelp, mainHelp.size(), 1);
////        helpText = TextBoxGroup(*whichText, Box{top, bottom, left, right}, whichText->size(), 1);
////        helpText = makeCached<TextBoxGroup>(*whichText, Box{top, bottom, left, right}, whichText->size(), 1);
//    }
}

void ThreeDAudioProcessorEditor::newOpenGLContextCreated()
{
    /** This method is called when the component creates a new OpenGL context.
     
     A new context may be created when the component is first used, or when it
     is moved to a different window, or when the window is hidden and re-shown,
     etc.
     
     You can use this callback as an opportunity to set up things like textures
     that your context needs.
     
     New contexts are created on-demand by the makeCurrentContextActive() method - so
     if the context is deleted, e.g. by changing the pixel format or window, no context
     will be created until the next call to makeCurrentContextActive(), which will
     synchronously create one and call this method. This means that if you're using
     a non-GUI thread for rendering, you can make sure this method is be called by
     your renderer thread.
     
     When this callback happens, the context will already have been made current
     using the makeCurrentContextActive() method, so there's no need to call it
     again in your code.
     */
    
    // I'm assuming GL stetup stuff should go here ...
    /*         ... and some guy thinks so too:
     onSurfaceChanged:
     this means you have a new GL context so it's a fresh state machine
     ready to be configured.
     set viewport and enable and configure things that will always be used,
     like texturing, texture coordinate arrays, vert arrays, erase color,
     etc
     you can set your projection matrix here if it's going to be fixed
     load textures, vbos and anything else into vram */
    
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_SMOOTH);
    
    // materials for lighting
    GLfloat mat_specular[] = { 1.0, 0.1, 1.0, 1.0 };
    GLfloat mat_shininess[] = { 50.0 };
    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    
    // make sure all source lights (0-6) will be visible
    GLfloat source_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat source_diffuse[] = { 0.5, 0.4, 1.0, 1.0 };
    GLfloat source_specular[] = { 0.0, 1.0, 0.0, 1.0 };
    for (int i = 0; i < 7; i++) {
        glLightfv(GL_LIGHT0+i, GL_AMBIENT, source_ambient);
        glLightfv(GL_LIGHT0+i, GL_DIFFUSE, source_diffuse);
        glLightfv(GL_LIGHT0+i, GL_SPECULAR, source_specular);
    }
    
    // ambient light
    GLfloat light_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
    glLightfv(GL_LIGHT7, GL_AMBIENT, light_ambient);
    
    //glEnable(GL_LINE_SMOOTH); // enable antialiasing
    
    //glEnable(GL_DEPTH_TEST); //Make sure 3D drawing works when one object is in front of another
    //glDepthFunc (GL_LEQUAL);
    
    // Windows no like
    //glutInitDisplayMode (GLUT_DOUBLE); // Set up a basic display buffer (for double buffering)
    
    // now the GL stuff is ready to go
    glInited = true;
    
    // do setup of GL_PROJECTION matrix for the view
    //resized(); no longer needed
}

void ThreeDAudioProcessorEditor::openGLContextClosing()
{
    // will need to reinitialize the GL if the gl context closes
    glInited = false;
}

//// Replaces gluPerspective. Sets the frustum to perspective mode.
//// fovY     - Field of vision in degrees in the y direction
//// aspect   - Aspect ratio of the viewport
//// zNear    - The near clipping distance
//// zFar     - The far clipping distance
//void ThreeDAudioProcessorEditor::perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
//{
//    const GLdouble pi = 3.1415926535897932384626433832795;
//    GLdouble fW, fH;
//    
//    fH = tan( fovY / 360 * pi ) * zNear;
//    fW = fH * aspect;
//    
//    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
//}

void ThreeDAudioProcessorEditor::drawArray(float* array, int L)
{
    double maxMag = 0;
//    double minPow = 0;
    double x_scale = 0.9;
    double y_scale = 0.9;
//    double freqDataPower[L];

    // find max magnitude of all freq bins for scaling
    for (int i = 0; i < L; i++) {
        if (array[i] > maxMag) {
            maxMag = array[i];
        }
    }

//    // calc freq bin's relative power to max power in dB + find minimum power
//    for (int i = 0; i < L; i++) {
//        freqDataPower[i] = 20*log10(array[i]/maxMag);
//        if (freqDataPower[i] < minPow) {
//            minPow = freqDataPower[i];
//        }
//    }

    // draw the freq bins (every other)
    glBegin(GL_LINES);
    for (int i = 0; i < L; i += 2) {

        double x = x_scale*(-1.0 + 2.0*i/L);
        double y1, y2 = 0.0;
        double z = 0.0;

//        glColor3f(1.0, 0.0, 0.2);

        // lower y
        //y1 = y_scale * -1.0;
        y1 = 0;

        // upper y
        //y2 = y_scale * (-1.0 + 2.0*(array[i]/maxMag));
        y2 = y_scale * array[i]/maxMag;

        glVertex3f(x, y1, z);
        glVertex3f(x, y2, z);
    }
    glEnd();
}

void ThreeDAudioProcessorEditor::drawSelectableOrb(const PointXYZ<float>& pos,
                                                   const float normalRadius,
                                                   const int numSlices,
                                                   const int numStacks,
                                                   const Colour normalColor,
                                                   const Colour mouseOverColor,
                                                   const bool mouseOver,
                                                   bool& prevMouseOver,
                                                   Multi<Animation>& mouseOverAnimations,
                                                   const bool selected,
                                                   bool& prevSelected,
                                                   Multi<Animation>& selectAnimations,
                                                   const int orbID,
                                                   const float alpha,
                                                   const bool haloEnabled,
                                                   const bool positionLinesEnabled)
{
    if (prevMouseOver != mouseOver)
        mouseOverAnimations.add(orbID, mouseOver ? 0.2f : 0.25f, true);
    prevMouseOver = mouseOver;
    
    if (prevSelected != selected)
        selectAnimations.add(orbID, selected ? 0.2f : 0.25f, true);
    prevSelected = selected;
    
    Colour color;
    float radius;
    cauto mouseOverRadius = normalRadius * 1.25f;
    
    auto mouseOverAnimation = mouseOverAnimations.get(orbID);
    if (mouseOverAnimation) {
        if (mouseOver) {
            color = normalColor.interpolatedWith(mouseOverColor, mouseOverAnimation->getProgress());
            radius = normalRadius + (mouseOverRadius - normalRadius) * mouseOverAnimation->getProgress();
        } else {
            color = mouseOverColor.interpolatedWith(normalColor, mouseOverAnimation->getProgress());
            radius = mouseOverRadius - (mouseOverRadius - normalRadius) * mouseOverAnimation->getProgress();
        }
    } else {
        if (mouseOver) {
            color = mouseOverColor;
            radius = mouseOverRadius;
        } else {
            color = normalColor;
            radius = normalRadius;
        }
    }
    
    auto selectAnimation = selectAnimations.get(orbID);
    if (selectAnimation) {
        if (selected)
            color = mouseOverColor;
        else
            color = mouseOverColor.interpolatedWith(normalColor, selectAnimation->getProgress());
    } else if (selected)
        color = mouseOverColor;

    glColour(color.withAlpha(alpha));    
    cauto sphere = glpp::SolidSphere(radius, numSlices, numStacks);
    sphere.draw(pos.x, pos.y, pos.z);
    if (haloEnabled && (selected || selectAnimation)) {
        cauto sourceSelectHaloColor = mouseOverColor.withAlpha(0.5f);
        if (selectAnimation) {
            cauto prog = std::pow(selectAnimation->getProgress(), 0.5f);
            if (selected) {
                radius *= (2.75f - 1.75f * prog);
                glColour(sourceSelectHaloColor.withMultipliedAlpha(prog));
            } else {
                radius *= (1.25f + 1.75f * prog);
                glColour(sourceSelectHaloColor.withMultipliedAlpha(1 - prog));
            }
        } else {
            radius *= 1.35f;
            glColour(sourceSelectHaloColor);
        }
    
        cauto sphere = glpp::SolidSphere(radius, numSlices, 50/*numStacks*/);
        sphere.draw(pos.x, pos.y, pos.z);
    }

    // lines for visualizing position of source
    if (positionLinesEnabled && (/*mouseOver ||*/ selected || /*mouseOverAnimation ||*/ selectAnimation)) {
        auto prog = 1.0f;
        if (selectAnimation) {
            if (selected)
                prog = selectAnimation->getProgress();
                //glColour(color.withMultipliedAlpha(selectAnimation->getProgress()));
            else
                prog = 1 - selectAnimation->getProgress();
                //glColour(color.withMultipliedAlpha(1 - selectAnimation->getProgress()));
        }
//        else if (selected) {
//            //prog = 1.0f;
//            //glColour(color.withAlpha(alpha));
//        } else if (mouseOverAnimation) {
//            if (mouseOver)
//                prog = mouseOverAnimation->getProgress();
//                //glColour(color.withMultipliedAlpha(mouseOverAnimation->getProgress()));
//            else
//                prog = 1 - mouseOverAnimation->getProgress();
//                //glColour(color.withMultipliedAlpha(1 - mouseOverAnimation->getProgress()));
//        }

        //glColour(color.withAlpha(alpha * prog));
        cauto midColor = mouseOverColor.withAlpha(alpha * prog * prog);
        cauto edgeColor = midColor.withMultipliedAlpha(prog);
        const PointXYZ<float> midAdj = {0.5f * pos.x,         0.0f, 0.5f * pos.z};
        const PointXYZ<float> midOpp = {       pos.x, 0.5f * pos.y,        pos.z};
        const PointXYZ<float> midHyp = {0.5f * pos.x, 0.5f * pos.y, 0.5f * pos.z};
        glDisable(GL_DEPTH_TEST); // semi transparent GL_LINES don't play so well with this
        glBegin(GL_LINES);
        
        glColour(edgeColor);
        glVertex(midAdj - (prog * midAdj));
        glColour(midColor);
        glVertex(midAdj);
        glVertex(midAdj);
        glColour(edgeColor);
        glVertex(midAdj + (prog * midAdj));
        
        glVertex(midOpp.subY(prog * 0.5f * pos.y));
        glColour(midColor);
        glVertex(midOpp);
        glVertex(midOpp);
        glColour(edgeColor);
        glVertex(midOpp.addY(prog * 0.5f * pos.y));
        
        glVertex(midHyp - (prog * midHyp));
        glColour(midColor);
        glVertex(midHyp);
        glVertex(midHyp);
        glColour(edgeColor);
        glVertex(midHyp + (prog * midHyp));
        glEnd();
//        glBegin(GL_LINES);
//        glVertex(midAdj - (prog * midAdj));
//        glVertex(midAdj + (prog * midAdj));
//        glVertex(midOpp.subY(prog * 0.5f * pos.y));
//        glVertex(midOpp.addY(prog * 0.5f * pos.y));
//        glVertex(midHyp - (prog * midHyp));
//        glVertex(midHyp + (prog * midHyp));
////        glVertex3f(0, 0, 0);
////        glVertex(pos);
////        glVertex3f(pos.x, 0, pos.z);
////        glVertex(pos);
////        glVertex3f(pos.x, 0, pos.z);
////        glVertex3f(0, 0, 0);
//        glEnd();
        glEnable(GL_DEPTH_TEST);
    }
    if (mouseOverAnimation) {
        mouseOverAnimation->advance(glWindow.frameRate);
        if (!mouseOverAnimation->isPlaying())
            mouseOverAnimations.remove(orbID);
    }
    if (selectAnimation) {
        selectAnimation->advance(glWindow.frameRate);
        if (!selectAnimation->isPlaying())
            selectAnimations.remove(orbID);
    }
}

void ThreeDAudioProcessorEditor::drawHead() const
{
    cfloat radius = 0.09f; // 9cm radius for head
    cint numSlices = 30;
    cint numStacks = 15;
    glpp::SolidSphere sphere (radius, numSlices, numStacks);
    sphere.draw(0, 0, 0);
}

void ThreeDAudioProcessorEditor::drawMain()
{
    int numSlices = 24;
    int numStacks = 12;
    //float radius = 0;
    cauto sourceRadius = 0.035f;
    cauto pathPtRadius = 0.03f;
//    GLUquadricObj* pQuadric = gluNewQuadric();
    
    if (tabs.isMouseOver() // disable mouseOver for the sources if other stuff "in front" is hovered over
        || dopplerSlider.getMouseOver()
        || helpButton.isMouseOver()
        || dopplerButton.isMouseOver()
        || (positionerText3DID.sourceIndex != -1 && positionerText3D.getBoundary().contains(getMousePosition()))
        || (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1
            && pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex]->getBoundary().contains(getMousePosition()))) {
        mouseOverSourceIndex = -1;
        mouseOverPathPointSourceIndex = -1;
        mouseOverPathPointIndex = -1;
    } else {
        // ******** setup for entering selection mode ********
        const float w = getWidth();
        const float h = getHeight();
        GLint mouse_x = getMouseXYRelative().getX();
        GLint mouse_y = getMouseXYRelative().getY();
        GLint viewport[4];
        
        glSelectBuffer(SELECT_BUF_SIZE, objSelectBuf);
        glRenderMode(GL_SELECT);
        
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        
        glGetIntegerv(GL_VIEWPORT, viewport);
        glpp::pickMatrix(mouse_x, viewport[3]-mouse_y, 5, 5, viewport);
        glpp::perspective(45.0f, w/h, 0.1f, 200.0f);
        glMatrixMode(GL_MODELVIEW);
        
        glLoadIdentity(); // reset the drawing perspective
        /* viewing transformation  */
        glpp::lookAt ({eyePos[0], eyePos[1], eyePos[2]},   // eye pos (x,y,z)
                      {     0.0f,      0.0f,      0.0f},   // direction looking at (x,y,z)
                      { eyeUp[0],  eyeUp[1],  eyeUp[2]});  // camera up vector (x,y,z)
        
        // start drawing selectable objects
        glInitNames();
        
        // draw the source(s)
        int k = 0;
        numSlices = 12;
        numStacks = 8;
        cauto sourceSphere = glpp::SolidSphere(sourceRadius, numSlices, numStacks);
        cauto pathPtSphere = glpp::SolidSphere(pathPtRadius, numSlices, numStacks);
        for (int i = 0; i < sources->size(); ++i) {
            glPushName(SOURCE + i);
            cauto pos = (*sources)[i].getPosXYZ();
            sourceSphere.draw(pos[0], pos[1], pos[2]);
//            glPushMatrix();
//            glTranslatef(pos[0], pos[1], pos[2]);
//            gluSphere(pQuadric, sourceRadius, numSlices, numStacks);
//            glPopMatrix();
            glPopName();
            if ((*sources)[i].getSourceSelected()) {
                // allow hit detection on path points if source is selected
                cauto pts = (*sources)[i].getPathPoints();
                for (int j = 0; j < pts.size(); ++j) {
                    glPushName(SOURCE_PTS + k);
                    pathPtSphere.draw(pts[j][0], pts[j][1], pts[j][2]);
//                    glPushMatrix();
//                    glTranslatef(pts[j][0], pts[j][1], pts[j][2]);
//                    gluSphere(pQuadric, pathPtRadius, numSlices, numStacks);
//                    glPopMatrix();
                    glPopName();
                    ++k;
                }
            }
        }
        
        // exit selection mode and process hits
        int hits;
        // restoring the original projection matrix
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glFlush();
        
        // returning to normal rendering mode
        hits = glRenderMode(GL_RENDER);
        
        // if there are hits process them
        if (hits != 0) {
            unsigned int i, j;
            GLuint names, *ptr, minZ, *ptrNames, numberOfNames;
            ptr = (GLuint*) objSelectBuf;
            minZ = 0xffffffff;
            for (i = 0; i < hits; ++i) {
                names = *ptr;
                ++ptr;
                if (*ptr < minZ) {
                    numberOfNames = names;
                    minZ = *ptr;
                    ptrNames = ptr+2;
                }
                ptr += names+2;
            }
            ptr = ptrNames;
            for (j = 0; j < numberOfNames; ++j, ++ptr) {
                if (*ptr >= SOURCE && *ptr < SOURCE+sources->size()) {
                    mouseOverSourceIndex = *ptr;
                }
                if (*ptr >= SOURCE_PTS && *ptr < SOURCE_PTS+k) {
                    int ptsIndex = *ptr - SOURCE_PTS;
                    for (int s = 0; s < sources->size(); ++s) {
                        if ((*sources)[s].getSourceSelected()) {
                            int numPts = (*sources)[s].getNumPathPoints();
                            if (ptsIndex >= numPts) {
                                ptsIndex -= numPts;
                            } else {
                                mouseOverPathPointSourceIndex = s;
                                mouseOverPathPointIndex = ptsIndex;
                                // break out of for each source loop
                                s = sources->size();
                            }
                        }
                    }
                }
            }
        }
        // Hit detection stuff above
        // *****************************************
    }
    
    if (volumeSlider.getMouseOver() || mixSlider.getMouseOver()) {
        mouseOverSourceIndex = -1;
        mouseOverPathPointSourceIndex = -1;
        mouseOverPathPointIndex = -1;
    }
    
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
    glLoadIdentity(); //Reset the drawing perspective
    /* viewing transformation  */
    glpp::lookAt ({eyePos[0], eyePos[1], eyePos[2]},  // eye pos (x,y,z)
                  {0.0f, 0.0f, 0.0f},                // direction looking at (x,y,z)
                  {eyeUp[0], eyeUp[1], eyeUp[2]});  // camera up vector (x,y,z)
//    gluLookAt (eyePos[0], eyePos[1], eyePos[2],  // eye pos (x,y,z)
//               0.0, 0.0, 0.0,                   // direction looking at (x,y,z)
//               eyeUp[0], eyeUp[1], eyeUp[2]);  // camera up vector (x,y,z)
    /* Remove hidden surfaces */
    glEnable(GL_DEPTH_TEST);  // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    // antialiasing on for 3d stuff
//    glEnable(GL_POLYGON_SMOOTH); // these don't work
//    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    
    // light for source(s)
    glEnable(GL_LIGHTING);
    int numLights = sources->size();
    if (numLights > 7)
        numLights = 7;
    
    for (int i = 0; i < 7; i++) {
        if (i < numLights && !(*sources)[i].getSourceMuted()) {
            cauto pos = (*sources)[i].getPosXYZ();
            float glpos[4];
            glpos[0] = pos[0];
            glpos[1] = pos[1];
            glpos[2] = pos[2];
            glpos[3] = 1; // must be nonzero to be a positional light sources as opposed to a directional one
            glLightfv(GL_LIGHT0+i, GL_POSITION, glpos);
            glEnable(GL_LIGHT0+i);
        } else {
            glDisable(GL_LIGHT0+i);
        }
    }
    
    // the ambient light
    glEnable(GL_LIGHT7);
    
    // draw head
    drawHead();
    
    glDisable(GL_LIGHTING);
    
    // draw the source(s)
    numSlices = 24;
    numStacks = 12;
    for (int s = 0; s < sources->size(); ++s) {
        cauto pos = (*sources)[s].getPosXYZ();
        cauto normalColor = Colour::fromFloatRGBA(1, 1, 1, 1);//Colour::fromRGBA(255, 209, 112, 255);
        cauto mouseOverColor = Colour::fromFloatRGBA(0, 1, 0, 1);
        cauto mouseOverSource = s == mouseOverSourceIndex || pointInsideSelectRegion({pos[0], pos[1], pos[2]});
        cauto sourceSelected = (*sources)[s].getSourceSelected();
        cauto alpha = (*sources)[s].getSourceMuted() ? 0.5f : 1.0f;
        cauto prevSelectSourceAnimation = selectSourceAnimations.get(s);
        drawSelectableOrb({pos[0], pos[1], pos[2]}, sourceRadius, numSlices, numStacks, normalColor, mouseOverColor,
                          mouseOverSource, prevMouseOverSources[s], mouseOverSourceAnimations,
                          sourceSelected, prevSelectedSources[s], selectSourceAnimations, s, alpha);
        cauto mouseOverSourceAnimation = mouseOverSourceAnimations.get(s);
        cauto selectSourceAnimation = selectSourceAnimations.get(s);
        cauto sourceUnselectedColor = Colour::fromFloatRGBA(0, 0, 1, 1);
        cauto sourceSelectedColor = Colour::fromFloatRGBA(1, 0, 0, 1);
        Colour pathPtColor;
        if (selectSourceAnimation)
            if (sourceSelected)
                pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, selectSourceAnimation->getProgress());
            else
                pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, 1 - selectSourceAnimation->getProgress());
        else if (sourceSelected)
            pathPtColor = sourceSelectedColor;
        else if (mouseOverSourceAnimation)
            if (mouseOverSource)
                pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, mouseOverSourceAnimation->getProgress());
            else
                pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, 1 - mouseOverSourceAnimation->getProgress());
        else if (mouseOverSource)
            pathPtColor = sourceSelectedColor;
        else
            pathPtColor = sourceUnselectedColor;
        cauto path = (*sources)[s].getPathPtr();
        if (processor->pathChanged || selectSourceAnimation || mouseOverSourceAnimation || prevSourceMuted[s] != (*sources)[s].getSourceMuted() || (prevSelectSourceAnimation && !selectSourceAnimation))
            pathDisplayList[s] = 0;
        prevSourceMuted[s] = (*sources)[s].getSourceMuted();
        InterpolatorLook pathLook (path, InterpolatorLook::THREE_D);
        pathLook.beginColor = pathLook.endColor = pathPtColor.withAlpha(alpha);
        pathLook.numVertices = path->getNumPoints() * 20;
        pathLook.lineType = InterpolatorLook::LineType::DASHED;
        draw(path, pathLook, pathDisplayList[s]);
        if (s == sources->size() - 1)
            processor->pathChanged = false;
        cauto points = (*sources)[s].getPathPoints();
        for (int j = 0; j < points.size(); ++j) {
            //cauto normalColor = Colour::fromFloatRGBA(0.7f, 0, 0, 1);
            cauto mouseOverColor = Colour::fromFloatRGBA(0.5f, 0, 1, 1);
            cauto mouseOver = (s == mouseOverPathPointSourceIndex && j == mouseOverPathPointIndex)
                || ((*sources)[s].getSourceSelected() && pointInsideSelectRegion({points[j][0], points[j][1], points[j][2]}));
            cauto selected = (*sources)[s].getPathPointSelected(j);
            cauto id = s * 1000 + j; // if there is more than 1000 path pts for a source, well fuck just make spacing bigger!
            bool prevMouseOver = prevMouseOverPathPts[s][j];
            bool prevSelected = prevSelectedPathPts[s][j];
            drawSelectableOrb({points[j][0], points[j][1], points[j][2]}, pathPtRadius, numSlices, numStacks, pathPtColor, mouseOverColor,
                              mouseOver, prevMouseOver, mouseOverPathPtAnimations,
                              selected, prevSelected, selectPathPtAnimations, id, alpha);
            prevMouseOverPathPts[s][j] = prevMouseOver;
            prevSelectedPathPts[s][j] = prevSelected;
        }
    }
    
    //gluDeleteQuadric(pQuadric);
    
    draw3DAxis();

    // antialiasing off for 2d stuff
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    
    // set up 2d projection for HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
    glOrtho(-1, 1, -1, 1, -1, 10);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    //glDisable(GL_CULL_FACE);
    
    // draw 2d stuff here ...
    
    // draw mouse drag boarders if mouse is being dragged to select a group of things
//    if (mouseDragging || mouseDragAnimation.isPlaying()) {
//        drawMouseDragging();
//    }
    draw(selectionBox, glWindow);
    //    // just testing azi stuff
    //    float raesource[3];
    //    sources[0]->getPosRAE(raesource);
    //    sprintf(str, "%f", raesource[1]);
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.3, 0.5, 0.0);
    
    // draw time stamp info from DAW
    //    float ppqPos = processor->posPPQ;
    //    float secPos = processor->posSEC;
    //    float bpm = processor->BPM;
    //    glColor3f(1.0, 0.0, 0.0);
    //	sprintf(str, "BPM: %f", bpm);
    //	drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -0.9, -0.7, 0.0);
    //    sprintf(str, "PPQ: %f", ppqPos);
    //	drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -0.9, -0.8, 0.0);
    //    sprintf(str, "SEC: %f", secPos);
    //	drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -0.9, -0.9, 0.0);
    
    //    float sourcePos[3];
    //    sources[0]->getPosRAE(sourcePos);
    //    sprintf(str, "SourceRad: %f", sourcePos[0]);
    //	drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, -0.9, 0.0);
    
    //showMouseDetails();
    
    //    sprintf(str, "fs: %f", processor->fs);
    //	drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, -0.9, 0.0);
    
    //    // testing NthDegPoly
    //    NthDegPoly p(3);
    //    float x1, y1, x2, y2;
    //    x1 = 0.5; y1 = 0.3;
    //    x2 = -0.8;  y2 = -0.1;
    //    p.addConstraint(y1, x1, 0);
    //    p.addConstraint(y2, x2, 0);
    //    p.addConstraint(-5, x1, 1);
    //    p.addConstraint(0, x2, 1);
    //    //p.addConstraint(1/2.3, 1, 1);
    //    p.calcPoly();
    //    glPushMatrix();
    //    glScalef(0.75,0.75,0.75);
    //    glBegin(GL_LINES);
    //    for (float x = -1.5; x < 1.5 ; x += 2.0/100) {
    //        glVertex2f(x, p.getValAt(x));
    //    }
    //    glEnd();
    //    drawCircle(x1, y1, 0.02, 10);
    //    drawCircle(x2, y2, 0.02, 10);
    //    glPopMatrix();
    
    //    // testing Interpolator
    //    std::vector<std::vector<float>> points ({{0.7,0.9},{-0.9,0.7},{-0.6,0.9},{-0.5,-0.5},{0.3,0.5},{0.4,0.5},{0.8,-0.6},{0.92,0}});
    //    functionalSort(points);
    //    std::unique_ptr<Interpolator<float>> interp = make_unique<ClosedParametricInterpolator<float>>(points);
    //    //std::unique_ptr<Interpolator<float>> interp = make_unique<FunctionalInterpolator<float>>(points);
    //    interp->setPointsSelected({1,2,3,4}, true);
    //    interp->setSelectedSplinesType(LINEAR);
    //    interp->setAllPointsSelected(false);
    //    interp->setPointsSelected({5,6}, true);
    //    interp->setSelectedSplinesType(EMPTY);
    //    interp->setAllPointsSelected(false);
    //    interp->setPointsSelected({1,3,5}, true);
    //    interp->copySelectedPoints();
    //    interp->moveSelectedPoints({0.4,-0.2});
    //    glPushMatrix();
    //    glScalef(0.75,1,1);
    //    glBegin(GL_LINES);
    //    std::vector<float> pt(2);
    //    for (float x = interp->getInputRange()[0]; x < interp->getInputRange()[1]; x += 0.01) {
    //        if (interp->pointAt(x, pt))
    //            glVertex2f(pt[0], pt[1]);
    ////        else
    ////            glVertex2f(x, 0);
    //    }
    //    glEnd();
    //    for (int i = 0; i < interp->getPoints().size(); ++i) {
    //        drawCircle(interp->getPoints()[i][0], interp->getPoints()[i][1], 0.02, 5);
    //    }
    //    glColor3f(1, 0, 0.1);
    //    for (int i = 0; i < interp->getSelectedPoints().size(); ++i) {
    //        drawCircle(interp->getSelectedPoints()[i][0], interp->getSelectedPoints()[i][1], 0.025, 5);
    //    }
    //    glPopMatrix();
    
    // draw source path pts indecies
    //char str[10];
    for (int s = 0; s < pathIndexTexts.size(); ++s) {
        cauto sourceSelected = (*sources)[s].getSourceSelected();
        cauto animation = selectSourceAnimations.get(s);
        if (sourceSelected || animation) {
            TextLook* look = nullptr;
            if (animation) {
                if (sourceSelected) {
                    look = &pathIndexSourceSelectAnimationLook;
                    look->color = pathIndexTextLook.color.withAlpha(animation->getProgress());
                    look->relativeScale = 2 - animation->getProgress();
                } else {
                    look = &pathIndexSourceDeselectAnimationLook;
                    look->color = pathIndexTextLook.color.withAlpha(1 - animation->getProgress());
                    look->relativeScale = 1 + animation->getProgress();
                }
            }
            for (int i = 0; i < pathIndexTexts[s].size(); ++i) {
//                if (positionerText3DID.sourceIndex == s && positionerText3DID.pathPtIndex == i && positionerText3D.getShowAnimation().isPlaying()) {
//                    auto b = pathIndexTexts[s][i].getBoundary();
//                    cauto xyz = (*sources)[s].getPathPtr()->getPoint(i);
//                    cauto dxy = getDeltaXYFromCenterToTopRight({xyz[0], xyz[1], xyz[2]}, pathPtRadius) * (1 / glWindow.frameRate) / positionerText3D.getShowAnimation().getDuration();
//                    //cauto y = pixelsToNormalized(10, glWindow.height) * (1 / glWindow.frameRate) / positionerText3D.getShowAnimation().getDuration();
//                    b.move(0, dxy[1]);
//                    pathIndexTexts[s][i].setBoundary(b);
//                }
                if (look) {
                    pathIndexTexts[s][i]->setLook(look);
                } else if (pathIndexTexts[s][i]->getLook() != &pathIndexTextLook) {
                    pathIndexTexts[s][i]->setLook(&pathIndexTextLook);
                }
                if (positionerText3DID.sourceIndex == s && positionerText3DID.pathPtIndex == i) {
                    auto q = (EditableTextBox*)pathIndexTexts[s][i].get();
                    q->draw(glWindow, getMousePosition(), !(volumeSlider.getMouseOver() || mixSlider.getMouseOver()));
//                    if (!q->getSelectAnimation().isPlaying())
//                        positionerText3DID = {-1, -1};
                } else
                    pathIndexTexts[s][i]->draw(glWindow);//, getMousePosition()), positionerText3DID.sourceIndex == s && positionerText3DID.pathPtIndex == i);
            }
        }
    }
//    for (auto& s : pathIndexTexts)
//        if ((*sources)[&s - &pathIndexTexts.front()].getSourceSelected()) {
//            //const auto ptSelecteds = (*sources)[&s - &pathIndexTexts.front()].getSelectedPathPoints();
//            positionerText3DID.sourceIndex == s && positionerText3DID.pathPtIndex == i;
//            for (auto& i : s)
//                i.draw(glWindow, getMousePosition(), ptSelecteds[&i - &s.front()]);
//        }
    
//    std::array<float, 3> pos;
//    float poser[3];
//    for (int i = 0; i < sources->size(); ++i) {
//        pos = (*sources)[i].getPosXYZ();
//        poser[0] = pos[0];
//        poser[1] = pos[1];
//        poser[2] = pos[2];
//        // color depending on user interaction
//        if (i == mouseOverSourceIndex || (*sources)[i].getSourceSelected() || pointInsideSelectRegion(poser)) {
//            float alpha = 1.0f;
//            if ((*sources)[i].getSourceMuted())
//                alpha = 0.6f;
//            // draw selectable pts
//            const auto points = (*sources)[i].getPathPoints();
//            float pos3d[3];
//            float pos2d[2];
//            for (int j = 0; j < points.size(); ++j) {
//                pos3d[0] = points[j][0];
//                pos3d[1] = points[j][1];
//                pos3d[2] = points[j][2];
//                if (to2D(pos3d, pos2d)) {
//                    if (j == mouseOverPathPointIndex || (*sources)[i].getPathPointSelected(j)
//                        || ((*sources)[i].getSourceSelected() && pointInsideSelectRegion2D(pos2d)))
//                        glColor4f(0, 1, 0, alpha);
//                    else
//                        glColor4f(1, 1, 1, alpha);
//                    sprintf(str, "%d", j+1);
//                    drawStringStroke(str, pos2d[0]-0.01f/*+0.04*/, pos2d[1]-0.015f/*-0.04*/, 0);
//                }
//            }
//        }
//    }
    
    // draw the doppler speed of sound control slider
    cauto dopplerOn = processor->dopplerOn;
    cauto animation = dopplerButton.getPressAnimation();
    if (dopplerOn || animation.isPlaying()) {
        auto color = dopplerSlider.getColor();
        if (animation.isPlaying()
            || (dopplerOn && !color.isOpaque())
            || (!dopplerOn && !color.isTransparent())) {
            color = color.withAlpha(dopplerOn ? animation.getProgress() : 1 - animation.getProgress());
            dopplerSlider.setColor(color);
        }
        if (dopplerOn) // update the slider value if the preset changes
            dopplerSlider.setValue(processor->speedOfSound);
        cauto mouseOverEnabled = !selectionBox.isActive()
            && !(positionerText3DID.sourceIndex != -1
            && positionerText3D.getBoundary().contains(getMousePosition()));
        dopplerSlider.draw(glWindow, getMousePosition(), mouseOverEnabled);
//        const float m_x = getMouseX();
//        const float m_y = getMouseY();
//        glColor4f(1.0, 0.0, 0.0, 1.0);
//        mouseOverDopplerSlider = (-0.52 < m_x && m_x < 0.52
//                                && -1.0 < m_y && m_y < -0.86);
//        if (mouseOverDopplerSlider)
//            glLineWidth(2.0);
//        else
//            glLineWidth(1.0);
//        glBegin(GL_LINES);
//        glVertex2f(-0.5, -0.93);
//        glVertex2f(0.5, -0.93);
//        float speedOfSoundRange = processor->maxSpeedOfSound - processor->minSpeedOfSound;
//        float xPos = -0.5 + (processor->speedOfSound - processor->minSpeedOfSound)/speedOfSoundRange;
//        glVertex2f(xPos, -0.91);
//        glVertex2f(xPos, -0.95);
//        float speedOfSoundToDisplay = processor->speedOfSound;
//        if (mouseOverDopplerSlider) {
//            glColor4f(1.0, 0.0, 0.0, 0.5);
//            xPos = m_x;
//            if (xPos < -0.5) xPos = -0.5;
//            if (xPos >  0.5) xPos =  0.5;
//            glVertex2f(xPos, -0.89);
//            glVertex2f(xPos, -0.97);
//            speedOfSoundToDisplay = processor->minSpeedOfSound + (xPos+0.5)*speedOfSoundRange;
//            newDopplerSpeedOfSound = speedOfSoundToDisplay;
//        }
//        glEnd();
//        glColor4f(1.0, 0.0, 0.0, 1.0);
//        glLineWidth(1.0); // reset back to default so other lines aren't affected
//        sprintf(str, "Speed of Sound: DUM m/s");
//        float stroke_scale = 0.00035;
//        float aspect = ((float)getHeight())/((float)getWidth());
//        float xlen = 0;
//        for (int i = 0; i < strlen(str); ++i) {
//            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
//        }
//        xlen *= stroke_scale*aspect;
//        sprintf(str, "Speed of Sound: %.0f m/s", speedOfSoundToDisplay);
//        drawStringStroke(str, -0.5*xlen, -0.98, 0);
//        sprintf(str, "Doppler On");
//        stroke_scale = 0.00035;
//        aspect = ((float)getHeight())/((float)getWidth());
//        xlen = 0;
//        for (int i = 0; i < strlen(str); ++i) {
//            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
//        }
//        xlen *= stroke_scale*aspect;
//        drawStringStroke(str, -0.5*xlen, -0.91, 0);
    }
    
    if (positionerText3D.isVisible()) {
        positionerText3D.mouseOverEnabled = !selectionBox.isActive();
        if (secPos != processor->posSEC && processor->getLockSourcesToPaths() && positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex == -1) { // reset position over source if it isn't and the source has moved on its path since the last frame
            secPos = processor->posSEC;
            cauto pos = (*sources)[positionerText3DID.sourceIndex].getPosXYZ();
            cfloat xyz[3] = {pos[0], pos[1], pos[2]};
            float xy[2];
            to2D(xyz, xy);
            cauto b = positionerText3D.getBoundary();
            if (b.getLeft() != xy[0] || b.getBottom() != xy[1]) {
                updatePositioner3DTextValueAndPosition();
                positionerText3D.mouseOverEnabled = false; // don't allow editing of text when the text is constantly being updated cuz that can lead to crashes
            }
        }
        positionerText3D.draw(glWindow, getMousePosition(), positionerText3D.mouseOverEnabled);
    }
//    // draw lpf freq
//    if (processor->lowPassOn) {
//        sprintf(str, "%d", (int)(processor->lowPassCutoff.load()));
//        drawStringStroke(str, -0.3, 0.0, 0.0);
//    }

//    // draw the HRIR data for the first source
//    if (sources->size() > 0) {
//        const std::array<float, 3> rae = sources->getFirst()->getPosRAE();
//        
//        // get the inner + outer rad,azi,ele indicies that define the 3d region bounded by the hrtf/dvf sampling resolution that the source is currently located in
//        const int innerRadiusIndex = std::max(0, std::min((int)((std::log(rae[0])-std::log(distanceBegin))/std::log(distanceEnd/distanceBegin)*(numDistanceSteps-1)), numDistanceSteps-2));//-1);
////        const int outerRadiusIndex = innerRadiusIndex+1;// std::min(innerRadiusIndex+1, numDistanceSteps-1);
//        
//        const int lowerElevationIndex = std::max(0, std::min((int)std::floor(rae[2]/M_PI*numElevationSteps), numElevationSteps-1));
//        const int upperElevationIndex = lowerElevationIndex+1;
//        
//        const float revAzi = std::fmod(4*M_PI-rae[1], 2*M_PI); // fix reversed azimuth indexing with hrir array's, this caused lowerAzimuthIndex = -1 without fmod
//        int lowerAzimuthIndex = std::min((int)std::floor(revAzi/(2*M_PI)*numAzimuthSteps), numAzimuthSteps-1);
////        int upperAzimuthIndex = (lowerAzimuthIndex+1) % numAzimuthSteps;
//        
////        // inner/outer surface radius values
////        const float rIn  = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)innerRadiusIndex)/(numDistanceSteps-1));
////        const float rOut = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)outerRadiusIndex)/(numDistanceSteps-1));
////        
////        // upper/lower azimuth values
////        const float aP = ((float)(upperAzimuthIndex))*(2*M_PI)/numAzimuthSteps;
////        const float aM = ((float)(lowerAzimuthIndex))*(2*M_PI)/numAzimuthSteps;
//        
//        // upper/lower elevation values
//        const float eM = ((float)lowerElevationIndex)*M_PI/numElevationSteps;
//        const float eP = ((float)upperElevationIndex)*M_PI/numElevationSteps;
//
////        sprintf(str, "%d, %d, %d", innerRadiusIndex, lowerAzimuthIndex, lowerElevationIndex);
////        drawStringStroke(str, 0.3, 0.0, 0.0);
//        
//        float *leftHRIR, *rightHRIR;
//        if ((eP-rae[2] > eM-rae[2]) ? (lowerElevationIndex == 0 ? true : false) : (upperElevationIndex == numElevationSteps ? true : false)) {
//            int eleIndex = 0;
//            if (lowerElevationIndex == 0)
//                eleIndex = 0;
//            else if (upperElevationIndex == 0)
//                eleIndex = 1;
//            leftHRIR  = HRIRdataPoles[innerRadiusIndex][eleIndex][0];
//            rightHRIR = HRIRdataPoles[innerRadiusIndex][eleIndex][1];
//        } else if (lowerAzimuthIndex <= numAzimuthSteps/2) {
//            leftHRIR  = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][std::max(lowerElevationIndex-1, 0)][0];
//            rightHRIR = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][std::max(lowerElevationIndex-1, 0)][1];
//        } else {
//            leftHRIR  = HRIRdata[innerRadiusIndex][numAzimuthSteps-lowerAzimuthIndex][std::max(lowerElevationIndex-1, 0)][1];
//            rightHRIR = HRIRdata[innerRadiusIndex][numAzimuthSteps-lowerAzimuthIndex][std::max(lowerElevationIndex-1, 0)][0];
//        }
//        glColor4f(0.0, 0.0, 1.0, 0.2);
//        drawArray(leftHRIR, numTimeSteps);
//        glColor4f(1.0, 0.0, 0.0, 0.2);
//        glPushMatrix();
//        glTranslatef(0.005, 0.0, 0.0);
//        drawArray(rightHRIR, numTimeSteps);
//        glPopMatrix();
//    }
    
    // Making sure we can render 3d again, important!
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//std::array<float, 2> ThreeDAudioProcessorEditor::getDeltaXYFromCenterToEdgeOfSphere(const std::array<float, 3>& center,
//                                                                                    const float radius)
//{
//    std::array<float, 2> dxy;
//    float center2d[2];
//    float edge2d[2];
//    to2D({center[0], center[1], center[2]}, center2d);
//    float edge[3];
//    to2D(edge, edge2d);
//    return dxy;
//}

void ThreeDAudioProcessorEditor::draw3DAxis()
{
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, -1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(-1.0, 0.0, 0.0);
    glEnd();
    glDisable(GL_LINE_SMOOTH);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, -1.0, 0.0);
    glEnd();
    glEnable(GL_LINE_SMOOTH);

    for (auto& label : axis3dLabels)
        label.draw(glWindow);
}

void ThreeDAudioProcessorEditor::drawPathControl()
{
    // get mouse position
    cauto m_x = getMouseX();
    cauto m_y = getMouseY();
 
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
    glLoadIdentity(); //Reset the drawing perspective
    /* viewing transformation  */
    glpp::lookAt ({eyePos[0], eyePos[1], eyePos[2]},  // eye pos (x,y,z)
                  {     0.0f,      0.0f,      0.0f},                   // direction looking at (x,y,z)
                  { eyeUp[0],  eyeUp[1],  eyeUp[2]});  // camera up vector (x,y,z)
//    gluLookAt (eyePos[0], eyePos[1], eyePos[2],  // eye pos (x,y,z)
//               0.0, 0.0, 0.0,                   // direction looking at (x,y,z)
//               eyeUp[0], eyeUp[1], eyeUp[2]);  // camera up vector (x,y,z)
    /* Remove hidden surfaces */
    glEnable(GL_DEPTH_TEST);  // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    glEnable(GL_LINE_SMOOTH); // antialiasing on for 3d stuff
    
    // begin of old drawMain()
    // light for source(s)
    glEnable(GL_LIGHTING);
    const int numLights = std::min((int)sources->size(), 7);
    
    // must loop through all potentially ON gl lights to refresh each's state based on the current sources
    for (int i = 0; i < 7; ++i) {
        // only show lights for sources that aren't muted
        if (i < numLights && !(*sources)[i].getSourceMuted()) {
            const auto pos = (*sources)[i].getPosXYZ();
            float glpos[4];
            glpos[0] = pos[0];
            glpos[1] = pos[1];
            glpos[2] = pos[2];
            glpos[3] = 1.0; // must be nonzero to be a positional light sources as opposed to a directional one
            glLightfv(GL_LIGHT0+i, GL_POSITION, glpos);
            glEnable(GL_LIGHT0+i);
        } else {
            glDisable(GL_LIGHT0+i);
        }
    }
    
    // the ambient light
    glEnable(GL_LIGHT7);
    
    // draw head
    drawHead();
    
    glDisable(GL_LIGHTING);
    
//    // a newly loaded preset doesn't update visually for the PATH_AUTOMATION view if we don't do this...
//    // see if any sources are selected
//    bool noSourcesSelected = true;
//    for (int s = 0; s < sources->size(); ++s) {
//        if ((*sources)[s]->getSourceSelected()) {
//            noSourcesSelected = false;
//            break;
//        }
//    }
//    // if the user didn't select any explicitly, just automatically select all the sources for automating
//    if (noSourcesSelected) {
//        for (int s = 0; s < sources->size(); ++s)
//            (*sources)[s]->setSourceSelected(true);
//    }
    
//    // reset this thing if not dragging some points with the mouse, put this back down below...
//    if (!pathAutomationPointsGrabbedWithMouse)
//        mouseOverPathAutomationPointIndex = {-1, -1, -1};
////    else { // trying to avoid crash below...
////        const auto pathPos = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr();
////        mouseOverPathAutomationPointIndex[0] = boundsCheck(mouseOverPathAutomationPointIndex[0], 0, (int)sources->size());
////        mouseOverPathAutomationPointIndex[1] = boundsCheck(mouseOverPathAutomationPointIndex[1], 0, pathPos->getNumPoints());
////        mouseOverPathAutomationPointIndex[2] = boundsCheck(mouseOverPathAutomationPointIndex[2], 0, pathPos->getNumSelectedPoints());
////    }
    
    // draw the source(s)
    cauto sourceRadius = 0.03f;
    cauto pathPtRadius = 0.025f;
    
    cauto numSlices = 24;
    cauto numStacks = 12;
    for (int s = 0; s < sources->size(); ++s) {
        cauto sourceSelected = (*sources)[s].getSourceSelected();
        if (sourceSelected) {
            cauto pos = (*sources)[s].getPosXYZ();
            cauto normalColor = Colour::fromFloatRGBA(1, 1, 1, 1);
            cauto mouseOverColor = Colour::fromFloatRGBA(0, 1, 0, 1);
            cauto mouseOverSource = false;//s == mouseOverSourceIndex || pointInsideSelectRegion({pos[0], pos[1], pos[2]});
            cauto alpha = (*sources)[s].getSourceMuted() ? 0.5f : 1.0f;
            cauto prevSelectSourceAnimation = selectSourceAnimations.get(s);
            drawSelectableOrb({pos[0], pos[1], pos[2]}, sourceRadius, numSlices, numStacks, normalColor, mouseOverColor,
                              mouseOverSource, prevMouseOverSources[s], mouseOverSourceAnimations,
                              sourceSelected, prevSelectedSources[s], selectSourceAnimations, s, alpha, false, true);
            //cauto mouseOverSourceAnimation = nullptr;//mouseOverSourceAnimations.get(s);
            cauto selectSourceAnimation = selectSourceAnimations.get(s);
            cauto sourceUnselectedColor = Colour::fromFloatRGBA(0, 0, 1, 1);
            cauto sourceSelectedColor = Colour::fromFloatRGBA(1, 0, 0, 1);
            Colour pathPtColor;
            if (selectSourceAnimation)
                if (sourceSelected)
                    pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, selectSourceAnimation->getProgress());
                else
                    pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, 1 - selectSourceAnimation->getProgress());
            else if (sourceSelected)
                pathPtColor = sourceSelectedColor;
//            else if (mouseOverSourceAnimation)
//                if (mouseOverSource)
//                    pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, mouseOverSourceAnimation->getProgress());
//                else
//                    pathPtColor = sourceUnselectedColor.interpolatedWith(sourceSelectedColor, 1 - mouseOverSourceAnimation->getProgress());
            else if (mouseOverSource)
                pathPtColor = sourceSelectedColor;
            else
                pathPtColor = sourceUnselectedColor;
            cauto path = (*sources)[s].getPathPtr();
			if (selectSourceAnimation || processor->pathChanged/* || mouseOverSourceAnimation*/ || prevSourceMuted[s] != (*sources)[s].getSourceMuted() || (prevSelectSourceAnimation && !selectSourceAnimation))
                pathDisplayList[s] = 0;
            prevSourceMuted[s] = (*sources)[s].getSourceMuted();
            InterpolatorLook pathLook (path, InterpolatorLook::THREE_D);
            pathLook.beginColor = pathLook.endColor = pathPtColor.withAlpha(alpha);
            pathLook.numVertices = path->getNumPoints() * 20;
            pathLook.lineType = InterpolatorLook::LineType::DASHED;
            draw(path, pathLook, pathDisplayList[s]);
            if (s == sources->size() - 1)
                processor->pathChanged = false;
            cauto points = (*sources)[s].getPathPoints();
            for (int j = 0; j < points.size(); ++j) {
                //cauto normalColor = Colour::fromFloatRGBA(0.7f, 0, 0, 1);
                cauto mouseOverColor = Colour::fromFloatRGBA(0.5f, 0, 1, 1);
                cauto mouseOver = false;//(s == mouseOverPathPointSourceIndex && j == mouseOverPathPointIndex)
                    //|| ((*sources)[s].getSourceSelected() && pointInsideSelectRegion({points[j][0], points[j][1], points[j][2]}));
                cauto selected = false;//(*sources)[s].getPathPointSelected(j);
                cauto id = s * 1000 + j; // if there is more than 1000 path pts for a source, well fuck just make spacing bigger!
                bool prevMouseOver = prevMouseOverPathPts[s][j];
                bool prevSelected = prevSelectedPathPts[s][j];
                drawSelectableOrb({points[j][0], points[j][1], points[j][2]}, pathPtRadius, numSlices, numStacks, pathPtColor, mouseOverColor,
                                  mouseOver, prevMouseOver, mouseOverPathPtAnimations,
                                  selected, prevSelected, selectPathPtAnimations, id, alpha, false, false);
                prevMouseOverPathPts[s][j] = prevMouseOver;
                prevSelectedPathPts[s][j] = prevSelected;
            }
            if ((*sources)[s].getNumPathPoints() > 1
                && std::abs(m_x) <= 1/*x_scale*/
                && std::abs(m_y) <= 1/*y_scale*/) {
                // draw the position of the source that conincides with the mouse's current vertical position
                float paraVal = 0.5f * (m_y / y_scale + 1.0f);
                if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                    // crashed here in Cubase when deleting a selected path point that the mouse was over, moved the reset mouseOverPathAutomationPointIndex code from below here to just above to try and fix, crashed again when mouseOverPathAutomationPointIndex[1] = numPts.size() (just out of bounds) when copying?/deleting? pts being dragged...
                    paraVal = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1])[1];
                }
                if (paraVal < 0)
                    paraVal = 0;
                if (paraVal > 1)
                    paraVal = 1;
                cauto range = (*sources)[s].getPathPtr()->getInputRange()[1] * 0.99999f;
                std::vector<float> paraPos;
                (*sources)[s].getPathPtr()->pointAt(paraVal*range, paraPos);
                glColor4f(1.0, 1.0, 1.0, alpha);
                cauto radius = 0.03f;
                cauto sourceSphere = glpp::SolidSphere(radius, numSlices, numStacks);
                sourceSphere.draw(paraPos[0], paraPos[1], paraPos[2]);
//                glPushMatrix();
//                glTranslatef(paraPos[0], paraPos[1], paraPos[2]);
//                gluSphere(pQuadric,radius,numSlices,numStacks);
//                glPopMatrix();
            }
        }
    }
    
//    gluDeleteQuadric(pQuadric);
    
    draw3DAxis();
    
    glDisable(GL_LINE_SMOOTH); // antialiasing off for 2d stuff
    
    // set up 2d projection for HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
    glOrtho(-1, 1, -1, 1, -1, 10);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    //glDisable(GL_CULL_FACE);
    
    // draw 2d stuff here ...
    
    // draw dark translucent curtain over 3d scene portion
    glColor4f(0, 0, 0, 0.25f);
    windowCoordinates.drawFill();
//    glBegin(GL_POLYGON);
//    glVertex2f(-1.0, -1.0);
//    glVertex2f(-1.0, +1.0);
//    glVertex2f(+1.0, +1.0);
//    glVertex2f(+1.0, -1.0);
//    glEnd();

    // draw the parametric position automation grid and its labels
    cauto dy = pixelsToNormalized(5, getHeight());// (1 - y_scale) * 0.15f;
    cauto dxp = pixelsToNormalized(5, getWidth());// normalizedXYConvert(dy, getHeight(), getWidth());
    cauto dxm = pixelsToNormalized(6, getWidth());
    glColor4f(1, 1, 1, 1);
    glBegin(GL_LINES);
    glVertex2f(-x_scale, -y_scale);
    glVertex2f(-x_scale, +y_scale);
    glVertex2f(-x_scale, -y_scale);
    glVertex2f(+x_scale, -y_scale);
    glVertex2f(-x_scale, -y_scale + dy);
    glVertex2f(-x_scale, -y_scale - dy);
    glVertex2f(x_scale,  -y_scale + dy);
    glVertex2f(x_scale,  -y_scale - dy);
    glVertex2f(-x_scale + dxp, -y_scale);
    glVertex2f(-x_scale - dxm, -y_scale);
    glVertex2f(-x_scale + dxp, y_scale);
    glVertex2f(-x_scale - dxm, y_scale);
    glEnd();
    
    automationViewLowerTimeText.setText(getFormattedTimeString(pathAutomationView.xMinCurrent()));
    automationViewLowerTimeText.draw(glWindow);
    automationViewUpperTimeText.setText(getFormattedTimeString(pathAutomationView.xMaxCurrent()));
    automationViewUpperTimeText.draw(glWindow);
////    getFormattedTime(pathAutomationView.xMinCurrent(), str);
////    //sprintf(str, "%.2f", automationViewOffset-automationViewWidth/2.0);
////    
////    drawStringStroke(str, -x_scale, -y_scale-0.07, 0.0);
////    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -x_scale, -y_scale-0.07, 0.0);
////    getFormattedTime(pathAutomationView.xMaxCurrent(), str);
////    //sprintf(str, "%.2f", automationViewOffset+automationViewWidth/2.0);
//    float xlen;
//    const float stroke_scale = 0.00035;
//    const float aspect = ((float)getHeight())/((float)getWidth());
//////    float xlen = 0;
//////    for (int i = 0; i < strlen(str); i++) {
//////        xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, '0'/*str[i]*/);
//////    }
//////    xlen *= stroke_scale*aspect;
////    // less jumpy this way...
////    float xlen = strlen(str) * glutStrokeWidth(GLUT_STROKE_ROMAN, '0') * stroke_scale * aspect * 0.8;
////    drawStringStroke(str, x_scale-xlen, -y_scale-0.07, 0.0);
////    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, x_scale-0.035*3.0, -y_scale-0.07, 0.0);
//    const float topMax = 119.05*stroke_scale; // also used below
//////    sprintf(str, "%d", 0);
//////    drawStringStroke(str, -x_scale-0.06, -y_scale-topMax*0.5, 0.0);
//////    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -x_scale-0.06, -y_scale, 0.0);
//////    sprintf(str, "%d", 1);
//////    drawStringStroke(str, -x_scale-0.06, y_scale-topMax*0.5, 0.0);
//////    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -x_scale-0.09, y_scale-0.05, 0.0);
    
    secPos = processor->posSEC;
    
    // processor's time stamp info controls view offset
    //    if (secPos > automationViewWidth/2.0) {
    //        automationViewOffset = secPos;
    //    } else {
    //        automationViewOffset = automationViewWidth/2.0;
    //    }
    
//    // indicate if doppler is on
//    if (processor->dopplerOn) {
//        glColor4f(1.0, 0.0, 0.0, 1.0);
//        sprintf(str, "Doppler On");
////        stroke_scale = 0.00035;
////        aspect = ((float)getHeight())/((float)getWidth());
//        xlen = 0;
//        for (int i = 0; i < strlen(str); ++i) {
//            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
//        }
//        xlen *= stroke_scale*aspect;
//        drawStringStroke(str, -0.5*xlen, 0.94, 0);
//    }
    
    // (moved above) reset this thing if not draggin some points with the mouse
    if (!pathAutomationPointsGrabbedWithMouse)
        mouseOverPathAutomationPointIndex = {-1, -1, -1};
    
    // draw the path automation points of the selected sources
    // precomputation of constants for view setting
    
    const float thing1 = pathAutomationView.getWidth() * 0.5f - pathAutomationView.getXPosition();
    const float thing2 = 2.0f / pathAutomationView.getWidth();
    for (int s = 0; s < sources->size(); ++s) {
        if ((*sources)[s].getSourceSelected()) {
            float x, y;
            // draw each point
            //FunctionalInterpolator<float> pathPos = (*sources)[s]->getPathPos();
            // not having to copy the FunctionalInterp is much less expensive when there are lots of points
            const auto pathPos = (*sources)[s].getPathPosPtr();
//            //std::vector<std::vector<float>> pts = pathPos.getPoints();
            const auto pts = pathPos->getPoints();
//            for (int i = 0; i < pts.size(); ++i) {
//                x = x_scale*((pts[i][0]+thing1)*thing2-1.0);
//                // if the point is within the automation view grid, draw it
//                if (x > -1.05 && x < 1.05) {
//                    y = y_scale*(pts[i][1]*2.0-1.0);
//                    float xy[2] = {x, y};
//                    // if mouse is over a path automation point or the point is already selected, then indicate with color
//                    if (mouseOverPathAutomationPointIndex[0] == -1
//                        && x > m_x-aspect*0.025 && x < m_x+aspect*0.025 && y > m_y-0.025 && y < m_y+0.025
//                        && !helpButton.isMouseOver()
//                        && !dopplerButton.isMouseOver()) {
//                        mouseOverPathAutomationPointIndex = {s, i, -1};
//                        glColor3f(1.0, 0.0, 0.0);
//                        radius = 0.023;
//                    } else if (pathAutomationPointsGrabbedWithMouse
//                               && mouseOverPathAutomationPointIndex[0] == s
//                               && mouseOverPathAutomationPointIndex[1] == i) {
//                        glColor3f(1.0, 0.0, 0.0);
//                        radius = 0.023;
//                    } else {
//                        radius = 0.02;
//                        if (pathPos->getPointSelected(i) || pointInsideSelectRegion2D(xy)) {
//                            glColor3f(0.8, 0.0, 0.1);
//                        } else {
//                            glColor3f(0.0, 0.3, 1.0);
//                        }
//                    }
//                    drawCircle(x, y, radius/*0.02*/, 8);
////                    // draw pts index
////                    sprintf(str, "%d", i);
////                    drawStringStroke(str, x, y-0.05, 0);
//                }
//            }
            if (pts.size() > 0)
            {
                glPushMatrix();
                glScalef    (x_scale, y_scale, 1.0);
                glTranslatef(  -1.0, -1.0, 0.0);
                glScalef    (thing2,  2.0, 1.0);
                glTranslatef(thing1,  0.0, 0.0);
                
//                glScalef(x_scale, 1.0, 1.0);
//                glTranslatef(-1.0, 0.0, 0.0);
//                glScalef(thing2, 1.0, 1.0);
//                glTranslatef(thing1, 0.0, 0.0);
                // draw the dotted line for the pathPos interp
//                if (pathPosDisplayList[s] == 0 || processor->pathPosChanged || prevAutomationViewWidth != automationViewWidth || prevAutomationViewOffset != automationViewOffset) {
                    // if the interp changed (or the gl view was closed and the local points got cleared) we gotta update all the locally stored interp points used for drawing (more cpu intensive, would like to avoid)
//                    glDeleteLists(pathPosDisplayList[s], 1);
//                    pathPosDisplayList[s] = glGenLists(1);
//                    glNewList(pathPosDisplayList[s], GL_COMPILE_AND_EXECUTE);
                    //glColor3f(0.5, 0.0, 1.0);
                    //glLineWidth(2);
                if (processor->pathPosChanged /*|| prevAutomationViewWidth != automationViewWidth || prevAutomationViewOffset != automationViewOffset*/)
                    pathAutomationDisplayList[s] = 0;
                InterpolatorLook look (pathPos, InterpolatorLook::TWO_D);
                look.numVertices = 200;
                look.begin = pathAutomationView.getXPosition() - pathAutomationView.getWidth() * 0.5f / x_scale;
                look.end = pathAutomationView.getXPosition() + pathAutomationView.getWidth() * 0.5f / x_scale;
                look.beginColor = Colour::fromFloatRGBA(0.7f, 0.0f, 0.3f, 1.0f);
                look.endColor = Colour::fromFloatRGBA(0.3f, 0.0f, 0.7f, 1.0f);
                look.numColorCycles = 3;
                look.colorCyclePhase = (float)s / sources->size();
                //look.lineSize = 1;
                //look.lineType = InterpolatorLook::DASHED;
                draw(pathPos, look, pathAutomationDisplayList[s]);
                
//                const auto points = convertPoints(pts/*pathPos->getPoints()*/);
//                const auto selectedStates = pathPos->getPointsSelected();
                const auto points = pathPos->getSelectablePoints();
                const std::array<float, 2> range {pathAutomationView.getXPosition() - 0.5f * pathAutomationView.getWidth() / x_scale, pathAutomationView.getXPosition() + 0.5f * pathAutomationView.getWidth() / x_scale};
                const bool mouseOverEnabled = !helpButton.isMouseOver() && !dopplerButton.isMouseOver() && !pathAutomationPointsGrabbedWithMouse && !(volumeSlider.getMouseOver() || mixSlider.getMouseOver());
                const auto ptInView = pathAutomationView.holderToView({m_x, m_y});// View2DFuncs::getPoint(view, {m_x, m_y});
                const auto selectBox = selectionBox.isActive() ? pathAutomationView.holderToView(selectionBox)/*View2DFuncs::getBox(view, selectionBox)*/ : Box();
                const auto antiSelectBox = positionerText.getSelected() ? pathAutomationView.holderToView(positionerText.getBoundary())/*View2DFuncs::getBox(view, positionerText.getBoundary())*/ : Box();
                int mouseOverIndex;
                drawPoints2D(points, pointLook, pointStates[s], ptInView, glWindow, selectBox, antiSelectBox, mouseOverIndex, mouseOverEnabled, pathAutomationView.getWidth(),
                             range);
                if (mouseOverIndex != -1 && mouseOverPathAutomationPointIndex != std::array<int, 3>{s, mouseOverIndex, -1}) {
                    mouseOverPathAutomationPointIndex = {s, mouseOverIndex, -1};
                    positionerTextMouseMoved(); // primarily to show text in right color
                }
                
//                    draw(pathPos, automationViewOffset - automationViewWidth * 0.5f / x_scale, automationViewOffset + automationViewWidth * 0.5f / x_scale, 200);
                    //glLineWidth(1);
//                    glBegin(GL_LINE_STRIP);
//    //                // the x-value way
//    //                int N = 50; // num GL pts for drawing pathPos interp
//    //                float begin = std::max<float>(pathPos.getInputRange()[0], automationViewOffset-automationViewWidth*0.5);
//    //                float end = std::min<float>(pathPos.getInputRange()[1], automationViewOffset+automationViewWidth*0.5);
//    //                float incr = (end - begin) / N;
//    //                //float incr = (pathPos.getInputRange()[1] - pathPos.getInputRange()[0]) / ((pathPos.getNumPoints()-1)*N);
//    //                //for (float x = pathPos.getInputRange()[0]; x < pathPos.getInputRange()[1]; x += incr) {
//    //                for (float x = begin; x < end; x += incr) {
//    //                    std::vector<float> pt;
//    //                    // only draw the dotted line over the portions that are not open/empty segments, also avoiding drawing anything between 2pts with the same x val
//    //                    if (pathPos.pointAtSmart(x, pt) /*&& pts[segmentIndex][0] != pts[segmentIndex+1][0]*/) {
//    //                        y = y_scale*(2.0*pt[0]-1.0);
//    //                        glVertex2f(x, y);
//    //                    } else {
//    //                        // finish drawing any unfinished GL_LINE segments
//    //                        glVertex2f(x, y);
//    //                        // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
//    //                        glEnd();
//    //                        glBegin(GL_LINE_STRIP);
//    //                        x = pathPos.getNextSplineBegin(); //incr; // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
//    //                    }
//    //                }
//                    // the new parametric way, just drawing the on screen part
//                    const int N = 50; // num GL pts for drawing pathPos interp across the visible screen
//                    const auto inputRange = pathPos->getInputRange();
//                    const float begin = std::max<float>(inputRange[0], automationViewOffset-automationViewWidth*0.5/x_scale);
//                    const float end = std::min<float>(inputRange[1], automationViewOffset+automationViewWidth*0.5/x_scale);
//                    const float paraIntvl = (end - begin) / ((float)N);
//                    //std::vector<float> pt;
//                    float pt;
//                    x = begin;
//                    int prev_spline = 0;
//                    int pprev_spline = 0;
//                    //for (int i = 0; i < N; ++i)
//                    while (x < end) {
//                        // only draw the dotted line over the portions that are not open/empty segments,
//                        if (pathPos->pointAtSmart(x, &pt, prev_spline)) {
////                            // if we are on a new segment, do a vertex at the point we just passed
////                            if (prev_spline >= 0 && pathPos->prev_spline_index != prev_spline)
////                            {
//////                                int dif = pathPos->prev_spline_index - prev_spline;
//////                                for (int j = 1; j <= dif; ++j)
//////                                    glVertex2f(pts[prev_spline+j][0], y_scale*(2.0*pts[prev_spline+j][1]-1.0));
////                                glVertex2f(pts[prev_spline+1][0], pts[prev_spline+1][1]);
////                            }
//                            if ((pprev_spline+2 <= prev_spline) ? pts[pprev_spline+1][0] != pts[prev_spline][0] : true)
//                                glVertex2f(x, pt);
//                            else { // avoiding drawing anything between 2 or more pts with the same x val
//                                glEnd();
//                                glBegin(GL_LINE_STRIP);
//                            }
//                            x += paraIntvl;
//                        }
//                        else {
//                            // finish drawing any unfinished GL_LINE segments
////                            if (i == N)
////                                glVertex2f(pts.back()[0], pts.back()[1]);
////                            else
//                                glVertex2f(pts[prev_spline][0], pts[prev_spline][1]);
//                            // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
//                            glEnd();
//                            glBegin(GL_LINE_STRIP);
//                            if (prev_spline+1 < pts.size()) {
//                                x = pts[prev_spline+1][0];// pathPos->getNextSplineBegin(); // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
//                            } else
//                                x = end; // exit the while loop
//                        }
//                        pprev_spline = prev_spline;
//                        //prev_spline = pathPos->prev_spline_index;
//                    }
//    //                // the parametric way
//    //                int N = 26; // num GL pts per segment for drawing pathPos interp
//    //                float paraIntvl = 1.0/(pts.size()-1.0);
//    //                std::vector<float> pt;
//    //                for (int i = 0; i < (((int)(pts.size()))-1)*N; ++i) {
//    //                    float paraVal = ((float)i)/((float)(pts.size()-1)*N);// + 0.001;, caused segmentIndex bad access crash
//    //                    int segmentIndex = floor(paraVal*(pts.size()-1));
//    //                    x = pts[segmentIndex][0] + (paraVal-segmentIndex*paraIntvl) * (pts[segmentIndex+1][0]-pts[segmentIndex][0])/paraIntvl;
//    //                    // only draw the dotted line over the portions that are not open/empty segments, also avoiding drawing anything between 2pts with the same x val
//    //                    if (pathPos.pointAt(x, pt) && pts[segmentIndex][0] != pts[segmentIndex+1][0]) {
//    //                        y = y_scale*(2.0*pt[0]-1.0);
//    //                        glVertex2f(x, y);
//    //                    } else {
//    //                        // finish drawing any unfinished GL_LINE segments
//    //                        glVertex2f(x, y);
//    //                        // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
//    //                        glEnd();
//    //                        glBegin(GL_LINES);
//    //                        i += N-1; // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
//    //                    }
//    //                }
                    //glEnd();
                    //glEndList();
                    // tell the processor/source that we have saved the changes that we are concerned about
                    if (s == sources->size() - 1)
                        processor->pathPosChanged = false;
//                } else {
//                    // if the interp hasn't changed since the last frame, we can just draw with the locally stored points (less cpu intensive)
//                    glCallList(pathPosDisplayList[s]);
//    //                for (auto l : pathPosDisplayList[s])
//    //                    glCallList(l);
//    //                }
//                }
                glPopMatrix();
            }
            if (!(*sources)[s].getSourceMuted() && (*sources)[s].getNumPathPoints() > 1 && processor->getLockSourcesToPaths()) {
                // draw the source's current position
                std::vector<float> pt;
                float y1;
                if (pathPos->pointAt(secPos, pt))
                    y1 = pt[0];
                else
                    y1 = 0;
                glColor4f(0, 1, 0, 0.9f);
                x = x_scale * ((secPos + thing1) * thing2 - 1);
                y = y_scale * (2 * y1 - 1);
                cauto r = pixelsToNormalized(5, getHeight());
                glEnable(GL_LINE_SMOOTH);
                drawCircle({x, y}, r, 10, getWidth(), getHeight());
                glDisable(GL_LINE_SMOOTH);
            }
        }
    }
    
    glLineWidth(1); // reset this to 1 b/c it ain't so necessarily
    
    // mr clock, mr clock, what time is it?
    cauto x = x_scale * ((secPos-pathAutomationView.getXPosition()+pathAutomationView.getWidth()/2.0f)*2.0f/pathAutomationView.getWidth() - 1.0f);
    if (std::abs(x) <= x_scale) {
//        getFormattedTime(secPos, str);
//        xlen = strlen(str) * glutStrokeWidth(GLUT_STROKE_ROMAN, '0') * aspect * stroke_scale * 0.8;
//        drawStringStroke(str, x-xlen*0.5, -y_scale-0.05, 0.0);
        automationViewTimeText.setText(getFormattedTimeString(secPos));
        auto b = automationViewTimeText.getBoundary();
        b.setLeft(-2 + x);
        b.setRight(2 + x);
        automationViewTimeText.setBoundary(b);
        cauto dw = 0.5f * automationViewTimeTextWidth;
        b.setLeft(-dw + x);
        b.setRight(dw + x);
        glColor4f(0, 0, 0, 0.75f);
        b.drawFill();
//        glColor3f(0, 1, 0);
//        glBegin(GL_LINES);
//        glVertex2f(x, -y_scale - 0.02f);
//        glVertex2f(x, -y_scale + 0.02f);
//        glEnd();
        automationViewTimeText.draw(glWindow, -1, -1, nullptr, nullptr, 4, -1, 0, &automationViewTimeTextWidth);
        // draw the parametric position indicator for each active source
        if (processor->getLockSourcesToPaths()) {
            automationViewPercentTexts.resize(sources->size());
            for (int s = 0; s < sources->size(); ++s) {
                if ((*sources)[s].getSourceSelected() && !(*sources)[s].getSourceMuted()
                    && (*sources)[s].getNumPathPoints() > 1) {
                    std::vector<float> parametricPos;
                    float y1;
                    if ((*sources)[s].getPathPosPtr()->pointAt(secPos, parametricPos))
                        y1 = parametricPos[0];
                    else
                        y1 = -1;
                    cauto y = y_scale * (2 * y1 - 1);
//                    glColor3f(0, 1, 0);
//                    glBegin(GL_LINES);
//                    glVertex2f(-x_scale - 0.02f, y);
//                    glVertex2f(-x_scale + 0.02f, y);
//                    glEnd();
                    //sprintf(str, "%d%%", (int)(y1*100));
                    auto txt = std::to_string((int)(y1 * 100)) + "%";
                    if (txt.length() == 2)
                        txt = "  " + txt;
                    automationViewPercentTexts[s].setText(txt);
                    cauto fontSize = automationViewTimeTextLook.fontSize;
                    cauto hd2 = pixelsToNormalized(fontSize, getHeight()) * 0.5f / automationViewTimeTextLook.verticalPad;
                    automationViewPercentTexts[s].setBoundary({y + hd2, y - hd2, -1.0f, -x_scale});
                    automationViewTimeTextLook.fontSize = -1;
                    automationViewPercentTexts[s].setLook(automationViewTimeTextLook);
                    automationViewPercentTexts[s].draw(glWindow);
                    automationViewTimeTextLook.fontSize = fontSize;
//                    sprintf(str, "%d%%", (int)(y1*100));
//                    xlen = 0;
//                    for (int i = 0; i < strlen(str); ++i) {
//                        xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
//                    }
//                    xlen *= aspect*stroke_scale;
//                    drawStringStroke(str, fmax(-0.998, -x_scale-0.025-xlen), y-topMax*0.5, 0.0);
                }
            }
        }
    }
    
    if (processor->loopingEnabled || loopRegionToggleAnimation.isPlaying()/*processor->loopRegionBegin != -1 && processor->loopRegionEnd != -1*/)
        drawLoopingRegion();

    // draw mouse drag boarders if mouse is being dragged to select a group of things
//    if (mouseDragging || mouseDragAnimation.isPlaying())
//        drawMouseDragging();
    draw(selectionBox, glWindow);
    
    // draw the mouse's position on the path automation graph
    if (!pathAutomationPointsGrabbedWithMouse && !loopRegionBeginSelected && !loopRegionEndSelected
        && !positionerText.getSelected() && !positionerText.getSelectAnimation().isPlaying())
        positionerTextVisible &= !volumeSlider.getMouseOver() && !mixSlider.getMouseOver();
    if (positionerTextVisible) {
        // draw dark shading so any background objects are visible, but darkened
        glColor4f(0, 0, 0, 0.75f);
        positionerText.getBoundary().drawFill();
        if (positionerTextDrawEditable) {
            bool deselectAnimationJustEnded = !positionerText.getSelected() && positionerText.getSelectAnimation().isPlaying();
            positionerText.draw(glWindow, {m_x, m_y}, positionerText.getSelected() && !pathAutomationPointsGrabbedWithMouse);
            deselectAnimationJustEnded &= !positionerText.getSelectAnimation().isPlaying();
            if (deselectAnimationJustEnded) { // keep the value of the edited point visible until the deselect animation finishes
                pathAutomationPointEditableTextIndex = {-1, -1, -1};
                loopRegionBeginHasPositionerTextFocus = false;
                loopRegionEndHasPositionerTextFocus = false;
                positionerText.setTextInputRestrictor(std::make_unique<PathAutomationInputRestrictor>());
                positionerText.doubleClickSeparator = ", ";
            }
        } else
            positionerText.TextBox::draw(glWindow);
    }
//    const auto helpAndDopplerButtonBounds = combined(helpButton.getBoundary(), dopplerButton.getBoundary());
//    if (std::abs(m_x) <= 1.0 && -1.0 <= m_y &&
//        m_y < tabs.getBoundary().getBottom() &&
//        !(m_x > helpAndDopplerButtonBounds.getLeft() && m_y > helpAndDopplerButtonBounds.getBottom()) &&
//        !selectionBox.isActive()) {
//        float tVal, yVal, x, y;
//        drawEditableText = false;
//        if (pathAutomationPointIndexValid(pathAutomationPointEditableTextIndex)) {
//            const auto ptMouseIsOver = (*sources)[pathAutomationPointEditableTextIndex[0]].getPathPosPtr()->getPoint(pathAutomationPointEditableTextIndex[1]);
//            tVal = ptMouseIsOver[0];
//            yVal = ptMouseIsOver[1];
//            x = x_scale*((tVal+thing1)*thing2-1) + pixelsToNormalized(10, glWindow.width);
//            y = y_scale*(yVal*2-1) + pixelsToNormalized(3, glWindow.height);
//            positionerLook.fontStyle = Font::FontStyleFlags::bold;
//            positionerLook.color = Colour::fromFloatRGBA(1, 0, 0, 1);
//            drawEditableText = true;
//        } else if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
//            //glLineWidth(1.5);
//            // crashed here before in getPoint() when editing path auto pts, mouseOverPathAutomationPointIndex = {0, 7, 0}, out of bounds ??, pathPosPtr() null??
//            const auto ptMouseIsOver = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1]);
//            if (loopRegionBeginSelected || loopRegionEndSelected) {
//                //glColor3ub(101, 255, 145);
//                positionerLook.color = loopingRegionColor;
//            } else {
//                //glColor4f(1.0, 0.0, 0.0, 1.0);
//                positionerLook.color = Colour::fromFloatRGBA(1, 0, 0, 1);
//            }
//            tVal = ptMouseIsOver[0];
//            yVal = ptMouseIsOver[1];
//            x = x_scale*((tVal+thing1)*thing2-1.0) + pixelsToNormalized(10, glWindow.width);
//            y = y_scale*(yVal*2.0-1.0) + pixelsToNormalized(3, glWindow.height);
//            positionerLook.fontStyle = Font::FontStyleFlags::bold;
//        } else if (mouseOverLoopRegionBegin || loopRegionBeginSelected
//                   || mouseOverLoopRegionEnd || loopRegionEndSelected) {
//            //glLineWidth(1.5);
//            //glColor3ub(101, 255, 145);
//            positionerLook.color = loopingRegionColor;
//            if (mouseOverLoopRegionBegin) {
//                tVal = processor->loopRegionBegin;
//            } else if (mouseOverLoopRegionEnd) {
//                tVal = processor->loopRegionEnd;
//            }
//            //yVal = 0.5*(m_y/y_scale+1.0);
//            x = x_scale*((tVal+thing1)*thing2-1.0) + pixelsToNormalized(10, glWindow.width);
//            y = m_y + pixelsToNormalized(3, glWindow.height);
//            positionerLook.fontStyle = Font::FontStyleFlags::bold;
//        } else {
//            positionerLook.color = Colour::fromFloatRGBA(1, 1, 1, 0.9);
//            //glColor4f(1.0, 1.0, 1.0, 0.7);
//            yVal = 0.5*(m_y/y_scale+1.0);
//            tVal = (m_x/x_scale+1.0)*automationViewWidth/2.0+automationViewOffset-automationViewWidth/2.0;
//            if (yVal < 0)
//                yVal = 0;
//            if (yVal > 1)
//                yVal = 1;
//            if (tVal < 0)
//                tVal = 0;
//            x = m_x + pixelsToNormalized(10, glWindow.width);
//            y = m_y + pixelsToNormalized(3, glWindow.height);
//            positionerLook.fontStyle = Font::FontStyleFlags::plain;
//        }
//       
////        char anotherstr[20];
////        getFormattedTime(tVal, anotherstr);
////        if (mouseOverLoopRegionBegin || loopRegionBeginSelected
////            || mouseOverLoopRegionEnd || loopRegionEndSelected)
////            sprintf(str, "%s", anotherstr);
////        else
////            sprintf(str, "%s  %d%%", anotherstr, (int)(yVal*100));
////        xlen = 0;
////        for (int i = 0; i < strlen(str); ++i) {
////            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
////        }
////        xlen *= aspect*stroke_scale;
////        if (y + topMax > y_scale) {
////            y = y_scale-topMax;
////            x += 0.02;
////        }
////        if (x + xlen > 1) {
////            x = 1-xlen;
////        }
////        if (x + xlen > helpAndDopplerButtonBounds.getLeft() && y+topMax > helpAndDopplerButtonBounds.getBottom()-0.01) {
////            y = helpAndDopplerButtonBounds.getBottom()-0.01-topMax;
////        }
//        
//        std::string timeStr = getFormattedTimeString(tVal);
//        if (!(mouseOverLoopRegionBegin || loopRegionBeginSelected || mouseOverLoopRegionEnd || loopRegionEndSelected))
//            timeStr += "  " + std::to_string((int)(yVal*100)) + "%";
//        positionerText.setLook(positionerLook);
//        positionerText.setText(timeStr);
//        Box b {y + 0.06f, y, x, x + 100};
//        const auto font = positionerLook.getFont(timeStr, b, glWindow.width, glWindow.height);
//        b.setRight(x + pixelsToNormalized(font.getStringWidthFloat(timeStr), glWindow.width));
//        if (b.getRight() > helpAndDopplerButtonBounds.getLeft() && b.getTop() > helpAndDopplerButtonBounds.getBottom())
//            move(b, 0, helpAndDopplerButtonBounds.getBottom() - b.getTop());
//        else if (b.getTop() > y_scale)
//            move(b, 0, y_scale - b.getTop());
//        if (b.getRight() > 1)
//            move(b, 1 - b.getRight(), 0);
//        
////        const float dh = pixelsToNormalized(3, glWindow.height);
////        const float dw = pixelsToNormalized(10, glWindow.width);
////        const Box mouseBoundary = {m_y + dh, m_y - dh, m_x - dw, m_x + dw};
////        drawOutline(mouseBoundary);
//        //placeWithin(b, {1, -1, -1, 1}, {tabs.getBoundary(), helpAndDopplerButtonBounds, mouseBoundary});
//        positionerText.setBoundary(b);
//        
//        if (drawEditableText)
//            positionerText.draw(glWindow, {m_x, m_y});
//        else
//            positionerText.TextBox::draw(glWindow);
////        glColor4f(1, 1, 1, 1);
////        drawOutline(mousePositionText.getBoundary());
//        //drawStringStroke(str, x, y, 0.0);
//    }

    glLineWidth(1.0); // reset this
    
    // draw autoalign animation
    if (pathPtAutoAlignAnimationX.isPlaying()) {
        glColor4f(1, 1, 1, 1 - pathPtAutoAlignAnimationX.getProgress());
        glBegin(GL_LINES);
        //if (std::abs(autoAlignPos[0]) <= 1.0) {
        glVertex2f(pathPtAutoAlignX, -y_scale);
        glVertex2f(pathPtAutoAlignX, y_scale);
        //}
        glEnd();
        pathPtAutoAlignAnimationX.advance(glWindow.frameRate);
    }
    if (pathPtAutoAlignAnimationY.isPlaying()) {
        glColor4f(1, 1, 1, 1 - pathPtAutoAlignAnimationY.getProgress());
        glBegin(GL_LINES);
        //if (std::abs(autoAlignPos[1]) <= 1.0) {
        glVertex2f(-x_scale, pathPtAutoAlignY);
        glVertex2f(x_scale, pathPtAutoAlignY);
        //}
        glEnd();
        pathPtAutoAlignAnimationY.advance(glWindow.frameRate);
    }

    //showMouseDetails();
    
    // Making sure we can render 3d again
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

//void ThreeDAudioProcessorEditor::drawMouseDragging()
//{
//    const float alphaFactor = mouseDragging ? 1.0 : 1.0-mouseDragAnimation.getProgress();
//    mouseDragAnimation.advance(glWindow.frameRate);
//    glColor4f(1.0, 1.0, 1.0, 1.0*alphaFactor);
//    glBegin(GL_LINE_STRIP);
//    glVertex2f(mouseDragDownX, mouseDragDownY);
//    glVertex2f(mouseDragDownX, mouseDragCurrentY);
//    glVertex2f(mouseDragCurrentX, mouseDragCurrentY);
//    glVertex2f(mouseDragCurrentX, mouseDragDownY);
//    glVertex2f(mouseDragDownX, mouseDragDownY);
//    glEnd();
//    
//    glColor4f(0.6, 0.6, 0.6, 0.2*alphaFactor);
//    glBegin(GL_POLYGON);
//    glVertex2f(mouseDragDownX, mouseDragDownY);
//    glVertex2f(mouseDragDownX, mouseDragCurrentY);
//    glVertex2f(mouseDragCurrentX, mouseDragCurrentY);
//    glVertex2f(mouseDragCurrentX, mouseDragDownY);
//    glEnd();
//}

void ThreeDAudioProcessorEditor::drawInterpolatedPath(const int s)
{
    // if the path interp has changed since the last frame or if the local copy is empty (as is the case when the gl view is closed and opened again), then gotta recompute our locally held points for drawing the path (more cpu)
    if (pathDisplayList[s] == 0 || processor->pathChanged) {
        const auto interp = (*sources)[s].getPathPtr();
        if (interp) {
            const auto points = interp->getPoints();
            const float length = interp->getInputRange()[1] * 0.9999f;
            const int N = 20 * points.size();
            // add the glvertex() calls into display list for better performance with the cached data on later static drawing
            glDeleteLists(pathDisplayList[s], 1);
            pathDisplayList[s] = glGenLists(1);
            glNewList(pathDisplayList[s], GL_COMPILE_AND_EXECUTE);
            glBegin(GL_LINE_STRIP);
            std::vector<float> pt;
            for (int i = 0; i < N; ++i)
                if (interp->pointAt(((float)i) / ((float)(N-1)) * length, pt))
                    glVertex3f(pt[0], pt[1], pt[2]);
            glEnd();
            glEndList();
            // tell the processor/source that we have saved the changes that we are concerned about
            if (s == sources->size() - 1)
                processor->pathChanged = false;
        }
    } else
        glCallList(pathDisplayList[s]);
}

void ThreeDAudioProcessorEditor::drawLoopingRegion()
{
    const float beginX = timeValueToXPosition(processor->loopRegionBegin);
    const float endX = timeValueToXPosition(processor->loopRegionEnd);
    const float mouse_x = getMouseX();
    const float mouse_y = getMouseY();
    float mouseOverLoopBoarderX = -1.1f;
    
    // was preformed in mouseMove(), but done here to be consistent with mouse over path auto pt state determined in drawPathControl()
    if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
        const float t = processor->getPathAutomationPoints(mouseOverPathAutomationPointIndex[0])[mouseOverPathAutomationPointIndex[1]][0];
        if (loopRegionBeginSelected)
            processor->loopRegionBegin = t;
        else if (loopRegionEndSelected)
            processor->loopRegionEnd = t;
    } else if (loopRegionBeginSelected || loopRegionEndSelected) {
//                if (isMouseOverPathAutomationPointValid()) {
////                    // done in drawLoopingRegion()
////                    const float t = processor->getPathAutomationPoints(mouseOverPathAutomationPointIndex[0])[mouseOverPathAutomationPointIndex[1]][0];
////                    if (loopRegionBeginSelected)
////                        processor->loopRegionBegin = t;
////                    else if (loopRegionEndSelected)
////                        processor->loopRegionEnd = t;
//                } else {
        const float mouse_x = getMouseX();
        const float t = std::max(0.0f, (mouse_x / x_scale + 1) * pathAutomationView.getWidth() * 0.5f
                                      - (pathAutomationView.getWidth() * 0.5f - pathAutomationView.getXPosition()));
        if (loopRegionBeginSelected)
            processor->loopRegionBegin = std::max(0.0f, t);
        else if (loopRegionEndSelected)
            processor->loopRegionEnd = std::max(0.0f, t);
//                }
        if (processor->loopRegionBegin > processor->loopRegionEnd) {
            const float temp = processor->loopRegionBegin;
            processor->loopRegionBegin.store(processor->loopRegionEnd);
            processor->loopRegionEnd = temp;
        }
    }
    
    cauto alphaFactor = processor->loopingEnabled ? loopRegionToggleAnimation.getProgress() : 1 - loopRegionToggleAnimation.getProgress();
    glColor4ub(101, 255, 145, alphaFactor * 255);
    const bool mouseOverLoopRegionEnabled = processor->loopingEnabled
        && !selectionBox.isActive()
        && mouseOverPathAutomationPointIndex[0] == -1
        && std::abs(mouse_y) < y_scale
        && !helpButton.isMouseOver()
        && !dopplerButton.isMouseOver()
        && !positionerText.getBoundary().contains({mouse_x, mouse_y});
    const bool prevMouseOverLoopRegionBegin = mouseOverLoopRegionBegin;
    if (loopRegionBeginSelected || (std::abs(mouse_x - beginX) < pixelsToNormalized(4, getWidth()) && mouseOverLoopRegionEnabled)) {
        mouseOverLoopBoarderX = beginX;
        mouseOverLoopRegionBegin = true;
    } else {
        mouseOverLoopRegionBegin = false;
    }
    mouseOverLoopRegionBegin &= !volumeSlider.getMouseOver() && !mixSlider.getMouseOver();
    if (mouseOverLoopRegionBegin != prevMouseOverLoopRegionBegin)
        loopRegionBeginAnimation.restart(mouseOverLoopRegionBegin ? 0.25f : 0.5f);
    float lineWidth = mouseOverLoopRegionBegin ? 1 + 2 * loopRegionBeginAnimation.getProgress() : 3 - 2 * loopRegionBeginAnimation.getProgress();
    if (loopRegionBeginAnimation.isPlaying())
        loopRegionBeginAnimation.advance(glWindow.frameRate);
    if (lineWidth > 1.0f)
        glEnable(GL_LINE_SMOOTH);
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex2f(beginX, -y_scale);
    glVertex2f(beginX, +y_scale);
    glEnd();
    
    const bool prevMouseOverLoopRegionEnd = mouseOverLoopRegionEnd;
    if (loopRegionEndSelected || (std::abs(mouse_x - endX) < pixelsToNormalized(4, getWidth())
                                  && mouseOverLoopBoarderX == -1.1f && mouseOverLoopRegionEnabled)) {
        mouseOverLoopBoarderX = endX;
        mouseOverLoopRegionEnd = true;
    } else {
        mouseOverLoopRegionEnd = false;
    }
    mouseOverLoopRegionEnd &= !volumeSlider.getMouseOver() && !mixSlider.getMouseOver();
    if (mouseOverLoopRegionEnd != prevMouseOverLoopRegionEnd)
        loopRegionEndAnimation.restart(mouseOverLoopRegionEnd ? 0.25f : 0.5f);
    lineWidth = mouseOverLoopRegionEnd ? 1 + 2 * loopRegionEndAnimation.getProgress() : 3 - 2 * loopRegionEndAnimation.getProgress();
    if (loopRegionEndAnimation.isPlaying())
        loopRegionEndAnimation.advance(glWindow.frameRate);
    if (lineWidth > 1.0f)
        glEnable(GL_LINE_SMOOTH);
    else
        glDisable(GL_LINE_SMOOTH);
    glLineWidth(lineWidth);
    glBegin(GL_LINES);
    glVertex2f(endX, -y_scale);
    glVertex2f(endX, +y_scale);
    glEnd();
    
    // draw arrows
    if (loopRegionSelectAnimation.isPlaying())
        loopRegionSelectAnimation.advance(glWindow.frameRate);
    if (/*mouseOverLoopBoarderX > -1.0f*/(mouseOverLoopRegionBegin || mouseOverLoopRegionEnd) && processor->loopingEnabled) {
        glEnable(GL_LINE_SMOOTH);
        glLineWidth(1.5f);
        const auto arrowAlpha = mouseOverLoopRegionBegin ? loopRegionBeginAnimation.getProgress() : loopRegionEndAnimation.getProgress();
        const auto dxpx = (loopRegionBeginSelected || loopRegionEndSelected) ? 6 - 4 * loopRegionSelectAnimation.getProgress() : 2 + 4 * loopRegionSelectAnimation.getProgress();
        const auto dx = pixelsToNormalized(dxpx, glWindow.width) * arrowAlpha;
        const auto l = pixelsToNormalized(8, glWindow.width);
        const auto h = pixelsToNormalized(2, glWindow.height);
        glColor4ub(101, 255, 145, 255 * arrowAlpha);
        glBegin(GL_LINES);
        glVertex2f(mouseOverLoopBoarderX - dx, mouse_y);
        glVertex2f(mouseOverLoopBoarderX - dx - l, mouse_y);
        glVertex2f(mouseOverLoopBoarderX + dx, mouse_y);
        glVertex2f(mouseOverLoopBoarderX + dx + l, mouse_y);
        glVertex2f(mouseOverLoopBoarderX - dx - l, mouse_y);
        glVertex2f(mouseOverLoopBoarderX - dx - l/2, mouse_y + h);
        glVertex2f(mouseOverLoopBoarderX - dx - l/2, mouse_y - h);
        glVertex2f(mouseOverLoopBoarderX - dx - l, mouse_y);
        glVertex2f(mouseOverLoopBoarderX + dx + l, mouse_y);
        glVertex2f(mouseOverLoopBoarderX + dx + l/2, mouse_y + h);
        glVertex2f(mouseOverLoopBoarderX + dx + l/2, mouse_y - h);
        glVertex2f(mouseOverLoopBoarderX + dx + l, mouse_y);
        glEnd();
    }
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(1.0);
    
    // shading over loop area
    glColor4ub(101, 255, 145, 0.1f * 255 * alphaFactor);
    glBegin(GL_POLYGON);
    glVertex2f(beginX, -y_scale);
    glVertex2f(endX,   -y_scale);
    glVertex2f(endX,   +y_scale);
    glVertex2f(beginX, +y_scale);
    glEnd();
    
    if (loopRegionToggleAnimation.isPlaying())
        loopRegionToggleAnimation.advance(glWindow.frameRate);
}

inline float ThreeDAudioProcessorEditor::timeValueToXPosition(const float tVal) const
{
    const float thing1 = pathAutomationView.getWidth()*0.5-pathAutomationView.getXPosition();
    const float thing2 = 2.0/pathAutomationView.getWidth();
    return x_scale*((tVal+thing1)*thing2-1.0);
}
                  
float ThreeDAudioProcessorEditor::getMouseX() const
{
    return (2.0*getMouseXYRelative().getX())/getWidth() - 1.0;
}

float ThreeDAudioProcessorEditor::getMouseY() const
{
    return (-2.0*getMouseXYRelative().getY())/getHeight() + 1.0;
}
                
Point<float> ThreeDAudioProcessorEditor::getMousePosition() const
{
    return pixelsToNormalized(getMouseXYRelative(), getWidth(), getHeight());
}
                  
//void ThreeDAudioProcessorEditor::drawStringBitmap(void *font, char *s, float x, float y, float z) const
//{
//    glRasterPos3f(x, y, z);
//    for (int i = 0; i < strlen(s); ++i)
//        glutBitmapCharacter(font, s[i]);
//}

//void ThreeDAudioProcessorEditor::drawStringStroke(char *s, float x, float y, float z) const
//{
//    const float aspect = ((float)getHeight())/((float)getWidth());
//    const float stroke_scale = 0.00035;
//    glPushMatrix();
//    glTranslatef(x, y, z);
//    glScalef(aspect*stroke_scale, stroke_scale, stroke_scale);
//    for (int i = 0; i < strlen(s); ++i)
//        glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);
//    glPopMatrix();
//}
//void drawStringStroke(char *s, float x, float y, float z, float aspect, float scale)
//{
//    const float stroke_scale = 0.000346*scale;
//    glPushMatrix();
//    glTranslatef(x, y, z);
//    glScalef(aspect*stroke_scale, stroke_scale, stroke_scale);
//    for (int i = 0; i < strlen(s); ++i)
//        glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);
//    glPopMatrix();
//}

//std::string ThreeDAudioProcessorEditor::getFormattedTimeString(const float timeInSec) const
//{
//    char s[20];
//    getFormattedTime(timeInSec, s);
//    return std::string(s);
//}
//
//void ThreeDAudioProcessorEditor::getFormattedTime(const float timeInSec, char* str) const
//{
//    if (timeMode == 0) {
//        const int secs = std::floor(timeInSec);
//        const int mins = secs / 60 % 60;
//        const int hours = secs / 3600;
//        if (hours != 0)
//            if (std::fmod(timeInSec, 60) < 10)
//                sprintf(str, "%d:%.2d:0%.2f", hours, mins, std::fmod(timeInSec, 60));
//            else
//                sprintf(str, "%d:%.2d:%.2f", hours, mins, std::fmod(timeInSec, 60));
//        else if (mins != 0)
//            if (std::fmod(timeInSec, 60) < 10)
//                sprintf(str, "%d:0%.2f", mins, std::fmod(timeInSec, 60));
//            else
//                sprintf(str, "%d:%.2f", mins, std::fmod(timeInSec, 60));
//        else
//            sprintf(str, "%.2f", timeInSec);
//    } else if (timeMode == 1) {
//        const auto mbf = processor->getMeasuresBeatsFrac(timeInSec);
//        const int frac = static_cast<int>(std::get<2>(mbf)*1000.0);
////        if (frac < 100)
////            if (frac < 10)
////                sprintf(str, "%d|%d|00%d", std::get<0>(mbf), std::get<1>(mbf), frac);
////            else
////                sprintf(str, "%d|%d|0%d", std::get<0>(mbf), std::get<1>(mbf), frac);
////        else
//        sprintf(str, "%d|%d|%.3d", std::get<0>(mbf), std::get<1>(mbf), frac);
//    }
//}

std::string ThreeDAudioProcessorEditor::getFormattedTimeString(const float timeInSec) const
{
    if (timeMode == 0) {
        return StrFuncs::hrMinSecFromSec(timeInSec);
    } else if (timeMode == 1) {
        const auto mbf = processor->getMeasuresBeatsFrac(timeInSec);
        const int frac = static_cast<int>(std::get<2>(mbf)*1000.0);
        char str[4];
        sprintf(str, "%.3d", frac);
        return std::to_string(std::get<0>(mbf)) + "|" + std::to_string(std::get<1>(mbf)) + "|" + std::string(str);
    }
    return std::string();
}

//void ThreeDAudioProcessorEditor::drawRect(float centerX, float centerY,
//                              float deltaX, float deltaY)
//{
//    glBegin(GL_LINE_LOOP);
//
//    glVertex2f(centerX + deltaX/2.0, centerY + deltaY/2.0);
//    glVertex2f(centerX - deltaX/2.0, centerY + deltaY/2.0);
//    glVertex2f(centerX - deltaX/2.0, centerY - deltaY/2.0);
//    glVertex2f(centerX + deltaX/2.0, centerY - deltaY/2.0);
//
//    glEnd();
//}

void ThreeDAudioProcessorEditor::myResized()
{
    y_scale = 1 - pixelsToNormalized(28, getHeight());
    // make the x_scale reflect the same distance on screen as y_scale does
    x_scale = 1 - normalizedXYConvert(1 - y_scale, getHeight(), getWidth());
    pathAutomationView.setBoundary({y_scale, -y_scale, -x_scale, x_scale});
    
    // adjust time labels boundaries accordingly
    auto b = automationViewLowerTimeText.getBoundary();
    b.setTop(-y_scale);
    b.setBottom(-1);
    b.setLeft(-x_scale);
    automationViewLowerTimeText.setBoundary(b);
    b = automationViewUpperTimeText.getBoundary();
    b.setTop(-y_scale);
    b.setBottom(-1);
    b.setRight(x_scale);
    automationViewUpperTimeText.setBoundary(b);
    b = automationViewTimeText.getBoundary();
    b.setTop(-y_scale - pixelsToNormalized(1, getHeight()));
    b.setBottom(-1 + pixelsToNormalized(1, getHeight()));
    automationViewTimeText.setBoundary(b);
    
    // adjust tabs boundary
    b = tabs.getBoundary();
    b.setTop(1 - pixelsToNormalized(1, glWindow.height));
    b.setBottom(b.getTop() - pixelsToNormalized(20, glWindow.height) / tabs.getSelectedTextLook().verticalPad);
    tabs.setBoundary(b);
    
    cauto font = dopplerButton.getTextLook().getFontWithSize(16).boldened();
    cauto dopplerWidth = pixelsToNormalized(font.getStringWidthFloat(dopplerButton.getText()) / dopplerButton.getTextLook().horizontalPad, glWindow.width);
    auto top = b.getBottom() - pixelsToNormalized(3, glWindow.height);
    auto right = 1 - pixelsToNormalized(1, glWindow.width);
    auto left = right - dopplerWidth;
    b = {top, top - pixelsToNormalized(16, glWindow.height) / helpButton.getTextLook().verticalPad,
         left, right};
    helpButton.setBoundary(b);
    top = b.getBottom() - pixelsToNormalized(2, glWindow.height);
    b.setBottom(top - pixelsToNormalized(16, glWindow.height) / dopplerButton.getTextLook().verticalPad);
    b.setTop(top);
    dopplerButton.setBoundary(b);
    
    cauto gapHeight = 10;
    cauto volumeAndMixSliderHeight = 0.5f * (b.getBottom() + 1
                                     - pixelsToNormalized(55, getHeight()) // doppler slider height
                                     - 3*pixelsToNormalized(gapHeight, getHeight())
                                     - 4*pixelsToNormalized(volumeSliderTextLook.fontSize / volumeSliderTextLook.verticalPad, getHeight()));
    top = b.getBottom() - pixelsToNormalized(10, glWindow.height);
    b.setBottom(top - pixelsToNormalized(volumeSliderTextLook.fontSize / volumeSliderTextLook.verticalPad, glWindow.height));
    b.setTop(top);
    volumeSlider.getTitleTextBox().setBoundary(b);
    
    auto sliderLeft = right - dopplerWidth * 0.6666666f;
    auto sliderRight = right - dopplerWidth * 0.3333333f;
    b.setLeft(sliderLeft);
    b.setRight(sliderRight);
    top = b.getBottom() - pixelsToNormalized(4, glWindow.height);
    b.setBottom(top - volumeAndMixSliderHeight);
    b.setTop(top);
    volumeSlider.setSliderBoundary(b);
    
    b.setLeft(left + pixelsToNormalized(2, glWindow.width));
    b.setRight(right - pixelsToNormalized(2, glWindow.width));
    top = b.getBottom() - pixelsToNormalized(4, glWindow.height);
    b.setBottom(top - pixelsToNormalized(volumeSliderTextLook.fontSize / volumeSliderTextLook.verticalPad, glWindow.height));
    b.setTop(top);
    volumeSlider.getValueTextBox().setBoundary(b);

    top = b.getBottom() - pixelsToNormalized(10, glWindow.height);
    b.setBottom(top - pixelsToNormalized(mixSliderTextLook.fontSize / mixSliderTextLook.verticalPad, glWindow.height));
    b.setTop(top);
    mixSlider.getTitleTextBox().setBoundary(b);
    
    b.setLeft(sliderLeft);
    b.setRight(sliderRight);
    top = b.getBottom() - pixelsToNormalized(4, glWindow.height);
    b.setBottom(top - volumeAndMixSliderHeight);
    b.setTop(top);
    mixSlider.setSliderBoundary(b);
    
    b.setLeft(left + pixelsToNormalized(2, glWindow.width));
    b.setRight(right - pixelsToNormalized(2, glWindow.width));
    top = b.getBottom() - pixelsToNormalized(4, glWindow.height);
    b.setBottom(top - pixelsToNormalized(mixSliderTextLook.fontSize / mixSliderTextLook.verticalPad, glWindow.height));
    b.setTop(top);
    mixSlider.getValueTextBox().setBoundary(b);
    
//    // make the dopplerButton and helpButton have the same font size
//    std::vector<TextBox> texts {helpButton.getTextBox(), dopplerButton.getTextBox()};
//    makeFontsSameSize(texts, glWindow.width, glWindow.height);
    
//    dopplerSlider.getTitleTextBox().setBoundary(dopplerTitleBox);
//    dopplerSlider.getValueTextBox().setBoundary(dopplerValueBox);
{
    auto bottom = -1 + pixelsToNormalized(3, getHeight());
    auto top = bottom + pixelsToNormalized(dopplerSliderTextLook.fontSize / dopplerSliderTextLook.verticalPad, getHeight());
    dopplerTitleBox.setBottom(bottom);
    dopplerTitleBox.setTop(top);
    dopplerValueBox.setBottom(bottom);
    dopplerValueBox.setTop(top);
    dopplerSlider.horizontallyCenterAndRepositionText(dopplerTitleBox, dopplerValueBox, glWindow.width, glWindow.height, 0);
    bottom = top - pixelsToNormalized(5, getHeight());
    top = bottom + pixelsToNormalized(20, getHeight());
    auto left = -1 + pixelsToNormalized(30, getWidth());
    auto right = 1 - pixelsToNormalized(30, getWidth());
    dopplerSlider.setSliderBoundary({top, bottom, left, right});
}
    //centerAndRepositionTextBoxes(dopplerSlider.getTitleTextBox(), dopplerSlider.getValueTextBox(), {-0.9f, -0.99f, -0.8f, 0.8f}, glWindow.width, glWindow.height, 2);
    //dopplerSlider.makeTitleAndValueFontsSameSize(glWindow.width, glWindow.height);
//    // make the dopplerSlider title and valueText have the same font size
//    
//    auto helpLook = dopplerSlider. .getTextLook();
//    helpLook.fontSize = -1;
//    helpButton.setTextLook(helpLook);
//    auto dopplerLook = dopplerButton.getTextLook();
//    dopplerLook.fontSize = -1;
//    dopplerButton.setTextLook(dopplerLook);

{
    cauto fontSize = 25;
    processingModeOptions.setFontSize(fontSize);
    float len = 0;
    for (const auto& b : processingModeOptions.getTextBoxes())
        len += pixelsToNormalized(b.getTextLength(glWindow.width, glWindow.height), glWindow.width);
    len /= processingModeNormalLook.horizontalPad;
    auto top = tabs.getBoundary().getBottom() - pixelsToNormalized(30, getHeight());
    auto bottom = top - pixelsToNormalized(fontSize / processingModeNormalLook.verticalPad, getHeight());
    auto left = -len * 0.5f;
    auto right = len * 0.5f;
    processingModeOptions.setBoundary({top, bottom, left, right});
    
    processingModeHelpLook.fontSize = 18;
    processingModeHelp.setLook(&processingModeHelpLook);
    top = bottom - pixelsToNormalized(10, getHeight());
    bottom = top - pixelsToNormalized(20 * processingModeHelpLook.fontSize, getHeight());
    left = -1 + pixelsToNormalized(50, getWidth());
    right = -left;
    processingModeHelp.setBoundary({top, bottom, left, right});
}
{
    cauto fontSize = 25;
    cauto look = websiteButton.getTextLook();
    cauto bottom = -1 + pixelsToNormalized(25, getHeight());
    cauto top = bottom + pixelsToNormalized(fontSize / look.verticalPad, getHeight());
    cauto len = pixelsToNormalized(look.getFontWithSize(fontSize).getStringWidthFloat(websiteButton.getText()), getWidth());
    cauto left = len * -0.5f;
    cauto right = len * 0.5f;
    websiteButton.setBoundary({top, bottom, left, right});
}
//    b = positionerText.getBoundary();
//    b.setTop(b.getBottom() + pixelsToNormalized(16, getHeight()) / positionerText.getLook()->verticalPad);
//    positionerText.setBoundary(b);
    
    updatePositioner3DTextValueAndPosition();
    reindexPathIndexTexts(); // think this fixed it the crash below
    //repositionPathIndexTexts(); // crashed here when loading new plugin setting on settings page and then clicked on path automation tab
    
    repositionAxis3dLabels();
}

void ThreeDAudioProcessorEditor::updatePositioner3DTextValueAndPosition() {
    if (positionerText3D.isVisible()) {
        positionerText3D.releaseFocus(); // don't want to be able to edit text if it is auto-updating by its associated object being moved around
        Sources* sources = nullptr;
        const Locker lock (processor->sources.get(sources));
        if (!sources)
            return;
        std::array<float, 3> xyz {10101, 0, 0};
        float xy[2] = {10101, 0};
        if (positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex >= 0) {
            cauto vxyz = (*sources)[positionerText3DID.sourceIndex].getPathPtr()->getPoint(positionerText3DID.pathPtIndex);
            xyz = {vxyz[0], vxyz[1], vxyz[2]};
            cauto b = pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex]->getBoundary();
            xy[0] = b.centerX();
            xy[1] = b.getTop() + pixelsToNormalized(3, glWindow.height);
        } else if (positionerText3DID.sourceIndex >= 0) {
            xyz = (*sources)[positionerText3DID.sourceIndex].getPosXYZ();
            if (!to2D({xyz[0], xyz[1], xyz[2]}, xy))
                xy[0] = 10101;
        }
        if (xy[0] != 10101) {
            positionerText3D.setXYZ(xyz);
            positionerText3D.setBoundary({xy[1], xy[1], xy[0], xy[0]}); // to set bottom-left
            positionerText3D.fitBoundaryToFontSize();
            auto b = positionerText3D.getBoundary();
            adjustSoVisible(b);
            positionerText3D.setBoundary(b);
            positionerText3D.raeText.repaint();
        }
    }
}

// everything drawn by the GL
void ThreeDAudioProcessorEditor::renderOpenGL()
{
    if (!isShowing()) // no need to draw if window is not visible
        return;
    // need to synchronize data that is accessed for rendering from edits that happen on the message thread through click, keypress, ect. callbacks
	const std::lock_guard<decltype(glLock)> lockGL (glLock); // caused deadlock/crash issues with the blocking lock in resized(), so got rid of the one in resized()
//    const std::unique_lock<std::mutex> lockGL (glLock);  // this causes huge stutters in GUI's rendering, especially in Tracktion
//    if (!lockGL.owns_lock())
//        return;
    
    // for Windows we gotta reset this for every frame (just doing it in resized() don't work for some reason...)
//#ifdef WIN32
    // just resize the proj matrix here b/c doing it in resize requires locking to synchronize the gl and juce message threads
    const int w = getWidth();
    const int h = getHeight();
    // Tell OpenGL how to convert from coordinates to pixel values
    glViewport(0, 0, w, h);
    // Switch to setting the camera perspective
    glMatrixMode(GL_PROJECTION);
    // Reset the camera matrix
    glLoadIdentity();
    // Set the camera perspective
    //gluPerspective
    glpp::perspective (45.0,                  //The camera angle
                   (double)w / (double)h, //The width-to-height ratio
                   0.05,                 //The near z clipping coordinate
                   200.0);              //The far z clipping coordinate
    // switch back to drawing matrix
    glMatrixMode(GL_MODELVIEW);
//#endif
    
    glWindow.checkResized(w, h);
    if (glWindow.resized)
        myResized();
    
    // reset the playing state of the processor if processBlock() not called in a while
    processor->resetPlaying(glWindow.frameRate);
        
    const Point<float> mousePos = {getMouseX(), getMouseY()};
    
    // lock us a copy of the sources for drawing
    sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        
        if (processor->presetJustLoaded) {
            if (processor->dopplerOn != dopplerButton.isDown())
                dopplerButton.press();
            resizePathPtsPrevState();
            reindexPathIndexTexts();
            tryHidePositioner3D();
            //updatePositioner3DTextValueAndPosition();
//            processor->presetJustLoaded = false;
        }
        
        // reset this stuff for detecting mouse over selectable objects so it has to be reset during this render cycle for there to be any action on those selectable objects
        mouseOverSourceIndex = -1;
        mouseOverPathPointSourceIndex = -1;
        mouseOverPathPointIndex = -1;
        
//        // advance animation state
//        if (animationOn[0]) {
//            animationTime[0] += getTimerInterval(0) / glWindow.frameRate;// * 0.0001f;
//            if (animationTime[0] > animationDuration) {
//                animationTime[0] = 0;
//                autoAlignPos[0] = 1.1;
//                autoAlignPos[1] = 1.1;
//                animationOn[0] = false;
//            }
//        }
//        if (animationOn[1]) {
//            animationTime[1] += getTimerInterval(0) / glWindow.frameRate;//* 0.001f;
//            if (animationTime[1] > animationDuration) {
//                animationTime[1] = 0;
//                autoAlignPos[0] = 1.1;
//                autoAlignPos[1] = 1.1;
//                animationOn[1] = false;
//            }
//        }
        
        // clear screen
        glClear(GL_COLOR_BUFFER_BIT);
        
        // enable blending for translucency depending on alpha val (4th val of glcolor4f)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        switch (processor->displayState) {
            case DisplayState::MAIN:
                drawMain();
                break;
            case DisplayState::PATH_AUTOMATION:
                drawPathControl();
                break;
            case DisplayState::SETTINGS:
            {
                // set up 2d projection for HUD
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
                glOrtho(-1, 1, -1, 1, -1.0, 10.0);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
                
                processingModeOptions.setAutoDetected(processor->isHostRealTime ? 0 : 1);
                processingModeOptions.draw(glWindow, mousePos);
                glColor3f(1, 1, 1);
                processingModeOptions.getTextBoxes()[processingModeOptions.getSelected()].getBoundary().drawOutline();
                glColor4f(1, 1, 1, 0.4f);
                if (processingModeOptions.autoDetectSelected())
                    processingModeOptions.getTextBoxes()[processingModeOptions.getAutoDetected()].getBoundary().drawOutline();
                int newProcessingModeHelpIndex = 0;
                if (processingModeOptions.getMouseOver() >= 0)
                    newProcessingModeHelpIndex = processingModeOptions.getMouseOver();
                else
                    newProcessingModeHelpIndex = processingModeOptions.getSelected();
                if (newProcessingModeHelpIndex != currentProcessingModeHelpIndex) {
                    currentProcessingModeHelpIndex = newProcessingModeHelpIndex;
                    processingModeHelp.setText(processingModeHelpText[currentProcessingModeHelpIndex]);
                }
                processingModeHelp.draw(glWindow);
                
                //etb.draw(glWindow, mousePos);
                
                websiteButton.draw(glWindow, mousePos);
                //websiteMessage.draw(glWindow);
                
                // Making sure we can render 3d again
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
                break;
            }
            case DisplayState::NUM_DISPLAY_STATES:
                break;
        }
        
        // set up 2d projection for HUD
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
        glOrtho(-1, 1, -1, 1, -1.0, 10.0);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
        
        // draw 2d stuff here ...
        
        tabs.mouseOverEnabled = !selectionBox.isActive()/*mouseDragging*/ && !loopRegionBeginSelected && !loopRegionEndSelected && !pathAutomationPointsGrabbedWithMouse;
        tabs.draw(glWindow, mousePos);
        
        if (processor->displayState != DisplayState::SETTINGS) {
            cauto mouseOverEnabled =
            dopplerButton.mouseOverEnabled = !selectionBox.isActive()/*mouseDragging*/
                                          && !loopRegionBeginSelected
                                          && !loopRegionEndSelected
                                          && !pathAutomationPointsGrabbedWithMouse;
            dopplerButton.draw(glWindow, mousePos);
            
            if (processor->presetJustLoaded) {
                // these are to update slider values if a preset is loaded and changes the values
                volumeSlider.setValue(processor->wetOutputVolume + processor->dryOutputVolume);
                mixSlider.setValue(100.0f * processor->wetOutputVolume / volumeSlider.getValue());
            }
            
            volumeSlider.draw(glWindow, mousePos, mouseOverEnabled);
            mixSlider.draw(glWindow, mousePos, mouseOverEnabled);
            drawHelp();
        }
        
//        // draw some lines around the window boarder to help make it more visible
//        glColor4f(1, 1, 1, 1.f);
//        //glLineWidth(2);
//        glBegin(GL_LINE_STRIP);
//        const float w = 1 - pixelsToNormalized(1, glWindow.width);
//        const float h = 1 - pixelsToNormalized(1, glWindow.height);
//        glVertex2f(-w, h);
//        glVertex2f(-w, -h);
//        glVertex2f(w, -h);
//        glVertex2f(w, h);
//        glEnd();
//        //glLineWidth(1);
        
        // ehh....
//        const Box windowBoundary;
//        const bool mouseOver = windowBoundary.contains(mousePos);
//        if (prevMouseOver != mouseOver)
//            mouseOverWindowAnimation.restart();
//        prevMouseOver = mouseOver;
//        const bool mouseOverAnimationPlaying = mouseOverWindowAnimation.isPlaying();
//        if (mouseOverAnimationPlaying || !mouseOver) {
//            float alpha = 0.1f;
//            if (mouseOverAnimationPlaying) {
//                if (mouseOver)
//                    alpha *= (1 - mouseOverWindowAnimation.getProgress());
//                else
//                    alpha *= mouseOverWindowAnimation.getProgress();
//                mouseOverWindowAnimation.advance(glWindow.frameRate);
//            }
////            else {
//////                if (mouseOver)
//////                    alpha = 0;
//////                else
////                    alpha = 0.1f;
////            }
//            glColor4f(0, 0, 0, alpha);
//            windowBoundary.drawFill();
//        }
        
        // Making sure we can render 3d again
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        
//        // needed for deciding when to draw the pathPos interp
//        prevAutomationViewOffset = automationViewOffset;
//        prevAutomationViewWidth = automationViewWidth;
        
        // Windows no like
        // swap buffers for double buffering
        //glutSwapBuffers();
    }

    // doing this at the end of drawing the frame b/c positionerTextMouseMoved() depends on the mouse over checking that happens when drawing
    if (mouseX != getMouseX() || mouseY != getMouseY())
        myMouseMoved();
    
    glWindow.saveResized();
    
    processor->presetJustLoaded = false;
}


void ThreeDAudioProcessorEditor::timerCallback(const int timerID)
{
    // made GL rendering jerky and slow, probably not necessary
    //const std::lock_guard<std::mutex> lockGL (glLock);
    
    switch (timerID) {
        case 0: // tell the gl to repaint the scene on the interval that timer0 is set to
            if (processor != nullptr /*&& glInited*/) {
                openGLContext.triggerRepaint();
            }
            //repaint(); to call JUCE Component::paint()
            break;
        case 1: // timer1 is for checking if a double click has happened within timer1's interval after a mouseUp() event
            if (!mouseDoubleClicked && mouseJustDown /*&& !mouseClickedOnSomething*/ && !selectionBox.isActive()/*mouseDragging*/) {
                // a single click in space deselects everything
                processor->selectAllSources(false);
            }
            if (processor->displayState == DisplayState::MAIN) {
                // hide positionerText3D if its target is not selected anymore
                tryHidePositioner3D();
            }
            //mouseClickedOnSomething = false; // cannot remember why i used this, it seems to only make you need to click twice in empty space to deselect all...
            mouseJustDown = false;
            mouseDoubleClicked = false;
            // stop the double click timer until mouseUp() happens again
            stopTimer(1);
            break;
        case 2: // timer2 is for increasing speed of arrow key controls (adjusted in keyPressed()) if they are held down
            stopTimer(2);
            arrowKeySpeedFactor = initialArrowKeySpeedFactor;
            break;
        default:
            break;
    }
}

void ThreeDAudioProcessorEditor::tryHidePositioner3D() {
    if (positionerText3DID.sourceIndex != -1) {
        if (positionerText3DID.pathPtIndex != -1 &&
            !processor->getPathPointSelected(positionerText3DID.sourceIndex, positionerText3DID.pathPtIndex)) {
            positionerText3D.hide();
            positionerText3DID = {-1, -1};
        } else if (!processor->getSourceSelected(positionerText3DID.sourceIndex)) {
            positionerText3D.hide();
            positionerText3DID = {-1, -1};
        }
    }
}

void ThreeDAudioProcessorEditor::positionerTextMouseMoved(const bool forceRefreshText)
{
    const auto m_x = getMouseX();
    const auto m_y = getMouseY();
    // set position and look for the positionerText
    const auto helpAndDopplerButtonBounds = helpButton.getBoundary().combinedWith(dopplerButton.getBoundary());
    if (std::abs(m_x) <= 1 && -1 <= m_y &&
        m_y < tabs.getBoundary().getBottom() &&
        !(m_x > helpAndDopplerButtonBounds.getLeft() && m_y > helpAndDopplerButtonBounds.getBottom()) &&
        !selectionBox.isActive()) {
        positionerTextVisible = true;
        positionerTextDrawEditable = false;
        float tVal, yVal, x, y;
        const float thing1 = pathAutomationView.getWidth() * 0.5f - pathAutomationView.getXPosition();
        const float thing2 = 2.0f / pathAutomationView.getWidth();
        if (loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus) {
            goto LOOP_REGION_STUFF;
        } else if (pathAutomationPointIndexValid(pathAutomationPointEditableTextIndex)) {
            const auto ptMouseIsOver = (*sources)[pathAutomationPointEditableTextIndex[0]].getPathPosPtr()->getPoint(pathAutomationPointEditableTextIndex[1]);
            tVal = ptMouseIsOver[0];
            yVal = ptMouseIsOver[1];
            x = x_scale*((tVal+thing1)*thing2-1) + pixelsToNormalized(10, glWindow.width);
            y = y_scale*(yVal*2-1) + pixelsToNormalized(3, glWindow.height);
            positionerLook.fontStyle = Font::FontStyleFlags::bold;
            positionerLook.color = Colour::fromFloatRGBA(1, 0, 0, 1);
            positionerTextDrawEditable = true;
        } else if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
            const auto ptMouseIsOver = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1]);
            if (loopRegionBeginSelected || loopRegionEndSelected) {
                positionerLook.color = Colour::fromRGBA(101, 255, 145, 255);
            } else {
                positionerLook.color = Colour::fromFloatRGBA(1, 0, 0, 1);
            }
            tVal = ptMouseIsOver[0];
            yVal = ptMouseIsOver[1];
            x = x_scale * ((tVal + thing1) * thing2 - 1) + pixelsToNormalized(10, glWindow.width);
            y = y_scale * (yVal * 2 - 1) + pixelsToNormalized(3, glWindow.height);
            positionerLook.fontStyle = Font::FontStyleFlags::bold;
            positionerTextDrawEditable = true;
        } else if (processor->loopingEnabled && (mouseOverLoopRegionBegin || loopRegionBeginSelected
                   || mouseOverLoopRegionEnd || loopRegionEndSelected)
                   /*|| loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus*/) {
        LOOP_REGION_STUFF:
            //positionerText.minimumWidth = 0;
            positionerLook.color = Colour::fromRGBA(101, 255, 145, 255);
            if (loopRegionBeginHasPositionerTextFocus || (mouseOverLoopRegionBegin && !loopRegionEndHasPositionerTextFocus))
                tVal = processor->loopRegionBegin;
            else if (loopRegionEndHasPositionerTextFocus || mouseOverLoopRegionEnd)
                tVal = processor->loopRegionEnd;
            x = x_scale*((tVal+thing1)*thing2-1) + pixelsToNormalized(10, glWindow.width);
            if (!(loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus) || forceRefreshText)
                y = m_y + pixelsToNormalized(3, glWindow.height);
            else
                y = positionerText.getBoundary().getBottom();
            positionerLook.fontStyle = Font::FontStyleFlags::bold;
            positionerTextDrawEditable = true;
        } else {
            positionerLook.color = Colour::fromFloatRGBA(1, 1, 1, 0.9f);
            yVal = 0.5f*(m_y/y_scale+1);
            tVal = (m_x/x_scale+1)*pathAutomationView.getWidth()*0.5f+pathAutomationView.getXPosition()-pathAutomationView.getWidth()*0.5f;
            if (yVal < 0)
                yVal = 0;
            if (yVal > 1)
                yVal = 1;
            if (tVal < 0)
                tVal = 0;
            x = m_x + pixelsToNormalized(10, glWindow.width);
            y = m_y + pixelsToNormalized(3, glWindow.height);
            positionerLook.fontStyle = Font::FontStyleFlags::plain;
        }
        positionerText.setLook(positionerLook);
        if (!positionerText.getSelected() || forceRefreshText) {
            auto text = getFormattedTimeString(tVal);
            if (!(mouseOverLoopRegionBegin || loopRegionBeginSelected || mouseOverLoopRegionEnd || loopRegionEndSelected || loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus) && positionerText.getTextInputRestrictor()->getID() == "PathAutomation") {
//            if (!(!positionerText.getSelected() && (mouseOverLoopRegionBegin || loopRegionBeginSelected || mouseOverLoopRegionEnd || loopRegionEndSelected || loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus))) // wtf was this shit?
                text += dynamic_cast<const PathAutomationInputRestrictor*>(positionerText.getTextInputRestrictor())->divString + /*StrFuncs::roundedFloatString(yVal * 100);*/ std::to_string(yVal * 100);// + "%";
            }
            positionerText.setText(text);
        }
        Box b {y + pixelsToNormalized(positionerText.getLook()->fontSize, glWindow.height) / positionerText.getLook()->verticalPad,
               y, x, std::numeric_limits<float>::infinity()};
        cauto font = positionerLook.getFont(positionerText.getText(), b, glWindow.width, glWindow.height);
        b.setRight(x + pixelsToNormalized(font.getStringWidthFloat(positionerText.getText()) + 4, glWindow.width));
        positionerText.minimumWidth = 0; // so that boundary width will be set to the text length
        positionerText.setBoundary(b);
        boundsCheckPositionerText();
    } else if (positionerText.getSelected()) {
        positionerTextVisible = true;
    } else {
        positionerTextVisible = false;
    }
}

void ThreeDAudioProcessorEditor::boundsCheckPositionerText() noexcept
{
    auto b = positionerText.getBoundary();
    adjustSoVisible(b);
    positionerText.setBoundary(b);
}

void ThreeDAudioProcessorEditor::adjustSoVisible(Box& b) noexcept
{
    cauto helpAndDopplerButtonBounds = helpButton.getBoundary().combinedWith(dopplerButton.getBoundary());
    cauto px3w = pixelsToNormalized(3, getWidth());
    cauto px3h = pixelsToNormalized(3, getHeight());
    //cauto px5h = pixelsToNormalized(5, getHeight());
    if (b.getTop() > tabs.getBoundary().getBottom() - px3h)
        b.move(0, tabs.getBoundary().getBottom() - px3h - b.getTop());
    if (b.getRight() > helpAndDopplerButtonBounds.getLeft() - px3w)
        b.move(helpAndDopplerButtonBounds.getLeft() - px3w - b.getRight(), 0);
    if (b.getBottom() < -1 + px3h)
        b.move(0, -1 + px3h - b.getBottom());
    if (b.getLeft() < -1 + px3w)
        b.move(-1 + px3w - b.getLeft(), 0);
//    if (b.getRight() > helpAndDopplerButtonBounds.getLeft() - px3w && b.getTop() > helpAndDopplerButtonBounds.getBottom() - px3h)
//        b.move(0, helpAndDopplerButtonBounds.getBottom() - px3h - b.getTop());
//    else if (b.getTop() > tabs.getBoundary().getBottom() - px5h)
//        b.move(0, tabs.getBoundary().getBottom() - px5h - b.getTop());
//    else if (b.getBottom() < -1 + px3h)
//        b.move(0, -1 + px3h - b.getBottom());
//    if (b.getRight() > 1 - px3w)
//        b.move(1 - px3w - b.getRight(), 0);
//    else if (b.getLeft() < -1 + px3w)
//        b.move(-1 + px3w - b.getLeft(), 0);
}

void ThreeDAudioProcessorEditor::mouseEnter(const MouseEvent & event)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    switch (processor->displayState) {
        case DisplayState::MAIN:
            break;
        case DisplayState::PATH_AUTOMATION:
            positionerTextVisible = true;
            break;
        case DisplayState::SETTINGS:
            break;
    }
    //mouseOverWindowAnimation.restart();
}

void ThreeDAudioProcessorEditor::mouseExit(const MouseEvent & event)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    switch (processor->displayState) {
        case DisplayState::MAIN:
            break;
        case DisplayState::PATH_AUTOMATION:
            if (!positionerText.getSelected())
                positionerTextVisible = false;
            break;
        case DisplayState::SETTINGS:
            break;
    }
}

void ThreeDAudioProcessorEditor::myMouseMoved()
{
    mouseX = getMouseX();
    mouseY = getMouseY();
//    switch (processor->displayState) {
//        case DisplayState::MAIN:
//            break;
//        case DisplayState::PATH_AUTOMATION:
//            break;
//        case DisplayState::SETTINGS:
//            break;
//        default:
//            break;
//    }
   // if (!positionerText.getSelected())
    positionerTextMouseMoved();
}

void ThreeDAudioProcessorEditor::mouseMove(const MouseEvent & event)
{
    // there was a deadlock here once when the GL thread held this lock as well...
    const std::lock_guard<decltype(glLock)> lockGL (glLock);
    
    switch (processor->displayState) {
        case DisplayState::MAIN:
            break;
        case DisplayState::PATH_AUTOMATION:
        {
            //positionerTextMouseMoved();
            
            if (pathAutomationPointsGrabbedWithMouse && pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                const float mouse_x = getMouseX();
                const float mouse_y = getMouseY();
                const float t = (mouse_x / x_scale + 1.0) * pathAutomationView.getWidth()*0.5
                                - (pathAutomationView.getWidth()*0.5-pathAutomationView.getXPosition());
                const float y = (mouse_y / y_scale + 1.0) * 0.5;
                processor->moveSelectedPathAutomationPointsTo(mouseOverPathAutomationPointIndex[0],
                                                              mouseOverPathAutomationPointIndex[1],
                                                              mouseOverPathAutomationPointIndex[2], t, y);
                if (event.mods.isAltDown() && !(event.mods.isCommandDown() || event.mods.isCtrlDown()))
                    autoAlignAutomationPoints(true, true, false);
            }
//            else
//            if (loopRegionBeginSelected || loopRegionEndSelected) {
////                if (isMouseOverPathAutomationPointValid()) {
//////                    // done in drawLoopingRegion()
//////                    const float t = processor->getPathAutomationPoints(mouseOverPathAutomationPointIndex[0])[mouseOverPathAutomationPointIndex[1]][0];
//////                    if (loopRegionBeginSelected)
//////                        processor->loopRegionBegin = t;
//////                    else if (loopRegionEndSelected)
//////                        processor->loopRegionEnd = t;
////                } else {
//                const float mouse_x = getMouseX();
//                const float t = std::max(0.0, (mouse_x / x_scale + 1.0) * pathAutomationView.getWidth()*0.5
//                                              - (pathAutomationView.getWidth()*0.5-pathAutomationView.getXPosition()));
//                if (loopRegionBeginSelected)
//                    processor->loopRegionBegin = std::max((float)0.0, t);
//                else if (loopRegionEndSelected)
//                    processor->loopRegionEnd = std::max((float)0.0, t);
////                }
//                if (processor->loopRegionBegin > processor->loopRegionEnd) {
//                    const float temp = processor->loopRegionBegin;
//                    processor->loopRegionBegin.store(processor->loopRegionEnd);
//                    processor->loopRegionEnd = temp;
//                }
//            }
            break;
        }
        case DisplayState::SETTINGS:
            //processingModeOptions.updateFromMouse(getMouseX(), getMouseY(), false, windowAspectRatio);
            //processingModeText.loadText(processingModeOptions.getHelpText());
            break;
        default:
            break;
    }
//    if (!mouseDragging && !loopRegionBeginSelected && !loopRegionEndSelected && !pathAutomationPointsGrabbedWithMouse)
//        tabs.updateFromMouse(getMouseX(), getMouseY(), false);
}

void ThreeDAudioProcessorEditor::mouseDrag(const MouseEvent& event)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    
//    const float mouse_x = getMouseX();
//    const float mouse_y = getMouseY();
//    
    cauto mouseDown = pixelsToNormalized(event.getMouseDownPosition(), glWindow.width, glWindow.height);
    cauto mouseCurrent = pixelsToNormalized(event.getPosition(), glWindow.width, glWindow.height);
    
    if (processor->displayState == DisplayState::MAIN) {
        if (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1 &&
            ((EditableTextBox*)(pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex].get()))->mouseDragged(mouseDown, mouseCurrent))
        {}
        else if (positionerText3D.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (dopplerSlider.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (volumeSlider.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (mixSlider.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (event.getDistanceFromDragStart() > 3)  //true; (this used to work when mouseDrag() was not called before any old mouseUp() event, prior to JUCE 4)
        {
            mouseDragged(selectionBox, mouseDown, mouseCurrent);
        }
    } else if (processor->displayState == DisplayState::PATH_AUTOMATION) {
        if (positionerText.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (volumeSlider.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (mixSlider.mouseDragged(mouseDown, mouseCurrent)) {}
        else if (event.getDistanceFromDragStart() > 3  //true; (this used to work when mouseDrag() was not called before any old mouseUp() event, prior to JUCE 4)
             && !pathAutomationPointsGrabbedWithMouse && !loopRegionBeginSelected && !loopRegionEndSelected) { // bad things can happen if we mouse drag when the path pts are grabbed {
            mouseDragged(selectionBox, mouseDown, mouseCurrent);
        }
    } else if (processor->displayState == DisplayState::SETTINGS) {
        //if (etb.mouseDragged(mouseDown, mouseCurrent)) {}
    }
//    if (positionerText3D.mouseDragged(mouseDown, mouseCurrent)) {}
//    else if (positionerText.mouseDragged(mouseDown, mouseCurrent)) {}
//    else if (processor->displayState == DisplayState::MAIN
//        && dopplerSlider.mouseDragged(mouseDown, mouseCurrent)) {}
//    else if (processor->displayState == DisplayState::SETTINGS
//             && etb.mouseDragged(mouseDown, mouseCurrent)) {}
//    else if (event.getDistanceFromDragStart() > 3  //true; (this used to work when mouseDrag() was not called before any old mouseUp() event, prior to JUCE 4)
//             && !pathAutomationPointsGrabbedWithMouse && mouseCurrent.y < tabs.getBoundary().getBottom() && !loopRegionBeginSelected && !loopRegionEndSelected// bad things can happen if we mouse drag when the path pts are grabbed
//             /*&& !dopplerSlider.getMouseOver()*/) {
//        mouseDragged(selectionBox, mouseDown, mouseCurrent);
//        //positionerTextVisible = false;
//    }
    
//    else if (mouseDragging/*area(selectionBox) > 0*/) {
//        // updating current mouse drag bounds
//        selectionBox.setLeft(mouseCurrent.x);
//        selectionBox.setTop(mouseCurrent.y);
////        mouseDragCurrentX = mouse_x;
////        mouseDragCurrentY = mouse_y;
//    } else {
//        // first mouseDrag callback of mouse drag event
//        mouseDragging = event.getDistanceFromDragStart() > 1  //true; (this used to work when mouseDrag() was not called before any old mouseUp() event, prior to JUCE 4)
//            && !pathAutomationPointsGrabbedWithMouse && mouseCurrent.y < tabs.getBoundary().getBottom() // bad things can happen if we mouse drag when the path pts are grabbed
//            && !dopplerSlider.getMouseOver();
//        if (mouseDragging)
//            selectionBox = {mouseDown.y, mouseCurrent.y, mouseDown.x, mouseCurrent.x};
////        mouseDragDownX = mouse_x;
////        mouseDragDownY = mouse_y;
////        mouseDragCurrentX = mouse_x;
////        mouseDragCurrentY = mouse_y;
//    }
}

void ThreeDAudioProcessorEditor::mouseUp(const MouseEvent & event)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
 
    sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        
        cauto m_x = getMouseX();
        cauto m_y = getMouseY();
        
        if (!selectionBox.isActive()/*mouseDragging*/ && !loopRegionBeginSelected && !loopRegionEndSelected && !pathAutomationPointsGrabbedWithMouse) {
            cint newDisplayState = tabs.mouseClicked();
            if (newDisplayState != -1) {
                processor->displayState = (DisplayState)newDisplayState;
                if (processor->displayState == DisplayState::PATH_AUTOMATION)
                    processor->makeSourcesVisibleForPathAutomationView();
                if (helpButton.isDown())
                    loadHelpText();
                    //repaint(); // tell the plugin editor to call paint() soon again when it can so that the help/doppler text state can be updated
                glWindow.resized = true; // make sure that the new view's text is properly scaled
                for (auto& x : pathDisplayList)
                    x = 0;
                goto SKIP;
            }
            if (processor->displayState != DisplayState::SETTINGS) {
                if (helpButton.mouseClicked()) {
                    if (helpButton.isDown())
                        loadHelpText();
                    goto SKIP;
                }
                if (dopplerButton.mouseClicked()) {
                    processor->toggleDoppler();
                    if (dopplerButton.isDown()) { // make set value animation happen
                        dopplerSlider.setValue(dopplerSlider.getValue());
                        dopplerSlider.repaint(); // and make sure 
                    }
                    goto SKIP;
                }
                if (volumeSlider.mouseClicked(getMousePosition())) {
                    cauto v = 0.01f * mixSlider.getValue();
                    processor->wetOutputVolume = v * volumeSlider.getValue();
                    processor->dryOutputVolume = (1 - v) * volumeSlider.getValue();
                    // might need to release the focus
                    mixSlider.mouseClicked(getMousePosition());
                    dopplerSlider.mouseClicked(getMousePosition());
                    positionerText.mouseClicked();
                    positionerText3D.mouseClicked();
                    if (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1)
                        ((EditableTextBox*)(pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex].get()))->mouseClicked();
                    goto SKIP;
                }
                if (mixSlider.mouseClicked(getMousePosition())) {
                    cauto v = 0.01f * mixSlider.getValue();
                    processor->wetOutputVolume = v * volumeSlider.getValue();
                    processor->dryOutputVolume = (1 - v) * volumeSlider.getValue();
//                    processor->dryOutputVolume = 1.0f - v;
//                    processor->wetOutputVolume = v;
                    // might need to release the focus
                    dopplerSlider.mouseClicked(getMousePosition());
                    positionerText.mouseClicked();
                    positionerText3D.mouseClicked();
                    if (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1)
                        ((EditableTextBox*)(pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex].get()))->mouseClicked();
                    goto SKIP;
                }
            }
        }
    
        switch (processor->displayState) {
            case DisplayState::MAIN:
            {
                auto reindexPathPts = false;
                if (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1 &&
                    ((EditableTextBox*)(pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex].get()))->mouseClicked()) {
                    positionerText3D.mouseClicked(); // to release focus if necessary
                    dopplerSlider.getValueTextBox().mouseClicked();
                } else if (positionerText3D.mouseClicked()) {
                    dopplerSlider.getValueTextBox().mouseClicked(); // to release focus if necessary
                } else if (event.mods.isRightButtonDown() && (mouseOverSourceIndex != -1 || (mouseOverPathPointIndex != -1 && mouseOverPathPointSourceIndex != -1))) {
                    // right clicking on a source or source's path point
                    std::array<float, 3> xyz;
                    TextLook *look;
                    float xy[2];
                    if (mouseOverSourceIndex != -1) {
                        xyz = (*sources)[mouseOverSourceIndex].getPosXYZ();
                        to2D ({xyz[0], xyz[1], xyz[2]}, xy);
                        look = &sourcePositionerTextLook;
                        processor->setSourceSelected(mouseOverSourceIndex, true);
                        positionerText3DID = {mouseOverSourceIndex, -1};
//                        positionerText3D.raeText.setHightColor(look->color.withAlpha(0.3f));
//                        positionerText3D.xyzText.setHightColor(look->color.withAlpha(0.3f));
                    } else {
                        cauto vxyz = (*sources)[mouseOverPathPointSourceIndex].getPathPtr()->getPoint(mouseOverPathPointIndex);
                        xyz = {vxyz[0], vxyz[1], vxyz[2]};
                        look = &sourcePathPointPositionerTextLook;
                        processor->setPathPointSelectedState(mouseOverPathPointSourceIndex, mouseOverPathPointIndex, true);
                        positionerText3DID = {mouseOverPathPointSourceIndex, mouseOverPathPointIndex};
                        cauto b = pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex]->getBoundary();
                        xy[0] = b.centerX();
                        xy[1] = b.getTop() + pixelsToNormalized(3, glWindow.height);
//                        positionerText3D.raeText.setHightColor(Colour::fromRGBA(255, 2, 229, 255).withAlpha(0.3f));
//                        positionerText3D.xyzText.setHightColor(Colour::fromRGBA(255, 2, 229, 255).withAlpha(0.3f));
                    }
                    Box boundary (xy[1], xy[1], xy[0], xy[0]);
                    positionerText3D.show(boundary, xyz, look);
                    boundary = positionerText3D.getBoundary();
                    adjustSoVisible(boundary);
                    positionerText3D.setBoundary(boundary);
                    reindexPathPts = true;
                } else if (dopplerSlider.mouseClicked({getMouseX(), getMouseY()})) {
                    processor->setSpeedOfSound(dopplerSlider.getValue());
                } else if (selectionBox.isActive()) {
                    // select the sources in the selection region and mark em as selected
                    std::array<float, 3> xyz;
                    for (int i = 0; i < sources->size(); ++i) {
                        bool pathPtsSelected = false;
                        if ((*sources)[i].getSourceSelected()) {
                            // check and see if any of the selected sources' path points are inside selected region
                            cauto pts = (*sources)[i].getPathPoints();
                            for (int j = 0; j < pts.size(); ++j) {
                                xyz[0] = pts[j][0];
                                xyz[1] = pts[j][1];
                                xyz[2] = pts[j][2];
                                float poser[3] = {xyz[0], xyz[1], xyz[2]};
                                if (pointInsideSelectRegion(poser)) {
                                    // if a point is inside the selected region, mark it as selected
                                    processor->setPathPointSelectedState(i, j, true);
                                    pathPtsSelected = true;
                                } else if (!(event.mods.isShiftDown() && (*sources)[i].getPathPointSelected(j))) {
                                    // if point is outside selected region and it was not previously selected with shift held down, unselect it
                                    processor->setPathPointSelectedState(i, j, false);
                                }
                            }
                        }
                        xyz = (*sources)[i].getPosXYZ();
                        float poser[3] = {xyz[0], xyz[1], xyz[2]};
                        if (pointInsideSelectRegion(poser) || ((*sources)[i].getSourceSelected() && pathPtsSelected) || mouseOverSourceIndex == i) {
                            processor->setSourceSelected(i, true);
                            reindexPathPts = true;
                        } else if (!(event.mods.isShiftDown() && processor->getSourceSelected(i)))
                            processor->setSourceSelected(i, false);
                    }
                } else if (mouseOverSourceIndex != -1) {
                    // if mouse is clicked directly over a source, toggle its selected state
                    cauto newSelected = !(*sources)[mouseOverSourceIndex].getSourceSelected();
                    processor->setSourceSelected(mouseOverSourceIndex, newSelected);
                    reindexPathPts = newSelected;
                } else if (mouseOverPathPointIndex != -1 && mouseOverPathPointSourceIndex != -1) {
                    // if mouse is clicked over a path point, toggle its selected sate
                    processor->togglePathPointSelected(mouseOverPathPointSourceIndex, mouseOverPathPointIndex);
                } else {
                    // start timer for checking if there was a double click
                    startTimer(1, 250); // much less than 300 makes it very hard to double click fast enough with my old fashioned mouse
                }
                tryHidePositioner3D();
                if (reindexPathPts)
                    reindexPathIndexTexts();
                break;
            }
            case DisplayState::PATH_AUTOMATION:
            {
                if (positionerText.getMouseOver() || positionerText.getMouseDragging()) {
                    positionerText.mouseClicked();
                } else if (selectionBox.isActive()/*mouseDragging*/) {
                    cauto thing1 = pathAutomationView.getWidth() * 0.5f - pathAutomationView.getXPosition();
                    cauto thing2 = 2.0f / pathAutomationView.getWidth();
                    // if shift not held down, unselect those previously selected fucking cunts
                    if (!event.mods.isShiftDown()) {
                        processor->deselectAllPathAutomationPoints();
                        // unselect the positioner text if selected and its not contained in the select region and shift not held down
                        if (/*positionerText.getSelected() &&*/pathAutomationPointIndexValid(pathAutomationPointEditableTextIndex)) {
                            cauto pts = (*sources)[pathAutomationPointEditableTextIndex[0]].getPathPosPtr()->getPoints();
                            auto pt = pts[pathAutomationPointEditableTextIndex[1]];
                            pt[0] = x_scale * ((pt[0] + thing1)*thing2 - 1);
                            pt[1] = y_scale * (pt[1] * 2 - 1);
                            if (!selectionBox.contains({pt[0], pt[1]}) /*contain(selectionBox, {pt[0], pt[1]})*/) {
                                pathAutomationPointEditableTextIndex = {-1, -1, -1};
                                positionerText.releaseFocus();
                            }
                        } else { // can't safely check if the select box contains the positioner path point so just deselect it anyways
                            pathAutomationPointEditableTextIndex = {-1, -1, -1};
                            loopRegionBeginHasPositionerTextFocus = false;
                            loopRegionEndHasPositionerTextFocus = false;
                            positionerText.setTextInputRestrictor(std::make_unique<PathAutomationInputRestrictor>());
                            positionerText.doubleClickSeparator = ", ";
                            positionerText.releaseFocus();
                        }
                        mouseOverPathAutomationPointIndex = {-1, -1, -1};
                        selectionBox.setActive(false);
                        positionerTextMouseMoved();
                        selectionBox.setActive(true);
                    }
                    // if ending a mouse drag selection, then select all path automation points in that region
                    float xy[2];
                    for (int i = 0; i < sources->size(); ++i) {
                        if ((*sources)[i].getSourceSelected()) {
                            // check and see if any of the selected sources' path automation points are inside selected region
                            cauto pts = (*sources)[i].getPathPosPtr()->getPoints();
                            for (int j = 0; j < pts.size(); ++j) {
                                xy[0] = x_scale * ((pts[j][0] + thing1)*thing2 - 1);
                                xy[1] = y_scale * (pts[j][1]*2 - 1);
                                if (pointInsideSelectRegion2D(xy)
                                    || (xy[0] > m_x-0.025f && xy[0] < m_x+0.025f && xy[1] > m_y-0.025f && xy[1] < m_y+0.025f)) {
                                    // if a point is inside the selected region, mark it as selected
                                    processor->setPathAutomationPointSelectedState(i, j, true);
                                }
                            }
                        }
                    }
                } else {
                    if (event.mods.isRightButtonDown()) {
                        if (mouseOverLoopRegionBegin || mouseOverLoopRegionEnd) {
                            pathAutomationPointEditableTextIndex = {-1, -1, -1};//{-1, -1, mouseOverLoopRegionBegin ? 0 : 1};
                            loopRegionBeginHasPositionerTextFocus = mouseOverLoopRegionBegin;
                            loopRegionEndHasPositionerTextFocus = mouseOverLoopRegionEnd;
                            positionerText.setTextInputRestrictor(std::make_unique<HoursMinSecInputRestrictor>());
                            positionerText.doubleClickSeparator = ":";
                            positionerTextMouseMoved(true);
                            positionerText.gainFocus();
                            pathAutomationPointsGrabbedWithMouse = false;
                            if (loopRegionBeginSelected || loopRegionEndSelected)
                                loopRegionSelectAnimation.restart();
                            loopRegionBeginSelected = false;
                            loopRegionEndSelected = false;
                            processor->deselectAllPathAutomationPoints();
                            break;
                        } else if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                            pathAutomationPointEditableTextIndex = mouseOverPathAutomationPointIndex;
                            loopRegionBeginHasPositionerTextFocus = false;
                            loopRegionEndHasPositionerTextFocus = false;
                            processor->setPathAutomationPointSelectedState(mouseOverPathAutomationPointIndex[0], mouseOverPathAutomationPointIndex[1], true);
                            positionerText.setTextInputRestrictor(std::make_unique<PathAutomationInputRestrictor>());
                            positionerText.doubleClickSeparator = ", ";
                            positionerTextMouseMoved(true);
                            positionerText.gainFocus();
                            pathAutomationPointsGrabbedWithMouse = false;
                            break;
                        } else {
                            pathAutomationPointEditableTextIndex = {-1,-1,-1};
                            loopRegionBeginHasPositionerTextFocus = false;
                            loopRegionEndHasPositionerTextFocus = false;
                            if (positionerText.getSelected()) {
                                positionerText.releaseFocus();
                                positionerTextMouseMoved();
                            }
                        }
                    } else if (loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus ||
                               !pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex) || mouseOverPathAutomationPointIndex == pathAutomationPointEditableTextIndex) {
                        pathAutomationPointEditableTextIndex = {-1,-1,-1};
                        if (positionerText.getSelected()) {
                            positionerText.releaseFocus();
                        }
                        if (loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus)
                            positionerText.finishAnimations();
                        loopRegionBeginHasPositionerTextFocus = false;
                        loopRegionEndHasPositionerTextFocus = false;
                        positionerText.setTextInputRestrictor(std::make_unique<PathAutomationInputRestrictor>());
                        positionerText.doubleClickSeparator = ", ";
                        positionerTextMouseMoved();
                    }
                    
//                    if (loopRegionBeginSelected) {
//                        loopRegionBeginSelected = false;
//                        loopRegionSelectAnimation.restart(0.1f);
//                    } else if (loopRegionEndSelected) {
//                        loopRegionEndSelected = false;
//                    }
                    if (mouseOverLoopRegionBegin/* && !loopRegionBeginHasPositionerTextFocus*/) {
                        loopRegionBeginSelected = ! loopRegionBeginSelected;
                        loopRegionSelectAnimation.restart(0.1f);
                        processor->deselectAllPathAutomationPoints();
                    } else if (mouseOverLoopRegionEnd/* && !loopRegionEndHasPositionerTextFocus*/) {
                        loopRegionEndSelected = ! loopRegionEndSelected;
                         loopRegionSelectAnimation.restart(0.1f);
                        processor->deselectAllPathAutomationPoints();
                    } else {
                        if (pathAutomationPointsGrabbedWithMouse) {
                            //mouseOverPathAutomationPointIndex = {-1,-1,-1}; // ??
                            pathAutomationPointsGrabbedWithMouse = false;
                            processor->deselectAllPathAutomationPoints();
                        } else if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                            processor->togglePathAutomationPointSelected(mouseOverPathAutomationPointIndex[0],
                                                                         mouseOverPathAutomationPointIndex[1]);
                            // make sure positioner text is deselected if it's point is unselected
                            if (mouseOverPathAutomationPointIndex == pathAutomationPointEditableTextIndex &&
                                !(*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPointSelected(mouseOverPathAutomationPointIndex[1])) {
                                pathAutomationPointEditableTextIndex = {-1, -1, -1};
                                positionerText.releaseFocus();
                            }
                        }
//                        else if (mouseOverLoopRegionBegin/* && !loopRegionBeginHasPositionerTextFocus*/) {
//                            loopRegionBeginSelected = ! loopRegionBeginSelected;
//                            loopRegionSelectAnimation.restart(0.1f);
//                            processor->deselectAllPathAutomationPoints();
//                        } else if (mouseOverLoopRegionEnd/* && !loopRegionEndHasPositionerTextFocus*/) {
//                            loopRegionEndSelected = ! loopRegionEndSelected;
//                            processor->deselectAllPathAutomationPoints();
//                        }
                        else {
                            processor->deselectAllPathAutomationPoints();
                        }
                    }
                }
                
                if (!positionerText.getSelected() && !pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex))
                    positionerText.finishAnimations();
                break;
            }
                
            case DisplayState::SETTINGS:
            {
                //etb.mouseClicked();
                int selectedMode = processingModeOptions.mouseClicked();
                if (selectedMode >= 0) {
                    processor->setProcessingMode((ProcessingMode)selectedMode);
                    processingModeOptions.setAutoDetected(processor->isHostRealTime ? 0 : 1);
                } else if (websiteButton.mouseClicked()) {
                    const URL url ("http://www.freedomaudioplugins.com");
                    url.launchInDefaultBrowser();
                }
//                int selectedMode = processingModeOptions.updateFromMouse(m_x, m_y, true, windowAspectRatio);
//                if (selectedMode > 0) {
//                    processor->setProcessingMode((ProcessingMode)(selectedMode-1));
//                    processingModeOptions.setAutoDetectOption(processor->isHostRealTime);
//                }
                break;
            }
            case DisplayState::NUM_DISPLAY_STATES:
                break;
        }
    } // end if (sources)
SKIP:
    // mouse is no longer down, duh
    mouseIsDown = false;
    // mouseDoubleClick callback comes after mouseUp, assume no d-click follows so that d-clicking inside positionerText3D to select field doesn't cause mouseDoubleClicked = true to linger and cause releasing the focus to require two clicks in space
    mouseDoubleClicked = false;
    
    // reset mouse dragging state
//    mouseDragDownX = 0;
//    mouseDragDownY = 0;
//    mouseDragCurrentX = 0;
//    mouseDragCurrentY = 0;
    if (selectionBox.isActive()/*mouseDragging*/)
        selectionBox.getAnimation().restart();// mouseDragAnimation.restart();
    selectionBox.setActive(false);// mouseDragging = false;
    //mouseDragAnimation.count = 0;
}

void ThreeDAudioProcessorEditor::mouseDown(const MouseEvent & event)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    
    mouseIsDown = true;
    mouseJustDown = true;
}

void ThreeDAudioProcessorEditor::mouseDoubleClick(const MouseEvent & event)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    
    mouseDoubleClicked = true;
    cauto mouse_x = getMouseX();
    cauto mouse_y = getMouseY();
    
//    if (etb.mouseDoubleClicked())
//        return;
    
    // don't do any of the stuff below if we click in the tab bar area or the help doppler buttons
    if (mouse_y > tabs.getBoundary().getBottom()
        || (processor->displayState != DisplayState::SETTINGS && (helpButton.isMouseOver() || dopplerButton.isMouseOver())))
        return;
  
    if (processor->displayState != DisplayState::SETTINGS) {
        if (volumeSlider.mouseDoubleClicked()) {
//            processor->wetOutputVolume = volumeSlider.getValue();
            return;
        }
        if (mixSlider.mouseDoubleClicked()) {
//            cauto v = 0.01f * mixSlider.getValue();
//            processor->dryOutputVolume = 1.0f - v;
//            processor->wetOutputVolume = v;
            return;
        }
    }
    switch (processor->displayState) {
        case DisplayState::MAIN:
            if (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1 &&
                ((EditableTextBox*)(pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex].get()))->mouseDoubleClicked()) {
                positionerText3D.mouseDoubleClicked(); // to release focus if necessary
            } 
			else if (positionerText3D.mouseDoubleClicked()) {}
            else if (dopplerSlider.mouseDoubleClicked()) {}
            else if (mouseOverSourceIndex != -1) {
                // if mouse is clicked directly over a source, toggle its selected state
                processor->setSourceSelected(mouseOverSourceIndex, !processor->getSourceSelected(mouseOverSourceIndex));
                //mouseClickedOnSomething = true;
            } else if (mouseOverPathPointIndex != -1 && mouseOverPathPointSourceIndex != -1) {
                // if mouse is clicked over a path point, toggle its selected state
                processor->togglePathPointSelected(mouseOverPathPointSourceIndex, mouseOverPathPointIndex);
                //mouseClickedOnSomething = true;
            }
//            else if (mouseOverDopplerSlider) {
//                // if mouse is over the doppler slider, set speed of sound to the new value
//                processor->setSpeedOfSound(newDopplerSpeedOfSound);
//            }
            else {
                // get the mouse's 3d click position
                float pt[3];
                to3D(mouse_x, mouse_y, pt);
                // try to add a path point for the selected sources
                if (!processor->dropPathPoint(pt)) {
                    // if no sources are selected add a new source at the 3d click position
                    processor->addSourceAtXYZ(pt);
//                    if (processor->addSourceAtXYZ(pt))
//                        pathIndexTexts.emplace_back();
                } else {
                    resizePathPtsPrevState();
                    reindexPathIndexTexts();
                }
            }
            break;
            
        case DisplayState::PATH_AUTOMATION:
//            if (pathAutomationPointIndexValid(pathAutomationPointEditableTextIndex)) {}
//            else
            if (positionerText.mouseDoubleClicked())
                return;
            if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                // pick up all selected points to move them
                // select a point if unselected but mouse is over it
                processor->setPathAutomationPointSelectedState(mouseOverPathAutomationPointIndex[0],
                                                               mouseOverPathAutomationPointIndex[1], true);
                const int indexAmongSelecteds = processor->getPathAutomationPointIndexAmongSelectedPoints
                                                              (mouseOverPathAutomationPointIndex[0],
                                                               mouseOverPathAutomationPointIndex[1]);
                mouseOverPathAutomationPointIndex[2] = indexAmongSelecteds;
                pathAutomationPointsGrabbedWithMouse = true;
                positionerText.releaseFocus();
                if (pathAutomationPointEditableTextIndex != mouseOverPathAutomationPointIndex)
                    positionerText.finishAnimations();
                pathAutomationPointEditableTextIndex = {-1, -1, -1};
            } else if (mouseOverLoopRegionBegin || loopRegionBeginSelected) {
                loopRegionBeginSelected = ! loopRegionBeginSelected;
                loopRegionSelectAnimation.restart(0.1f);
            } else if (mouseOverLoopRegionEnd || loopRegionEndSelected) {
                loopRegionEndSelected = ! loopRegionEndSelected;
                loopRegionSelectAnimation.restart(0.1f);
            } else {
                // add a new path automation point at the mouse's position
                float pt2d[2] = { (float)((mouse_x/x_scale)*pathAutomationView.getWidth()/2.0+pathAutomationView.getXPosition()), (float)((mouse_y/y_scale+1.0)/2.0) };
                // force the new point to be in bounds
                if (pt2d[0] < 0)
                    pt2d[0] = 0;
                if (pt2d[1] < 0)
                    pt2d[1] = 0;
                if (pt2d[1] > 1)
                    pt2d[1] = 1;
                processor->addPathAutomationPtAtXY(pt2d);
            }
            break;
        case DisplayState::SETTINGS:
            break;
        case DisplayState::NUM_DISPLAY_STATES:
            break;
    }
}

void ThreeDAudioProcessorEditor::resizePathPtsPrevState()
{
    Sources* sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (!sources)
        return;
    for (int s = 0; s < sources->size(); ++s) {
        cauto numPts = (*sources)[s].getNumPathPoints();
        cauto prevNumPts = prevMouseOverPathPts[s].size();
        prevMouseOverPathPts[s].resize(numPts);
        prevSelectedPathPts[s].resize(numPts);
        if (numPts < prevNumPts) {
            for (int i = 0; i < numPts; ++i) {
                prevMouseOverPathPts[s][i] = false;
                prevSelectedPathPts[s][i] = (*sources)[s].getPathPointSelected(i);
            }
        }
    }
}

void ThreeDAudioProcessorEditor::mouseWheelMove	(const MouseEvent & event,
                                                 const MouseWheelDetails & wheel)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    
    mouseWheeldX = 0.5f * wheel.deltaX;
    mouseWheeldY = 0.5f * wheel.deltaY;
    bool DXLargerThanDY = std::abs(mouseWheeldX) >= std::abs(mouseWheeldY);
    
    // Windows doesn't default to horizontal scroll with shift held down so explicitly do that here...
    if (event.mods.isShiftDown() && !DXLargerThanDY) {
        mouseWheeldX = mouseWheeldY;
        DXLargerThanDY = true;
    }
    
    if (dopplerSlider.mouseWheelMove(mouseWheeldX, mouseWheeldY))
        return;
//    if (etb.mouseWheelMove(mouseWheeldX, mouseWheeldY))
//        return;
    
    if (processor->displayState == DisplayState::MAIN) {
        int sourcesMoved = 0;
        if (!(event.mods.isCommandDown() || event.mods.isCtrlDown())) {
            // atempt to move selected sources
            if (DXLargerThanDY) {
                if (event.mods.isAltDown())
                    sourcesMoved = processor->moveSelectedSourcesRAE(1+mouseWheeldX, 0, 0, positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
                else
                    sourcesMoved = processor->moveSelectedSourcesRAE(1, mouseWheeldX, 0, positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
            } else {
                if (event.mods.isAltDown())
                    sourcesMoved = processor->moveSelectedSourcesRAE(1+mouseWheeldY, 0, 0, positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
                else
                    sourcesMoved = processor->moveSelectedSourcesRAE(1, 0, mouseWheeldY, positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
            }
        }
        else {
            // its nice to save the source state if were going from moving the sources to moving the view as the mouseWheelMove method has a tail period where it gets called and the user might accidently let off ctrl/cmd while they don't mean to have this tail of callbacks affect the source position
            if (!wasCtrlCmdDownPrev) {
                processor->saveCurrentState(1);
                processor->saveCurrentState(0);
            }
        }
        wasCtrlCmdDownPrev = (event.mods.isCommandDown() || event.mods.isCtrlDown());
        
        // tried to move sources but none were selected, move camera instead
        if (sourcesMoved == 0) {
            
            if (event.mods.isAltDown()) {
                
                if (DXLargerThanDY)
                    eyeRad += mouseWheeldX;
                else
                    eyeRad += mouseWheeldY;
                
                if (eyeRad < 0.2f)
                    eyeRad = 0.2f;
                
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else if (DXLargerThanDY) {
                
                eyeAzi -= mouseWheeldX;
                
                if (eyeAzi < 0)
                    eyeAzi += 2*M_PI;
                
                if (eyeAzi > 2*M_PI)
                    eyeAzi -= 2*M_PI;
                
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else {
                
                eyeEle += upDir*mouseWheeldY;
                
                if (eyeEle < 0) {
                    eyeEle *= -1.0f;
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1.0f;
                }
                
                if (eyeEle > M_PI) {
                    eyeEle = M_PI-(eyeEle-M_PI);
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1.0f;
                }
                
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
                // update up vector if we filp the camera upside down during our rotations
                eyeUp[0] = 0;
                eyeUp[1] = upDir;
                eyeUp[2] = 0;
            }
            
            updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
        }
        
        updatePositioner3DTextValueAndPosition();
        //repositionPositionerText3DToTarget();
        repositionPathIndexTexts();
        //reindexPathIndexTexts();
        
    } else if (processor->displayState == DisplayState::PATH_AUTOMATION) {
//        // scrolling to move selected points is just kinda an odd way of doing things... if desired though, this chunk of code just needs to be uncommented
//        const bool anyPtsSelected = processor->areAnySelectedSourcesPathAutomationPointsSelected();
//        int pathAutomationPointsMoved = 0;
//        if (!(event.mods.isCommandDown() || event.mods.isCtrlDown()) && !pathAutomationPointsGrabbedWithMouse) {
//            // only try moving points if have not recently autoaligned, both for cpu efficiency and so that the autoalign will not cause the points to skip to the begining if the points are close in terms of the view and how the autoalign function traverses the points (from beginning to end)
////            if (!(animationOn[0] || animationOn[1])) {
//            // atempt to move selected path automation points, together
//            if (DXLargerThanDY && !animationOn[0]) {
//                const float adjust = -mouseWheeldX*automationViewWidth/3.0;
//                pathAutomationPointsMoved = processor->moveSelectedPathAutomationPoints(adjust, 0);
//            } else if (!animationOn[1]) {
//                const float adjust = mouseWheeldY/3.0;
//                pathAutomationPointsMoved = processor->moveSelectedPathAutomationPoints(0, adjust);
//            }
//            if (event.mods.isAltDown() && pathAutomationPointsMoved > 0) {
//                autoAlignAutomationPoints(DXLargerThanDY, !DXLargerThanDY, true);
//            }
////            } else
////                return; // do not move views below
//            if (animationOn[0] || animationOn[1])
//                return; // do not move views below
//        } else if (anyPtsSelected) {
//            if (pathAutomationPointsGrabbedWithMouse) {
//                if (event.mods.isCommandDown() || event.mods.isCtrlDown())
//                    goto MOVE_3D_VIEW;
//                else
//                    goto MOVE_2D_VIEW;
//            }
//            else
//                goto MOVE_2D_VIEW; // unwarrented, reckless use of goto for the sake of rebellion
//        }
//        
//        // tried to move selected path automation points, but none were selected
//        if (pathAutomationPointsMoved == 0) {
//            
        if (event.mods.isCommandDown() || event.mods.isCtrlDown()) {
//        MOVE_3D_VIEW:
            // move camera
            if (event.mods.isAltDown()) {
                
                if (DXLargerThanDY)
                    eyeRad += mouseWheeldX;
                else
                    eyeRad += mouseWheeldY;
                //eyeRad += mouseWheeldY;
                
                if (eyeRad < 0.2f)
                    eyeRad = 0.2f;
                
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else if (DXLargerThanDY) {
                
                eyeAzi -= mouseWheeldX;
                
                if (eyeAzi < 0)
                    eyeAzi += 2*M_PI;
                
                if (eyeAzi > 2*M_PI)
                    eyeAzi -= 2*M_PI;
                
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else {
                
                eyeEle += upDir*mouseWheeldY;
                
                if (eyeEle < 0) {
                    eyeEle *= -1.0f;
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1.0f;
                }
                
                if (eyeEle > M_PI) {
                    eyeEle = M_PI-(eyeEle-M_PI);
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1.0f;
                }
                
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
                // update up vector if we filp the camera upside down during our rotations
                eyeUp[0] = 0;
                eyeUp[1] = upDir;
                eyeUp[2] = 0;
            }
            
            updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
            //reindexPathIndexTexts();
        } else {
//        MOVE_2D_VIEW:
            // adjust the automation grid view ...
            if (DXLargerThanDY) { // ... in width
                pathAutomationView.setXPosition(pathAutomationView.getXPosition() - mouseWheeldX * pathAutomationView.getWidth());
            } else { // ... in zoom
                float x;
                if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                    const auto viewX = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1])[0];
                    x = pathAutomationView.viewToHolderX(viewX);
                } else
                    x = getMouseX();
                auto deltaX = pathAutomationView.holderToViewX(x/*getMouseX()*/);
                pathAutomationView.setWidth(pathAutomationView.getWidth() * (1 + 0.3f * mouseWheeldY));
                deltaX -= pathAutomationView.holderToViewX(x/*getMouseX()*/);
                pathAutomationView.setXPosition(pathAutomationView.getXPosition() + deltaX);
            }
            for (auto& dl : pathAutomationDisplayList)
                dl = 0;
            //keepGrabbedPathAutoPointUnderMouse();
            //if (positionerText.getSelected()) {
            positionerTextMouseMoved();
            //}
        }
//        }
    }
}

//void ThreeDAudioProcessorEditor::keepGrabbedPathAutoPointUnderMouse()
//{
//    // don't let grabbed path point stray from mouse position
//    if (pathAutomationPointsGrabbedWithMouse && pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)
//        && (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPointSelected(mouseOverPathAutomationPointIndex[1])/* &&
//        getMouseX() > -x_scale && -y_scale < getMouseY() && getMouseY() < y_scale*/) {
//        const auto p = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1]);
//        const Point<float> pt (p[0], p[1]);
//        const auto mousePt = pathAutomationView.holderToView({mouseX/*getMouseX()*/, mouseY/*getMouseY()*/});// View2DFuncs::getPoint(view, {getMouseX(), getMouseY()});
//        if (pt != mousePt && mousePt.x >= 0 && 0 <= mousePt.y && mousePt.y <= 1) {
//            const auto d = mousePt - pt;
//            //const std::vector<float> delta = {d.x , d.y};
//            processor->moveSelectedPathAutomationPoints(d.x, d.y);
//            // check that the mouse is still over the grabbed point
//            
//            //(*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->moveSelectedPoints(delta);
//        }
//    }
//
//}

void ThreeDAudioProcessorEditor::autoAlignAutomationPoints(const bool alignInX,
                                                           const bool alignInY,
                                                           const bool alignWithMouse)
{
    sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        
        // thing that holds potential adjustment info for alignment
        float autoAlignedIndex [] = {-1, -1, 0, 0}; // [source index, pt index, ∆x, ∆y]
        
        // line up the moving points with other points that are within this distance (on screen) of each other.
        const float distThresh = 0.04;
        
        // mouse position
        const float mouse_x = getMouseX();
        const float mouse_y = getMouseY();
        
        // get all the points that we want to check to line up with
        std::vector<std::vector<std::vector<float>>> allAutoPts;
        for (int s = 0; s < sources->size(); ++s) {
            if ((*sources)[s].getSourceSelected())
                allAutoPts.emplace_back((*sources)[s].getPathPosPtr()->getPoints());
            else // need to put an empty thing in otherwise the source indecies will be messed up
                allAutoPts.emplace_back(std::vector<std::vector<float>>());
        }
        
        // precomputation of constants to save cpu in loops below
        const float A = pathAutomationView.getWidth()/2.0-pathAutomationView.getXPosition();
        const float B = 2.0/pathAutomationView.getWidth();
        
        // did we align in x/y yet?
        bool alignedInX = false, alignedInY = false;
        
        // check if any of the path automation points want to "line up" with each other ...
        // if they do and we are not moving the points "too fast" in the x direction (or y), then set the selected points' position to line up as desired
        // otherwise just move the points as per usual
        for (int s = 0; s < sources->size(); ++s) {
            if ((*sources)[s].getSourceSelected()) {
                const std::vector<std::vector<float>> pts = (*sources)[s].getPathPosPtr()->getSelectedPoints();
                const std::vector<int> pts_indecies = (*sources)[s].getPathPosPtr()->getSelectedPointIndices();
                for (int i = 0; i < pts.size(); ++i) {
                    const float pt_x = (pts[i][0]+A)*B-1.0;
                    if (std::abs(pt_x) < 1.0) { // if the point is on the screen (allows one to effeciently match up an endpoint of a long series of selected points by zooming in on the area of interest, otherwise there is a lot of checking to do!)
                        // if we want to align with the mouse and its over the graph region
                        if (alignWithMouse && (std::abs(mouse_x) < x_scale && std::abs(mouse_y) < y_scale)) {
                            // first check if the selected point lines up with mouse
                            if (alignInX && !alignedInX) {
                                const float d = std::abs(x_scale*pt_x - mouse_x);
                                if (d < distThresh) {
                                    autoAlignedIndex[0] = s,
                                    autoAlignedIndex[1] = pts_indecies[i]; // the auto aligned pt's index
                                    autoAlignedIndex[2] = (((mouse_x/x_scale+1)-1)*pathAutomationView.getWidth()*0.5+pathAutomationView.getXPosition()) - pts[i][0]; // aligned in x dim (∆ = new - old)
                                    pathPtAutoAlignX = mouse_x;
                                    alignedInX = true;
                                    // leave all these nested loops if done aligning
                                    if (alignInY) {
                                        if (alignedInY)
                                            goto ALIGNED;
                                    } else
                                        goto ALIGNED;
                                }
                            }
                            if (alignInY && !alignedInY) {
                                const float pt_y = pts[i][1]*2.0-1.0;
                                const float d = std::abs(y_scale*pt_y - mouse_y);
                                if (d < distThresh) {
                                    autoAlignedIndex[0] = s,
                                    autoAlignedIndex[1] = pts_indecies[i]; // the auto aligned pt's index
                                    autoAlignedIndex[3] = 0.5*(mouse_y/y_scale+1) - pts[i][1]; // aligned in y dim (∆ = new - old)
                                    pathPtAutoAlignY = mouse_y;
                                    alignedInY = true;
                                    // leave all these nested loops if done aligning
                                    if (alignInX) {
                                        if (alignedInX)
                                            goto ALIGNED;
                                    } else
                                        goto ALIGNED;
                                }
                            }
                        }
                        // then check to align with other non-selected points
                        for (int s2 = 0; s2 < allAutoPts.size(); ++s2) {
                            const std::vector<bool> selPts2 = (*sources)[s2].getPathPosPtr()->getPointsSelected();
                            for (int j = 0; j < allAutoPts[s2].size(); ++j) {
                                if (!selPts2[j] // if the point we are trying to match up with is not selected (being moved also)
                                    && !(s == s2 && j == pts[i].back())) { // and we are not looking to match up with ourself
                                    const float lineUpPtX = x_scale*((allAutoPts[s2][j][0]+A)*B-1.0);
                                    if (lineUpPtX >= -x_scale && lineUpPtX <= x_scale) { // if the point we're matching up to is on screen
                                        if (alignInX && !alignedInX) {
                                            const float ptX = x_scale*((pts[i][0]+A)*B-1.0);
                                            const float d = std::abs(ptX - lineUpPtX);
                                            if (d < distThresh) {
                                                autoAlignedIndex[0] = s,
                                                autoAlignedIndex[1] = pts_indecies[i]; // the auto aligned pt's index
                                                autoAlignedIndex[2] = allAutoPts[s2][j][0] - pts[i][0]; // aligned in x dim (∆ = new - old)
                                                pathPtAutoAlignX = lineUpPtX;
                                                alignedInX = true;
                                                // leave all these nested loops if done aligning
                                                if (alignInY) {
                                                    if (alignedInY)
                                                        goto ALIGNED;
                                                } else
                                                    goto ALIGNED;
                                            }
                                        }
                                        if (alignInY && !alignedInY) {
                                            const float lineUpPtY = y_scale*(allAutoPts[s2][j][1]*2.0-1.0);
                                            const float ptY = y_scale*(pts[i][1]*2.0-1.0);
                                            const float d = std::abs(ptY - lineUpPtY);
                                            if (d < distThresh) {
                                                autoAlignedIndex[0] = s,
                                                autoAlignedIndex[1] = pts_indecies[i]; // the auto aligned pt's index
                                                autoAlignedIndex[3] = allAutoPts[s2][j][1] - pts[i][1]; // aligned in y dim (∆ = new - old)
                                                pathPtAutoAlignY = lineUpPtY;
                                                alignedInY = true;
                                                // leave all these nested loops if done aligning
                                                if (alignInX) {
                                                    if (alignedInX)
                                                        goto ALIGNED;
                                                } else
                                                    goto ALIGNED;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // return point from nested for loops if an alignment was made
    ALIGNED:
        // if we autoaligned with a point, then also stop the remaining selected points along the aligned dimension (for ease of copying and pasting automation path segments
        if (autoAlignedIndex[0] != -1) {
            if (processor->moveSelectedPathAutomationPoints(autoAlignedIndex[2], autoAlignedIndex[3])) {
                if (autoAlignedIndex[2]) {
                    pathPtAutoAlignAnimationX.restart();
//                    animationOn[0] = true;
//                    animationTime[0] = 0;
                }
                if (autoAlignedIndex[3]) {
                    pathPtAutoAlignAnimationY.restart();
//                    animationOn[1] = true;
//                    animationTime[1] = 0;
                }
            }
        }
    } // end if (sources)
}

void ThreeDAudioProcessorEditor::mouseMagnify (const MouseEvent & event,
                                               float scaleFactor)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);
    
    mouseZoomFactor = scaleFactor;
    
    if (processor->displayState == DisplayState::MAIN) {
        
        int sourcesMoved = 0;
        if (!(event.mods.isCommandDown() || event.mods.isCtrlDown()))
            sourcesMoved = processor->moveSelectedSourcesRAE(mouseZoomFactor, 0, 0, positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
        
        // tried to move selected sources, but there were none so move the camera
        if (sourcesMoved == 0) {
            // move the eye radially
            if (mouseZoomFactor != 1.0f) {
                eyeRad *= mouseZoomFactor;
                if (eyeRad < 0.2f)
                    eyeRad = 0.2f;
                updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
            }
        }
        
        updatePositioner3DTextValueAndPosition();
        repositionPathIndexTexts();
        
    } else if (processor->displayState == DisplayState::PATH_AUTOMATION) {
        
        if (event.mods.isCommandDown() || event.mods.isCtrlDown()) {
//            if (processor->areAnySelectedSourcesPathAutomationPointsSelected()) { // why did i do this?
//                if (pathAutomationPointsGrabbedWithMouse)
//                    goto MOVE_3D_VIEW;
//                else
//                    goto MOVE_2D_VIEW;
//            } else {
            MOVE_3D_VIEW:
                // move the eye radially
                if (mouseZoomFactor != 1.0f) {
                    eyeRad *= mouseZoomFactor;
                    if (eyeRad < 0.2f)
                        eyeRad = 0.2f;
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                    //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                }
                //reindexPathIndexTexts();
//            }
        }
        else {
        MOVE_2D_VIEW:
            // adjust the automation grid view width
            if (mouseZoomFactor != 0) {
                float x;
                if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                    const auto viewX = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1])[0];
                    x = pathAutomationView.viewToHolderX(viewX);
                } else
                    x = getMouseX();
                auto deltaX = pathAutomationView.holderToViewX(x);
                pathAutomationView.setWidth(pathAutomationView.getWidth() / mouseZoomFactor);
                deltaX -= pathAutomationView.holderToViewX(x);
                pathAutomationView.setXPosition(pathAutomationView.getXPosition() + deltaX);
//                keepGrabbedPathAutoPointUnderMouse();
//                if (positionerText.getSelected()) {
//                    positionerTextMouseMoved(true);
//                }
                positionerTextMouseMoved();
                for (auto& dl : pathAutomationDisplayList)
                    dl = 0;
            }
        }
    }
}

bool ThreeDAudioProcessorEditor::keyPressed	(const KeyPress & key)
{
	const std::lock_guard<decltype(glLock)> lockGL (glLock);

    auto str = key.getTextDescription();
    if (!key.getModifiers().isShiftDown())
        str = str.toLowerCase();
    
    if (processor->displayState == DisplayState::MAIN) {
        
        if (positionerText3DID.sourceIndex != -1 && positionerText3DID.pathPtIndex != -1) {
            cauto pathIndexETB = (EditableTextBox*) pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex].get();
            if (pathIndexETB->keyPressed(str.toStdString())) {
                if (!pathIndexETB->getSelected()) {
                    cauto txt = pathIndexETB->getText();
                    if (txt.empty()) {
                        pathIndexTexts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex]
                            ->setText(std::to_string(positionerText3DID.pathPtIndex));
                    } else {
                        cauto newIndex = std::stoi(txt);
                        processor->setSelectedPathPointIndecies(positionerText3DID.sourceIndex, positionerText3DID.pathPtIndex, newIndex);
                        selectPathPtAnimations.clear();
                        prevSelectedPathPts[positionerText3DID.sourceIndex][newIndex] = true;
                        prevSelectedPathPts[positionerText3DID.sourceIndex][positionerText3DID.pathPtIndex] = false;
                        positionerText3DID.pathPtIndex = newIndex;
                        reindexPathIndexTexts();
                        //updatePositioner3DTextValueAndPosition();
//                        positionerText3D.hide();
//                        positionerText3DID = {-1, -1};
                    }
                }
                repositionPathIndexTexts(); // to recenter the index text box with the new character added
                return true;
            }
        }
        cauto prevRAESelected = positionerText3D.raeText.getSelected();
        cauto prevXYZSelected = positionerText3D.xyzText.getSelected();
        if (positionerText3D.keyPressed(str.toStdString())) {
            Sources* sources = nullptr;
            const Locker lock (processor->sources.get(sources));
            if (sources) {
                if (prevRAESelected && !positionerText3D.raeText.getSelected()) {
                    std::array<float, 3> rae;
                    if (positionerText3DID.pathPtIndex != -1) {
                        auto vxyz = (*sources)[positionerText3DID.sourceIndex].getPathPtr()->getPoint(positionerText3DID.pathPtIndex);
                        XYZtoRAE(vxyz.data(), rae.data());
                    } else
                        rae = (*sources)[positionerText3DID.sourceIndex].getPosRAE();
                    PositionerText3D::worldToDisplayRAE(rae);
                    cauto newRae = positionerText3D.getRAE(rae);
                    cauto dr = newRae[0] / rae[0];
                    cauto da = toRadians(newRae[1] - rae[1]);
                    cauto de = toRadians(rae[2] - newRae[2]);
                    processor->moveSelectedSourcesRAE(dr, da, de, positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
                    reindexPathIndexTexts();
                    updatePositioner3DTextValueAndPosition();
    //                positionerText3D.hide();
    //                positionerText3DID = {-1, -1};
                } else if (prevXYZSelected && !positionerText3D.xyzText.getSelected()) {
                    std::array<float, 3> xyz;
                    if (positionerText3DID.pathPtIndex != -1) {
                        auto vxyz = (*sources)[positionerText3DID.sourceIndex].getPathPtr()->getPoint(positionerText3DID.pathPtIndex);
                        xyz = {vxyz[0], vxyz[1], vxyz[2]};
                    } else
                        xyz = (*sources)[positionerText3DID.sourceIndex].getPosXYZ();
                    debug({NAMEANDVALUE(xyz[0]), NAMEANDVALUE(xyz[1]), NAMEANDVALUE(xyz[2])});
                    cauto newXyz = positionerText3D.getXYZ(xyz);
                    debug({NAMEANDVALUE(newXyz[0]), NAMEANDVALUE(newXyz[1]), NAMEANDVALUE(newXyz[2])}, PrintToFileMode::APPEND);
                    processor->moveSelectedSourcesXYZ(newXyz[0] - xyz[0], newXyz[1] - xyz[1], newXyz[2] - xyz[2], positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
                    cauto realxyz = (*sources)[positionerText3DID.sourceIndex].getPosXYZ();
                    debug({NAMEANDVALUE(realxyz[0]), NAMEANDVALUE(realxyz[1]), NAMEANDVALUE(realxyz[2])}, PrintToFileMode::APPEND);
                    reindexPathIndexTexts();
                    updatePositioner3DTextValueAndPosition();
    //                positionerText3D.hide();
    //                positionerText3DID = {-1, -1};
                }
            }
            return true;
        }
        bool wasSelected = dopplerSlider.getValueTextBox().getSelected();
        if (dopplerSlider.keyPressed(str.toStdString())) {
            if (wasSelected && !dopplerSlider.getValueTextBox().getSelected())
                processor->setSpeedOfSound(dopplerSlider.getValue());
            return true;
        }
        wasSelected = volumeSlider.getValueTextBox().getSelected();
        if (volumeSlider.keyPressed(str.toStdString())) {
            if (wasSelected && !volumeSlider.getValueTextBox().getSelected()) {
                cauto v = 0.01f * mixSlider.getValue();
                processor->wetOutputVolume = v * volumeSlider.getValue();
                processor->dryOutputVolume = (1 - v) * volumeSlider.getValue();
            }
            return true;
        }
        wasSelected = mixSlider.getValueTextBox().getSelected();
        if (mixSlider.keyPressed(str.toStdString())) {
            if (wasSelected && !mixSlider.getValueTextBox().getSelected()) {
                cauto v = 0.01f * mixSlider.getValue();
                processor->wetOutputVolume = v * volumeSlider.getValue();
                processor->dryOutputVolume = (1 - v) * volumeSlider.getValue();
//                processor->wetOutputVolume = mixSlider.getValue();
//                processor->dryOutputVolume = 1 - mixSlider.getValue();
            }
            return true;
        }
    } else if (processor->displayState == DisplayState::PATH_AUTOMATION) {
        if (positionerText.getSelected() && positionerText.keyPressed(str.toStdString())) {
            boundsCheckPositionerText();
            if (!positionerText.getSelected()) { // if the key pressed releases the focus of the positioner text thus entering the text as the new value
                if (pathAutomationPointIndexValid(pathAutomationPointEditableTextIndex)) {
                    const auto pt = (*sources)[pathAutomationPointEditableTextIndex[0]].getPathPosPtr()->getPoint(pathAutomationPointEditableTextIndex[1]);
                    std::array<float, 2> newPt;
                    if (positionerText.getText().empty())
                        newPt = {pt[0], pt[1]};
                    else {
                        const auto text = positionerText.getText();
                        const auto divStr = dynamic_cast<const PathAutomationInputRestrictor*>(positionerText.getTextInputRestrictor())->divString;
                        int div = std::distance(text.begin(), std::find(text.begin(), text.end(), divStr.front()));
                        const auto timeText = text.substr(0, div);
                        div += divStr.length();
                        const auto percentText = div < text.length() ? text.substr(div, text.length()) : "";
                        const auto x = StrFuncs::secFromHrMinSec(positionerText.getText());
                        auto y = pt[1];
                        try {
                            y = std::stof(percentText.substr(0, text.length() - 1)) * 0.01f;
                        } catch (...) {}
                        newPt = {x, y};
                    }
                    const auto dx = newPt[0] - pt[0];
                    const auto dy = newPt[1] - pt[1];
                    pathAutomationPointEditableTextIndex[2] = processor->getPathAutomationPointIndexAmongSelectedPoints(pathAutomationPointEditableTextIndex[0], pathAutomationPointEditableTextIndex[1]);
                    processor->moveSelectedPathAutomationPoints(dx, dy);
                    pathAutomationPointEditableTextIndex[1] = (*sources)[pathAutomationPointEditableTextIndex[0]].getPathPosPtr()->getSelectedPointIndices()[pathAutomationPointEditableTextIndex[2]];
                    positionerTextMouseMoved();
                } else if (loopRegionBeginHasPositionerTextFocus || loopRegionEndHasPositionerTextFocus) {
                    if (!positionerText.getText().empty()) {
                        if (loopRegionBeginHasPositionerTextFocus) {
                            processor->loopRegionBegin = StrFuncs::secFromHrMinSec(positionerText.getText());
                        } else {
                            processor->loopRegionEnd = StrFuncs::secFromHrMinSec(positionerText.getText());
                        }
                        if (processor->loopRegionBegin > processor->loopRegionEnd) {
                            const float tmp = processor->loopRegionBegin;
                            processor->loopRegionBegin.store(processor->loopRegionEnd);
                            processor->loopRegionEnd = tmp;
                        }
                        positionerTextMouseMoved();
                    }
                }
            }
            return true;
        }
        
        bool wasSelected = volumeSlider.getValueTextBox().getSelected();
        if (volumeSlider.keyPressed(str.toStdString())) {
            if (wasSelected && !volumeSlider.getValueTextBox().getSelected()) {
                cauto v = 0.01f * mixSlider.getValue();
                processor->wetOutputVolume = v * volumeSlider.getValue();
                processor->dryOutputVolume = (1 - v) * volumeSlider.getValue();
            }
            return true;
        }
        wasSelected = mixSlider.getValueTextBox().getSelected();
        if (mixSlider.keyPressed(str.toStdString())) {
            if (wasSelected && !mixSlider.getValueTextBox().getSelected()) {
                cauto v = 0.01f * mixSlider.getValue();
                processor->wetOutputVolume = v * volumeSlider.getValue();
                processor->dryOutputVolume = (1 - v) * volumeSlider.getValue();
            }
            return true;
        }
    } else if (processor->displayState == DisplayState::SETTINGS) {
//        if (etb.keyPressed(str.toStdString()))
//            return true;
    }
    
    // 'v' to get to path automation control state, and to go back to main view
    if (key.getTextDescription().equalsIgnoreCase("V")) {
        processor->displayState = (DisplayState) (((int)processor->displayState.load() + 1) % (int)DisplayState::SETTINGS/*NUM_DISPLAY_STATES*/);
        if (processor->displayState == DisplayState::PATH_AUTOMATION)
            processor->makeSourcesVisibleForPathAutomationView();
//        if (processor->displayState == DisplayState::MAIN) {
//            processor->displayState = DisplayState::PATH_AUTOMATION;
//            //processor->selectAllSourcesIfNoneSelected(); implemented in drawPathControl() instead because it this needs to happen to properly update the view when a new plugin setting is loaded
//        } else {
//            processor->displayState = DisplayState::MAIN;
//        }
        const bool enableAnimation = true;
        tabs.setSelected((int)processor->displayState.load(), enableAnimation);
        if (helpButton.isDown()/*showHelp*/)
            loadHelpText();
            //repaint(); // tell the plugin editor to call paint() soon again when it can so that the help/doppler text state can be updated
        glWindow.resized = true; // make sure that the new view's text is properly scaled
        for (auto& x : pathDisplayList)
            x = 0;
    }
    
    // 'h' to toggle showing the help text
    if (key.getTextDescription().equalsIgnoreCase("H")) {
        //showHelp = ! showHelp;
        helpButton.press();
        if (helpButton.isDown()/*showHelp*/)
            loadHelpText();
        //repaint(); // tell the plugin editor to call paint() soon again when it can so that the help text state can be updated
    }
    
//    if (key.getTextDescription().equalsIgnoreCase("Q")) {
//        processor->lowPassCutoff = processor->lowPassCutoff - 10*arrowKeySpeedFactor;
//        processor->resetLPF();
//    }
//    if (key.getTextDescription().equalsIgnoreCase("W")) {
//        processor->lowPassCutoff = processor->lowPassCutoff + 10*arrowKeySpeedFactor;
//        processor->resetLPF();
//    }
//    if (key.getTextDescription().equalsIgnoreCase("E")) {
//        processor->lowPassOn = ! processor->lowPassOn;
//    }
    
    // undo last action
    if (key.getTextDescription().equalsIgnoreCase("COMMAND + Z") || key.getTextDescription().equalsIgnoreCase("CTRL + Z")) {
        processor->saveCurrentState(1);
        processor->undo();
        if (positionerText.getSelected()) {
            positionerText.releaseFocus();
            pathAutomationPointEditableTextIndex = {-1, -1, -1};
            positionerTextMouseMoved();
        }
    }
    
    // redo last undone action
    if (key.getTextDescription().equalsIgnoreCase("SHIFT + COMMAND + Z") || key.getTextDescription().equalsIgnoreCase("CTRL + SHIFT + Z")) {
        processor->redo();
        positionerTextMouseMoved();
        if (positionerText.getSelected()) {
            positionerText.releaseFocus();
            pathAutomationPointEditableTextIndex = {-1, -1, -1};
            positionerTextMouseMoved();
        }
    }
    
    if (key.getTextDescription().equalsIgnoreCase("D")) {
        processor->toggleDoppler();
        dopplerButton.press();
        if (dopplerButton.isDown()) { // make set value animation happen
            dopplerSlider.setValue(dopplerSlider.getValue());
            dopplerSlider.repaint();
        }
    }
    
    // 'm' to toggle if sources are locked to move on their paths
    if (key.getTextDescription().equalsIgnoreCase("M")) {
        processor->toggleLockSourcesToPaths();
    }
    
    // so holding down arrow keys can cause things to be moved quicker
    if (key.isKeyCode(KeyPress::upKey) || key.isKeyCode(KeyPress::downKey)
        || key.isKeyCode(KeyPress::leftKey) || key.isKeyCode(KeyPress::rightKey))
    {
        if (isTimerRunning(2))
            arrowKeySpeedFactor *= 1.2f;
        startTimer(2, 250); // (re)start timer to see if arrow keys are held down
    }
    
    int numMoved = 0; // number of sources moved if any are selected
    switch (processor->displayState) {
        case DisplayState::MAIN:
            
            // 'p' to toggle the path type for the selected sources
            if (key.getTextDescription().equalsIgnoreCase("P"))
            {
                processor->toggleSelectedSourcesPathType();
            }
            
            // 'd' or backspace to delete selected sources/path points
            if (key.isKeyCode(KeyPress::backspaceKey) /*|| key.getTextCharacter() == 'd' || key.getTextCharacter() == 'D'*/)
            {
                processor->deleteSelectedSources();
                for (auto& x : pathDisplayList)
                    x = 0; // clear these so that old paths aren't drawn if a copy is done later
                for (int i = 0; i < prevSelectedSources.size(); ++i)
                    if (!processor->getSourceSelected(i))
                        prevSelectedSources[i] = false; // prevent deselect animation for incorrect sources
            }
            
            // 'x' to place a path point
            if (key.getTextDescription().equalsIgnoreCase("X"))
            {
                processor->dropPathPoint();
            }
            
            // command/ctrl + 'c' to copy a source and its path control points
            if (key.getTextDescription().equalsIgnoreCase("COMMAND + C") || key.getTextDescription().equalsIgnoreCase("CTRL + C"))
            {
                processor->copySelectedSources();
            }
            
            // command/ctrl + 'a' to select all sources, then select their path points
            if (key.getTextDescription().equalsIgnoreCase("COMMAND + A") || key.getTextDescription().equalsIgnoreCase("CTRL + A"))
            {
                processor->selectAllSources(true);
            }
            
            // directional keys to move selected stuff / view
            if (key.isKeyCode(KeyPress::upKey))
            {
                if (!(key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown())) {
                    float dir[3];
                    if (key.getModifiers().isAltDown())
                        getOnScreenDirection(5, dir);
                    else
                        getOnScreenDirection(0, dir);
                    const float mag = baseArrowKeySpeed * arrowKeySpeedFactor;
                    dir[0] *= mag;  dir[1] *= mag;  dir[2] *= mag;
                    // try to move any selected stuff
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2], positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
//                    if (key.getModifiers().isAltDown()/*.isShiftDown()*/)
//                        numMoved = processor->moveSelectedSourcesXYZ(0, +0.02, 0);
//                    else
//                        numMoved = processor->moveSelectedSourcesXYZ(+0.02, 0, 0);
                }
                
                // if nothing selected move the view
                if (numMoved == 0)
                {
                    // if alt is down move the view in radially
                    if (key.getModifiers().isAltDown()/*isShiftDown()*/)
                    {
                        eyeRad -= 0.05f * eyeRad;
                        
                        // don't move view inside head
                        if (eyeRad < 0.2f)
                            eyeRad = 0.2f;
                        
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if shift is not down, move view ele up (down in number value)
                    {
                        eyeEle -= upDir*4*M_PI/180;
                        
                        if (eyeEle <= 0) {
                            eyeEle *= -1;
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        
                        if (eyeEle >= M_PI) {
                            eyeEle = M_PI-(eyeEle-M_PI);
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
                    
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                }
            }
            if (key.isKeyCode(KeyPress::downKey))
            {
                if (!(key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown())) {
                    float dir[3];
                    if (key.getModifiers().isAltDown())
                        getOnScreenDirection(4, dir);
                    else
                        getOnScreenDirection(2, dir);
                    const float mag = baseArrowKeySpeed * arrowKeySpeedFactor;
                    dir[0] *= mag; dir[1] *= mag; dir[2] *= mag;
                    // try to move any selected stuff
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2], positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
//                    if (key.getModifiers().isAltDown()/*isShiftDown()*/)
//                        numMoved = processor->moveSelectedSourcesXYZ(0, -0.02, 0);
//                    else
//                        numMoved = processor->moveSelectedSourcesXYZ(-0.02, 0, 0);
                }
                // if nothing selected move the view
                if (numMoved == 0)
                {
                    // if shift is down move the view out radially
                    if (key.getModifiers().isAltDown()/*isShiftDown()*/)
                    {
                        eyeRad += 0.05f * eyeRad;
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if shift is not down, move view ele up (down in number value)
                    {
                        eyeEle += upDir*4*M_PI/180;
                        
                        if (eyeEle <= 0) {
                            eyeEle *= -1.0;
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        
                        if (eyeEle >= M_PI) {
                            eyeEle = M_PI-(eyeEle-M_PI);
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
                    
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                }
            }
            if (key.isKeyCode(KeyPress::leftKey))
            {
                if (!(key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown())) {
                    // try to move selected stuff
                    float dir[3];
                    getOnScreenDirection(1, dir);
                    const float mag = baseArrowKeySpeed * arrowKeySpeedFactor;
                    dir[0] *= mag; dir[1] *= mag; dir[2] *= mag;
                    // try to move any selected stuff
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2], positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
//                    numMoved = processor->moveSelectedSourcesXYZ(0, 0, -0.02);
                }
                
                // move the view azimuth
                if (numMoved == 0)
                {
                    eyeAzi += 4*M_PI/180;
                    
                    if (eyeAzi < 0)
                        eyeAzi += 2*M_PI;
                    
                    if (eyeAzi > 2*M_PI)
                        eyeAzi -= 2*M_PI;
                    
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                    //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                }
            }
            if (key.isKeyCode(KeyPress::rightKey))
            {
                if (!(key.getModifiers().isCommandDown() || key.getModifiers().isCtrlDown())) {
                    float dir[3];
                    getOnScreenDirection(3, dir);
                    const float mag = baseArrowKeySpeed * arrowKeySpeedFactor;
                    dir[0] *= mag; dir[1] *= mag; dir[2] *= mag;
                    // try to move any selected stuff
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2], positionerText3DID.sourceIndex >= 0 && positionerText3DID.pathPtIndex == -1);
//                    numMoved = processor->moveSelectedSourcesXYZ(0, 0, +0.02);
                }
                // move the view azimuth
                if (numMoved == 0)
                {
                    eyeAzi -= 4*M_PI/180;
                    
                    if (eyeAzi < 0)
                        eyeAzi += 2*M_PI;
                    
                    if (eyeAzi > 2*M_PI)
                        eyeAzi -= 2*M_PI;
                    
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                    //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                }
            }
            
            // undo / redo might put this out of bounds and cause a crash in updatePositioner3DTextValueAndPosition() below
            if (positionerText3DID.sourceIndex != -1 & positionerText3DID.pathPtIndex != -1 && ! isPositionerText3DIDInBounds()) {
                positionerText3D.hide();
                positionerText3DID = {-1, -1};
            }
            reindexPathIndexTexts();
            if (key.isKeyCode(KeyPress::leftKey) || key.isKeyCode(KeyPress::rightKey) ||
                key.isKeyCode(KeyPress::upKey) || key.isKeyCode(KeyPress::downKey) ||
                key.getTextDescription().containsIgnoreCase("COMMAND + Z") ||
                key.getTextDescription().containsIgnoreCase("CTRL + C")) {
                updatePositioner3DTextValueAndPosition();
            }
            tryHidePositioner3D(); // for delete, copy, etc.
            //reindexPathIndexTexts();
            resizePathPtsPrevState();
            break;
            
        case DisplayState::PATH_AUTOMATION:
            
            // 'd' or backspace to delete selected path automation points
            if (key.isKeyCode(KeyPress::backspaceKey) /*|| key.getTextCharacter() == 'd' || key.getTextCharacter() == 'D'*/)
            {
                if (pathAutomationPointIndexValid(pathAutomationPointEditableTextIndex) &&
                    (*sources)[pathAutomationPointEditableTextIndex[0]].getPathPosPtr()->getPointSelected(pathAutomationPointEditableTextIndex[1])) {
                    pathAutomationPointEditableTextIndex = {-1, -1, -1};
                }
                if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex) &&
                    (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPointSelected(mouseOverPathAutomationPointIndex[1])) {
                    mouseOverPathAutomationPointIndex = {-1, -1, -1};
                }
                processor->deleteSelectedAutomationPoints();
                pathAutomationPointsGrabbedWithMouse = false;
                positionerTextMouseMoved();
            }
            
            // command/ctrl + 'c' to copy all selected sources's path automation points
            if (key.getTextDescription().equalsIgnoreCase("COMMAND + C") || key.getTextDescription().equalsIgnoreCase("CTRL + C"))
            {
                processor->copySelectedPathAutomationPoints();
                sources = nullptr;
                const Locker lock (processor->sources.get(sources));
                if (sources) {
                    // make select animations restart
                    for (int s = 0; s < sources->size(); ++s)
                        for (int i = 0; i < pointStates[s].prevSelecteds.size(); ++i)
                            pointStates[s].prevSelecteds[i] = false;
                    // force grab the newly copied and selected points with the mouse
                    if (!pathAutomationPointsGrabbedWithMouse) {
                        for (int s = 0; s < sources->size(); ++s) {
                            const auto ptsSelected = (*sources)[s].getSelectedPathAutomationPoints();
                            const int index = std::distance(ptsSelected.cbegin(), std::find(ptsSelected.cbegin(), ptsSelected.cend(), true));
                            if (index < ptsSelected.size()) {
                                mouseOverPathAutomationPointIndex[0] = s;
                                mouseOverPathAutomationPointIndex[1] = std::distance(ptsSelected.cbegin(), std::find(ptsSelected.cbegin(), ptsSelected.cend(), true));
                                mouseOverPathAutomationPointIndex[2] = 0;
                                pathAutomationPointsGrabbedWithMouse = true;
                                positionerText.releaseFocus();
                                break;
                            }
                        }
                    }
                }
            }
            
            // if just a 'c' make the selected path auto points segment(s) curvy
            if (key.getTextDescription().equalsIgnoreCase("C"))
            {
                processor->setSelectedPathAutomationPointsSegmentType(1);
            }
            
            // 's' to make the selected path auto points segment(s) straight
            if (key.getTextDescription().equalsIgnoreCase("S"))
            {
                processor->setSelectedPathAutomationPointsSegmentType(0);
            }
            
            // 'o' to make the selected path auto points segment(s) open/empty
            if (key.getTextDescription().equalsIgnoreCase("O"))
            {
                processor->setSelectedPathAutomationPointsSegmentType(2);
            }
            
            // command/ctrl + 'a' to select all selected sources's path automation points
            if (key.getTextDescription().equalsIgnoreCase("COMMAND + A") || key.getTextDescription().equalsIgnoreCase("CTRL + A"))
            {
                processor->selectAllPathAutomationView(true);
                if (pathAutomationPointsGrabbedWithMouse) // gotta do this otherwise anchor point changes and crash possibly happens
                    mouseOverPathAutomationPointIndex[2] = mouseOverPathAutomationPointIndex[1];
            }
            
            // use first and last selected path auto points to define a region of time to loop over, if less than two pts selected, turn off looping
            //if (key.getTextDescription().equalsIgnoreCase("COMMAND + L") || key.getTextDescription().equalsIgnoreCase("CTRL + L") )
            if (key.getTextDescription().equalsIgnoreCase("L"))
            {
                processor->toggleLooping(pathAutomationView.xMinCurrent(), pathAutomationView.xMaxCurrent());
                // lazy, dirty tricks to update positioner text appropriately if mouse is over one of the looping regions
                if (!processor->loopingEnabled || mouseOverLoopRegionBegin || mouseOverLoopRegionEnd) {
                    mouseOverLoopRegionBegin = false;
                    mouseOverLoopRegionEnd = false;
                    loopRegionBeginSelected = false;
                    loopRegionEndSelected = false;
                    loopRegionEndHasPositionerTextFocus = false;
                    loopRegionBeginHasPositionerTextFocus = false;
                } else
                    drawLoopingRegion();
                positionerTextMouseMoved();
                loopRegionToggleAnimation.restart();
                //processor->defineLoopingRegionUsingSelectedPathAutomationPoints();
            }
            
            // decided to abandon this feature since I can't find a good way to deal with tempo / bpm changes, but leaving the partially working code intact for now
//            // toggle time display mode from sec / beats
//            if (key.getTextDescription().equalsIgnoreCase("T"))
//            {
//                timeMode = (timeMode + 1) % 2;
//                auto ir = dynamic_cast<PathAutomationInputRestrictor*>(positionerText.getTextInputRestrictor());
//                ir->setTimeMode((PathAutomationInputRestrictor::TimeMode)timeMode);
//                positionerTextMouseMoved();
//                if (positionerText.getSelected()) {
//                    //positionerText.releaseFocus();
//                    positionerText.gainFocus();
//                }
//            }
            
            // directional keys to move selected stuff / view
            if (key.isKeyCode(KeyPress::upKey))
            {
                if (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()) {
                    if (processor->areAnySelectedSourcesPathAutomationPointsSelected()) {
                        goto MOVE_2D_VIEW_IN;
                    }
                    // if alt is down move the view in radially
                    if (key.getModifiers().isAltDown()/*isShiftDown()*/)
                    {
                        eyeRad -= 0.2f;
                        // don't move view inside head
                        if (eyeRad < 0.2f)
                            eyeRad = 0.2f;
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if alt is not down, move view ele up (down in number value)
                    {
                        eyeEle -= upDir*4*M_PI/180;
                        if (eyeEle <= 0) {
                            eyeEle *= -1;
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        if (eyeEle >= M_PI) {
                            eyeEle = M_PI-(eyeEle-M_PI);
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                } else {
                    numMoved = processor->moveSelectedPathAutomationPoints(0, 0.005f*arrowKeySpeedFactor);
                    if (key.getModifiers().isAltDown() && numMoved > 0) {
                        //const bool alignInX = false;
                        autoAlignAutomationPoints(false, true, true);
                    }
                    // adjust the automation grid view in zoom
                    if (numMoved == 0) {
                    MOVE_2D_VIEW_IN:
                        float x;
                        if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                            const auto viewX = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1])[0];
                            x = pathAutomationView.viewToHolderX(viewX);
                        } else
                            x = getMouseX();
                        auto deltaX = pathAutomationView.holderToViewX(x);
                        pathAutomationView.setWidth(pathAutomationView.getWidth() * 0.8f);
                        deltaX -= pathAutomationView.holderToViewX(x);
                        pathAutomationView.setXPosition(pathAutomationView.getXPosition() + deltaX);
                        for (auto& dl : pathAutomationDisplayList)
                            dl = 0;
                    }
//                    if (animationOn[1])
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
                }
            }
            if (key.isKeyCode(KeyPress::downKey))
            {
                if (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()) {
                    if (processor->areAnySelectedSourcesPathAutomationPointsSelected()) {
                        goto MOVE_2D_VIEW_OUT;
                    }
                    // if alt is down move the view out radially
                    if (key.getModifiers().isAltDown()/*isShiftDown()*/)
                    {
                        eyeRad += 0.2f;
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if alt is not down, move view ele up (down in number value)
                    {
                        eyeEle += upDir*4*M_PI/180;
                        if (eyeEle <= 0) {
                            eyeEle *= -1;
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        if (eyeEle >= M_PI) {
                            eyeEle = M_PI-(eyeEle-M_PI);
                            eyeAzi = M_PI+eyeAzi;
                            upDir *= -1;
                        }
                        //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                } else {
                    numMoved = processor->moveSelectedPathAutomationPoints(0, -0.005f*arrowKeySpeedFactor);
                    if (key.getModifiers().isAltDown() && numMoved > 0) {
                        autoAlignAutomationPoints(false, true, true);
                    }
                    if (numMoved == 0) {
                    MOVE_2D_VIEW_OUT:
                        float x;
                        if (pathAutomationPointIndexValid(mouseOverPathAutomationPointIndex)) {
                            const auto viewX = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1])[0];
                            x = pathAutomationView.viewToHolderX(viewX);
                        } else
                            x = getMouseX();
                        auto deltaX = pathAutomationView.holderToViewX(x);
                        pathAutomationView.setWidth(pathAutomationView.getWidth() * 1.2f);
                        deltaX -= pathAutomationView.holderToViewX(x);
                        pathAutomationView.setXPosition(pathAutomationView.getXPosition() + deltaX);
                        for (auto& dl : pathAutomationDisplayList)
                            dl = 0;
                    }
                }
            }
            if (key.isKeyCode(KeyPress::leftKey))
            {
                // move the view azimuth
                if (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()) {
                    if (processor->areAnySelectedSourcesPathAutomationPointsSelected()) {
                        goto MOVE_2D_VIEW_LEFT;
                    }
                    eyeAzi += 4*M_PI/180;
                    if (eyeAzi > 2*M_PI)
                        eyeAzi -= 2*M_PI;
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                    //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                } else {
                    numMoved = processor->moveSelectedPathAutomationPoints(-0.005f * arrowKeySpeedFactor * pathAutomationView.getWidth(), 0);
                    if (key.getModifiers().isAltDown() && numMoved > 0) {
                        autoAlignAutomationPoints(true, false, true);
                    }
                    if (numMoved == 0) {
                    MOVE_2D_VIEW_LEFT:
                        pathAutomationView.setXPosition(pathAutomationView.getXPosition() - 0.02f * pathAutomationView.getWidth() * arrowKeySpeedFactor);
                        positionerTextMouseMoved();
                        for (auto& dl : pathAutomationDisplayList)
                            dl = 0;
                    }
//                    if (animationOn[0])
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
                }
            }
            if (key.isKeyCode(KeyPress::rightKey))
            {
                // move the view azimuth
                if (key.getModifiers().isCtrlDown() || key.getModifiers().isCommandDown()) {
                    if (processor->areAnySelectedSourcesPathAutomationPointsSelected()) {
                        goto MOVE_2D_VIEW_RIGHT;
                    }
                    eyeAzi -= 4*M_PI/180;
                    if (eyeAzi < 0)
                        eyeAzi += 2*M_PI;
                    updateEyePositionRAE(eyeRad, eyeAzi, eyeEle);
                    //updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                } else {
                    numMoved = processor->moveSelectedPathAutomationPoints(0.005f * arrowKeySpeedFactor * pathAutomationView.getWidth(), 0);
                    if (key.getModifiers().isAltDown() && numMoved > 0) {
                        autoAlignAutomationPoints(true, false, true);
                    }
                    if (numMoved == 0) {
                    MOVE_2D_VIEW_RIGHT:
                        // adjust the automation grid view in width
                        pathAutomationView.setXPosition(pathAutomationView.getXPosition() + 0.02f * pathAutomationView.getWidth() * arrowKeySpeedFactor);
                        positionerTextMouseMoved();
                        for (auto& dl : pathAutomationDisplayList)
                            dl = 0;
                    }
//                    if (animationOn[0])
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
                }
            }
            
            reindexPathIndexTexts();
            
            break;
        case DisplayState::SETTINGS:
            break;
        case DisplayState::NUM_DISPLAY_STATES:
            break;
    }
    
    // return true to consume KeyPress event, false to pass to other listeners
    return true;
}


bool ThreeDAudioProcessorEditor::isPositionerText3DIDInBounds() const noexcept
{
    const Sources* sources;
    const Locker lock (processor->sources.get(sources));
    //auto lockedSources = std::move(processor->sources.get());
    if (!sources) return false;
    return positionerText3DID.sourceIndex == std::clamp(positionerText3DID.sourceIndex, 0, std::max((int)sources->size() - 1, 0))
        && positionerText3DID.pathPtIndex == std::clamp(positionerText3DID.pathPtIndex,
                                                        0, std::max((*sources)[positionerText3DID.sourceIndex].getNumPathPoints() - 1, 0));
}

void ThreeDAudioProcessorEditor::reindexPathIndexTexts()
{
    //if (openGLContext.isActive()) { // not sure why I needed this (written for mac), its causing problems on windows without solving any known problems
    Sources* sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        pathIndexTexts.resize(sources->size());
        for (int s = 0; s < pathIndexTexts.size(); ++s) {
            if ((*sources)[s].getSourceSelected()) {
                cauto pts = (*sources)[s].getPathPoints();
                pathIndexTexts[s].resize(pts.size());
                for (int i = 0; i < pathIndexTexts[s].size(); ++i) {
                    if (positionerText3DID.sourceIndex == s && positionerText3DID.pathPtIndex == i) {
                        pathIndexTexts[s][i] = std::make_unique<EditableTextBox>(TextBox(), &glWindow);
                        auto q = ((EditableTextBox*) pathIndexTexts[s][i].get());
                        q->setTextInputRestrictor(new DecimalNumberRestrictor(0, pts.size() - 1, 0));
                        q->extendToFitSameSizeFont = true;
                        q->horizontalAnchor = EditableTextBox::HorizontalAnchor::CENTER;
                    }
                    else
                        pathIndexTexts[s][i] = std::make_unique<TextBox>();
                    auto txt = std::to_string(i);
                    pathIndexTexts[s][i]->setText(txt);
                    if ((*sources)[s].getPathPointSelected(i))
                        pathIndexTexts[s][i]->setLook(&pathIndexSelectedTextLook);
                    else
                        pathIndexTexts[s][i]->setLook(&pathIndexTextLook);
                    float xy[2] = {101, 101};
                    to2D({pts[i][0], pts[i][1], pts[i][2]}, xy);
                    cauto wd2 = 0.5f * pixelsToNormalized(pathIndexTexts[s][i]->getFont(0, 0).getStringWidthFloat(txt) + 6, glWindow.width)
                                / pathIndexTexts[s][i]->getLook()->horizontalPad;
                    cauto hd2 = 0.5f * pixelsToNormalized(pathIndexTexts[s][i]->getLook()->fontSize, glWindow.height)
                                / pathIndexTexts[s][i]->getLook()->verticalPad;
                    pathIndexTexts[s][i]->setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
                }
            }
        }
    }
    //}
}

void ThreeDAudioProcessorEditor::repositionPathIndexTexts()
{
    Sources* sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        for (int s = 0; s < pathIndexTexts.size(); ++s) {
            if ((*sources)[s].getSourceSelected()) {
                cauto pts = (*sources)[s].getPathPoints();
                for (int i = 0; i < pathIndexTexts[s].size(); ++i) {
                    float xy[2] = {101, 101};
                    to2D({pts[i][0], pts[i][1], pts[i][2]}, xy);
                    //cauto wd2 = 0.5f * pathIndexTexts[s][i]->getBoundary().width();
                    cauto txt = pathIndexTexts[s][i]->getText();
                    cauto wd2 = 0.5f * pixelsToNormalized(pathIndexTexts[s][i]->getFont(0, 0).getStringWidthFloat(txt) + 6, glWindow.width)
                                / pathIndexTexts[s][i]->getLook()->horizontalPad;
                    cauto hd2 = 0.5f * pixelsToNormalized(pathIndexTexts[s][i]->getLook()->fontSize, glWindow.height)
                                / pathIndexTexts[s][i]->getLook()->verticalPad;
                    cauto b = Box(xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2);
                    if (positionerText3DID.equals(s, i))
                        ((EditableTextBox*)pathIndexTexts[s][i].get())->setBoundary(b);
                    else
                        pathIndexTexts[s][i]->setBoundary(b);
                }
            }
        }
    }
}

//void ThreeDAudioProcessorEditor::drawCircle(float x, float y, float r, int segments) const
//{
//    glBegin( GL_TRIANGLE_FAN );
//    glVertex2f(x, y);
//    float aspect = ((float)getHeight())/((float)getWidth());
//    for( int n = 0; n <= segments; ++n ) {
//        float const t = 2*M_PI*(float)n/(float)segments;
//        glVertex2f(x + aspect*std::sin(t)*r, y + std::cos(t)*r);
//    }
//    glEnd();
//}

//void ThreeDAudioProcessorEditor::showMouseDetails() const
//{
//    double mouse_x = getMouseXYRelative().getX();//(2.0*getMouseXYRelative().getX())/getWidth() - 1.0;
//    double mouse_y = getMouseXYRelative().getY();//-1.0*((2.0*getMouseXYRelative().getY())/getHeight() - 1.0);
//    
//    char str[20];
//    sprintf(str, "MouseZoom: %f", mouseZoomFactor);
//    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.9, 0);
//    sprintf(str, "MouseWheel dX: %f", mouseWheeldX);
//    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.8, 0);
//    sprintf(str, "MouseWheel dY: %f", mouseWheeldY);
//    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.7, 0);
//    //    sprintf(str, "MouseWheel Rev: %d", mouseWheelReversed);
//    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.6, 0);
//    //    sprintf(str, "MouseWheel Smo: %d", mouseWheelSmooth);
//    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.5, 0);
//    sprintf(str, "Mouse X:  %f", mouse_x);
//    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.4, 0);
//    sprintf(str, "Mouse Y:  %f", mouse_y);
//    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.3, 0);
//    sprintf(str, "Mouse Down:  %d", mouseIsDown);
//    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.2, 0);
//}

//void ThreeDAudioProcessorEditor::setProcessor(ThreeDAudioProcessor * processorPtr)
//{
//    processor = processorPtr;
//}

void ThreeDAudioProcessorEditor::repositionAxis3dLabels()
{
    cauto hd2 = pixelsToNormalized(axis3dTextLook.fontSize, getHeight());
    cauto wd2 = pixelsToNormalized(axis3dTextLook.fontSize, getWidth());
    float xy[2];
    to2D({1.1f, 0, 0}, xy);
    axis3dLabels[0].setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
    to2D({-1.1f, 0, 0}, xy);
    axis3dLabels[1].setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
    to2D({0, 0, -1.1f}, xy);
    axis3dLabels[2].setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
    to2D({0, 0, 1.1f}, xy);
    axis3dLabels[3].setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
    to2D({0, 1.1f, 0}, xy);
    axis3dLabels[4].setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
    to2D({0, -1.1f, 0}, xy);
    axis3dLabels[5].setBoundary({xy[1] + hd2, xy[1] - hd2, xy[0] - wd2, xy[0] + wd2});
//    axis3dLabels[1] = TextBox("B", Box(), &axis3dTextLook);
//    axis3dLabels[2] = TextBox("L", Box(), &axis3dTextLook);
//    axis3dLabels[3] = TextBox("R", Box(), &axis3dTextLook);
//    axis3dLabels[4] = TextBox("U", Box(), &axis3dTextLook);
//    axis3dLabels[5] = TextBox("D", Box(), &axis3dTextLook);
}

void ThreeDAudioProcessorEditor::updateEyePositionRAE(cfloat rad, cfloat azi, cfloat ele)
{
    float rae[3] = {rad, azi, ele};
    float xyz[3];
    RAEtoXYZ(rae, xyz);
    updateEyePositionXYZ(xyz[0], xyz[1], xyz[2]);
}

void ThreeDAudioProcessorEditor::updateEyePositionXYZ(cfloat x, cfloat y, cfloat z)
{
    eyePos[0] = x;
    eyePos[1] = y;
    eyePos[2] = z;
    
    repositionAxis3dLabels();
}

//void ThreeDAudioProcessorEditor::updateXYZ(float rad, float azi, float ele, float *xyz) const
//{
//    const float rae[3] = {rad, azi, ele};
//    RAEtoXYZ(rae, xyz);
//}

// map 3d world coord to a 2d screen coord, returns true if the object is in front of the camera, false if behind
bool ThreeDAudioProcessorEditor::to2D(const float (&xyz)[3], float (&xy)[2]) const
{
    // distance from origin to eye
    const float eyeMag = std::sqrt(eyePos[0]*eyePos[0]
                                 + eyePos[1]*eyePos[1]
                                 + eyePos[2]*eyePos[2]);
    
    // normal vector of plane perpendicular to camera pos
    const float _n[3] = { eyePos[0]/eyeMag,
                          eyePos[1]/eyeMag,
                          eyePos[2]/eyeMag };
    
    // line vector from pt of interest to camera
    const float d[3] = { eyePos[0]-xyz[0],
                         eyePos[1]-xyz[1],
                         eyePos[2]-xyz[2] };

    // find this distance
    const float dMag = std::sqrt(d[0]*d[0] + d[1]*d[1] + d[2]*d[2]);
    
    // find the appropriated 2d coord
    
    // basis vectors for plane that screen coords map to
    float _x[3];
    float _y[3];
    
    _x[0] = -eyePos[2];
    _x[1] = 0;
    _x[2] = eyePos[0];
    const float _xMag = std::sqrt(_x[0]*_x[0] + _x[1]*_x[1] + _x[2]*_x[2]);
    _x[0] /= _xMag;
    _x[1] /= _xMag;
    _x[2] /= _xMag;
    
    _y[0] = eyePos[1]*_x[2];
    _y[1] = eyePos[2]*_x[0] - eyePos[0]*_x[2];
    _y[2] = -eyePos[1]*_x[0];
    const float _yMag = std::sqrt(_y[0]*_y[0] + _y[1]*_y[1] + _y[2]*_y[2]);
    _y[0] /= _yMag;
    _y[1] /= _yMag;
    _y[2] /= _yMag;
    
    // solve for parametric var that gives us the coord of pt projected onto "screen's plane"
    const float t = (-1.0*xyz[0]*_n[0] + -1.0*xyz[1]*_n[1] + -1.0*xyz[2]*_n[2]) / (d[0]*_n[0] + d[1]*_n[1] + d[2]*_n[2]);
    
    // point of interest's projection onto plane perpendicular to camera pos
    const float pt[3] = { xyz[0] + t*d[0],
                          xyz[1] + t*d[1],
                          xyz[2] + t*d[2] };
    
    const float fovy = M_PI/180 * 45.0/2.0;
    const float w = getWidth();
    const float h = getHeight();
    const float fovx = fovy;
    const float xMag = eyeMag * std::tan(fovy) * w/h;
    const float yMag = eyeMag * std::tan(fovx);
    const float sign = -upDir/std::abs(upDir);
    
    // find each component of this projected point's basis vectors that make up the plane to get corresponding xy sreen coord
    xy[0] = (_x[0]*pt[0] + _x[1]*pt[1] + _x[2]*pt[2])*sign/xMag;
    xy[1] = (_y[0]*pt[0] + _y[1]*pt[1] + _y[2]*pt[2])*sign/yMag;
    
    // find angle between origin-to-camera and pt-to-camera vectors
    const float theta = std::acos((eyePos[0]*d[0] + eyePos[1]*d[1] + eyePos[2]*d[2]) / (dMag*eyeMag));
    if (theta < M_PI/2.0)
        return true;
    
    return false;
}

// map 2d screen coord to 3d world coord of intersecting pt on a plane perpendicular to the camera (the world origin defines the "center" of the plane)
void ThreeDAudioProcessorEditor::to3D(const float x, const float y, float* xyz) const
{
    // basis vectors for plane that screen coords map to
    float _x[3];
    float _y[3];
    
    const float eyeMag = std::sqrt(eyePos[0]*eyePos[0] + eyePos[1]*eyePos[1] + eyePos[2]*eyePos[2]);
    const float fovy = M_PI/180 * 45.0/2.0;
    const float w = getWidth();
    const float h = getHeight();
    const float fovx = fovy;
    const float xMag = eyeMag * std::tan(fovy) * w/h;
    const float yMag = eyeMag * std::tan(fovx);
    
    _x[0] = -eyePos[2];
    _x[1] = 0;
    _x[2] = eyePos[0];
    const float _xMag = std::sqrt(_x[0]*_x[0] + _x[1]*_x[1] + _x[2]*_x[2]);
    _x[0] /= _xMag;
    _x[1] /= _xMag;
    _x[2] /= _xMag;
    
    _y[0] = eyePos[1]*_x[2];
    _y[1] = eyePos[2]*_x[0] - eyePos[0]*_x[2];
    _y[2] = -eyePos[1]*_x[0];
    const float _yMag = std::sqrt(_y[0]*_y[0] + _y[1]*_y[1] + _y[2]*_y[2]);
    _y[0] /= _yMag;
    _y[1] /= _yMag;
    _y[2] /= _yMag;
    
    const float sign = -upDir/std::abs(upDir);
    xyz[0] = (sign*xMag*_x[0]*x + sign*yMag*_y[0]*y);
    xyz[1] = (sign*xMag*_x[1]*x + sign*yMag*_y[1]*y);
    xyz[2] = (sign*xMag*_x[2]*x + sign*yMag*_y[2]*y);
}

bool ThreeDAudioProcessorEditor::pointInsideSelectRegion(const float (&xyz)[3]) const
//bool ThreeDAudioProcessorEditor::pointInsideSelectRegion(const std::array<float,3>& xyz) const
{
    if (!selectionBox.isActive()/*mouseDragging*/)
        return false;
    
    // convert sources center xyz point to 2d screen coords
    float xy[2];
    const bool inFrontOfEye = to2D(xyz, xy);
    
//    // boundaries of selection region depend on how it is drawn
//    float xLow, xHigh, yLow, yHigh;
//    if (mouseDragDownX > mouseDragCurrentX) {
//        xLow = mouseDragCurrentX;
//        xHigh = mouseDragDownX;
//    } else {
//        xLow = mouseDragDownX;
//        xHigh = mouseDragCurrentX;
//    }
//    if (mouseDragDownY > mouseDragCurrentY) {
//        yLow = mouseDragCurrentY;
//        yHigh = mouseDragDownY;
//    } else {
//        yLow = mouseDragDownY;
//        yHigh = mouseDragCurrentY;
//    }
    
    // if the 2d screen coordinate of that source's center is on screen and in the selection region, mark it as a selected source
    if (inFrontOfEye && selectionBox.contains({xy[0], xy[1]}) /*contain(selectionBox, {xy[0], xy[1]})*/
        /*std::abs(xy[0]) <= 1.0 && std::abs(xy[1]) <= 1.0
        && xy[0] > xLow && xy[0] < xHigh && xy[1] > yLow && xy[1] < yHigh*/)
    {
        return true;
    }
    return false;
}

bool ThreeDAudioProcessorEditor::pointInsideSelectRegion2D(const float (&xy)[2]) const
{
    if (!selectionBox.isActive()/*mouseDragging*/)
        return false;
    
//    // boundaries of selection region depend on how it is drawn
//    float xLow, xHigh, yLow, yHigh;
//    if (mouseDragDownX > mouseDragCurrentX) {
//        xLow = mouseDragCurrentX;
//        xHigh = mouseDragDownX;
//    } else {
//        xLow = mouseDragDownX;
//        xHigh = mouseDragCurrentX;
//    }
//    if (mouseDragDownY > mouseDragCurrentY) {
//        yLow = mouseDragCurrentY;
//        yHigh = mouseDragDownY;
//    } else {
//        yLow = mouseDragDownY;
//        yHigh = mouseDragCurrentY;
//    }
    
    // if the 2d screen coordinate of that source's center is in the selection region, mark it as a selected source
    if (selectionBox.contains({xy[0], xy[1]}) /*contain(selectionBox, {xy[0], xy[1]})*/
        /*std::abs(xy[0]) <= 1.0 && std::abs(xy[1]) <= 1.0
        && xy[0] > xLow && xy[0] < xHigh && xy[1] > yLow && xy[1] < yHigh*/)
    {
        return true;
    }
    return false;
}

// fill one dim of the xyz vector with a +/-1 to indicate which xyz direction aligns closest to the on-screen xy direction requested; dir = 0 is up, 1 is left, 2 is down, 3 is right, 4 forward, 5 back
void ThreeDAudioProcessorEditor::getOnScreenDirection(const int dir, float (&xyz)[3]) const
{
    if (dir == 4 || dir == 5)
    {// maybe not the most efficient and elegant (not called often enough from keyPressed() to really matter either), but this ensures forward/back are mutually exclusive from up/down, left/right
        float test1[3], test2[3];
        getOnScreenDirection(0, test1);
        getOnScreenDirection(1, test2);
        xyz[0] = test1[0] + test2[0];
        xyz[1] = test1[1] + test2[1];
        xyz[2] = test1[2] + test2[2];
        int sign = 1;
        // woe to me and my inconsistent logic!
        if (test1[0] < 0 || test1[1] < 0 || test1[2] > 0)
            sign *= -1;
        if (test2[0] > 0 || test2[1] > 0 || test2[2] < 0)
            sign *= -1;
        // we loose the sign info by doing this masking thing which is why we need all the if's for flipping sign
        if (xyz[0] == 0) xyz[0] = 1;
        else             xyz[0] = 0;
        if (xyz[1] == 0) xyz[1] = 1;
        else             xyz[1] = 0;
        if (xyz[2] == 0) xyz[2] = 1;
        else             xyz[2] = 0;
        if (dir == 5) { xyz[0] *= -1; xyz[1] *= -1; xyz[2] *= -1; }
        if (xyz[1] == 0) {
            xyz[0] *= sign; xyz[1] *= sign; xyz[2] *= sign;
        } else if (eyeEle > M_PI_2) { // just yikes man, but it works
            xyz[1] *= -1;
        }
    }
    else {
        float zp[2], zm[2], yp[2], ym[2], xp[2], xm[2];
        to2D({0,0,+1}, zp);
        to2D({0,0,-1}, zm);
        to2D({0,+1,0}, yp);
        to2D({0,-1,0}, ym);
        to2D({+1,0,0}, xp);
        to2D({-1,0,0}, xm);
        int sign, xory;
        switch (dir) {
            case 0:
                sign = 1; xory = 1;
                break;
            case 1:
                sign = -1; xory = 0;
                break;
            case 2:
                sign = -1; xory = 1;
                break;
            case 3:
                sign = 1; xory = 0;
                break;
            default: // didn't get passed a valid direction
                xyz[0] = 0; xyz[1] = 0; xyz[2] = 0;
                return;
        }
        std::array<float,3> dirs { std::max(std::abs(zp[xory]), std::abs(zm[xory])),
                                   std::max(std::abs(yp[xory]), std::abs(ym[xory])),
                                   std::max(std::abs(xp[xory]), std::abs(xm[xory])) };
        const int dist = std::distance(dirs.begin(), std::max_element(dirs.begin(), dirs.end()));
        switch (dist) {
            case 0:
                if (zp[xory] < 0) sign *= -1;
                xyz[0] = 0; xyz[1] = 0; xyz[2] = sign;
                break;
            case 1:
                if (yp[xory] < 0) sign *= -1;
                xyz[0] = 0; xyz[1] = sign; xyz[2] = 0;
                break;
            case 2:
                if (xp[xory] < 0) sign *= -1;
                xyz[0] = sign; xyz[1] = 0; xyz[2] = 0;
                break;
            default: // didn't find a valid maximum direction
                xyz[0] = 0; xyz[1] = 0; xyz[2] = 0;
                return;
        }
    }
}

bool ThreeDAudioProcessorEditor::pathAutomationPointIndexValid(const std::array<int, 3>& index) const noexcept
{
    return (index[0] != -1 && index[1] != -1) &&
        index[1] < (*sources)[index[0]].getPathPosPtr()->getNumPoints() &&
        index[2] < (*sources)[index[0]].getPathPosPtr()->getNumSelectedPoints();
}



TextInputRestrictor* PathAutomationInputRestrictor::clone() const
{
    return new PathAutomationInputRestrictor(*this);
}

bool PathAutomationInputRestrictor::insert(std::string& text,
                                           int& cursorPosition,
                                           const std::string& textToInsert) const
{
    auto result = false;
    auto div = text.length();
    for (const auto& c : divString) {
        div = std::distance(text.begin(), std::find(text.begin(), text.end(), c));
        if (div < text.length())
            break;
    }
    //auto div = std::distance(text.begin(), std::find(text.begin(), text.end(), divString.front()));
    if (div < text.length()) {
        if (cursorPosition <= div) {
            auto timePosText = text.substr(0, div);
            result = timePosition->insert(timePosText, cursorPosition, textToInsert);
            text = timePosText + text.substr(div, text.length());
        } else {
            while (std::find(divString.begin(), divString.end(), text[div]) != divString.end() && div < text.length())
                ++div;
            auto pathPosText = text.substr(div, text.length());
            cursorPosition -= div;
            result = pathPosition.insert(pathPosText, cursorPosition, textToInsert);
            cursorPosition += div;
            text = text.substr(0, div) + pathPosText;
        }
    } else {
        result = timePosition->insert(text, cursorPosition, textToInsert);
    }
    if (!result && std::find(divString.begin(), divString.end(), textToInsert.front()) != divString.end()
        && std::count(text.begin(), text.end(), textToInsert.front()) < std::count(divString.begin(), divString.end(), textToInsert.front())) {
        text.insert(cursorPosition++, textToInsert);
        result = true;
    }
    return result;
    
//    // just this happens to work for filtering the pathPosition also...
//    bool result = timePosition->insert(text, cursorPosition, textToInsert);//... and a decimal for the percent position
//    // except for the divString characters
//    if (!result && std::find(divString.begin(), divString.end(), textToInsert.front()) != divString.end()) {
//        text.insert(cursorPosition++, textToInsert);
//        result = true;
//    }
//    return result;
}

void PathAutomationInputRestrictor::validateInput(std::string& text) const
{
    //timePosition->validateInput(text);
    int div = text.length();
    for (const auto& c : divString) {
        div = std::distance(text.begin(), std::find(text.begin(), text.end(), c));
        if (div < text.length())
            break;
    }
    //auto div = std::distance(text.begin(), std::find(text.begin(), text.end(), divString.front()));
    if (div != text.length()) {
        auto timePosText = text.substr(0, div);
        timePosition->validateInput(timePosText);
        while (std::find(divString.begin(), divString.end(), text[div]) != divString.end() && div < text.length()) {
            ++div;
        }
        auto pathPosText = text.substr(div/*+ divString.length()*/, text.length());
        pathPosition.validateInput(pathPosText);
        text = timePosText + divString + pathPosText;
    } else {
        timePosition->validateInput(text);
    }
}

void PathAutomationInputRestrictor::setTimeMode(TimeMode mode)
{
    if (mode == TimeMode::HR_MIN_SEC) {
        //PolyPtr<TextInputRestrictor> tmp = makePolyPtr<HoursMinSecInputRestrictor>();
        timePosition = makePolyPtr<HoursMinSecInputRestrictor>();//tmp;
    } else if (mode == TimeMode::MEAS_BEAT_FRAC) {
//        timePosition.reset();
        timePosition = makePolyPtr<MeasureBeatFractionInputRestrictor>();
    }
}



HoursMinSecInputRestrictor::HoursMinSecInputRestrictor(const int numDecimalPlacesForSec)
    : secIR ({0, 60, numDecimalPlacesForSec}),
      minIR ({0, 60, 0}),
      hrIR  ({0, std::numeric_limits<float>::infinity(), 0})
{
}

TextInputRestrictor* HoursMinSecInputRestrictor::clone() const
{
    return new HoursMinSecInputRestrictor(*this);
}

bool HoursMinSecInputRestrictor::insert(std::string& text,
                                        int& cursorPosition,
                                        const std::string& textToInsert) const
{
    bool result = false;
    if (textToInsert == ":") {
        if (std::count(text.begin(), text.end(), ':') < 2
            && cursorPosition <= text.find('.')) {
            text.insert(cursorPosition++, textToInsert);
            result = true;
        }
    } else {
        const int minSecDiv = text.rfind(':');
        if (minSecDiv < 0) { // no ':' in text
            result = secIR.insert(text, cursorPosition, textToInsert);
        } else if (cursorPosition > minSecDiv) { // in sec field
            auto secTxt = text.substr(minSecDiv, text.length());
            int secCurPos = cursorPosition - minSecDiv;
            result = secIR.insert(secTxt, secCurPos, textToInsert);
            text.replace(minSecDiv, text.length() - minSecDiv, secTxt);
            cursorPosition = secCurPos + minSecDiv;
        } else {
            auto hrMinTxt = text.substr(0, minSecDiv);
            const int hrMinDiv = hrMinTxt.rfind(':');
            if (hrMinDiv < 0) { // no second ':' in text, in min field
                auto minTxt = hrMinTxt;
                result = minIR.insert(minTxt, cursorPosition, textToInsert);
                text.replace(0, minSecDiv, minTxt);
            } else if (cursorPosition > hrMinDiv) { // in min field
                auto minTxt = hrMinTxt.substr(hrMinDiv, hrMinTxt.length());
                int minCurPos = cursorPosition - hrMinDiv;
                result = minIR.insert(minTxt, minCurPos, textToInsert);
                text.replace(hrMinDiv, hrMinTxt.length() - hrMinDiv, minTxt);
                cursorPosition = minCurPos + hrMinDiv;
            } else { // in hour field
                auto hrTxt = hrMinTxt.substr(0, hrMinDiv);
                result = hrIR.insert(hrTxt, cursorPosition, textToInsert);
                text.replace(0, hrMinDiv, hrTxt);
            }
        }
    }
    return result;
}

void HoursMinSecInputRestrictor::validateInput(std::string& text) const
{
//    if (text.empty())
//        return;
    int i, count = 0;
    std::string hr, min, sec;
    while ((i = text.rfind(':')) >= 0) {
        count++;
        if (count == 1) {
            sec = text.substr(i + 1, text.npos);
            min = text.substr(0, i);
            text = min;//text.substr(0, i);
        } else if (count == 2) {
            min = text.substr(i + 1, text.npos);
            hr = text.substr(0, i);
            break;
        }
    }
    if (sec.empty())
        sec = text;
    text = "";
    if (! hr.empty()) {
        hrIR.validateInput(hr);
        text += hr + ":";
    }
    if (! min.empty()) {
        minIR.validateInput(min);
        text += min + ":";
    }
    if (! sec.empty()) {
        secIR.validateInput(sec);
        text += sec;
    }
}



MeasureBeatFractionInputRestrictor::MeasureBeatFractionInputRestrictor()
    : fracIR        ({0, 100, 0}),
      measAndBeatIR ({1, std::numeric_limits<float>::infinity(), 0})
{
}

TextInputRestrictor* MeasureBeatFractionInputRestrictor::clone() const
{
    return new MeasureBeatFractionInputRestrictor(*this);
}

bool MeasureBeatFractionInputRestrictor::insert(std::string& text,
                                                int& cursorPosition,
                                                const std::string& textToInsert) const
{
    bool result = false;
    if (textToInsert == "|") {
        if (std::count(text.begin(), text.end(), '|') < 2) {
            text.insert(cursorPosition++, textToInsert);
            result = true;
        }
    } else {
        const int beatFracDiv = text.rfind('|');
        if (beatFracDiv < 0) { // no '|' in text
            result = fracIR.insert(text, cursorPosition, textToInsert);
        } else if (cursorPosition > beatFracDiv) { // in fraction field
            auto fracTxt = text.substr(beatFracDiv, text.length());
            int fracCurPos = cursorPosition - beatFracDiv;
            result = fracIR.insert(fracTxt, fracCurPos, textToInsert);
            text.replace(beatFracDiv, text.length() - beatFracDiv, fracTxt);
            cursorPosition = fracCurPos + beatFracDiv;
        } else {
            auto measBeatTxt = text.substr(0, beatFracDiv);
            const int measBeatDiv = measBeatTxt.rfind('|');
            if (measBeatDiv < 0) { // no second '|' in text, in beat field
                auto beatTxt = measBeatTxt;
                result = measAndBeatIR.insert(beatTxt, cursorPosition, textToInsert);
                text.replace(0, beatFracDiv, beatTxt);
            } else if (cursorPosition > measBeatDiv) { // in beat field
                auto beatTxt = measBeatTxt.substr(measBeatDiv, measBeatTxt.length());
                int beatCurPos = cursorPosition - measBeatDiv;
                result = measAndBeatIR.insert(beatTxt, beatCurPos, textToInsert);
                text.replace(measBeatDiv, measBeatTxt.length() - measBeatDiv, beatTxt);
                cursorPosition = beatCurPos + measBeatDiv;
            } else { // in hour field
                auto measTxt = measBeatTxt.substr(0, measBeatDiv);
                result = measAndBeatIR.insert(measTxt, cursorPosition, textToInsert);
                text.replace(0, measBeatDiv, measTxt);
            }
        }
    }
    return result;
    //return true;
}

void MeasureBeatFractionInputRestrictor::validateInput(std::string& text) const
{
    int i, count = 0;
    std::string meas, beat, frac;
    while ((i = text.rfind('|')) >= 0) {
        count++;
        if (count == 1) {
            frac = text.substr(i + 1, text.npos);
            beat = text.substr(0, i);
            meas = beat;
        } else if (count == 2) {
            beat = text.substr(i + 1, text.npos);
            meas = text.substr(0, i);
            break;
        }
    }
    if (frac.empty())
        frac = text;
    text = "";
    if (! meas.empty()) {
        measAndBeatIR.validateInput(meas);
        text += meas + "|";
    }
    if (! beat.empty()) {
        measAndBeatIR.validateInput(beat);
        text += beat + "|";
    }
    if (! frac.empty()) {
        fracIR.validateInput(frac);
        text += frac;
    }
}

PositionerText3D::PositionerText3D(OpenGLWindow* window) noexcept
    : raeText ({}, window),
      xyzText ({}, window),
      visible (false)
{
    const auto pInf = std::numeric_limits<float>::infinity();
    const auto nInf = -std::numeric_limits<float>::infinity();
    TextInputRestrictor* rIR = new DecimalNumberRestrictor(0.09f, pInf, 2, "m");
    TextInputRestrictor* aIR = new DecimalNumberRestrictor(-180, 180, 1, "º");
    TextInputRestrictor* eIR = new DecimalNumberRestrictor(-90, 90, 1, "º");
    auto raeFieldIRs = {rIR, aIR, eIR};
    raeText.setTextInputRestrictor(new MultiFieldTextInputRestrictor(raeFieldIRs, ", "));
    raeText.doubleClickSeparator = ", ";
    
    TextInputRestrictor* xIR = new DecimalNumberRestrictor(nInf, pInf, 2, "m");
    TextInputRestrictor* yIR = new DecimalNumberRestrictor(nInf, pInf, 2, "m");
    TextInputRestrictor* zIR = new DecimalNumberRestrictor(nInf, pInf, 2, "m");
    auto xyzFieldIRs = {xIR, yIR, zIR};
    xyzText.setTextInputRestrictor(new MultiFieldTextInputRestrictor(xyzFieldIRs, ", "));
    xyzText.doubleClickSeparator = ", ";
}

void PositionerText3D::show(Box boundary,
                            const std::array<float, 3>& xyz,
                            TextLook* look)
{
    setXYZ(xyz);
    setBoundary(boundary);
    finalLook = *look;
    xyzText.setLook(&finalLook);
    raeText.setLook(&finalLook);
    fitBoundaryToFontSize();
    //if (!visible) {
    showAnimation.restart(0.15f);
    visible = true;
    //}
}

void PositionerText3D::hide() noexcept
{
    if (visible) {
        showAnimation.restart(0.5f);
        visible = false;
    }
}

bool PositionerText3D::isVisible() const noexcept
{
    return visible || showAnimation.isPlaying();
}

void PositionerText3D::releaseFocus() noexcept
{
    raeText.releaseFocus();
    xyzText.releaseFocus();
}

void PositionerText3D::draw(OpenGLWindow& window,
                            const Point<float>& mousePosition,
                            bool mouseOverEnabled)
{
    auto backgroundAlpha = 0.7f;
    if (showAnimation.isPlaying()) {
        showAnimation.advance(window.frameRate);
        if (visible) {
            look.color = finalLook.color.withMultipliedAlpha(showAnimation.getProgress());
            look.relativeScale = 0.5f + 0.5f * showAnimation.getProgress();
            backgroundAlpha *= showAnimation.getProgress();
        } else {
            look.color = finalLook.color.withMultipliedAlpha(1 - showAnimation.getProgress());
            //look.relativeScale = 1 + 0.05f * showAnimation.getProgress();
            backgroundAlpha *= 1 - showAnimation.getProgress();
        }
        raeText.setLook(&look);
        xyzText.setLook(&look);
    }
    glColor4f(0, 0, 0, backgroundAlpha);
    getBoundary().drawFill();
    raeText.draw(window, mousePosition, mouseOverEnabled && visible);
    xyzText.draw(window, mousePosition, mouseOverEnabled && visible);
}

const OpenGLWindow* PositionerText3D::getWindow() const noexcept
{
    return raeText.getWindow();
}

void PositionerText3D::fitBoundaryToFontSize()
{
    if (finalLook.fontSize > 0) {
        auto boundary = getBoundary();
        const auto raeLen = look.getFontWithSize(finalLook.fontSize).getStringWidthFloat(raeText.getText());
        const auto xyzLen = look.getFontWithSize(finalLook.fontSize).getStringWidthFloat(xyzText.getText());
        const auto right = boundary.getLeft() + pixelsToNormalized(std::max(raeLen, xyzLen) + 1, getWindow()->width) / finalLook.horizontalPad;
        const auto top = boundary.getBottom() + 2 * pixelsToNormalized(finalLook.fontSize, getWindow()->height) / finalLook.verticalPad;
        boundary.setTop(top);
        boundary.setRight(right);
        setBoundary(boundary);
    }
}

Box PositionerText3D::getBoundary() const noexcept
{
    return raeText.getBoundary().combinedWith(xyzText.getBoundary());
}

void PositionerText3D::setBoundary(const Box& boundary) noexcept
{
    raeText.setBoundary({boundary.getTop(), boundary.getTop() - boundary.height() * 0.5f, boundary.getLeft(), boundary.getRight()});
    xyzText.setBoundary({boundary.getTop() - boundary.height() * 0.5f, boundary.getBottom(), boundary.getLeft(), boundary.getRight()});
}

void PositionerText3D::worldToDisplayRAE(std::array<float, 3>& rae) noexcept
{
    rae[1] = toDegrees(rae[1]);
    if (rae[1] > 180)
        rae[1] -= 360;
    rae[2] = -toDegrees(rae[2]) + 90;
}

void PositionerText3D::setXYZ(const std::array<float, 3>& xyz) noexcept
{
    //float rae [3];
    std::array<float, 3> rae;
    
    XYZtoRAE(xyz.data(), rae.data());
    worldToDisplayRAE(rae);
//    rae[1] = toDegrees(rae[1]);
//    if (rae[1] > 180)
//        rae[1] -= 360;
//    rae[2] = -toDegrees(rae[2]) + 90;
    auto raeTxt = StrFuncs::roundedFloatString(rae[0]) + xyzText.doubleClickSeparator + StrFuncs::roundedFloatString(rae[1]) + xyzText.doubleClickSeparator + StrFuncs::roundedFloatString(rae[2]);
    raeText.setText(raeTxt);
    auto xyzTxt = StrFuncs::roundedFloatString(xyz[0]) + xyzText.doubleClickSeparator + StrFuncs::roundedFloatString(xyz[1]) + xyzText.doubleClickSeparator + StrFuncs::roundedFloatString(xyz[2]);
    xyzText.setText(xyzTxt);
}

const Animation& PositionerText3D::getShowAnimation() const noexcept
{
    return showAnimation;
}



bool PositionerText3D::mouseClicked()
{
    if (mouseOverEnabled) {
        cauto a = xyzText.mouseClicked();
        cauto b = raeText.mouseClicked();
        return a || b;
    }
    return false;
}

bool PositionerText3D::mouseDoubleClicked()
{
    if (mouseOverEnabled) {
        cauto a = xyzText.mouseDoubleClicked();
        cauto b = raeText.mouseDoubleClicked();
        return a || b;
    }
    return false;
}

bool PositionerText3D::mouseDragged(const Point<float>& mouseDownPosition,
                                    const Point<float>& mouseCurrentPosition,
                                    const bool adjustScroll)
{
    if (mouseOverEnabled) {
        cauto a = xyzText.mouseDragged(mouseDownPosition, mouseCurrentPosition, adjustScroll);
        cauto b = raeText.mouseDragged(mouseDownPosition, mouseCurrentPosition, adjustScroll);
        return a || b;
    }
    return false;
}

bool PositionerText3D::keyPressed(const std::string& key)
{
    const auto a = xyzText.keyPressed(key);
    const auto b = raeText.keyPressed(key);
    return a || b;
}

std::array<float, 3> PositionerText3D::getRAE(const std::array<float, 3>& defaultRAE) const noexcept
{
    auto ir = dynamic_cast<const MultiFieldTextInputRestrictor*>(raeText.getTextInputRestrictor());
    auto str = ir->getField(raeText.getText(), 0);
    const auto r = str.empty() ? defaultRAE[0] : std::stof(str);
    str = ir->getField(raeText.getText(), 1);
    const auto a = str.empty() ? defaultRAE[1] : std::stof(str);
    str = ir->getField(raeText.getText(), 2);
    const auto e = str.empty() ? defaultRAE[2] : std::stof(str);
    return {r, a, e};
}

std::array<float, 3> PositionerText3D::getXYZ(const std::array<float, 3>& defaultXYZ) const noexcept
{
    auto ir = dynamic_cast<const MultiFieldTextInputRestrictor*>(xyzText.getTextInputRestrictor());
    auto str = ir->getField(xyzText.getText(), 0);
    const auto x = str.empty() ? defaultXYZ[0] : std::stof(str);
    str = ir->getField(xyzText.getText(), 1);
    const auto y = str.empty() ? defaultXYZ[1] : std::stof(str);
    str = ir->getField(xyzText.getText(), 2);
    const auto z = str.empty() ? defaultXYZ[2] : std::stof(str);
    return {x, y, z};
}



MultiFieldTextInputRestrictor::MultiFieldTextInputRestrictor(const std::vector<TextInputRestrictor*>& newFieldIRs,
                                                             const std::string& fieldSeparator) noexcept
    : fieldSeparator (fieldSeparator)
{
    setFieldIRs(newFieldIRs);
}

void MultiFieldTextInputRestrictor::setFieldIRs(const std::vector<TextInputRestrictor*>& newFieldIRs) noexcept
{
    fieldIRs.resize(newFieldIRs.size());
    for (int i = 0; i < newFieldIRs.size(); ++i)
        fieldIRs[i] = newFieldIRs[i];
}

TextInputRestrictor* MultiFieldTextInputRestrictor::getFieldIR(const std::size_t index) const noexcept
{
    if (index < fieldIRs.size())
        return fieldIRs[index];
    return nullptr;
}


TextInputRestrictor* MultiFieldTextInputRestrictor::clone() const
{
    return new MultiFieldTextInputRestrictor(*this);
}

bool MultiFieldTextInputRestrictor::insert(std::string& text,
                                           int& cursorPosition,
                                           const std::string& textToInsert) const
{
    bool result = false;
    if (fieldSeparator.find(textToInsert) != fieldSeparator.npos // entering a field separator character
        && std::count(text.begin(), text.end(), textToInsert.front()) < fieldIRs.size() - 1) { // and there is room for a new separator
        text.insert(cursorPosition, fieldSeparator); // insert the whole separator string
        cursorPosition += fieldSeparator.length();
        result = true;
    } else { // non-field separator characters
        int curPos = cursorPosition, fieldIndex = 0, i;
        std::string beforeField, afterField, fieldText, txt = text;
        while ((i = txt.find(fieldSeparator)) != txt.npos && fieldIndex < fieldIRs.size()) {
            if (curPos <= i) {
                fieldText = txt.substr(0, i);
                afterField = txt.substr(i, txt.npos);
                goto FOUND_FIELD;
            } else {
                ++fieldIndex;
                beforeField += txt.substr(0, i + fieldSeparator.length());
                txt = txt.substr(i + fieldSeparator.length(), txt.npos);
                curPos -= i + fieldSeparator.length();
            }
        }
        if (fieldText.empty())
            fieldText = txt;
    FOUND_FIELD:
//        debug({NAMEANDVALUE(curPos)});
//        debug({NAMEANDVALUE(fieldIndex)}, PrintToFileMode::APPEND);
//        debug({NAMEANDVALUESTR(fieldText)}, PrintToFileMode::APPEND);
//        debug({NAMEANDVALUESTR(txt)}, PrintToFileMode::APPEND);
//        debug({NAMEANDVALUESTR(text)}, PrintToFileMode::APPEND);
//        debug({NAMEANDVALUESTR(beforeField)}, PrintToFileMode::APPEND);
//        debug({NAMEANDVALUESTR(afterField)}, PrintToFileMode::APPEND);
        if (curPos < 0) // happens if inserting text in the middle of a separator string
            curPos = 0;
        result = fieldIRs[fieldIndex]->insert(fieldText, curPos, textToInsert);
        cursorPosition = curPos + beforeField.length();
        text = beforeField + fieldText + afterField;
    }
    return result;
}

void MultiFieldTextInputRestrictor::validateInput(std::string& text) const
{
    std::size_t i, fieldIndex = 0;
    std::vector<std::string> fields (fieldIRs.size());
    while ((i = text.find(fieldSeparator)) != text.npos && fieldIndex < fieldIRs.size()) {
        fields[fieldIndex++] = text.substr(0, i);
        text = text.substr(i + fieldSeparator.length(), text.npos);
    }
    fields[fieldIndex] = text; // last field or only field if no separators found
    text = "";
    for (i = 0; i < fieldIRs.size(); ++i) {
        if (!fields[i].empty())
            fieldIRs[i]->validateInput(fields[i]);
        text += fields[i] + ((i != fieldIRs.size() - 1) ? fieldSeparator : "");
    }
}

std::string MultiFieldTextInputRestrictor::getID() const noexcept
{
    return "MultiField";
}

std::string MultiFieldTextInputRestrictor::getField(const std::string& text,
                                                    const std::size_t index) const noexcept
{
    if (index < fieldIRs.size()) {
        std::size_t i, count = 0;
        auto txt = text;
        while ((i = txt.find(fieldSeparator)) != txt.npos) {
            if (count++ == index)
                return txt.substr(0, i);
            txt = txt.substr(i + fieldSeparator.length(), text.npos);
        }
        return txt;
    } else
        return "INDEX OUT OF RANGE";
}
