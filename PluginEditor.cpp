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

#include "PluginEditor.h"
//#include <GLUT/glut.h>
#include "Functions.h"
//#include <iostream>
//#include <fstream>
// file to print debuging stuff to
//std::ofstream logFile;

// the global hrir data that gets one instance across multiple plugin instances, this just references the one instance defined in PluginProcessor.cpp
//extern float***** HRIRdata;
//extern float**** HRIRdataPoles;

int Tabs::updateFromMouse(const float mouseX, const float mouseY, const bool mouseClicked) noexcept
{
    if (bottom <= mouseY && mouseY <= top) {
        const float alpha = (mouseY-bottom)*(top-bottom);
        float baseX, xLow, xHigh;
        for (int i = 0; i < tabs.size(); ++i) {
            if (i != selectedTab) {
                baseX = left+gap+i*(tab+3*gap);
                xLow = baseX + alpha*gap;
                xHigh = baseX+gap+tab + (1-alpha)*gap;
                if (xLow <= mouseX && mouseX <= xHigh) {
                    mouseOverTab = i;
                    if (mouseClicked) {
                        selectedTab = i;
                        return i;
                    }
                    return -1;
                }
            }
        }
    }
    mouseOverTab = -1;
    return -1;
}

//void Tabs::updateWindowSize(const float width, const float height) noexcept
//{
//    windowAspectRatio = height/width;
//}

void Tabs::setSelectedTab(const int newSelectedTab) noexcept
{
    selectedTab = newSelectedTab;
}

float Tabs::getBottom() const noexcept
{
    return bottom;
}

void Tabs::draw(const float windowAspectRatio) const noexcept
{
    if (tabs.size() > 0) {
        glLineWidth(1.2);
        const float longestStrLength = stroke_scale * windowAspectRatio
                                       * glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)tabs[longestStrIndex]);
        float textScale = tab/longestStrLength;
        if (topMax * textScale > textTop - textBottom) {
            textScale = (textTop - textBottom) / topMax;
        }
        float offset = selectedTab*(tab+gap*3.0) + gap;
        float x = left;
        float stringX, align;
        glColor4f(0.0, 0.0, 0.0, 0.85);
        glBegin(GL_QUADS);
        glVertex2f(left, bottom);
        glVertex2f(left, 1.0);
        glVertex2f(right, 1.0);
        glVertex2f(right, bottom);
        glEnd();
        glColor4f(1.0, 1.0, 1.0, 1.0);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, bottom); x += offset;
        glVertex2f(x, bottom); x += gap; stringX = x;
        glVertex2f(x, top);    x += tab;
        glVertex2f(x, top);    x += gap;
        glVertex2f(x, bottom);
        glVertex2f(right, bottom);
        glEnd();

        align = 0.5 * (tab - textScale * stroke_scale * windowAspectRatio
                             * glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)tabs[selectedTab]));
        drawStringStroke(tabs[selectedTab], stringX+align, textBottom, 0.0, windowAspectRatio, textScale);
        
        x = left+gap;
        for (int i = 0; i < tabs.size(); ++i) {
            if (i == mouseOverTab) {
                const float xWas = x;
                glColor4f(1.0, 1.0, 1.0, 0.3);
                glBegin(GL_QUADS);
                glVertex2f(x, bottom); x += gap;
                glVertex2f(x, top);    x += tab;
                glVertex2f(x, top);    x += gap;
                glVertex2f(x, bottom); x += gap;
                glEnd();
                x = xWas;
            }
            if (i != selectedTab) {
                glColor4f(1.0, 1.0, 1.0, 1.0);
                glBegin(GL_LINE_STRIP);
                glVertex2f(x, bottom); x += gap; stringX = x;
                glVertex2f(x, top);    x += tab;
                glVertex2f(x, top);    x += gap;
                glVertex2f(x, bottom); x += gap;
                glEnd();
                align = 0.5 * (tab - textScale * stroke_scale * windowAspectRatio
                                     * glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)tabs[i]));
                drawStringStroke(tabs[i], stringX+align, textBottom, 0.0, windowAspectRatio, textScale);
            } else {
                x += tab+gap*3.0;
            }
        }
        glLineWidth(1.0);
    }
}

void RadioOption::draw(const float windowAspectRatio) const noexcept
{
    const float realStrLength = strLength * stroke_scale * windowAspectRatio;
    float textScale = (right - left) / realStrLength;
    if (topMax * textScale > top - bottom) {
        textScale = (top - bottom) / topMax;
    }
    const float offsetY = bottom + 0.5 * ((top - bottom) - topMax * textScale);
    float offsetX = left + 0.5 * ((right - left) - realStrLength * textScale);
//    glColor4f(0, 1, 0, 0.5);
//    glBegin(GL_LINE_STRIP);
//    glVertex2f(left, top);
//    glVertex2f(right, top);
//    glVertex2f(right, bottom);
//    glVertex2f(left, bottom);
//    glVertex2f(left, top);
//    glEnd();
    //glColor4f(86.0/255.0, 1.0, 86.0/255.0, 1.0);
    for (int i = 0; i < options.size(); ++i) {
        if (i == mouseOverOption || i == selectedOption || (selectedOption == 3 && i == autoDetectOption))
            glLineWidth(3);
        else
            glLineWidth(1.2);
        if (i == mouseOverOption && i != selectedOption)
            glColor4f(86.0/255.0, 1.0, 86.0/255.0, 0.7);
        else
            glColor4f(86.0/255.0, 1.0, 86.0/255.0, 1.0);
        drawStringStroke(options[i], offsetX, offsetY, 0.0, windowAspectRatio, textScale);
        offsetX += glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)options[i]) * stroke_scale * windowAspectRatio * textScale;
    }
    glLineWidth(1.0);
}

int RadioOption::updateFromMouse(const float mouseX, const float mouseY, const bool mouseClicked, const float windowAspectRatio) noexcept
{
    if (bottom <= mouseY && mouseY <= top) {
        const float realStrLength = strLength * stroke_scale * windowAspectRatio;
        float textScale = (right - left) / realStrLength;
        if (topMax * textScale > top - bottom) {
            textScale = (top - bottom) / topMax;
        }
        //const float offsetY = bottom + 0.5 * ((top - bottom) - topMax * textScale);
        const float halfASpace = 0.5 * glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)" ");
        float offsetX = left + 0.5 * ((right - left) - realStrLength * textScale)
                        + (glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)options[0]) - halfASpace) * stroke_scale * windowAspectRatio * textScale;
        float optionLength;
        for (int i = 1; i < options.size(); ++i) {
            optionLength = glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)options[i]) * stroke_scale * windowAspectRatio * textScale;
            if (i == options.size()-1) {
                optionLength += 2*halfASpace * stroke_scale * windowAspectRatio * textScale;
            }
            if (offsetX <= mouseX && mouseX <= offsetX+optionLength) {
                mouseOverOption = i;
                if (mouseClicked) {
                    selectedOption = i;
                    return i;
                }
                return -1;
            }
            offsetX += optionLength;
        }
    }
    mouseOverOption = -1;
    return -1;
}

void RadioOption::setAutoDetectOption(const bool isHostRealTime) noexcept
{
    if (isHostRealTime)
        autoDetectOption = 1;
    else
        autoDetectOption = 2;
}

void RadioOption::setSelectedOption(const int newSelectedOption) noexcept
{
    selectedOption = newSelectedOption+1;
}

std::string RadioOption::getHelpText() const noexcept
{
    if (mouseOverOption > 0)
        return helpText[mouseOverOption-1];
    else
        return helpText[selectedOption-1];
}

// parses string of text into words separated by white space.  the white spaces are saved into words as well to preserve the length of the white space.
void GlutTextBox::loadText(const std::string& text) noexcept
{
    words.clear();
    int begin = 0;
    for (int i = 0; i < text.size(); ++i) {
        if (text[i] == ' ') { // if we found the ending of a non-whitespace word
            if (i > begin) { // don't skip loading first word if it is white space
                words.emplace_back(text.substr(begin, i-begin));
            }
            for (int j = i+1; j <= text.size(); ++j) { // load white space words
                if (j < text.size() && text[j] != ' ') {
                    words.emplace_back(text.substr(i, j-i));
                    begin = j;
                    i = j-1;
                    j = text.size(); // exit for j loop
                } else if (j == text.size()) { // load last word if it is white space
                    words.emplace_back(text.substr(i, text.size()-i));
                    i = j-1;
                }
            }
        } else if (i == text.size()-1) { // load last non-whitespace word
            words.emplace_back(text.substr(begin, text.size()-begin));
        }
    }
    resize(aspectRatio);
}

void GlutTextBox::resize(const float windowAspectRatio) noexcept
{
    aspectRatio = windowAspectRatio;
    // start with initial desired text scale
    textScale = desiredTextHeight / topMax;
    // if longest word doesn't fit on one line, scale back further
    float longestWordLength = 0, wordLength = 0;
    for (const auto& w : words) {
        wordLength = glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)w.c_str());
        if (wordLength > longestWordLength)
            longestWordLength = wordLength;
            }
    longestWordLength *= stroke_scale * windowAspectRatio * textScale;
    if (longestWordLength > right-left) {
        textScale *= (right-left) / longestWordLength;
    }
    textScale *= 0.995;
    // scale back further still if the text cannot be contained in the specified box height-wise
TRY_AGAIN:
    int numLines = (top - bottom) / (topMax*textScale*1.15); // 1.15 is for extra spacing between lines;
    float horizontalScale = stroke_scale * windowAspectRatio * textScale;
    float textLength = 0;
    int lineCount = 1, lastWord = 0;
    for (int i = 0; i < words.size(); ++i) {
        textLength += glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)words[i].c_str()) * horizontalScale;
        if (textLength > right-left) {
            ++lineCount;
            if (lineCount > numLines) {
                lastWord = i;
                goto OUT_OF_ROOM;
            }
            textLength = 0;
            if (words[i][0] != ' ')
                --i;
        }
    }
    return;
OUT_OF_ROOM:
    // micro-adjust scale until text fits height-wise
    textScale *= 0.98;
    goto TRY_AGAIN;
}

void GlutTextBox::draw() const noexcept
{
//    glBegin(GL_LINE_STRIP);
//    glVertex2f(left, top);
//    glVertex2f(right, top);
//    glVertex2f(right, bottom);
//    glVertex2f(left, bottom);
//    glVertex2f(left, top);
//    glEnd();
    
    float nextOffset = left;
    float offsetX = left;
    float offsetY = top;
    int line = 1;
    for (int i = 0; i < words.size(); ++i) {
        nextOffset += glutStrokeLength(GLUT_STROKE_ROMAN, (unsigned char*)words[i].c_str()) * stroke_scale * aspectRatio * textScale;
        if (nextOffset > right) {
            ++line;
            nextOffset = offsetX = left;
            if (words[i][0] != ' ') {
                --i;
            }
        } else {
            drawStringStroke((char*)words[i].c_str(), offsetX, offsetY-line*textScale*topMax*1.15, 0.0, aspectRatio, textScale);
            offsetX = nextOffset;
        }
    }
}

//==============================================================================
ThreeDAudioProcessorEditor::ThreeDAudioProcessorEditor (ThreeDAudioProcessor* ownerFilter)
: AudioProcessorEditor (ownerFilter)
{
    //processingModeText.loadText("       My mommy    loves me.   Thank you Jesus!!! Bob, joe, sally, and sue went to the zoo and saw a cookoo!  Blah, blah, blah, blah-blah, blah...");
    //tabs = Tabs({"Edit Sound Sources", "Automate Source Movement", "Settings"});
    
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
    automationViewWidth = processor->automationViewWidth;
    automationViewOffset = processor->automationViewOffset;
    //displayState = processor->displayState;
    showHelp = processor->showHelp;
    // ********************************************************
    
    tabs.setSelectedTab((int)processor->displayState.load());
    
    // set the eye's initial xyz position
    updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
    
    // make this class the one with the OpenGL rendering callbacks
    openGLContext.setRenderer (this);
    openGLContext.attachTo (*this);
    
    // start 30fps timer for OpenGL rendering
    startTimer(0, 1000.0/openGLFrameRate);
    
    // tells this class it wants to be able to recieve key events
    setWantsKeyboardFocus(true);
    
    // chooses whether a click on this component automatically grabs the focus
    setMouseClickGrabsKeyboardFocus(true);
    
    // add the resizer stuff and set its limits
    resizeLimits.setSizeLimits (10, 10, 5000, 5000);
    addAndMakeVisible (resizerCorner = new ResizableCornerComponent (this, &resizeLimits));
    //addAndMakeVisible (resizerCorner2 = new ResizableCornerComponent (this, &resizeLimits));
    //addAndMakeVisible (resizerBorder = new ResizableBorderComponent (this, &resizeLimits));
    resizerCorner->setAlwaysOnTop(true);
    //resizerBorder->setAlwaysOnTop(true);
    
    // set the plugin's editor window's inital size
    setSize (processor->lastUIWidth, processor->lastUIHeight);
}

ThreeDAudioProcessorEditor::~ThreeDAudioProcessorEditor()
{
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
        processor->automationViewWidth = automationViewWidth;
        processor->automationViewOffset = automationViewOffset;
        //processor->displayState = displayState;
        processor->showHelp = showHelp;
    }
    // ******************************************************
}

void ThreeDAudioProcessorEditor::updateSize()
{
    setSize(processor->lastUIWidth, processor->lastUIHeight);
}

void ThreeDAudioProcessorEditor::resized()
{
    //const ScopedLock lockIt(resizerLock);
    // get the new window width and height
    const int w = getWidth();
    const int h = getHeight();
    
    windowAspectRatio = ((float)h) / ((float)w);
    processingModeText.resize(windowAspectRatio);
    //tabs.updateWindowSize((float)w, (float)h);
    
    // either seems to work, neither fixes flickering on resizing
    {   // need to get a lock for the message manager here to make the (component).setBounds() call thread-safe
        const MessageManagerLock mmLock;
        // do the resizing of the corner resizer component
        resizerCorner->setBounds(w - 15, h - 15, 15, 15);
        //resizerCorner2->setBounds(0, h - 15, 15, 15);
        //resizerBorder->setBounds(this->getBounds()/* 0, 0, w, h*/);
    }   // mmLock goes out of scope here and is released
//    {
//        MessageManagerLock mml (Thread::getCurrentThread());
//        if (mml.lockWasGained())
//        {
//            // do the resizing of the corner resizer component
//            resizerCorner->setBounds(w - 15, h - 15, 15, 15);
//        }
//    }
    
    if (glInited) {
        // Tell OpenGL how to convert from coordinates to pixel values
        glViewport(0, 0, w, h);
        
        // Switch to setting the camera perspective
        glMatrixMode(GL_PROJECTION);
        
        // Reset the camera matrix
        glLoadIdentity();
        
        // Set the camera perspective
        // Both seem to work:
        //        gluPerspective(45.0,                  //The camera angle
        //                       (double)w / (double)h, //The width-to-height ratio
        //                       1.0,                 //The near z clipping coordinate
        //                       200.0);              //The far z clipping coordinate
        perspectiveGL(45.0,                  //The camera angle
                      (double)w / (double)h, //The width-to-height ratio
                      0.1,                 //The near z clipping coordinate
                      200.0);              //The far z clipping coordinate
        
        // tell the GL scene to be redawn soon with the new camera perspective, doesn't seem to be necessary...
        //openGLContext.triggerRepaint();
    }
    
    // update processor with new width and height
    processor->lastUIWidth = w;
    processor->lastUIHeight = h;
}

//==============================================================================
void ThreeDAudioProcessorEditor::paint (Graphics& g)
{
    // dimensions of window
    const int w = getWidth();
    const int h = getHeight();

//    if (processor->dopplerOn && displayState == MAIN)
//    {
//        String dopplerText ("Doppler On");
//        g.setColour(Colours::red);
//        int textBoarderX = std::ceil(0.4*w);
//        int textBoarderY = std::ceil(0.04*h);
//        int maximumNumberOfLines = 1;
//        int fontSize = std::floor( std::min(textBoarderY*1.0, 2.5*(w-2.0*textBoarderX)/dopplerText.length()) );
//        g.setFont(fontSize);
//        float minimumHorizontalScale = 0.7f; // allow a bit of text compression so no "..." shows up instead of text
//        const Rectangle<int> area (textBoarderX, h-2.5*textBoarderY, // x/y postion of top left corner
//                                   w - 2*textBoarderX, fontSize);    // width/height of rectangle (height set to font size)
//        g.drawFittedText (dopplerText,
//                          area,
//                          Justification::centredTop,
//                          maximumNumberOfLines,
//                          minimumHorizontalScale);
//    }
    
    // only show the help text if that is the current GUI state
    if (showHelp && processor->displayState != DisplayState::SETTINGS)
    {
        g.fillAll(Colour::fromFloatRGBA(0.1, 0.1, 0.3, 0.3));
        
        // how much empty space around text is there
        int textBoarderX = std::ceil(0.05*w);
        int textBoarderY = std::ceil(0.05*h);
        
        // other stuff for position of each text line in a rectangle area
        Justification justificationFlags = Justification::centred; // horizontally centered text in area rectangle
        int maximumNumberOfLines = 1; // one line limit per line of text (no wrapping to another line)
        float minimumHorizontalScale = 0.7f; // allow a bit of text compression so no "..." shows up instead of text
        
        // white as snow
        g.setColour(Colours::white);
        //g.setOpacity(0.95); // well maybe not quite...
        
        // the help text to draw depending on the view state
        /*const*/ String** text = nullptr;
        int numLines = 0; // how many lines of text?
        switch (processor->displayState) {
            case DisplayState::MAIN:
                numLines = sizeof(mainHelp)/sizeof(String);
                text = new /*const*/ String*[numLines];
                for (int i = 0; i < numLines; ++i)
                    text[i] = (String*) & mainHelp[i];
                break;
            case DisplayState::PATH_AUTOMATION:
                numLines = sizeof(automationHelp)/sizeof(String);
                text = new /*const*/ String*[numLines];
                for (int i = 0; i < numLines; ++i)
                    text[i] = (String*) & automationHelp[i];
                break;
        }
        
        // find the longest text string in the text block so the font size can be chosen to fit the window nicely
        int maxTextLength = 0;
        for (int i = 0; i < numLines; ++i)
        {
            if (text[i]->length() > maxTextLength)
                maxTextLength = text[i]->length();
        }
        
        // choose the font size based on what will fit the minimum window dimension (text width is about 2.5x as narrow as text height for the font of a given size)
        int fontSize = std::floor( std::min((h-2.0*textBoarderY)/numLines, 2.5*(w-2.0*textBoarderX)/maxTextLength) );//20;
        g.setFont (fontSize);
        
        // displacement so text block is vertically centered
        int roomY = std::floor( (h - 2*textBoarderY - fontSize*numLines)/2.0 );
        
        // draw the text (each area rectangle defines the position/bounds of each line of text)
        for (int i = 0; i < numLines; ++i)
        {
            const Rectangle<int> area (textBoarderX, textBoarderY + i*fontSize + roomY, // x/y postion of top left corner
                                       w - 2*textBoarderX, fontSize);                   // width/height of rectangle (height set to font size)
            g.drawFittedText (*text[i],
                              area,
                              justificationFlags,
                              maximumNumberOfLines,
                              minimumHorizontalScale);
        }
        
        // has crashed here before... when text was const
        delete[] text;
    }
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
    
    //glEnable(GL_DEPTH_TEST); //Make sure 3D drawing works when one object is in front of another
    //glDepthFunc (GL_LEQUAL);
    
    glutInitDisplayMode (GLUT_DOUBLE); // Set up a basic display buffer (for double buffering)
    
    // now the GL stuff is ready to go
    glInited = true;
    
    // do setup of GL_PROJECTION matrix for the view
    resized();
}

void ThreeDAudioProcessorEditor::openGLContextClosing()
{
    // will need to reinitialize the GL if the gl context closes
    glInited = false;
}

// Replaces gluPerspective. Sets the frustum to perspective mode.
// fovY     - Field of vision in degrees in the y direction
// aspect   - Aspect ratio of the viewport
// zNear    - The near clipping distance
// zFar     - The far clipping distance
void ThreeDAudioProcessorEditor::perspectiveGL(GLdouble fovY, GLdouble aspect, GLdouble zNear, GLdouble zFar)
{
    const GLdouble pi = 3.1415926535897932384626433832795;
    GLdouble fW, fH;
    
    fH = tan( fovY / 360 * pi ) * zNear;
    fW = fH * aspect;
    
    glFrustum( -fW, fW, -fH, fH, zNear, zFar );
}

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

void ThreeDAudioProcessorEditor::drawMain()
{
    // ******** setup for entering selection mode ********
    const double w = getWidth();
    const double h = getHeight();
    GLint mouse_x = getMouseXYRelative().getX();
    GLint mouse_y = getMouseXYRelative().getY();
    GLint viewport[4];
    
    glSelectBuffer(SELECT_BUF_SIZE, objSelectBuf);
    glRenderMode(GL_SELECT);
    
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    
    glGetIntegerv(GL_VIEWPORT, viewport);
    gluPickMatrix(mouse_x, viewport[3]-mouse_y, 5, 5, viewport);
    gluPerspective(45, w/h, 0.1, 200);
    glMatrixMode(GL_MODELVIEW);
    
    glLoadIdentity(); // reset the drawing perspective
    /* viewing transformation  */
    gluLookAt (eyePos[0], eyePos[1], eyePos[2],  // eye pos (x,y,z)
               0.0, 0.0, 0.0,                   // direction looking at (x,y,z)
               eyeUp[0], eyeUp[1], eyeUp[2]);  // camera up vector (x,y,z)
    
    // start drawing selectable objects
    glInitNames();
    
    // draw the source(s)
    int k = 0;
    int numSlices;
    int numStacks;
    float radius;
    GLUquadricObj* pQuadric = gluNewQuadric();
    //assert(pQuadric != NULL);
    for (int i = 0; i < sources->size(); ++i) {
        glPushName(SOURCE + i);
        std::array<float,3> pos = (*sources)[i].getPosXYZ();
        numSlices = 6;
        numStacks = 4;
        radius = 0.05;
        glPushMatrix();
        glTranslatef(pos[0], pos[1], pos[2]);
        gluSphere(pQuadric,radius,numSlices,numStacks);
        glPopMatrix();
        glPopName();
        if ((*sources)[i].getSourceSelected()) {
            // allow hit detection on path points if source is selected
            std::vector<std::vector<float>> pts = (*sources)[i].getPathPoints();
            for (int j = 0; j < pts.size(); ++j) {
                glPushName(SOURCE_PTS + k);
                numSlices = 3;
                numStacks = 2;
                radius = 0.02;
                glPushMatrix();
                glTranslatef(pts[j][0], pts[j][1], pts[j][2]);
                gluSphere(pQuadric,radius,numSlices,numStacks);
                glPopMatrix();
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
    
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
    glLoadIdentity(); //Reset the drawing perspective
    /* viewing transformation  */
    gluLookAt (eyePos[0], eyePos[1], eyePos[2],  // eye pos (x,y,z)
               0.0, 0.0, 0.0,                   // direction looking at (x,y,z)
               eyeUp[0], eyeUp[1], eyeUp[2]);  // camera up vector (x,y,z)
    /* Remove hidden surfaces */
    glEnable(GL_DEPTH_TEST);  // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    
    // light for source(s)
    glEnable(GL_LIGHTING);
    int numLights = sources->size();
    if (numLights > 7)
        numLights = 7;
    
    for (int i = 0; i < 7; i++) {
        if (i < numLights && !(*sources)[i].getSourceMuted()) {
            std::array<float,3> pos = (*sources)[i].getPosXYZ();
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
    radius = 0.09; // 9cm radius for head
    numSlices = 20;
    numStacks = 10;
    //GLUquadricObj* pQuadric = gluNewQuadric();
    //assert(pQuadric != NULL);
    gluSphere(pQuadric,radius,numSlices,numStacks);
    
    glDisable(GL_LIGHTING);
    
    // draw the source(s)
    for (int i = 0; i < sources->size(); ++i) {
        std::array<float,3> pos = (*sources)[i].getPosXYZ();
        float poser[3] = {pos[0],pos[1],pos[2]};
        float alpha = 0.95;
        if ((*sources)[i].getSourceMuted())
            alpha = 0.6;
        // color depending on user interaction
        if (i == mouseOverSourceIndex || (*sources)[i].getSourceSelected() || pointInsideSelectRegion(poser)) {
            // draw interpolated path
//            if ((*sources)[i]->getSourceMuted()) {
//                glColor4f(1.0, 0.0, 0.7, 0.6);
//            } else {
//                glColor4f(1.0, 0.0, 0.7, 0.95);
//            }
            glColor4f(1.0, 0.0, 0.7, alpha);
            drawInterpolatedPath(i);
            // draw selectable pts
            std::vector<std::vector<float>> points = (*sources)[i].getPathPoints();
            numSlices = 8;
            numStacks = 5;
            for (int j = 0; j < points.size(); ++j) {
                float xyz[3] = { points[j][0], points[j][1], points[j][2] };
                if ((*sources)[i].getPathPointSelected(j) || (i == mouseOverPathPointSourceIndex && j == mouseOverPathPointIndex) || ((*sources)[i].getSourceSelected() && pointInsideSelectRegion(xyz))) {
                    radius = 0.033;
//                    if ((*sources)[i]->getSourceMuted()) {
//                        glColor4f(0.5, 0.0, 1.0, 0.6);
//                    } else {
//                        glColor4f(0.5, 0.0, 1.0, 0.95);
//                    }
                    glColor4f(0.5, 0.0, 1.0, alpha);
                    // lines for visualizing position of path control points
                    glBegin(GL_LINES);
                    glVertex3f(0.0, 0.0, 0.0);
                    glVertex3f(xyz[0], xyz[1], xyz[2]);
                    
                    glVertex3f(xyz[0], 0.0, xyz[2]);
                    glVertex3f(xyz[0], xyz[1], xyz[2]);
                    
                    glVertex3f(xyz[0], 0.0, xyz[2]);
                    glVertex3f(0.0, 0.0, 0.0);
                    // right angle marking
                    float size = 0.05;
                    glVertex3f(xyz[0], size*xyz[1], xyz[2]);
                    glVertex3f(xyz[0] - size*xyz[0], size*xyz[1], xyz[2] - size*xyz[2]);
                    glVertex3f(xyz[0] - size*xyz[0], 0.0, xyz[2] - size*xyz[2]);
                    glVertex3f(xyz[0] - size*xyz[0], size*xyz[1], xyz[2] - size*xyz[2]);
                    glEnd();
                } else {
                    radius = 0.02;
//                    if ((*sources)[i]->getSourceMuted()) {
//                        glColor4f(0.7, 0.0, 0.0, 0.6);
//                    } else {
//                        glColor4f(0.7, 0.0, 0.0, 0.95);
//                    }
                    glColor4f(0.7, 0.0, 0.0, alpha);
                }
                glPushMatrix();
                glTranslatef(points[j][0], points[j][1], points[j][2]);
                gluSphere(pQuadric,radius,numSlices,numStacks);
                glPopMatrix();
            }
            // draw source
//            if ((*sources)[i]->getSourceMuted()) {
//                glColor4f(0.0, 1.0, 0.0, 0.6);
//            } else {
//                glColor4f(0.0, 1.0, 0.0, 0.95);
//            }
            glColor4f(0.0, 1.0, 0.0, alpha);
            radius = 0.03;
            glPushMatrix();
            glTranslatef(pos[0], pos[1], pos[2]);
            gluSphere(pQuadric,radius,numSlices,numStacks);
            glPopMatrix();
            // lines for visualizing position of source
            glBegin(GL_LINES);
            glVertex3f(0.0, 0.0, 0.0);
            glVertex3f(pos[0], pos[1], pos[2]);
            glVertex3f(pos[0], 0.0, pos[2]);
            glVertex3f(pos[0], pos[1], pos[2]);
            glVertex3f(pos[0], 0.0, pos[2]);
            glVertex3f(0.0, 0.0, 0.0);
            // right angle marking
            //            float area = fabs(pos[1]*pow(pow(pos[0], 2.0) + pow(pos[2], 2.0), 0.5));
            //            float size = 0.5*area;
            //            float dirX = pos[0]/fabs(pos[0]);
            //            float dirZ = pos[2]/fabs(pos[2]);
            //            glVertex3f(pos[0]            , size, pos[2]);
            //            glVertex3f(pos[0] - dirX*size, size, pos[2] - dirZ*size);
            //            glVertex3f(pos[0] - dirX*size,  0.0, pos[2] - dirZ*size);
            //            glVertex3f(pos[0] - dirX*size, size, pos[2] - dirZ*size);
            const float size = 0.05;
            glVertex3f(pos[0], size*pos[1], pos[2]);
            glVertex3f(pos[0] - size*pos[0], size*pos[1], pos[2] - size*pos[2]);
            glVertex3f(pos[0] - size*pos[0], 0.0, pos[2] - size*pos[2]);
            glVertex3f(pos[0] - size*pos[0], size*pos[1], pos[2] - size*pos[2]);
            glEnd();
        } else {
            // draw source
            glColor4f(1.0, 1.0, 1.0, alpha);
            radius = 0.03;
            glPushMatrix();
            glTranslatef(pos[0], pos[1], pos[2]);
            gluSphere(pQuadric,radius,numSlices,numStacks);
            glPopMatrix();
            // draw interpolated path
            if ((*sources)[i].getSourceMuted())
                glColor4f(0.8, 0.8, 0.8, 0.5);
            else
                glColor4f(0.2, 0.1, 1.0, 0.9);
            drawInterpolatedPath(i);
            // draw selectable pts
            std::vector<std::vector<float>> points = (*sources)[i].getPathPoints();
            numSlices = 8;
            numStacks = 5;
            for (int i = 0; i < points.size(); ++i) {
                radius = 0.02;
                glPushMatrix();
                glTranslatef(points[i][0], points[i][1], points[i][2]);
                gluSphere(pQuadric,radius,numSlices,numStacks);
                glPopMatrix();
            }
//            } else {
//                // source is not muted
//                // draw source
//                glColor4f(1.0, 1.0, 1.0, 0.9);
//                radius = 0.03;
//                glPushMatrix();
//                glTranslatef(pos[0], pos[1], pos[2]);
//                gluSphere(pQuadric,radius,numSlices,numStacks);
//                glPopMatrix();
//                // draw interpolated path
//                glColor4f(0.2, 0.1, 1.0, 0.9);
//                drawInterpolatedPath(i);
//                // draw selectable pts
//                std::vector<std::vector<float>> points = (*sources)[i]->getPathPoints();
//                numSlices = 8;
//                numStacks = 5;
//                for (int i = 0; i < points.size(); i++) {
//                    radius = 0.02;
//                    glPushMatrix();
//                    glTranslatef(points[i][0], points[i][1], points[i][2]);
//                    gluSphere(pQuadric,radius,numSlices,numStacks);
//                    glPopMatrix();
//                }
//            }
        }
    }
    
    // draw axis
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(-1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, -1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, -1.0);
    glEnd();
    char str[20];
    //    sprintf(str, "X");
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 1.1, 0.0, 0.0);
    //    sprintf(str, "Y");
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 1.1, 0.0);
    //    sprintf(str, "Z");
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 0.0, 1.1);
    sprintf(str, "F");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 1.1, 0.0, 0.0);
    sprintf(str, "U");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 1.1, 0.0);
    sprintf(str, "R");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 0.0, 1.1);
    sprintf(str, "B");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -1.1, 0.0, 0.0);
    sprintf(str, "D");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, -1.1, 0.0);
    sprintf(str, "L");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 0.0, -1.1);
    // end old drawMain()
    
    // set up 2d projection for HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
    glOrtho(-1, 1, -1, 1, -1.0, 10.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    //glDisable(GL_CULL_FACE);
    
    // draw 2d stuff here ...
    
    // draw mouse drag boarders if mouse is being dragged to select a group of things
    if (mouseDragging) {
        drawMouseDragging();
    }
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
    std::array<float,3> pos;
    float poser[3];
    for (int i = 0; i < sources->size(); ++i) {
        pos = (*sources)[i].getPosXYZ();
        poser[0] = pos[0];
        poser[1] = pos[1];
        poser[2] = pos[2];
        // color depending on user interaction
        if (i == mouseOverSourceIndex || (*sources)[i].getSourceSelected() || pointInsideSelectRegion(poser)) {
            float alpha = 1.0;
            if ((*sources)[i].getSourceMuted())
                alpha = 0.6;
            // draw selectable pts
            std::vector<std::vector<float>> points = (*sources)[i].getPathPoints();
            float pos3d[3];
            float pos2d[2];
            for (int j = 0; j < points.size(); ++j) {
                pos3d[0] = points[j][0];
                pos3d[1] = points[j][1];
                pos3d[2] = points[j][2];
                if (to2D(pos3d, pos2d)) {
                    if (j == mouseOverPathPointIndex || (*sources)[i].getPathPointSelected(j)
                        || ((*sources)[i].getSourceSelected() && pointInsideSelectRegion2D(pos2d)))
                        glColor4f(0.0, 1.0, 0.0, alpha);
                    else
                        glColor4f(1.0, 1.0, 1.0, alpha);
                    sprintf(str, "%d", j+1);
                    drawStringStroke(str, pos2d[0]-0.01/*+0.04*/, pos2d[1]-0.015/*-0.04*/, 0);
                }
            }
        }
    }
    
    // draw the doppler speed of sound control slider
    if (processor->dopplerOn) {
        const float m_x = getMouseX();
        const float m_y = getMouseY();
        glColor4f(1.0, 0.0, 0.0, 1.0);
        mouseOverDopplerSlider = (-0.52 < m_x && m_x < 0.52
                                && -1.0 < m_y && m_y < -0.86);
        if (mouseOverDopplerSlider)
            glLineWidth(2.0);
        else
            glLineWidth(1.0);
        glBegin(GL_LINES);
        glVertex2f(-0.5, -0.93);
        glVertex2f(0.5, -0.93);
        float speedOfSoundRange = processor->maxSpeedOfSound - processor->minSpeedOfSound;
        float xPos = -0.5 + (processor->speedOfSound - processor->minSpeedOfSound)/speedOfSoundRange;
        glVertex2f(xPos, -0.91);
        glVertex2f(xPos, -0.95);
        float speedOfSoundToDisplay = processor->speedOfSound;
        if (mouseOverDopplerSlider) {
            glColor4f(1.0, 0.0, 0.0, 0.5);
            xPos = m_x;
            if (xPos < -0.5) xPos = -0.5;
            if (xPos >  0.5) xPos =  0.5;
            glVertex2f(xPos, -0.89);
            glVertex2f(xPos, -0.97);
            speedOfSoundToDisplay = processor->minSpeedOfSound + (xPos+0.5)*speedOfSoundRange;
            newDopplerSpeedOfSound = speedOfSoundToDisplay;
        }
        glEnd();
        glColor4f(1.0, 0.0, 0.0, 1.0);
        glLineWidth(1.0); // reset back to default so other lines aren't affected
        sprintf(str, "Speed of Sound: DUM m/s");
        float stroke_scale = 0.00035;
        float aspect = ((float)getHeight())/((float)getWidth());
        float xlen = 0;
        for (int i = 0; i < strlen(str); ++i) {
            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
        }
        xlen *= stroke_scale*aspect;
        sprintf(str, "Speed of Sound: %.0f m/s", speedOfSoundToDisplay);
        drawStringStroke(str, -0.5*xlen, -0.98, 0);
        sprintf(str, "Doppler On");
        stroke_scale = 0.00035;
        aspect = ((float)getHeight())/((float)getWidth());
        xlen = 0;
        for (int i = 0; i < strlen(str); ++i) {
            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
        }
        xlen *= stroke_scale*aspect;
        drawStringStroke(str, -0.5*xlen, -0.91, 0);
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

void ThreeDAudioProcessorEditor::drawPathControl()
{
    // get mouse position
    const float m_x = getMouseX();
    const float m_y = getMouseY();
 
    const float x_scale = 0.9;
    const float y_scale = 0.9;
    
    //Clear screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW); //Switch to the drawing perspective
    glLoadIdentity(); //Reset the drawing perspective
    /* viewing transformation  */
    gluLookAt (eyePos[0], eyePos[1], eyePos[2],  // eye pos (x,y,z)
               0.0, 0.0, 0.0,                   // direction looking at (x,y,z)
               eyeUp[0], eyeUp[1], eyeUp[2]);  // camera up vector (x,y,z)
    /* Remove hidden surfaces */
    glEnable(GL_DEPTH_TEST);  // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    
    // begin of old drawMain()
    // light for source(s)
    glEnable(GL_LIGHTING);
    int numLights = sources->size();
    if (numLights > 7)
        numLights = 7;
    
    // must loop through all potentially ON gl lights to refresh each's state based on the current sources
    for (int i = 0; i < 7; ++i) {
        // only show lights for sources that aren't muted
        if (i < numLights && !(*sources)[i].getSourceMuted()) {
            std::array<float,3> pos = (*sources)[i].getPosXYZ();
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
    float radius = 0.09; // 9cm radius for head
    float numSlices = 20;
    float numStacks = 10;
    GLUquadricObj* pQuadric = gluNewQuadric();
    //assert(pQuadric != NULL);
    gluSphere(pQuadric,radius,numSlices,numStacks);
    
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
    
    // reset this thing if not dragging some points with the mouse
    if (!pathAutomationPointsGrabbedWithMouse)
        mouseOverPathAutomationPointIndex = {-1, -1, -1};
    
    // draw the source(s)
    for (int s = 0; s < sources->size(); ++s) {
        std::array<float,3> pos = (*sources)[s].getPosXYZ();
        // only draw sources that we are editing position automation of, or draw all of them if none are explicitly selected
        if ((*sources)[s].getSourceSelected()) {
            
            if ((*sources)[s].getNumPathPoints() > 1
                && std::abs(m_x) <= 1/*x_scale*/
                && std::abs(m_y) <= 1/*y_scale*/) {
                // draw the position of the source that conincides with the mouse's current vertical position
                float paraVal = 0.5*(m_y/y_scale+1.0);
                if (mouseOverPathAutomationPointIndex[0] != -1) {
                    // crashed here in Cubase when deleting a selected path point that the mouse was over, moved the reset mouseOverPathAutomationPointIndex code from below here to just above to try and fix
                    paraVal = (*sources)[mouseOverPathAutomationPointIndex[0]]
                              .getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1])[1];
                    //glColor4ub(255, 182, 87, 0.95*255);
                }
//                else {
//                    glColor4f(1.0, 1.0, 1.0, 0.95);
//                }
                if (paraVal < 0)
                    paraVal = 0;
                if (paraVal > 1)
                    paraVal = 1;
                float range = (*sources)[s].getPath()->getInputRange()[1]*0.99999;
                std::vector<float> paraPos;
                (*sources)[s].getPath()->pointAt(paraVal*range, paraPos);
                glColor4f(1.0, 1.0, 1.0, 0.95);
                radius = 0.03;
                glPushMatrix();
                glTranslatef(paraPos[0], paraPos[1], paraPos[2]);
                gluSphere(pQuadric,radius,numSlices,numStacks);
                glPopMatrix();
            }
            
            // draw muted sources slightly differently
            if ((*sources)[s].getSourceMuted()) {
                
                // draw interpolated path
                glColor4f(0.7, 0.7, 0.7, 0.6);
                drawInterpolatedPath(s);
                
                // draw path control points
                std::vector<std::vector<float>> points = (*sources)[s].getPathPoints();
                numSlices = 10;
                numStacks = 6;
                radius = 0.02;
                for (int j = 0; j < points.size(); j++) {
                    glPushMatrix();
                    glTranslatef(points[j][0], points[j][1], points[j][2]);
                    gluSphere(pQuadric,radius,numSlices,numStacks);
                    glPopMatrix();
                }
                
                // draw source
                glColor4f(1.0, 1.0, 1.0, 0.6);
                radius = 0.03;
                glPushMatrix();
                glTranslatef(pos[0], pos[1], pos[2]);
                gluSphere(pQuadric,radius,numSlices,numStacks);
                glPopMatrix();
                
            } else {
                // draw interpolated path
                glColor3f(1.0, 0.0, 0.7);
                drawInterpolatedPath(s);

                // draw path control points
                glColor4f(0.7, 0.0, 0.0, 0.95);
                std::vector<std::vector<float>> points = (*sources)[s].getPathPoints();
                numSlices = 10;
                numStacks = 6;
                radius = 0.02;
                for (int j = 0; j < points.size(); j++) {
                    glPushMatrix();
                    glTranslatef(points[j][0], points[j][1], points[j][2]);
                    gluSphere(pQuadric,radius,numSlices,numStacks);
                    glPopMatrix();
                }
                
                // draw source
                glColor4f(0.0, 1.0, 0.0, 0.95);
                radius = 0.03;
                glPushMatrix();
                glTranslatef(pos[0], pos[1], pos[2]);
                gluSphere(pQuadric,radius,numSlices,numStacks);
                glPopMatrix();
                
                // lines for visualizing position of source
                glBegin(GL_LINES);
                glVertex3f(0.0, 0.0, 0.0);
                glVertex3f(pos[0], pos[1], pos[2]);
                
                glVertex3f(pos[0], 0.0, pos[2]);
                glVertex3f(pos[0], pos[1], pos[2]);
                
                glVertex3f(pos[0], 0.0, pos[2]);
                glVertex3f(0.0, 0.0, 0.0);
                
                // right angle marking
                float size = 0.05;
                glVertex3f(pos[0], size*pos[1], pos[2]);
                glVertex3f(pos[0] - size*pos[0], size*pos[1], pos[2] - size*pos[2]);
                glVertex3f(pos[0] - size*pos[0], 0.0, pos[2] - size*pos[2]);
                glVertex3f(pos[0] - size*pos[0], size*pos[1], pos[2] - size*pos[2]);
                glEnd();
            }
        }
    }
    
    // draw axis
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(-1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, -1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, -1.0);
    glEnd();
    char str[20];
    //    sprintf(str, "X");
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 1.1, 0.0, 0.0);
    //    sprintf(str, "Y");
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 1.1, 0.0);
    //    sprintf(str, "Z");
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 0.0, 1.1);
    sprintf(str, "F");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 1.1, 0.0, 0.0);
    sprintf(str, "U");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 1.1, 0.0);
    sprintf(str, "R");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 0.0, 1.1);
    sprintf(str, "B");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -1.1, 0.0, 0.0);
    sprintf(str, "D");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, -1.1, 0.0);
    sprintf(str, "L");
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.0, 0.0, -1.1);
    // end old drawMain()
    
    // set up 2d projection for HUD
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
    glOrtho(-1, 1, -1, 1, -1.0, 10.0);
    
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
    //glDisable(GL_CULL_FACE);
    
    // draw 2d stuff here ...
    
    // draw dark translucent curtain over 3d scene portion
    glColor4f(0.0, 0.0, 0.0, 0.2);
    glBegin(GL_POLYGON);
    glVertex2f(-1.0, -1.0);
    glVertex2f(-1.0, +1.0);
    glVertex2f(+1.0, +1.0);
    glVertex2f(+1.0, -1.0);
    glEnd();
    
    // draw the parametric position automation grid and its labels
    glColor3f(1.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex2f(-1.0*x_scale, -1.0*y_scale);
    glVertex2f(-1.0*x_scale, +1.0*y_scale);
    glVertex2f(-1.0*x_scale, -1.0*y_scale);
    glVertex2f(+1.0*x_scale, -1.0*y_scale);
    glVertex2f(-x_scale, -0.985*y_scale);
    glVertex2f(-x_scale, -1.02*y_scale);
    glVertex2f(x_scale, -0.985*y_scale);
    glVertex2f(x_scale, -1.02*y_scale);
    glVertex2f(-0.985*x_scale, -y_scale);
    glVertex2f(-1.02*x_scale, -y_scale);
    glVertex2f(-0.985*x_scale, y_scale);
    glVertex2f(-1.02*x_scale, y_scale);
    glEnd();
    getFormattedTime(automationViewOffset-automationViewWidth/2.0, str);
    //sprintf(str, "%.2f", automationViewOffset-automationViewWidth/2.0);
    drawStringStroke(str, -x_scale, -y_scale-0.07, 0.0);
    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -x_scale, -y_scale-0.07, 0.0);
    getFormattedTime(automationViewOffset+automationViewWidth/2.0, str);
    //sprintf(str, "%.2f", automationViewOffset+automationViewWidth/2.0);
    const float stroke_scale = 0.00035;
    const float aspect = ((float)getHeight())/((float)getWidth());
//    float xlen = 0;
//    for (int i = 0; i < strlen(str); i++) {
//        xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, '0'/*str[i]*/);
//    }
//    xlen *= stroke_scale*aspect;
    // less jumpy this way...
    float xlen = strlen(str) * glutStrokeWidth(GLUT_STROKE_ROMAN, '0') * stroke_scale * aspect * 0.8;
    drawStringStroke(str, x_scale-xlen, -y_scale-0.07, 0.0);
    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, x_scale-0.035*3.0, -y_scale-0.07, 0.0);
    const float topMax = 119.05*stroke_scale; // also used below
//    sprintf(str, "%d", 0);
//    drawStringStroke(str, -x_scale-0.06, -y_scale-topMax*0.5, 0.0);
//    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -x_scale-0.06, -y_scale, 0.0);
//    sprintf(str, "%d", 1);
//    drawStringStroke(str, -x_scale-0.06, y_scale-topMax*0.5, 0.0);
//    //drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, -x_scale-0.09, y_scale-0.05, 0.0);
    
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
    
//    // (moved above) reset this thing if not draggin some points with the mouse
//    if (!pathAutomationPointsGrabbedWithMouse)
//        mouseOverPathAutomationPointIndex = {-1, -1, -1};
    
    // draw the path automation points of the selected sources
    // precomputation of constants for view setting
    const float thing1 = automationViewWidth*0.5-automationViewOffset;
    const float thing2 = 2.0/automationViewWidth;
    for (int s = 0; s < sources->size(); ++s) {
        if ((*sources)[s].getSourceSelected()) {
            float x, y;
            // draw each point
            //FunctionalInterpolator<float> pathPos = (*sources)[s]->getPathPos();
            // not having to copy the FunctionalInterp is much less expensive when there are lots of points
            FunctionalInterpolator<float>* pathPos = (*sources)[s].getPathPosPtr();
            //std::vector<std::vector<float>> pts = pathPos.getPoints();
            std::vector<std::vector<float>> pts = pathPos->getPoints();
            for (int i = 0; i < pts.size(); ++i) {
                x = x_scale*((pts[i][0]+thing1)*thing2-1.0);
                // if the point is within the automation view grid, draw it
                if (x > -1.05 && x < 1.05) {
                    y = y_scale*(pts[i][1]*2.0-1.0);
                    float xy[2] = {x, y};
                    // if mouse is over a path automation point or the point is already selected, then indicate with color
                    if ((mouseOverPathAutomationPointIndex[0] == -1
                        && x > m_x-aspect*0.025 && x < m_x+aspect*0.025 && y > m_y-0.025 && y < m_y+0.025)) {
                        mouseOverPathAutomationPointIndex = {s, i, -1};
                        glColor3f(1.0, 0.0, 0.0);
                        radius = 0.023;
                    } else if (pathAutomationPointsGrabbedWithMouse
                               && mouseOverPathAutomationPointIndex[0] == s
                               && mouseOverPathAutomationPointIndex[1] == i) {
                        glColor3f(1.0, 0.0, 0.0);
                        radius = 0.023;
                    } else {
                        radius = 0.02;
                        if (pathPos->getPointSelected(i) || pointInsideSelectRegion2D(xy)) {
                            glColor3f(0.8, 0.0, 0.1);
                        } else {
                            glColor3f(0.0, 0.3, 1.0);
                        }
                    }
                    drawCircle(x, y, radius/*0.02*/, 6);
//                    // draw pts index
//                    sprintf(str, "%d", i);
//                    drawStringStroke(str, x, y-0.05, 0);
                }
            }
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
                if (pathPosDisplayList[s] == 0 || processor->pathPosChanged || prevAutomationViewWidth != automationViewWidth || prevAutomationViewOffset != automationViewOffset) {
                    // if the interp changed (or the gl view was closed and the local points got cleared) we gotta update all the locally stored interp points used for drawing (more cpu intensive, would like to avoid)
                    glDeleteLists(pathPosDisplayList[s], 1);
                    pathPosDisplayList[s] = glGenLists(1);
                    glNewList(pathPosDisplayList[s], GL_COMPILE_AND_EXECUTE);
                    glColor3f(0.5, 0.0, 1.0);
                    glBegin(GL_LINES);//_STRIP);
    //                // the x-value way
    //                int N = 50; // num GL pts for drawing pathPos interp
    //                float begin = std::max<float>(pathPos.getInputRange()[0], automationViewOffset-automationViewWidth*0.5);
    //                float end = std::min<float>(pathPos.getInputRange()[1], automationViewOffset+automationViewWidth*0.5);
    //                float incr = (end - begin) / N;
    //                //float incr = (pathPos.getInputRange()[1] - pathPos.getInputRange()[0]) / ((pathPos.getNumPoints()-1)*N);
    //                //for (float x = pathPos.getInputRange()[0]; x < pathPos.getInputRange()[1]; x += incr) {
    //                for (float x = begin; x < end; x += incr) {
    //                    std::vector<float> pt;
    //                    // only draw the dotted line over the portions that are not open/empty segments, also avoiding drawing anything between 2pts with the same x val
    //                    if (pathPos.pointAtSmart(x, pt) /*&& pts[segmentIndex][0] != pts[segmentIndex+1][0]*/) {
    //                        y = y_scale*(2.0*pt[0]-1.0);
    //                        glVertex2f(x, y);
    //                    } else {
    //                        // finish drawing any unfinished GL_LINE segments
    //                        glVertex2f(x, y);
    //                        // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
    //                        glEnd();
    //                        glBegin(GL_LINE_STRIP);
    //                        x = pathPos.getNextSplineBegin(); //incr; // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
    //                    }
    //                }
                    // the new parametric way, just drawing the on screen part
                    int N = 500; // num GL pts for drawing pathPos interp across the visible screen
                    std::vector<float> inputRange = pathPos->getInputRange();
                    float begin = std::max<float>(inputRange[0], automationViewOffset-automationViewWidth*0.5/x_scale);
                    float end = std::min<float>(inputRange[1], automationViewOffset+automationViewWidth*0.5/x_scale);
                    float paraIntvl = (end - begin) / ((float)N);
                    //std::vector<float> pt;
                    float pt;
                    x = begin;
                    int prev_spline = 0;
                    int pprev_spline = 0;
                    //for (int i = 0; i < N; ++i)
                    while (x < end)
                    {
                        // only draw the dotted line over the portions that are not open/empty segments,
                        if (pathPos->pointAtSmart(x, &pt, prev_spline))
                        {
//                            // if we are on a new segment, do a vertex at the point we just passed
//                            if (prev_spline >= 0 && pathPos->prev_spline_index != prev_spline)
//                            {
////                                int dif = pathPos->prev_spline_index - prev_spline;
////                                for (int j = 1; j <= dif; ++j)
////                                    glVertex2f(pts[prev_spline+j][0], y_scale*(2.0*pts[prev_spline+j][1]-1.0));
//                                glVertex2f(pts[prev_spline+1][0], pts[prev_spline+1][1]);
//                            }
                            if ((pprev_spline+2 <= prev_spline) ? pts[pprev_spline+1][0] != pts[prev_spline][0] : true)
                            {
                                glVertex2f(x, pt);
                            }
                            else // avoiding drawing anything between 2 or more pts with the same x val
                            {
                                glEnd();
                                glBegin(GL_LINES);//_STRIP);
                            }
                            x += paraIntvl;
                        }
                        else
                        {
                            // finish drawing any unfinished GL_LINE segments
//                            if (i == N)
//                                glVertex2f(pts.back()[0], pts.back()[1]);
//                            else
                                glVertex2f(pts[prev_spline][0], pts[prev_spline][1]);
                            // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
                            glEnd();
                            glBegin(GL_LINES);//_STRIP);
                            if (prev_spline+1 < pts.size()) {
                                x = pts[prev_spline+1][0];// pathPos->getNextSplineBegin(); // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
                            } else
                                x = end; // exit the while loop
                        }
                        pprev_spline = prev_spline;
                        //prev_spline = pathPos->prev_spline_index;
                    }
    //                // the parametric way
    //                int N = 26; // num GL pts per segment for drawing pathPos interp
    //                float paraIntvl = 1.0/(pts.size()-1.0);
    //                std::vector<float> pt;
    //                for (int i = 0; i < (((int)(pts.size()))-1)*N; ++i) {
    //                    float paraVal = ((float)i)/((float)(pts.size()-1)*N);// + 0.001;, caused segmentIndex bad access crash
    //                    int segmentIndex = floor(paraVal*(pts.size()-1));
    //                    x = pts[segmentIndex][0] + (paraVal-segmentIndex*paraIntvl) * (pts[segmentIndex+1][0]-pts[segmentIndex][0])/paraIntvl;
    //                    // only draw the dotted line over the portions that are not open/empty segments, also avoiding drawing anything between 2pts with the same x val
    //                    if (pathPos.pointAt(x, pt) && pts[segmentIndex][0] != pts[segmentIndex+1][0]) {
    //                        y = y_scale*(2.0*pt[0]-1.0);
    //                        glVertex2f(x, y);
    //                    } else {
    //                        // finish drawing any unfinished GL_LINE segments
    //                        glVertex2f(x, y);
    //                        // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
    //                        glEnd();
    //                        glBegin(GL_LINES);
    //                        i += N-1; // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
    //                    }
    //                }
                    glEnd();
                    glEndList();
                    // tell the processor/source that we have saved the changes that we are concerned about
                    if (s == maxNumSources-1)
                        processor->pathPosChanged = false;
                } else {
                    // if the interp hasn't changed since the last frame, we can just draw with the locally stored points (less cpu intensive)
                    glCallList(pathPosDisplayList[s]);
    //                for (auto l : pathPosDisplayList[s])
    //                    glCallList(l);
    //                }
                }
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
                glColor4f(0.0, 1.0, 0.0, 0.9);
                x = x_scale * ((secPos+thing1)*thing2-1.0);
                y = y_scale * (2.0*y1 - 1.0);
                drawCircle(x, y, 0.015, 8);
            }
        }
    }
    
    // mr clock, mr clock, what time is it?
    float x = x_scale * ((secPos-automationViewOffset+automationViewWidth/2.0)*2.0/automationViewWidth - 1.0);
    if (std::abs(x) <= x_scale) {
        glColor3f(0.0, 1.0, 0.0);
        glBegin(GL_LINES);
        glVertex2f(x, -y_scale-0.02);
        glVertex2f(x, -y_scale+0.02);
        glEnd();
        //sprintf(str, "%.2f", secPos);
        getFormattedTime(secPos, str);
//        xlen = 0;
//        for (int i = 0; i < strlen(str); i++) {
//            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
//        }
//        xlen *= aspect*stroke_scale;
        xlen = strlen(str) * glutStrokeWidth(GLUT_STROKE_ROMAN, '0') * aspect * stroke_scale * 0.8;
        drawStringStroke(str, x-xlen*0.5, -y_scale-0.05, 0.0);
     
        // draw the parametric position indicator for each active source
        if (processor->getLockSourcesToPaths()) {
            for (int s = 0; s < sources->size(); ++s) {
                if ((*sources)[s].getSourceSelected() && !(*sources)[s].getSourceMuted()
                    && (*sources)[s].getNumPathPoints() > 1) {
                    std::vector<float> parametricPos;
                    float y1;
                    if ((*sources)[s].getPathPosPtr()->pointAt(secPos, parametricPos))
                        y1 = parametricPos[0];
                    else
                        y1 = -1;
                    float y = y_scale*(2.0*y1 - 1.0);
                    glColor3f(0.0, 1.0, 0.0);
                    glBegin(GL_LINES);
                    glVertex2f(-x_scale-0.02, y);
                    glVertex2f(-x_scale+0.02, y);
                    glEnd();
                    //sprintf(str, "%.2f", y1);
                    sprintf(str, "%d%%", (int)(y1*100));
                    xlen = 0;
                    for (int i = 0; i < strlen(str); ++i) {
                        xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
                    }
                    xlen *= aspect*stroke_scale;
                    drawStringStroke(str, fmax(-0.998, -x_scale-0.025-xlen), y-topMax*0.5, 0.0);
                }
            }
        }
    }
    
    if (processor->loopRegionBegin != -1 && processor->loopRegionEnd != -1)
        drawLoopingRegion();

    // draw mouse drag boarders if mouse is being dragged to select a group of things
    if (mouseDragging)
        drawMouseDragging();
    
    // draw the mouse's position and the numbers for the automation graph
    if (std::abs(m_x) <= 1.0 && -1.0 <= m_y && m_y < tabs.getBottom()) {
        float tVal, yVal, x, y;
        if (mouseOverPathAutomationPointIndex[0] != -1) {
            glLineWidth(1.5);
            const std::vector<float> ptMouseIsOver = (*sources)[mouseOverPathAutomationPointIndex[0]].getPathPosPtr()->getPoint(mouseOverPathAutomationPointIndex[1]);
            if (loopRegionBeginSelected || loopRegionEndSelected) {
                glColor3ub(101, 255, 145);
                //yVal = 0.5*(m_y/y_scale+1.0);
            } else {
                glColor4f(1.0, 0.0, 0.0, 1.0);
                //yVal = ptMouseIsOver[1];
            }
            tVal = ptMouseIsOver[0];
            yVal = ptMouseIsOver[1];
            x = x_scale*((tVal+thing1)*thing2-1.0) + 0.02;
            y = y_scale*(yVal*2.0-1.0) + 0.03;
        } else if (mouseOverLoopRegionBegin || loopRegionBeginSelected
                   || mouseOverLoopRegionEnd || loopRegionEndSelected) {
            glLineWidth(1.5);
            glColor3ub(101, 255, 145);
            if (mouseOverLoopRegionBegin) {
                tVal = processor->loopRegionBegin;
            } else if (mouseOverLoopRegionEnd) {
                tVal = processor->loopRegionEnd;
            }
            //yVal = 0.5*(m_y/y_scale+1.0);
            x = x_scale*((tVal+thing1)*thing2-1.0) + 0.02;
            y = m_y+0.022;
        } else {
            glColor4f(1.0, 1.0, 1.0, 0.7);
            yVal = 0.5*(m_y/y_scale+1.0);
            tVal = (m_x/x_scale+1.0)*automationViewWidth/2.0+automationViewOffset-automationViewWidth/2.0;
            if (yVal < 0)
                yVal = 0;
            if (yVal > 1)
                yVal = 1;
            if (tVal < 0)
                tVal = 0;
            x = m_x+0.01;
            y = m_y+0.022;
        }
        char anotherstr[20];
        getFormattedTime(tVal, anotherstr);
        if (mouseOverLoopRegionBegin || loopRegionBeginSelected
            || mouseOverLoopRegionEnd || loopRegionEndSelected)
            sprintf(str, "%s", anotherstr);
        else
            sprintf(str, "%d%% %s", (int)(yVal*100), anotherstr);
        xlen = 0;
        for (int i = 0; i < strlen(str); ++i) {
            xlen += glutStrokeWidth(GLUT_STROKE_ROMAN, str[i]);
        }
        xlen *= aspect*stroke_scale;
        if (y + topMax > y_scale) {
            y = y_scale-topMax;
            x += 0.02;
        }
        if (x + xlen > 1) {
            x = 1-xlen;
        }
        drawStringStroke(str, x, y, 0.0);
    }
    glLineWidth(1.0); // reset this
    
    // draw autoalign animation
    if (animationOn[0]) {
        glColor4f(1.0, 1.0, 1.0, 1.0-animationTime[0]/animationDuration);
        glBegin(GL_LINES);
        if (std::abs(autoAlignPos[0]) <= 1.0) {
            glVertex2f(autoAlignPos[0], -y_scale);
            glVertex2f(autoAlignPos[0], y_scale);
        }
        glEnd();
    }
    if (animationOn[1]) {
        glColor4f(1.0, 1.0, 1.0, 1.0-animationTime[1]/animationDuration);
        glBegin(GL_LINES);
        if (std::abs(autoAlignPos[1]) <= 1.0) {
            glVertex2f(-x_scale, autoAlignPos[1]);
            glVertex2f(x_scale, autoAlignPos[1]);
        }
        glEnd();
    }

    //showMouseDetails();
    
    // Making sure we can render 3d again
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void ThreeDAudioProcessorEditor::drawMouseDragging()
{
    glColor4f(1.0, 1.0, 1.0, 1.0);
    glBegin(GL_LINE_STRIP);
    glVertex2f(mouseDragDownX, mouseDragDownY);
    glVertex2f(mouseDragDownX, mouseDragCurrentY);
    glVertex2f(mouseDragCurrentX, mouseDragCurrentY);
    glVertex2f(mouseDragCurrentX, mouseDragDownY);
    glVertex2f(mouseDragDownX, mouseDragDownY);
    glEnd();
    
    glColor4f(0.6, 0.6, 0.6, 0.2);
    glBegin(GL_POLYGON);
    glVertex2f(mouseDragDownX, mouseDragDownY);
    glVertex2f(mouseDragDownX, mouseDragCurrentY);
    glVertex2f(mouseDragCurrentX, mouseDragCurrentY);
    glVertex2f(mouseDragCurrentX, mouseDragDownY);
    glEnd();
}

void ThreeDAudioProcessorEditor::drawInterpolatedPath(const int s)
{
    // draw interpolated path
    std::unique_ptr<ParametricInterpolator<float>> interp = std::move((*sources)[s].getPath());
    if (interp.get() != nullptr) {
        std::vector<std::vector<float>> points = interp->getPoints();
        // if the path interp has changed since the last frame or if the local copy is empty (as is the case when the gl view is closed and opened again)
        //   then gotta recompute our locally held points for drawing the path (more cpu)
        if (pathDisplayList[s] == 0 || processor->pathChanged) {
            float length = interp->getInputRange()[1];
            const int N = 20*points.size();
            // add the glvertex() calls into display list for better performance with the cached data on later static drawing
            glDeleteLists(pathDisplayList[s], 1);
            pathDisplayList[s] = glGenLists (1);
            glNewList (pathDisplayList[s], GL_COMPILE_AND_EXECUTE);
            glBegin(GL_LINES);
            std::vector<float> pt;
            for (int i = 1; i < N; ++i) {
                if (interp->pointAt(((float)i) / ((float)(N-1)) * length, pt))
                    glVertex3f(pt[0], pt[1], pt[2]);
            }
            glEnd();
            glEndList();
            // tell the processor/source that we have saved the changes that we are concerned about
            if (s == maxNumSources-1)
                processor->pathChanged = false;
        } else {
            glCallList(pathDisplayList[s]);
        }
    }
}

void ThreeDAudioProcessorEditor::drawLoopingRegion()
{
    const float beginX = timeValueToXPosition(processor->loopRegionBegin);
    const float endX = timeValueToXPosition(processor->loopRegionEnd);
    const float y_scale = 0.9;
    const float mouse_x = getMouseX();
    const float mouse_y = getMouseY();
    float mouseOverLoopBoarderX = -1.1;
    
    // was preformed in mouseMove(), but done here to be consistent with mouse over path auto pt state determined in drawPathControl()
    if (mouseOverPathAutomationPointIndex[0] != -1) {
        const float t = processor->getPathAutomationPoints(mouseOverPathAutomationPointIndex[0])[mouseOverPathAutomationPointIndex[1]][0];
        if (loopRegionBeginSelected)
            processor->loopRegionBegin = t;
        else if (loopRegionEndSelected)
            processor->loopRegionEnd = t;
    }
    
    glColor3ub(101, 255, 145);
    if (loopRegionBeginSelected ||
        (!mouseDragging && mouseOverPathAutomationPointIndex[0] == -1
        && std::abs(mouse_x-beginX) < 0.01 && std::abs(mouse_y) < y_scale)) {
        glLineWidth(2.0);
        mouseOverLoopBoarderX = beginX;
        mouseOverLoopRegionBegin = true;
    } else {
        glLineWidth(1.0);
        mouseOverLoopRegionBegin = false;
    }
    glBegin(GL_LINES);
    glVertex2f(beginX, -y_scale);
    glVertex2f(beginX, +y_scale);
    glEnd();
    
    if (loopRegionEndSelected ||
        (!mouseDragging && mouseOverPathAutomationPointIndex[0] == -1 && mouseOverLoopBoarderX < -1.1
        && std::abs(mouse_x-endX) < 0.01 && std::abs(mouse_y) < y_scale)) {
        glLineWidth(2.0);
        mouseOverLoopBoarderX = endX;
        mouseOverLoopRegionEnd = true;
    } else {
        glLineWidth(1.0);
        mouseOverLoopRegionEnd = false;
    }
    glBegin(GL_LINES);
    glVertex2f(endX, -y_scale);
    glVertex2f(endX, +y_scale);
    glEnd();
    
    glLineWidth(1.0);
    if (mouseOverLoopBoarderX > -1.0) {
        glBegin(GL_LINES);
        glVertex2f(mouseOverLoopBoarderX-0.01, mouse_y);
        glVertex2f(mouseOverLoopBoarderX-0.03, mouse_y);
        glVertex2f(mouseOverLoopBoarderX+0.01, mouse_y);
        glVertex2f(mouseOverLoopBoarderX+0.03, mouse_y);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(mouseOverLoopBoarderX-0.03, mouse_y);
        glVertex2f(mouseOverLoopBoarderX-0.02, mouse_y+0.0075);
        glVertex2f(mouseOverLoopBoarderX-0.02, mouse_y-0.0075);
        glVertex2f(mouseOverLoopBoarderX-0.03, mouse_y);
        glEnd();
        glBegin(GL_POLYGON);
        glVertex2f(mouseOverLoopBoarderX+0.03, mouse_y);
        glVertex2f(mouseOverLoopBoarderX+0.02, mouse_y+0.0075);
        glVertex2f(mouseOverLoopBoarderX+0.02, mouse_y-0.0075);
        glVertex2f(mouseOverLoopBoarderX+0.03, mouse_y);
        glEnd();
    }
    
    glColor4ub(101, 255, 145, 0.1*255);
    glBegin(GL_POLYGON);
    glVertex2f(beginX, -y_scale);
    glVertex2f(endX,   -y_scale);
    glVertex2f(endX,   +y_scale);
    glVertex2f(beginX, +y_scale);
    glEnd();
}

inline float ThreeDAudioProcessorEditor::timeValueToXPosition(const float tVal) const
{
    const float x_scale = 0.9;
    const float thing1 = automationViewWidth*0.5-automationViewOffset;
    const float thing2 = 2.0/automationViewWidth;
    return x_scale*((tVal+thing1)*thing2-1.0);
}

inline float ThreeDAudioProcessorEditor::getMouseX() const
{
    return (2.0*getMouseXYRelative().getX())/getWidth() - 1.0;
}

inline float ThreeDAudioProcessorEditor::getMouseY() const
{
    return (-2.0*getMouseXYRelative().getY())/getHeight() + 1.0;
}

void ThreeDAudioProcessorEditor::drawStringBitmap(void *font, char *s, float x, float y, float z) const
{
    glRasterPos3f(x, y, z);
    for (int i = 0; i < strlen(s); ++i)
        glutBitmapCharacter(font, s[i]);
}

void ThreeDAudioProcessorEditor::drawStringStroke(char *s, float x, float y, float z) const
{
    const float aspect = ((float)getHeight())/((float)getWidth());
    const float stroke_scale = 0.00035;
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(aspect*stroke_scale, stroke_scale, stroke_scale);
    for (int i = 0; i < strlen(s); ++i)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);
    glPopMatrix();
}
void drawStringStroke(char *s, float x, float y, float z, float aspect, float scale)
{
    const float stroke_scale = 0.000346*scale;
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(aspect*stroke_scale, stroke_scale, stroke_scale);
    for (int i = 0; i < strlen(s); ++i)
        glutStrokeCharacter(GLUT_STROKE_ROMAN, s[i]);
    glPopMatrix();
}

void ThreeDAudioProcessorEditor::getFormattedTime(const float timeInSec, char* str) const
{
    const int secs = std::floor(timeInSec);
    const int mins = secs / 60 % 60;
    const int hours = secs / 3600;
    if (hours != 0)
        if (std::fmod(timeInSec, 60) < 10)
            sprintf(str, "%d:%.2d:0%.2f", hours, mins, std::fmod(timeInSec, 60));
        else
            sprintf(str, "%d:%.2d:%.2f", hours, mins, std::fmod(timeInSec, 60));
    else if (mins != 0)
        if (std::fmod(timeInSec, 60) < 10)
            sprintf(str, "%d:0%.2f", mins, std::fmod(timeInSec, 60));
        else
            sprintf(str, "%d:%.2f", mins, std::fmod(timeInSec, 60));
    else
        sprintf(str, "%.2f", timeInSec);
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

// everything drawn by the GL
void ThreeDAudioProcessorEditor::renderOpenGL()
{
    // reset the playing state of the processor if processBlock() not called in a while
    processor->resetPlaying(openGLFrameRate);
    
    // lock us a copy of the sources for drawing
    sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        
        // reset this stuff for detecting mouse over selectable objects so it has to be reset during this render cycle for there to be any action on those selectable objects
        mouseOverSourceIndex = -1;
        mouseOverPathPointSourceIndex = -1;
        mouseOverPathPointIndex = -1;
        
        // advance animation state
        if (animationOn[0]) {
            animationTime[0] += getTimerInterval(0)/1000.0;
            if (animationTime[0] > animationDuration) {
                animationTime[0] = 0;
                autoAlignPos[0] = 1.1;
                autoAlignPos[1] = 1.1;
                animationOn[0] = false;
            }
        }
        if (animationOn[1]) {
            animationTime[1] += getTimerInterval(0)/1000.0;
            if (animationTime[1] > animationDuration) {
                animationTime[1] = 0;
                autoAlignPos[0] = 1.1;
                autoAlignPos[1] = 1.1;
                animationOn[1] = false;
            }
        }
        
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
                // set up 2d projection for HUD
                glMatrixMode(GL_PROJECTION);
                glPushMatrix();
                glLoadIdentity();
                // 2d screen from +/- 1 in x/y (and from -1 to +10 z if desired)
                glOrtho(-1, 1, -1, 1, -1.0, 10.0);
                glMatrixMode(GL_MODELVIEW);
                glLoadIdentity();
                glDisable(GL_DEPTH_TEST);   // either GL_DEPTH_TEST or GL_CULL_FACE seem to work
                
                processingModeOptions.setAutoDetectOption(processor->isHostRealTime);
                processingModeOptions.setSelectedOption((int)(processor->processingMode.load()));
                processingModeOptions.draw(windowAspectRatio);
                glColor4f(86.0/255.0, 1.0, 86.0/255.0, 1.0);
                processingModeText.draw();
                
                // Making sure we can render 3d again
                glMatrixMode(GL_PROJECTION);
                glPopMatrix();
                glMatrixMode(GL_MODELVIEW);
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
        tabs.draw(windowAspectRatio);
        
        // Making sure we can render 3d again
        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        
        // needed for deciding when to draw the pathPos interp
        prevAutomationViewOffset = automationViewOffset;
        prevAutomationViewWidth = automationViewWidth;
        
        // swap buffers for double buffering
        glutSwapBuffers();
    }
}


void ThreeDAudioProcessorEditor::timerCallback(const int timerID)
{
    switch (timerID) {
        case 0: // tell the gl to repaint the scene on the interval that timer0 is set to
            if (processor != nullptr && glInited) {
                openGLContext.triggerRepaint();
            }
            break;
        case 1: // timer1 is for checking if a double click has happened within timer1's interval after a mouseUp() event
            if (!mouseDoubleClicked && mouseJustDown /*&& !mouseClickedOnSomething*/ && !mouseDragging) {
                // a single click in space deselects everything
                processor->selectAllSources(false);
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

void ThreeDAudioProcessorEditor::mouseMove(const MouseEvent & event)
{
    switch (processor->displayState) {
        case DisplayState::MAIN:
            break;
        case DisplayState::PATH_AUTOMATION:
            if (pathAutomationPointsGrabbedWithMouse) {
                const float mouse_x = getMouseX();
                const float mouse_y = getMouseY();
                const float x_scale = 0.9, y_scale = 0.9;
                const float t = (mouse_x / x_scale + 1.0) * automationViewWidth*0.5
                                - (automationViewWidth*0.5-automationViewOffset);
                const float y = (mouse_y / y_scale + 1.0) * 0.5;
                processor->moveSelectedPathAutomationPointsTo(mouseOverPathAutomationPointIndex[0],
                                                              mouseOverPathAutomationPointIndex[1],
                                                              mouseOverPathAutomationPointIndex[2], t, y);
                if (event.mods.isAltDown() && !(event.mods.isCommandDown() || event.mods.isCtrlDown()))
                    autoAlignAutomationPoints(true, true, false);
            } else if (loopRegionBeginSelected || loopRegionEndSelected) {
                if (mouseOverPathAutomationPointIndex[0] != -1) {
//                    // done in drawLoopingRegion()
//                    const float t = processor->getPathAutomationPoints(mouseOverPathAutomationPointIndex[0])[mouseOverPathAutomationPointIndex[1]][0];
//                    if (loopRegionBeginSelected)
//                        processor->loopRegionBegin = t;
//                    else if (loopRegionEndSelected)
//                        processor->loopRegionEnd = t;
                } else {
                    const float mouse_x = getMouseX();
                    const float x_scale = 0.9;
                    const float t = std::max(0.0, (mouse_x / x_scale + 1.0) * automationViewWidth*0.5
                                                  - (automationViewWidth*0.5-automationViewOffset));
                    if (loopRegionBeginSelected)
                        processor->loopRegionBegin = std::max((float)0.0, t);
                    else if (loopRegionEndSelected)
                        processor->loopRegionEnd = std::max((float)0.0, t);
                }
                if (processor->loopRegionBegin > processor->loopRegionEnd) {
                    const float temp = processor->loopRegionBegin;
                    processor->loopRegionBegin.store(processor->loopRegionEnd);
                    processor->loopRegionEnd = temp;
                }
            }
            break;
        case DisplayState::SETTINGS:
            processingModeOptions.updateFromMouse(getMouseX(), getMouseY(), false, windowAspectRatio);
            processingModeText.loadText(processingModeOptions.getHelpText());
            break;
        default:
            break;
    }
    if (!mouseDragging && !loopRegionBeginSelected && !loopRegionEndSelected && !pathAutomationPointsGrabbedWithMouse)
        tabs.updateFromMouse(getMouseX(), getMouseY(), false);
}

void ThreeDAudioProcessorEditor::mouseDrag(const MouseEvent & event)
{
    const double mouse_x = getMouseX();
    const double mouse_y = getMouseY();
    if (mouseDragging) {
        // updating current mouse drag bounds
        mouseDragCurrentX = mouse_x;
        mouseDragCurrentY = mouse_y;
    } else {
        // first mouseDrag callback of mouse drag event
        mouseDragging = (event.getDistanceFromDragStart() > 0) && !pathAutomationPointsGrabbedWithMouse && mouse_y < tabs.getBottom(); // bad things can happen if we mouse drag when the path pts are grabbed
                        //true; (this used to work when mouseDrag() was not called before any old mouseUp() event, prior to JUCE 4)
        mouseDragDownX = mouse_x;
        mouseDragDownY = mouse_y;
        mouseDragCurrentX = mouse_x;
        mouseDragCurrentY = mouse_y;
    }
}

void ThreeDAudioProcessorEditor::mouseUp(const MouseEvent & event)
{
    sources = nullptr;
    const Locker lock (processor->sources.get(sources));
    if (sources) {
        
        const double m_x = getMouseX();
        const double m_y = getMouseY();
        
        if (!mouseDragging && !loopRegionBeginSelected && !loopRegionEndSelected && !pathAutomationPointsGrabbedWithMouse) {
            int newDisplayState = tabs.updateFromMouse(m_x, m_y, true);
            if (newDisplayState != -1) {
                processor->displayState = (DisplayState)newDisplayState;
                if (processor->displayState == DisplayState::PATH_AUTOMATION)
                    processor->makeSourcesVisibleForPathAutomationView();
                if (showHelp)
                    repaint(); // tell the plugin editor to call paint() soon again when it can so that the help/doppler text state can be updated
                goto SKIP;
            }
        }
    
        switch (processor->displayState) {
            case DisplayState::MAIN:
                
                if (mouseDragging) {
                    // select the sources in the selection region and mark em as selected
                    std::array<float,3> xyz;
                    for (int i = 0; i < sources->size(); ++i) {
                        bool pathPtsSelected = false;
                        if ((*sources)[i].getSourceSelected()) {
                            // check and see if any of the selected sources' path points are inside selected region
                            std::vector<std::vector<float>> pts = (*sources)[i].getPathPoints();
                            for (int j = 0; j < pts.size(); ++j) {
                                xyz[0] = pts[j][0];
                                xyz[1] = pts[j][1];
                                xyz[2] = pts[j][2];
                                float poser[3] = {xyz[0],xyz[1],xyz[2]};
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
                        float poser[3] = {xyz[0],xyz[1],xyz[2]};
                        if (pointInsideSelectRegion(poser) || ((*sources)[i].getSourceSelected() && pathPtsSelected) || mouseOverSourceIndex == i)
                            processor->setSourceSelected(i, true);
                        else if (!(event.mods.isShiftDown() && processor->getSourceSelected(i)))
                            processor->setSourceSelected(i, false);
                    }
                } else if (mouseOverSourceIndex != -1) {
                    // if mouse is clicked directly over a source, toggle its selected state
                    processor->setSourceSelected(mouseOverSourceIndex, !(*sources)[mouseOverSourceIndex].getSourceSelected());
                    //mouseClickedOnSomething = true;
                } else if (mouseOverPathPointIndex != -1 && mouseOverPathPointSourceIndex != -1) {
                    // if mouse is clicked over a path point, toggle its selected sate
                    processor->togglePathPointSelected(mouseOverPathPointSourceIndex, mouseOverPathPointIndex);
                    //mouseClickedOnSomething = true;
                } else if (mouseOverDopplerSlider) {
                    // if mouse is over the doppler slider, set speed of sound to the new value
                    processor->setSpeedOfSound(newDopplerSpeedOfSound);
                }
                //if (!mouseDragging) {
                else {
                    // start timer for checking if there was a double click
                    startTimer(1, 300.0); // much less than 300 makes it very hard to double click fast enough with my old fashioned mouse
                }
                break;
                
            case DisplayState::PATH_AUTOMATION:
            {
                const float x_scale = 0.9;
                const float y_scale = 0.9;
                // save the CPU !!!
                const float thing1 = automationViewWidth*0.5-automationViewOffset;
                const float thing2 = 2.0/automationViewWidth;
                if (mouseDragging) {
                    // if shift not held down, unselect those previously selected fucking cunts
                    if (!event.mods.isShiftDown())
                        processor->deselectAllPathAutomationPoints();
                    // if ending a mouse drag selection, then select all path automation points in that region
                    float xy[2];
                    for (int i = 0; i < sources->size(); ++i) {
                        if ((*sources)[i].getSourceSelected()) {
                            // check and see if any of the selected sources' path automation points are inside selected region
                            std::vector<std::vector<float>> pts = (*sources)[i].getPathPosPtr()->getPoints();
                            for (int j = 0; j < pts.size(); ++j) {
                                xy[0] = x_scale * ((pts[j][0] + thing1)*thing2 - 1.0);
                                xy[1] = y_scale * (pts[j][1]*2.0 - 1.0);
                                if (pointInsideSelectRegion2D(xy)
                                    || (xy[0] > m_x-0.025 && xy[0] < m_x+0.025 && xy[1] > m_y-0.025 && xy[1] < m_y+0.025)) {
                                    // if a point is inside the selected region, mark it as selected
                                    processor->setPathAutomationPointSelectedState(i, j, true);
                                }
                            }
                        }
                    }
                } else {
                    if (loopRegionBeginSelected) {
                        loopRegionBeginSelected = false;
                    } else if (loopRegionEndSelected) {
                        loopRegionEndSelected = false;
                    } else {
                        if (pathAutomationPointsGrabbedWithMouse) {
                            mouseOverPathAutomationPointIndex = {-1,-1,-1};
                            pathAutomationPointsGrabbedWithMouse = false;
                        }
                        if (mouseOverPathAutomationPointIndex[0] != -1) {
                            processor->togglePathAutomationPointSelected(mouseOverPathAutomationPointIndex[0],
                                                                         mouseOverPathAutomationPointIndex[1]);
                        } else if (mouseOverLoopRegionBegin) {
                            loopRegionBeginSelected = ! loopRegionBeginSelected;
                        } else if (mouseOverLoopRegionEnd) {
                            loopRegionEndSelected = ! loopRegionEndSelected;
                        } else {
                            processor->deselectAllPathAutomationPoints();
                        }
                    }
                }
                break;
            }
                
            case DisplayState::SETTINGS:
            {
                int selectedMode = processingModeOptions.updateFromMouse(m_x, m_y, true, windowAspectRatio);
                if (selectedMode > 0) {
                    processor->setProcessingMode((ProcessingMode)(selectedMode-1));
                    processingModeOptions.setAutoDetectOption(processor->isHostRealTime);
                }
                break;
            }
        }
    } // end if (sources)
SKIP:
    // mouse is no longer down, duh
    mouseIsDown = false;
    
    // reset mouse dragging state
    mouseDragDownX = 0;
    mouseDragDownY = 0;
    mouseDragCurrentX = 0;
    mouseDragCurrentY = 0;
    mouseDragging = false;
}

void ThreeDAudioProcessorEditor::mouseDown(const MouseEvent & event)
{
    mouseIsDown = true;
    mouseJustDown = true;
}

void ThreeDAudioProcessorEditor::mouseDoubleClick(const MouseEvent & event)
{
    mouseDoubleClicked = true;
    const double mouse_x = getMouseX();
    const double mouse_y = getMouseY();
    
    // don't do any of the stuff below if we click in the tab bar area
    if (mouse_y > tabs.getBottom()) 
        return;
    
    switch (processor->displayState) {
        case DisplayState::MAIN:
            if (mouseOverSourceIndex != -1) {
                // if mouse is clicked directly over a source, toggle its selected state
                processor->setSourceSelected(mouseOverSourceIndex, !processor->getSourceSelected(mouseOverSourceIndex));
                //mouseClickedOnSomething = true;
            } else if (mouseOverPathPointIndex != -1 && mouseOverPathPointSourceIndex != -1) {
                // if mouse is clicked over a path point, toggle its selected state
                processor->togglePathPointSelected(mouseOverPathPointSourceIndex, mouseOverPathPointIndex);
                //mouseClickedOnSomething = true;
            } else if (mouseOverDopplerSlider) {
                // if mouse is over the doppler slider, set speed of sound to the new value
                processor->setSpeedOfSound(newDopplerSpeedOfSound);
            } else {
                // get the mouse's 3d click position
                float pt[3];
                to3D(mouse_x, mouse_y, pt);
                // try to add a path point for the selected sources
                if (!processor->dropPathPoint(pt)) {
                    // if no sources are selected add a new source at the 3d click position
                    processor->addSourceAtXYZ(pt);
                }
            }
            break;
            
        case DisplayState::PATH_AUTOMATION:
            if (mouseOverPathAutomationPointIndex[0] != -1) {
                // pick up all selected points to move them
                // select a point if unselected but mouse is over it
                processor->setPathAutomationPointSelectedState(mouseOverPathAutomationPointIndex[0],
                                                               mouseOverPathAutomationPointIndex[1], true);
                const int indexAmongSelecteds = processor->getPathAutomationPointIndexAmongSelectedPoints
                                                              (mouseOverPathAutomationPointIndex[0],
                                                               mouseOverPathAutomationPointIndex[1]);
                mouseOverPathAutomationPointIndex[2] = indexAmongSelecteds;
                pathAutomationPointsGrabbedWithMouse = true;
            } else if (mouseOverLoopRegionBegin || loopRegionBeginSelected) {
                loopRegionBeginSelected = ! loopRegionBeginSelected;
            } else if (mouseOverLoopRegionEnd || loopRegionEndSelected) {
                loopRegionEndSelected = ! loopRegionEndSelected;
            } else {
                // add a new path automation point at the mouse's position
                const float x_scale = 0.9;
                const float y_scale = 0.9;
                float pt2d[2] = { (float)((mouse_x/x_scale)*automationViewWidth/2.0+automationViewOffset), (float)((mouse_y/y_scale+1.0)/2.0) };
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
    }
}

void ThreeDAudioProcessorEditor::mouseWheelMove	(const MouseEvent & event,
                                                 const MouseWheelDetails & wheel)
{
    mouseWheeldX = wheel.deltaX;
    mouseWheeldY = wheel.deltaY;
    const bool DXLargerThanDY = std::abs(mouseWheeldX) >= std::abs(mouseWheeldY);
    
    if (processor->displayState == DisplayState::MAIN) {
        int sourcesMoved = 0;
        if (!(event.mods.isCommandDown() || event.mods.isCtrlDown())) {
            // atempt to move selected sources
            if (DXLargerThanDY) {
                if (event.mods.isAltDown())
                    sourcesMoved = processor->moveSelectedSourcesRAE(1+mouseWheeldX, 0, 0);
                else
                    sourcesMoved = processor->moveSelectedSourcesRAE(1, mouseWheeldX, 0);
            } else {
                if (event.mods.isAltDown())
                    sourcesMoved = processor->moveSelectedSourcesRAE(1+mouseWheeldY, 0, 0);
                else
                    sourcesMoved = processor->moveSelectedSourcesRAE(1, 0, mouseWheeldY);
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
                
                if (eyeRad < 0.2)
                    eyeRad = 0.2;
                
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else if (DXLargerThanDY) {
                
                eyeAzi -= mouseWheeldX;
                
                if (eyeAzi < 0)
                    eyeAzi += 2*M_PI;
                
                if (eyeAzi > 2*M_PI)
                    eyeAzi -= 2*M_PI;
                
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else {
                
                eyeEle += upDir*mouseWheeldY;
                
                if (eyeEle < 0) {
                    eyeEle *= -1.0;
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1;
                }
                
                if (eyeEle > M_PI) {
                    eyeEle = M_PI-(eyeEle-M_PI);
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1;
                }
                
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
                // update up vector if we filp the camera upside down during our rotations
                eyeUp[0] = 0;
                eyeUp[1] = upDir;
                eyeUp[2] = 0;
            }
        }
        
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
                
                if (eyeRad < 0.2)
                    eyeRad = 0.2;
                
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else if (DXLargerThanDY) {
                
                eyeAzi -= mouseWheeldX;
                
                if (eyeAzi < 0)
                    eyeAzi += 2*M_PI;
                
                if (eyeAzi > 2*M_PI)
                    eyeAzi -= 2*M_PI;
                
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
            } else {
                
                eyeEle += upDir*mouseWheeldY;
                
                if (eyeEle < 0) {
                    eyeEle *= -1.0;
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1;
                }
                
                if (eyeEle > M_PI) {
                    eyeEle = M_PI-(eyeEle-M_PI);
                    eyeAzi = M_PI+eyeAzi;
                    upDir *= -1;
                }
                
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                
                // update up vector if we filp the camera upside down during our rotations
                eyeUp[0] = 0;
                eyeUp[1] = upDir;
                eyeUp[2] = 0;
            }
            
        } else {
//        MOVE_2D_VIEW:
            // adjust the automation grid view ...
            if (DXLargerThanDY) { // ... in width
                automationViewOffset -= mouseWheeldX*automationViewWidth;
                if (automationViewOffset < automationViewWidth/2.0) {
                    automationViewOffset = automationViewWidth/2.0;
                }
            } else { // ... in zoom
                float newAutomationViewWidth = automationViewWidth+automationViewWidth*0.3*mouseWheeldY;
                // keep view width positive otherwise it could get wonky
                if (newAutomationViewWidth < 0.001)
                    newAutomationViewWidth = 0.001;
                const float mouse_x = getMouseX();
                const float x_scale = 0.9;
                const float x = mouse_x/x_scale;
                automationViewOffset += x*(automationViewWidth - newAutomationViewWidth)/2.0;
                automationViewWidth = newAutomationViewWidth;
                // adjust the view offset if necessary so that we never stray into negative time
                if (automationViewOffset < automationViewWidth/2.0) {
                    automationViewOffset = automationViewWidth/2.0;
                }
            }
        }
//        }
    }
}

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
        
        // scaling for path auto grid
        const float x_scale = 0.9;
        const float y_scale = 0.9;
        
        // mouse position
        const float mouse_x = getMouseX();
        const float mouse_y = getMouseY();
        
        // get all the points that we want to check to line up with
        std::vector<std::vector<std::vector<float>>> allAutoPts;
        for (int s = 0; s < sources->size(); ++s) {
            if ((*sources)[s].getSourceSelected())
                allAutoPts.emplace_back((*sources)[s].getPathPosPtr()->getPoints());
        }
        
        // precomputation of constants to save cpu in loops below
        const float A = automationViewWidth/2.0-automationViewOffset;
        const float B = 2.0/automationViewWidth;
        
        // did we align in x/y yet?
        bool alignedInX = false, alignedInY = false;
        
        // check if any of the path automation points want to "line up" with each other ...
        // if they do and we are not moving the points "too fast" in the x direction (or y), then set the selected points' position to line up as desired
        // otherwise just move the points as per usual
        for (int s = 0; s < sources->size(); ++s) {
            if ((*sources)[s].getSourceSelected()) {
                const std::vector<std::vector<float>> pts = (*sources)[s].getPathPosPtr()->getSelectedPoints();
                const std::vector<int> pts_indecies = (*sources)[s].getPathPosPtr()->getSelectedPointIndicies();
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
                                    autoAlignedIndex[2] = (((mouse_x/x_scale+1)-1)*automationViewWidth*0.5+automationViewOffset) - pts[i][0]; // aligned in x dim (∆ = new - old)
                                    autoAlignPos[0] = mouse_x;
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
                                    autoAlignPos[1] = mouse_y;
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
                                                autoAlignPos[0] = lineUpPtX;
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
                                                autoAlignPos[1] = lineUpPtY;
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
                    animationOn[0] = true;
                    animationTime[0] = 0;
                }
                if (autoAlignedIndex[3]) {
                    animationOn[1] = true;
                    animationTime[1] = 0;
                }
            }
        }
    } // end if (sources)
}

void ThreeDAudioProcessorEditor::mouseMagnify (const MouseEvent & event,
                                               float scaleFactor)
{
    mouseZoomFactor = scaleFactor;
    
    if (processor->displayState == DisplayState::MAIN) {
        
        int sourcesMoved = 0;
        if (!(event.mods.isCommandDown() || event.mods.isCtrlDown()))
            sourcesMoved = processor->moveSelectedSourcesRAE(mouseZoomFactor, 0, 0);
        
        // tried to move selected sources, but there were none so move the camera
        if (sourcesMoved == 0) {
            // move the eye radially
            if (mouseZoomFactor != 1.0) {
                eyeRad *= mouseZoomFactor;
                if (eyeRad < 0.2)
                    eyeRad = 0.2;
                updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
            }
        }
        
    } else if (processor->displayState == DisplayState::PATH_AUTOMATION) {
        
        if (event.mods.isCommandDown() || event.mods.isCtrlDown()) {
            if (processor->areAnySelectedSourcesPathAutomationPointsSelected()) {
                if (pathAutomationPointsGrabbedWithMouse)
                    goto MOVE_3D_VIEW;
                else
                    goto MOVE_2D_VIEW;
            } else {
            MOVE_3D_VIEW:
                // move the eye radially
                if (mouseZoomFactor != 1.0) {
                    eyeRad *= mouseZoomFactor;
                    if (eyeRad < 0.2)
                        eyeRad = 0.2;
                    updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                }
            }
        }
        else {
        MOVE_2D_VIEW:
            // adjust the automation grid view width
            if (mouseZoomFactor != 0) {
                float newAutomationViewWidth = automationViewWidth/mouseZoomFactor;
                // keep view width positive otherwise it could get wonky
                if (newAutomationViewWidth < 0.001)
                    newAutomationViewWidth = 0.001;
                const float mouse_x = getMouseX();
                const float x_scale = 0.9; // might want to make this a global var
                const float x = mouse_x/x_scale;
                automationViewOffset += x*(automationViewWidth - newAutomationViewWidth)/2.0;
                automationViewWidth = newAutomationViewWidth;
                // adjust the view offset if necessary so that we never stray into negative time
                if (automationViewOffset < automationViewWidth/2.0) {
                    automationViewOffset = automationViewWidth/2.0;
                }
            }
        }
    }
}

bool ThreeDAudioProcessorEditor::keyPressed	(const KeyPress & key)
{
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
        tabs.setSelectedTab((int)processor->displayState.load());
        if (showHelp)
            repaint(); // tell the plugin editor to call paint() soon again when it can so that the help/doppler text state can be updated
    }
    
    // 'h' to toggle showing the help text
    if (key.getTextDescription().equalsIgnoreCase("H")) {
        showHelp = ! showHelp;
        repaint(); // tell the plugin editor to call paint() soon again when it can so that the help text state can be updated
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
    }
    
    // redo last undone action
    if (key.getTextDescription().equalsIgnoreCase("SHIFT + COMMAND + Z") || key.getTextDescription().equalsIgnoreCase("CTRL + SHIFT + Z")) {
        processor->redo();
    }
    
    if (key.getTextDescription().equalsIgnoreCase("D")) {
        processor->toggleDoppler();
        //repaint(); // tell the plugin editor to call paint() soon again when it can so that the doppler text state can be updated
    }
    
    // 'm' to toggle if sources are locked to move on their paths
    if (key.getTextDescription().equalsIgnoreCase("M")) {
        processor->toggleLockSourcesToPaths();
    }
    
    // so holding down arrow keys can cause things to be moved quicker
    if (key.isKeyCode(KeyPress::upKey) || key.isKeyCode(KeyPress::downKey)
        || key.isKeyCode(KeyPress::leftKey) || key.isKeyCode(KeyPress::rightKey))
        //|| key.getTextDescription().equalsIgnoreCase("Q") || key.getTextDescription().equalsIgnoreCase("W"))
    {
        if (isTimerRunning(2))
            arrowKeySpeedFactor *= 1.2;
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
                for (int s = 0; s < 8; ++s)
                    pathDisplayList[s] = 0; // clear these so that old paths aren't drawn if a copy is done later
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
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2]);
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
                        eyeRad -= 0.05 * eyeRad;
                        
                        // don't move view inside head
                        if (eyeRad < 0.2)
                            eyeRad = 0.2;
                        
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if shift is not down, move view ele up (down in number value)
                    {
                        eyeEle -= upDir*4*M_PI/180;
                        
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
                        
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
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
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2]);
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
                        eyeRad += 0.05 * eyeRad;
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
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
                        
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
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
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2]);
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
                    
                    updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
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
                    numMoved = processor->moveSelectedSourcesXYZ(dir[0], dir[1], dir[2]);
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
                    
                    updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                }
            }
            
            break;
            
        case DisplayState::PATH_AUTOMATION:
            
            // 'd' or backspace to delete selected path automation points
            if (key.isKeyCode(KeyPress::backspaceKey) /*|| key.getTextCharacter() == 'd' || key.getTextCharacter() == 'D'*/)
            {
                processor->deleteSelectedAutomationPoints();
                pathAutomationPointsGrabbedWithMouse = false;
            }
            
            // command/ctrl + 'c' to copy all selected sources's path automation points
            if (key.getTextDescription().equalsIgnoreCase("COMMAND + C") || key.getTextDescription().equalsIgnoreCase("CTRL + C"))
            {
                processor->copySelectedPathAutomationPoints();
                // force grab the newly copied and selected points with the mouse
                if (!pathAutomationPointsGrabbedWithMouse) {
                    sources = nullptr;
                    const Locker lock (processor->sources.get(sources));
                    if (sources) {
                        for (int s = 0; s < sources->size(); ++s) {
                            pathAutomationPointsGrabbedWithMouse = true;
                            const std::vector<bool> ptsSelected = (*sources)[s].getSelectedPathAutomationPoints();
                            mouseOverPathAutomationPointIndex[0] = s;
                            mouseOverPathAutomationPointIndex[1] = std::distance(ptsSelected.cbegin(), std::find(ptsSelected.cbegin(), ptsSelected.cend(), true));
                            mouseOverPathAutomationPointIndex[2] = 0;
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
                processor->defineLoopingRegionUsingSelectedPathAutomationPoints();
            }
            
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
                        eyeRad -= 0.2;
                        // don't move view inside head
                        if (eyeRad < 0.2)
                            eyeRad = 0.2;
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if alt is not down, move view ele up (down in number value)
                    {
                        eyeEle -= upDir*4*M_PI/180;
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
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
                } else {
//                    if (!animationOn[1]) {
                        numMoved = processor->moveSelectedPathAutomationPoints(0.0, 0.005*arrowKeySpeedFactor);
                        if (key.getModifiers().isAltDown() && numMoved > 0) {
                            //const bool alignInX = false;
                            autoAlignAutomationPoints(false, true, true);
                        }
                        // adjust the automation grid view in zoom
                        if (numMoved == 0) {
                        MOVE_2D_VIEW_IN:
                            float newAutomationViewWidth = automationViewWidth-automationViewWidth*0.2;
                            // keep view width positive otherwise it could get wonky
                            if (newAutomationViewWidth < 0.001)
                                newAutomationViewWidth = 0.001;
                            float mouse_x = getMouseX();
                            float x_scale = 0.9;
                            float x = mouse_x/x_scale;
                            automationViewOffset += x*(automationViewWidth - newAutomationViewWidth)/2.0;
                            automationViewWidth = newAutomationViewWidth;
                            // adjust the view offset if necessary so that we never stray into negative time
                            if (automationViewOffset < automationViewWidth/2.0) {
                                automationViewOffset = automationViewWidth/2.0;
                            }
                        }
//                    } else {
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
//                    }
                    if (animationOn[1])
                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
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
                        eyeRad += 0.2;
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                    }
                    else // if alt is not down, move view ele up (down in number value)
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
                        updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                        // update up vector if we filp the camera upside down during our rotations
                        eyeUp[0] = 0;
                        eyeUp[1] = upDir;
                        eyeUp[2] = 0;
                    }
                } else {
//                    if (!animationOn[1]) {
                        numMoved = processor->moveSelectedPathAutomationPoints(0.0, -0.005*arrowKeySpeedFactor);
                        if (key.getModifiers().isAltDown() && numMoved > 0) {
                            //const bool alignInX = false;
                            autoAlignAutomationPoints(false, true, true);
                        }
                        if (numMoved == 0) {
                        MOVE_2D_VIEW_OUT:
                            float newAutomationViewWidth = automationViewWidth+automationViewWidth*0.2;
                            float mouse_x = getMouseX();
                            float x_scale = 0.9;
                            float x = mouse_x/x_scale;
                            automationViewOffset += x*(automationViewWidth - newAutomationViewWidth)/2.0;
                            automationViewWidth = newAutomationViewWidth;
                            // adjust the view offset if necessary so that we never stray into negative time
                            if (automationViewOffset < automationViewWidth/2.0) {
                                automationViewOffset = automationViewWidth/2.0;
                            }
                        }
//                    } else {
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
//                    }
                    if (animationOn[1])
                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
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
                    updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                } else {
//                    if (!animationOn[0]) {
                        numMoved = processor->moveSelectedPathAutomationPoints(-0.005*arrowKeySpeedFactor*automationViewWidth, 0.0);
                        if (key.getModifiers().isAltDown() && numMoved > 0) {
                            //const bool alignInX = true;
                            autoAlignAutomationPoints(true, false, true);
                        }
                        if (numMoved == 0) {
                        MOVE_2D_VIEW_LEFT:
                            // adjust the automation grid view in width
                            automationViewOffset -= 0.02*automationViewWidth*arrowKeySpeedFactor;
                            if (automationViewOffset < automationViewWidth/2.0) {
                                automationViewOffset = automationViewWidth/2.0;
                            }
                        }
//                    } else {
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
//                    }
                    if (animationOn[0])
                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
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
                    updateXYZ(eyeRad, eyeAzi, eyeEle, eyePos);
                } else {
//                    if (!animationOn[0]) {
                        numMoved = processor->moveSelectedPathAutomationPoints(0.005*arrowKeySpeedFactor*automationViewWidth, 0.0);
                        if (key.getModifiers().isAltDown() && numMoved > 0) {
                            //const bool alignInX = true;
                            autoAlignAutomationPoints(true, false, true);
                        }
                        if (numMoved == 0) {
                        MOVE_2D_VIEW_RIGHT:
                            // adjust the automation grid view in width
                            automationViewOffset += 0.02*automationViewWidth*arrowKeySpeedFactor;
                        }
//                    } else {
//                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
//                    }
                    if (animationOn[0])
                        arrowKeySpeedFactor = initialArrowKeySpeedFactor; // reset this
                }
            }
            
            break;
    }
    
    // return true to consume KeyPress event, false to pass to other listeners
    return true;
}

void ThreeDAudioProcessorEditor::drawCircle(float x, float y, float r, int segments) const
{
    glBegin( GL_TRIANGLE_FAN );
    glVertex2f(x, y);
    float aspect = ((float)getHeight())/((float)getWidth());
    for( int n = 0; n <= segments; ++n ) {
        float const t = 2*M_PI*(float)n/(float)segments;
        glVertex2f(x + aspect*std::sin(t)*r, y + std::cos(t)*r);
    }
    glEnd();
}

void ThreeDAudioProcessorEditor::showMouseDetails() const
{
    double mouse_x = getMouseXYRelative().getX();//(2.0*getMouseXYRelative().getX())/getWidth() - 1.0;
    double mouse_y = getMouseXYRelative().getY();//-1.0*((2.0*getMouseXYRelative().getY())/getHeight() - 1.0);
    
    char str[20];
    sprintf(str, "MouseZoom: %f", mouseZoomFactor);
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.9, 0);
    sprintf(str, "MouseWheel dX: %f", mouseWheeldX);
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.8, 0);
    sprintf(str, "MouseWheel dY: %f", mouseWheeldY);
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.7, 0);
    //    sprintf(str, "MouseWheel Rev: %d", mouseWheelReversed);
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.6, 0);
    //    sprintf(str, "MouseWheel Smo: %d", mouseWheelSmooth);
    //    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.5, 0);
    sprintf(str, "Mouse X:  %f", mouse_x);
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.4, 0);
    sprintf(str, "Mouse Y:  %f", mouse_y);
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.3, 0);
    sprintf(str, "Mouse Down:  %d", mouseIsDown);
    drawStringBitmap(GLUT_BITMAP_HELVETICA_12, str, 0.6, 0.2, 0);
}

//void ThreeDAudioProcessorEditor::setProcessor(ThreeDAudioProcessor * processorPtr)
//{
//    processor = processorPtr;
//}

void ThreeDAudioProcessorEditor::updateXYZ(float rad, float azi, float ele, float *xyz) const
{
    const float rae[3] = {rad, azi, ele};
    RAEtoXYZ(rae, xyz);
}

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
    // convert sources center xyz point to 2d screen coords
    float xy[2];
    const bool inFrontOfEye = to2D(xyz, xy);
    
    // boundaries of selection region depend on how it is drawn
    float xLow, xHigh, yLow, yHigh;
    if (mouseDragDownX > mouseDragCurrentX) {
        xLow = mouseDragCurrentX;
        xHigh = mouseDragDownX;
    } else {
        xLow = mouseDragDownX;
        xHigh = mouseDragCurrentX;
    }
    if (mouseDragDownY > mouseDragCurrentY) {
        yLow = mouseDragCurrentY;
        yHigh = mouseDragDownY;
    } else {
        yLow = mouseDragDownY;
        yHigh = mouseDragCurrentY;
    }
    
    // if the 2d screen coordinate of that source's center is on screen and in the selection region, mark it as a selected source
    if (inFrontOfEye && std::abs(xy[0]) <= 1.0 && std::abs(xy[1]) <= 1.0
        && xy[0] > xLow && xy[0] < xHigh && xy[1] > yLow && xy[1] < yHigh)
    {
        return true;
    }
    return false;
}

bool ThreeDAudioProcessorEditor::pointInsideSelectRegion2D(const float (&xy)[2]) const
{
    // boundaries of selection region depend on how it is drawn
    float xLow, xHigh, yLow, yHigh;
    if (mouseDragDownX > mouseDragCurrentX) {
        xLow = mouseDragCurrentX;
        xHigh = mouseDragDownX;
    } else {
        xLow = mouseDragDownX;
        xHigh = mouseDragCurrentX;
    }
    if (mouseDragDownY > mouseDragCurrentY) {
        yLow = mouseDragCurrentY;
        yHigh = mouseDragDownY;
    } else {
        yLow = mouseDragDownY;
        yHigh = mouseDragCurrentY;
    }
    
    // if the 2d screen coordinate of that source's center is in the selection region, mark it as a selected source
    if (std::abs(xy[0]) <= 1.0 && std::abs(xy[1]) <= 1.0
        && xy[0] > xLow && xy[0] < xHigh && xy[1] > yLow && xy[1] < yHigh)
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
