//
//  TextUtils.h
//  ThreeDAudio
//
//  Created by Andrew Barker on 3/2/16.
//
//

#ifndef TextUtils_h
#define TextUtils_h

#include "../JuceLibraryCode/JuceHeader.h"
#include "Box.h"
#include "OpenGLWindow.h"
#include "Multi.h"
#include "Animation.h"
#include "ImageEffect.h"
#include "History.h"
#include "PolyPtr.h"
#include "NonlinearInterpolator.h"
#include "PrintToFile.h"
#include "StringFunctions.h"

#include <string>
#include <vector>
#include <array>
#include <map>
#include "OpenGL.h"

// replaces all occurences of the string, find, within str and replaces them with the replacement string, replace.
void findAndReplace(std::string& str, const std::string& find, const std::string& replace) noexcept;
std::string getLeadingWhitespace(const std::string& str) noexcept;
std::string getTrailingWhitespace(const std::string& str) noexcept;
int removeFirstWhitespaceIfLessThan(std::string& str, int spacesCount);
int removeLastWhitespaceIfLessThan(std::string& str, int spacesCount);
void removeFirstWhitespace(std::string& str) noexcept;
void removeLastWhitespace(std::string& str) noexcept;
// parses a string of text into words separated by white space or newlines.  the white spaces are saved into words as well to preserve the length of the white space. newlines are created by '\n' or '\r' characters in the string
std::vector<std::string> parseStringIntoWords(const std::string& text);

void glColour(Colour color);
static Colour currentGLColor = Colours::white;

//struct PointXY { float x = 0, y = 0; };

class MouseDependentAnimation : public Animation
{
public:
    MouseDependentAnimation(float durationInSeconds) noexcept;
    Point<float> mouse;
};

//class Box
//{
//public:
//	Box() noexcept : top(1), bottom(-1), left(-1), right(1) {}
//	Box(float top, float bottom, float left, float right) noexcept
//		: top(top), bottom(bottom), left(left), right(right) {}
//    Box(const Point<float>& pt, float radius) noexcept;
//    bool operator==(const Box& box) const noexcept;
//    bool operator!=(const Box& box) const noexcept;
//    void draw() const;
//    void drawFill() const;
//    float getWidth() const noexcept;
//    float getHeight() const noexcept;
//    std::vector<Point<float>> getBoundaryPoints() const noexcept;
//    bool contains(const Point<float>& pt) const noexcept;
//    bool overlaps(const Box& box) const noexcept;
//    void crop(const Box& box) noexcept;
//    float top = 1, bottom = -1, left = -1, right = 1;
//};
//
//Box combined(const Box& b1, const Box& b2) noexcept;
//Box getScaled(const Box& b, float hScale, float vScale) noexcept;

// conversions from x:[-1,+1] y:[-1,+1] to x:[0,width] y:[0,height] where (0,0) is top left of window
int normalizedToPixels(float mylength, int windowLength) noexcept;
Point<int> normalizedToPixels(const Point<float>& myPoint,
                              int windowWidth, int windowHeight) noexcept;
Rectangle<int> normalizedToPixels(const Box& myRect,
                                int windowWidth, int windowHeight) noexcept;
// ... and vice versa
float pixelsToNormalized(float lengthInPixels, int windowLengthPixels) noexcept;
Point<float> pixelsToNormalized(const Point<int>& ptInPixels,
                                int windowWidth, int windowHeight) noexcept;
float normalizedXYConvert(float fromLengthNormalized,
                          float fromWindowLengthInPixels,
                          float toWindowLengthInPixels) noexcept;
//class OpenGLWindow
//{
//public:
//	OpenGLWindow(OpenGLContext* context) noexcept : context(context) {}
//    void checkResized(int width, int height) noexcept;
//    void saveResized() noexcept;
//    OpenGLContext* context = nullptr;
//    int width = 0, height = 0;
//    float frameRate = 30;
//    bool resized = true;
//};

class Texture
{
public:
    Texture(const Box& boundary) noexcept;
    Texture(const Texture& copyFrom) noexcept;
    Texture& operator=(const Texture& copyFrom) noexcept;
    Box boundary;
    OpenGLTexture texture;
    juce::Rectangle<int> clip, pos;
    bool redrawToTexture;
};

class TextLook
{
public:
    void draw(const std::string& text,
              Texture& texture, OpenGLWindow& window,
              float otherFontSize = -1,
              float otherHorizontalPad = -1,
              std::vector<std::string>* const getLines = nullptr,
              Font* const getFont = nullptr,
              int numSpacesForTab = 4,
              float yScrollOffsetPercent = -1,
              float xScrollOffsetPercent = 0,
              float* const longestLineWidth = nullptr) const;
    
    Font getFont(const std::string& text,
                 const Box& boundary,
                 int windowWidth, int windowHeight,
                 float otherHorizontalPad = -1) const;
//    float getFontSize(const std::string& text,
//                      const Box& boundary,
//                      int windowWidth, int windowHeight,
//                      float otherHorizontalPad = -1) const;
    
    /** for multi-line drawing, returns the words by line and the maximum font size able to fit all the words into a bounding box of a certain size
     */
    std::pair<std::vector<std::string>, float>
        getLinesAndMaxFontSize(const std::vector<std::string>& words,
                               float boxWidth, float boxHeight) const;
    
    /** for multi-line drawing, returns the words by line given a fontSize and a bounding width.  the fontSize provided should already be correctly computed to fit the desired text height-wise.
     */
    std::vector<std::string>
        getLinesWithFontSize(const std::vector<std::string>& words,
                             float boxWidth, float fontSize) const;
    
//    std::vector<std::string> getLines(const std::string& text,
//                                      float boxWidth, float fontSize = -1) const;
    
    //Font getFont(/*float size = -1*/) const;
    Font getFontWithSize(float size) const;
    
    std::pair<std::vector<std::string>, Font>
        getLinesAndFont(const std::string& text, const Box& boundary, int windowWidth, int windowHeight,
                        float otherFontSize = -1, float otherHorizontalPad = -1) const;
    
    bool operator == (const TextLook& other) const noexcept;
    bool operator != (const TextLook& other) const noexcept;
    
    std::string fontName = "Arial";//"Trench";//"Noteworthy";//"Gill Sans";//"Good Times";
    Font::FontStyleFlags fontStyle = Font::FontStyleFlags::plain;
    Justification just = Justification::centred;
    Colour color = Colours::white;
    float fontSize = -1; // negative size means auto-size to texture bounds, positive size means use specified size
    float horizontalPad = 0.95f, // percentage of boundary region that the text will fill
          verticalPad   = 0.95f;
    float kerning = 0; // spacing factor between characters
    float horizontalOffset = 0; // some fonts had slightly funky horizontal positioning, so this was intended to provide some fine tune control over that
    std::vector<ImageEffect> effects; // for fancy stuff
    float relativeScale = 1; // scaling relative to other text(s)
    float linePad = 1; // only used with multi-line text
    bool multiLine = false; // multi-line or single-line mode
    bool wrap = false; // wrap lines when they exceed the boundary's width, only in multi-line mode when there is a specified font size
};

TextLook blend (const TextLook& look1, const TextLook& look2, float alpha);

class TextBox
{
public:
    TextBox() noexcept;
    TextBox(const std::string& text, const Box& boundary, TextLook* look) noexcept;
    
    void draw(OpenGLWindow& window,
              float otherFontSize = -1,
              float otherHorizontalPad = -1,
              std::vector<std::string>* const getLines = nullptr,
              Font* const getFont = nullptr,
              int numSpacesForTab = 4,
              float yScrollOffsetPercent = -1,
              float xScrollOffsetPercent = 0,
              float* const longestLineWidth = nullptr);
    //void draw(OpenGLWindow& window);

    void setText(const std::string& text) noexcept;
    std::string getText() const noexcept;
    
    /** returns the length of the text in pixels as constrained by the text box's boundary and look */
    float getTextLength(int windowWidth, int windowHeight) const;
    /** returns the length of the text in pixels as constrained by the text box's boundary height and look */
    float getTextLengthVerticallyConstrainedOnly(int windowWidth,
                                                 int windowHeight) const;
    
    bool setBoundary(const Box& boundary) noexcept;
    Box getBoundary() const noexcept;
    
    void setLook(TextLook& look) noexcept;
    void setLook(TextLook* look) noexcept;
    const TextLook* getLook() const noexcept;
    //TextLook* getLook() noexcept;
    
    void repaint() noexcept;
    bool needsRedraw() const noexcept;
//    std::pair<std::vector<std::string>, Font>
//        getTextByLineAndFont(int windowWidth, int windowHeight) const;
    Font getFont(int windowWidth, int windowHeight) const;
    
protected:
    std::string text;
    Texture texture;
    TextLook* look;
};

void makeFontsSameSize(std::vector<TextBox>& textBoxes, int windowWidth, int windowHeight);
void centerAndRepositionTextBoxes(TextBox& a, TextBox& b, const Box& boundary,
                                  int windowWidth, int windowHeight,
                                  int numSpacesBetween);

class TextBoxGroup
{
public:
    TextBoxGroup(const std::vector<std::string>& texts,
                 int numRows,
                 const Box& boundary,
                 TextLook* look,
                 bool horizontallyCompress = false) noexcept;
    
    void draw(OpenGLWindow& window);
    
    void setText(int index, const std::string& text) noexcept;
    std::string getText(int index) const noexcept;
    std::vector<std::string> getTexts() const noexcept;
    
    const std::vector<TextBox>& getTextBoxes() const noexcept;
    
    /** Need to provide a TextLook for the whole group in order to properly resize the boxes if the group is horizontally compressed.
     */
    void setBoundary(const Box& boundary, TextLook* look = nullptr, bool setLooks = false) noexcept;
    Box getBoundary() const noexcept;
    
    //void setScale(int index, float scale) noexcept;
    std::vector<Box> getSubBoundaries() const noexcept;
    
    /** Note that setting a look to one with multiline text when horizontal compression is on will not reformat the sub-boundaries to be non-compressed.
     */
    void setLook(TextLook* look, bool resizeFont = true) noexcept;
    void setLook(int index, TextLook* look, bool resizeFont = true) noexcept;
    const TextLook* getLook(int index) const noexcept;
    
    int getNumRows() const noexcept;
    int getNumColumns() const noexcept;
    int getSize() const noexcept;

    //bool getHorizontallyCompress() const noexcept;
    bool canHorizontallyCompress() const noexcept;
    
private:
//    class ScalableTextBox : public TextBox
//    {
//    public:
//        ScalableTextBox(const std::string& text,
//                        const Box& boundary, TextLook* look) noexcept;
//        void draw(OpenGLWindow&, float fontSize, float horizontalPad);
//        float scale;
//    };
//    std::vector<ScalableTextBox> textBoxes;
    std::vector<TextBox> textBoxes;
    int numRows;//, numColumns;
    Box boundary;
    float fontSize;
    bool horizontallyCompress;
};

class GLTextButton
{
public:
    GLTextButton(const std::string& text, const Box& boundary, const TextLook& look = TextLook()) noexcept;
    
    void draw(OpenGLWindow&, const Point<float>& mousePosition);
    bool mouseClicked() noexcept;
    void press() noexcept;
    bool isDown() const noexcept;
    bool isMouseOver() const noexcept;
    
    void setBoundary(const Box&) noexcept;
    Box getBoundary() const noexcept;
    
    void setColor(const Colour&) noexcept;
    Colour getColor() const noexcept;
    
    void setText(const std::string&) noexcept;
    std::string getText() const noexcept;
                        
    void setTextLook(const TextLook&) noexcept;
    TextLook getTextLook() const noexcept;
    
    TextBox& getTextBox() noexcept;
    
    const Animation& getPressAnimation() const noexcept;
    const Animation& getMouseOverAnimation() const noexcept;
    
    bool mouseOverEnabled {true};
    bool showsState {true};
    bool drawBoundary {true};
    
private:
    void changeTextLook() noexcept;
    
    TextBox textBox;
    TextLook textLook;
    Animation pressAnimation {0.25};
    Animation mouseOverAnimation {0.5};
    bool mouseOver {false};
    bool buttonDown {false};
};

//class URLButton : public GLTextButton
//{
//public:
//    
//private:
//    URL url;
//};

class GLTextTabs
{
public:
    enum Orientation { ABOVE, BELOW, LEFT, RIGHT };
    
    GLTextTabs(const std::vector<std::string>& texts, const Box& boundary, Orientation, const TextLook& look = TextLook(), const Box& windowBoundary = {0,0,0,0}) noexcept;
    
    void draw(OpenGLWindow& window, const Point<float>& mousePosition);
    int mouseClicked() noexcept;
    
    void setSelected(int selected, bool enableAnimation = true) noexcept;
    int getSelected() const noexcept;
    
    void setBoundary(const Box& newBoundary) noexcept;
    Box getBoundary() const noexcept;
    
    bool isMouseOver() const noexcept;
    bool mouseOverEnabled {true};
    
    TextLook getSelectedTextLook() const noexcept;
    
private:
    TextBoxGroup textBoxGroup;
    TextLook textLook;
    TextLook selectedTextLook;
    Box boundary; // tabBarBoundary
    Box windowBoundary;
    MouseDependentAnimation selectAnimation {0.25f};
    Point<float> mousePrev;
    Multi<Animation> mouseOverAnimations;
    //std::map<int, Animation> mouseOverAnimations;
    float mouseOverAnimationDuration {0.5f};
    Orientation orientation {ABOVE};
    int mouseOver {-1};
    int selected   {0};
    const float gapFactor = 0.07f;
};

class TextLookAnimation : public Animation
{
public:
    TextLookAnimation(float durationInSeconds,
                      bool startImmediately = false) noexcept
        : Animation(durationInSeconds, startImmediately) {}
    TextLook look;
};

class GLTextRadioButton
{
public:
    GLTextRadioButton(const TextBoxGroup& options,
                      int autoDetect = -1) noexcept;
    
    void draw(OpenGLWindow&, const Point<float>& mousePosition);
    int mouseClicked() noexcept;
    
    void setSelected(int selected, bool enableAnimation = true) noexcept;
    int getSelected() const noexcept;
    
    void setMouseOver(const Point<float>& mousePosition) noexcept;
    int getMouseOver() const noexcept;
    
    void setAutoDetected(int autoDetected) noexcept;
    int getAutoDetected() const noexcept;
    
    bool autoDetectSelected() const noexcept;
    
    void setNormalLook(TextLook*) noexcept;
    void setSelectedLook(TextLook*, TextLook* selectAnimationBeginLook) noexcept;
    void setMouseOverLook(TextLook*) noexcept;
    void setMouseOverAutoDetectLook(TextLook*) noexcept;
    
    void setFontSize(float fontSize) noexcept;
    
    Box getBoundary() const noexcept;
    void setBoundary(const Box& b) noexcept;
    
    const std::vector<TextBox>& getTextBoxes() const noexcept;
    
private:
    TextLook* normalLook;
    TextLook* selectedLook;
    TextLook* selectAnimationBeginLook;
    TextLook* mouseOverLook;
    TextLook* mouseOverAutoDetectLook;
    TextBoxGroup options;
    Multi<TextLookAnimation> mouseOverAnimations;
    const float mouseOverAnimationDuration = 0.25f;
    TextLookAnimation selectAnimation {0.25};
    int selected {0};
    int mouseOver {-1};
    const int autoDetect {-1};
    int autoDetected {0};
};

class GLTitledRadioButton : public GLTextRadioButton
{
public:
    GLTitledRadioButton(const TextBox& title,
                        const GLTextRadioButton& options) noexcept;
    void draw(OpenGLWindow&, const Point<float>& mousePosition);
    TextBox title;
};

//class GLTextEditor : public TextEditor
//{
//public:
//    void draw(OpenGLWindow&);
//private:
//    Texture texture;
//};

void drawThickenedLine(float a, float b, float mid, float delta, float aspect,
                 Colour innerColor, Colour outerColor, bool horizontal);

class TextInputRestrictor
{
public:
    virtual ~TextInputRestrictor() {}
    virtual TextInputRestrictor* clone() const = 0;
    virtual bool insert(std::string& text, int& cursorPosition, const std::string& textToInsert) const = 0;
    virtual void validateInput(std::string& text) const = 0;
    virtual std::string getID() const noexcept { return ""; }
};

class DecimalNumberRestrictor : public TextInputRestrictor
{
public:
    DecimalNumberRestrictor(float min, float max, int maxDecimalPlaces, const std::string& units = "");
    TextInputRestrictor* clone() const override;
    bool insert(std::string& text, int& cursorIndex, const std::string& textToInsert) const noexcept override;
    void validateInput(std::string& text) const noexcept override;
    float getMin() const noexcept;
    float getMax() const noexcept;
    int getMaxNumDigits() const noexcept;
    std::string getUnits() const noexcept;
private:
    int numDigitsLeftOfDecimal(float decimalNumber) const;
    long double min, max;
    int maxDecimalPlaces;
    std::string units;
};

class EditableTextBox : public TextBox
{
public:
    EditableTextBox() noexcept;
    EditableTextBox(const TextBox& textBox, OpenGLWindow* window /*= nullptr*/) noexcept;
    void draw(OpenGLWindow& window, const Point<float>& mousePosition, bool mouseOverEnabled = true);
    bool mouseClicked();
    bool mouseDoubleClicked();
    bool mouseDragged(const Point<float>& mouseDownPosition,
                      const Point<float>& mouseCurrentPosition,
                      bool adjustScroll = false);
    bool mouseWheelMove(float dx, float dy, bool moveOneDimension = true) noexcept;
    bool keyPressed(const std::string& key);
    void setTextInputRestrictor(const std::unique_ptr<TextInputRestrictor> ir);
    void setTextInputRestrictor(TextInputRestrictor* ir);
    const TextInputRestrictor* const getTextInputRestrictor() const noexcept;
    TextInputRestrictor* getTextInputRestrictor() noexcept;
//    void setTextInputRestrictor(std::unique_ptr<TextInputRestrictor> ir);
    const std::string& getText() const noexcept;
    bool getMouseOver() const noexcept;
    bool getSelected() const noexcept;
    bool getMouseDragging() const noexcept;
    void setText(std::string& text) noexcept;
    void setBoundary(const Box& newBoundary) noexcept;
    // these are only regarded in multiline mode, with singleline tab and return release the focus
    int tabKeySpaces = 4; // number of spaces that the tab key adds, <= 0 means that tab releases focus from the EditableTextBox
    bool returnKeyReleasesFocus = false; // if false return key adds a newline character
    
    bool releaseFocusClearsHistory = true;
    //Color color {1.0, 0.0, 0.0};
    
    void gainFocus() noexcept;
    void releaseFocus();
    void finishAnimations() noexcept;
    const Animation& getSelectAnimation() const noexcept;
    
    std::string doubleClickSeparator = " "; // put into an input restrictor instead??
    bool extendToFitSameSizeFont = false;
    enum class HorizontalAnchor { LEFT, CENTER, RIGHT }; // determines how extendToFitSameSizeFont works
    HorizontalAnchor horizontalAnchor = HorizontalAnchor::LEFT;
//private:
    float minimumWidth;
    
    const OpenGLWindow* getWindow() const noexcept;
    
    void setHightColor(Colour highlightColor) noexcept;
    Colour getHighlightColor() const noexcept;
    
protected:
    Colour highlightColor;
    
    bool inputText(const std::string& str);
    void drawCursor();
    struct CursorPosition {
        float x;
        float yLow;
        float yHigh;
        void set(const CursorPosition& newCursorPosition) noexcept
        {
            *this = newCursorPosition;
            needsUpdate = false;
        }
        void setIndex(int newIndex) noexcept
        {
            index = newIndex;
            needsUpdate = true;
        }
        int index = -1;
        int lineIndex = -1;
        int indexWithinLine = -1;
        bool needsUpdate = true;
    };
    CursorPosition cursorPosition;
    CursorPosition getCursorPosition(int otherCursorIndex = -1, float otherHorizontalPad = -1) const;
    CursorPosition getClosestCursorPosition(const Point<float>& pt, float otherHorizontalPad = -1) const;

    std::vector<Box> highlightBoxes;
    bool highlightBoxesNeedUpdate = true;
    void getHighlightBoxes();
    //Box getHighlightBox(int windowWidth, int windowHeight) const;
    //void initAnimations() noexcept;
    
    // nice to have hanging around because these can be comp intensive if they don't really need recomputing
    std::vector<std::string> lines;
    Font font {16.0f}; // specifing an arbitrary height here to avoid an uninitialized font which can lead to crashes (specifically when calling font.getHeight())
    
    //using TextBox::text;
//public:
    OpenGLWindow* window;

    bool selected = false;
    bool mouseOver = false;
    bool mouseDragging = false;
    
    // undo/redo history containing the text, cursor index, and highlight region indecies
    class State {
    public:
        bool operator== (const State& other) { return text == other.text; }
        bool operator!= (const State& other) { return ! (*this == other); }
        std::string text;
        int cursorIndex;
        std::array<int, 2> highlightedText;
    };
    History<State> history;
    
    // used for mouse dragging, this can be expensive to recompute if it don't need to be
    int prevMouseDownCursorIndex = -1;
    Point<float> prevMouseDownPosition;
    
    Point<float> mousePrev;
    std::array<int, 2> highlightedText {0, 0};
    std::array<int, 2> prevHighlightedText = highlightedText;
    
    float animationLength = 0.5f;
    Animation selectAnimation {animationLength};
    Animation mouseOverAnimation {animationLength};
    Animation cursorAnimation {animationLength*2};
    
    PolyPtr<TextInputRestrictor> inputRestrictor;
    //std::unique_ptr<TextInputRestrictor> inputRestrictor;
    
    float scrollOffsetX = 0;
    float scrollOffsetY = 0;
    float longestLineWidth = 0;
    bool verticalScrollingEnabled() const noexcept;
    bool horizontalScrollingEnabled() const noexcept;
    void drawVerticalScrollbar() const;
    void drawHorizontalScrollbar() const;
    float getTotalTextHeight() const noexcept;
    float getMaxScrollOffsetY() const noexcept;
    //std::array<float, 2> getScrollOffsetXBounds() const noexcept;
    float getMaxScrollOffsetX() const noexcept;
    //float getNormalizedScrollOffset(float offset) const noexcept;
    
    // stayin dry
    float getPaddedTop() const noexcept;
    float getPaddedBottom() const noexcept;
    float getLineHeight() const noexcept;
    
    void moveXScroll(float dx) noexcept;
    void moveYScroll(float dy) noexcept;
};

class GLSlider
{
public:
    GLSlider() noexcept;
    void setup(const Box& boundary, const TextBox& title, const EditableTextBox& valueText, Colour color = Colours::red) noexcept;
    GLSlider(const Box& boundary, const TextBox& title, const EditableTextBox& valueText,/* const std::unique_ptr<TextInputRestrictor> ir = std::make_unique<DecimalNumberRestrictor>(0, 1, 4),*/ Colour color = Colours::red) noexcept;
    //GLSlider(const TextBox& name, const Box& boundary, const TextBox& display) noexcept;
    void draw(OpenGLWindow& window, const Point<float>& mousePosition, bool mouseOverEnabled = true);
    bool mouseClicked(const Point<float>& mousePosition) noexcept;
    bool mouseDoubleClicked();
    bool mouseDragged(const Point<float>& mouseDownPosition,
                      const Point<float>& mouseCurrentPosition,
                      bool sliderDragEnabled = true);
    bool mouseWheelMove(float dx, float dy) noexcept;
    bool keyPressed(const std::string& key);
    bool getMouseOver() const noexcept;
    void setTextInputRestrictor(std::unique_ptr<DecimalNumberRestrictor> ir) noexcept;
    const DecimalNumberRestrictor* const getTextInputRestrictor() const noexcept;
    void setInterpolator(const std::unique_ptr<NonlinearInterpolator<float>> interpolator) noexcept;
    void setValue(float value) noexcept;
    float getValue() const noexcept;
    void setTextValue(float val) noexcept;
    void repaint() noexcept;
    void makeTitleAndValueFontsSameSize(int windowWidth, int windowHeight);
    void horizontallyCenterAndRepositionText(const Box& titleBoundary,
                                             const Box& valueBoundary,
                                             int windowWidth, int windowHeight,
                                             int numSpacesBetween);
    TextBox& getTitleTextBox() noexcept;
    EditableTextBox& getValueTextBox() noexcept;
    void setSliderBoundary(const Box& b) noexcept;
    void setColor(Colour color) noexcept;
    Colour getColor() const noexcept;
    
private:
    bool isMouseOver(const Point<float>& mousePosition) noexcept;
    float getValue(const Point<float>& mousePosition) noexcept;
    float getNormalizedValue() const noexcept;
 
    Box boundary;
    TextBox title;
    EditableTextBox valueText;
    Colour color;
    float value;
    PolyPtr<NonlinearInterpolator<float>> interpolator;
    bool mouseOver;
    bool mouseOverEnabled;
    Animation mouseOverAnimation;
    Animation setValueAnimation {0.25f};
    OpenGLWindow* window;
};

//
//class Slider
//{
//public:
//    Slider() {}
//    
//protected:
//    
//};

//class TexturedText
//{
//public:
//    TexturedText(const Box& boundary) noexcept;
//    virtual ~TexturedText();
//    
//    void draw(OpenGLWindow& glWindow);
//    
//    void setFont(const String& fontName) noexcept;
//    String getFont() const noexcept;
//    
//    void setFontStyle(Font::FontStyleFlags fontStyle) noexcept;
//    Font::FontStyleFlags getFontStyle() const noexcept;
//    
//    void setJustification(Justification just) noexcept;
//    Justification getJustification() const noexcept;
//    
//    void setKerning(float kerningFactor) noexcept;
//    float getKerning() const noexcept;
//    
//    void setDrawMultiLine(bool drawMultiLine) noexcept;
//    bool getDrawMultiLine() const noexcept;
//    
//    /** Only used for multi-line text drawing
//     0 < linePad < 1:  extra space between lines
//     1 < linePad:      "negative" space between lines, might overlap depending on font
//     */
//    void setLinePadding(float linePadFactor) noexcept;
//    float getLinePadding() const noexcept;
//    
//    /** Might need to fine tune the horizontal positioning for some fonts.
//     */
//    void setHorizontalOffset(float horizontalOffsetFactor) noexcept;
//    float getHorizontalOffset() const noexcept;
//    
//    void setColor(const Colour& color) noexcept;
//    Colour getColor() const noexcept;
//    
//    void setBoundary(const Box& boundary) noexcept;
//    Box getBoundary() const noexcept;
//    
//    /** Range of (0,1]:  0 being all padding, no space for text
//                         1 being no padding, text goes right up to boundary
//     */
//    void setBoundaryPadding(float horizontalPad,
//                            float verticalPad) noexcept;
//    std::array<float, 2> getBoundaryPadding() const noexcept;
//    
//protected:
//    virtual void drawSingleLine(int windowWidth, int windowHeight) = 0;
//    virtual void drawMultiLine(int windowWidth, int windowHeight) = 0;
//    void imageToTexture(const Image& image,
//                        const Point<int>& textureTopLeft);
//    /** for multi-line drawing, returns the words by line and the maximum font size able to fit all the words into a bounding box of a certain size
//     */
//    std::pair<std::vector<std::string>, float>
//        getLinesAndMaxFontSize(const std::vector<std::string>& words,
//                               float boxWidth, float boxHeight);
//    
//    /** for multi-line drawing, returns the words by line given a fontSize and a bounding width.  the fontSize provided should already be correctly computed to fit the desired text height-wise.
//     */
//    std::vector<std::string>
//        getLinesWithFontSize(const std::vector<std::string>& words,
//                             float boxWidth, float fontSize);
//
//    OpenGLTexture texture;
//    Rectangle<int> clip, pos;
//    Box boundary;
//    bool redrawToTexture {true};
//    
//    String fontName {"Arial"};//"Good Times"};
//    Font::FontStyleFlags fontStyle {Font::FontStyleFlags::plain};
//    Justification just {Justification::centred};
//    Colour color {Colours::white};
//    float horizontalBoundaryPad {.95},
//          verticalBoundaryPad   {.95};
//    float kerningFactor {0};
//    float horizontalOffsetFactor {0};
//    bool multiLine {false};
//    float linePadFactor {1};
//    
//    float fontSize {-1};
//    std::unique_ptr<ImageEffectFilter> effect;
//    bool effectBehind {false};
//    
////    bool glowOn {false};
////    bool glowBehind {true};
////    Colour glowColor {color};
////    float glowRadius {3};
//    
//};
//
//class OpenGLTextBox : public TexturedText
//{
//public:
//    OpenGLTextBox(const String& text, const Box& boundary) noexcept;
//    
//    void setText(const String& text) noexcept;
//    String getText() const noexcept;
//    
//private:
//    void drawSingleLine(int windowWidth, int windowHeight) override;
//    void drawMultiLine(int windowWidth, int windowHeight) override;
//
//    String text;
//};
//
//class OpenGLTextBoxGroup : public TexturedText
//{
//public:
//    OpenGLTextBoxGroup(const StringArray& texts, const Box& boundary,
//                       int numRows, int numColumns) noexcept;
//    
//    void drawBoundaries() const;
//    
//    void setTexts(const StringArray& texts, int numRows, int numColumns) noexcept;
//    StringArray getTexts() const noexcept;
//    
//    int getNumRows() const noexcept;
//    int getNumColumns() const noexcept;
//    
//    class SpecialFontAttributes
//    {
//    public:
//        Font::FontStyleFlags fontStyle {Font::FontStyleFlags::bold};
//        Colour color {Colours::white};
//        float relativeSize {1};
//    };
//    void addSpecialFont(int idNum, const SpecialFontAttributes& specialFont) noexcept;
//    void removeSpecialFont(int idNum) noexcept;
//    void setSpecialFont(int idNum, const SpecialFontAttributes& specialFont) noexcept;
//    const SpecialFontAttributes* getSpecialFont(int idNum) const noexcept;
//    
//private:
//    void drawSingleLine(int windowWidth, int windowHeight) override;
//    void drawMultiLine(int windowWidth, int windowHeight) override;
//    
//    StringArray texts;
//    int numRows, numColumns;
//    Multi<SpecialFontAttributes> specialFonts;
//};
//
//class OpenGLTextButton
//{
//public:
//    OpenGLTextButton(const String& text, const Box& boundary) noexcept;
//
//    void draw(OpenGLWindow& glWindow, const Point<float>& mousePos);
//    bool mouseClicked() noexcept;
//    void press() noexcept;
//    bool isDown() const noexcept;
//    bool isMouseOver() const noexcept;
//    
//    void setBoundary(const Box& boundary) noexcept;
//    Box getBoundary() const noexcept;
//    
//    const Animation& getPressAnimation() const noexcept;
//    const Animation& getMouseOverAnimation() const noexcept;
//    
//public:
//    OpenGLTextBox textBox;
//    Colour color;
//    bool mouseOverEnabled {true};
//    bool showsState {true};
//    
//private:
//    Animation pressAnimation {0.25};
//    Animation mouseOverAnimation {0.5};
//    bool mouseOver {false};
//    bool buttonDown {false};
//};
//
//class OpenGLTextTabs
//{
//public:
//    enum Orientation { ABOVE, BELOW, LEFT, RIGHT };
//    
//    OpenGLTextTabs(const StringArray& texts, const Box& boundary,
//                   Orientation orientation) noexcept;
//    
//    void draw(OpenGLWindow& glWindow, const Point<float>& mousePos);
//    int mouseClicked() noexcept;
//    
//    void setSelected(int selected, bool enableAnimation = true) noexcept;
//    int getSelected() const noexcept;
//    
//    void setBoundary(const Box& boundary) noexcept;
//    Box getBoundary() const noexcept;
//    
//    OpenGLTextBoxGroup textBoxGroup;
//    Colour color;
//    bool mouseOverEnabled {true};
//    
//private:
//    Box boundary;
//    MouseDependentAnimation selectAnimation {0.25};
//    Point<float> mousePrev;
//    Multi<Animation> mouseOverAnimations;
//    float mouseOverAnimationDuration {0.5};
//    Orientation orientation {ABOVE};
//    int mouseOver {-1};
//    int selected   {0};
//    const float gapFactor = 0.07;
//};
//
//class OpenGLTextRadioButton
//{
//public:
//    OpenGLTextRadioButton(const StringArray& texts, const Box& boundary,
//                          int numRows, int numColumns, int autoDetect = -1) noexcept;
//    
//    void draw(OpenGLWindow& glWindow, const Point<float>& mousePos);
//    int mouseClicked() noexcept;
//    
//    void setSelected(int selected) noexcept;
//    int getSelected() const noexcept;
//    
//    void setMouseOver(const Point<float>& mousePos) noexcept;
//    int getMouseOver() const noexcept;
//    
//    void setAutoDetected(int autoDetected) noexcept;
//    int getAutoDetected() const noexcept;
//    
//    void setColor(const Colour& color) noexcept;
//    
//    OpenGLTextBoxGroup options;
//    //Colour color; //{102.0/255.0, 1.0, 102.0/255.0};
//    
//private:
////    Multi<Animation> mouseOverAnimations;
////    Animation selectAnimation {0.25};
//    OpenGLTextBoxGroup::SpecialFontAttributes selectedFont;
//    OpenGLTextBoxGroup::SpecialFontAttributes mouseOverFont;
//    int selected {0};
//    int mouseOver {-1};
//    int autoDetect {-1};
//    int autoDetected {0};
//};
//
//class TitledRadioButton : public OpenGLTextRadioButton
//{
//public:
//    TitledRadioButton(const String& titleText, const Box& titleBoundary,
//                      const StringArray& optionsTexts, const Box& optionsBoundary,
//                      int optionsRows, int optionsColumns, int autoDetect = -1) noexcept;
//    void draw(OpenGLWindow& glWindow, const Point<float>& mousePos);
//    void setColor(const Colour& color) noexcept;
//    OpenGLTextBox title;
//};

///************************** OLD GLUT WAY *******************************/
//enum class Just { TOP_LEFT, CENTER, BOTTOM_RIGHT };
//
//class Color
//{
//public:
//    Color withAlpha(float a) { return {red, green, blue, a}; }
//    float red = 1, green = 1, blue = 1, alpha = 1;
//};
//
//// the interface between this TextUtil library and the drawer of the font
//class BasicFont
//{
//public:
//    //    BasicFont() = default;
//    //    BasicFont(const BasicFont& copyFrom) = default;
//    //    BasicFont(BasicFont&& moveFrom) = default;
//    //    BasicFont& operator=(const BasicFont& copyFrom) = default;
//    //    BasicFont& operator=(BasicFont&& moveFrom) = default;
//    virtual ~BasicFont() {}
//    virtual float length(const std::string& str) const = 0;
//    virtual float topMax() const = 0;
//    virtual float bottomMax() const = 0;
//    float totalHeight() const { return topMax() + bottomMax(); }
//    virtual void drawString(const std::string& str, const Point2D& pos, float aspect, float scale) const = 0;
//};
//
//// default implementation of BasicFont interface
//class GLUTRoman : public BasicFont
//{
//public:
//    float length(const std::string& str) const override;
//    float topMax() const override;
//    float bottomMax() const override;
//    void drawString(const std::string& str, const Point2D& pos, float aspect, float scale) const override;
//};
//
//// interface for drawing size + position formatted text
//class DrawableTextBox
//{
//public:
//    DrawableTextBox() = default;
//    DrawableTextBox(const DrawableTextBox& copyFrom)
//    {
//        boundary = copyFrom.boundary;
//        hJust = copyFrom.hJust;
//        vJust = copyFrom.vJust;
//        aspect = copyFrom.aspect;
//        hPad = copyFrom.hPad;
//        vPad = copyFrom.vPad;
//        font = std::make_unique<GLUTRoman>();
//    }
//    DrawableTextBox(DrawableTextBox&& moveFrom) = default;
//    DrawableTextBox& operator=(const DrawableTextBox& copyFrom) = default;
//    DrawableTextBox& operator=(DrawableTextBox&& moveFrom) = default;
//    virtual ~DrawableTextBox() = default;
//    virtual void drawText(float aspectRatio) = 0;
//    Box boundary {1, -1, -1, 1};
//    Just hJust   {Just::CENTER};
//    Just vJust   {Just::CENTER};
//    float aspect       {1};
//    float hPad = 0.95, vPad = 0.9;
//    std::unique_ptr<BasicFont> font = std::make_unique<GLUTRoman>();
//};
//
//// single line of text fit to box
//class TextBox : public DrawableTextBox
//{
//public:
//    TextBox() = default;
//    TextBox(const std::string& txt, const Box& bounds);
//    TextBox(const TextBox& copyFrom) = default;
//    TextBox(TextBox&& moveFrom) = default;
//    TextBox& operator=(const TextBox& copyFrom) = default;
//    TextBox& operator=(TextBox&& moveFrom) = default;
//    void drawText(float aspectRatio) override;
//    std::string text;
//protected:
//    std::array<float, 3> getBeginPositionAndScale() const noexcept;
//};
//
//// grid of single line text boxes fit to a bounding box
//class TextBoxGroup : public DrawableTextBox
//{
//public:
//    TextBoxGroup() = default;
//    TextBoxGroup(const std::vector<std::string>& txts, const Box& bounds, int numRows, int numColumns);
//    TextBoxGroup(const TextBoxGroup& copyFrom) = default;
//    TextBoxGroup(TextBoxGroup&& moveFrom) = default;
//    TextBoxGroup& operator=(const TextBoxGroup& copyFrom) = default;
//    TextBoxGroup& operator=(TextBoxGroup&& moveFrom) = default;
//    void drawText(float aspectRatio) override;
//    void drawText(float aspectRatio, const std::vector<int>& specialCells, const std::function<void(bool)>& special);
//    void drawBoundaries() const;
//    int getNumRows() const noexcept;
//    int getNumColumns() const noexcept;
//    int getSize() const noexcept;
//protected:
//    std::vector<std::string> texts;
//    int rows = 0, columns = 1;
//    GLuint displayList {0};
//};
//
//// multi-line text fit into single box
//class MultiLineTextBox : public DrawableTextBox
//{
//public:
//    MultiLineTextBox() = default;
//    MultiLineTextBox(const std::string& text, const Box& bounds);
//    MultiLineTextBox(const MultiLineTextBox& copyFrom) = default;
//    MultiLineTextBox(MultiLineTextBox&& moveFrom) = default;
//    MultiLineTextBox& operator=(const MultiLineTextBox& copyFrom) = default;
//    MultiLineTextBox& operator=(MultiLineTextBox&& moveFrom) = default;
//    void loadText(const std::string& text);
//    void drawText(float aspectRatio);
//protected:
//    std::vector<std::string> words;
//};
//
//class TextBoxButton : public TextBox
//{
//public:
//    TextBoxButton();
//    TextBoxButton(const std::string& txt, const Box& bounds);
//    void setClickCallback(const std::function<void()>& clickCallback) noexcept;
//    bool mouseClicked();
//    void press();
//    bool isPressed() const noexcept;
//    bool isMouseOver() const noexcept;
//    void draw(float aspectRatio, const Point2D& mousePos);
//    void draw(float aspectRatio, const Point2D& mousePos, bool mouseOverEnabled);
//    bool showsState = true;
//    std::function<void()> doClickCallback = [](){};
//    int mouseExitAnimationLength = 20;
//    int mouseExitAnimationCount = mouseExitAnimationLength;
//    int mousePressAnimationLength = mouseExitAnimationLength/2;
//    int mousePressAnimationCount = mousePressAnimationLength;
//    Color buttonColor {64.0/255.0, 1.0, 0.0, 1.0};
//    float backgroundTransparency[2] {0.4, 0.2};
//private:
//    bool mouseOverButton = false;
//    bool buttonPressed = false;
//    Point2D mousePrev;
//};
//
//class CountAnimation
//{
//public:
//    CountAnimation(int length) noexcept : len(length) {}
//    float getProgress() noexcept
//    {
//        if (count <= len)
//            return ((float)count++)/len;
//        else
//            return 1.0;
//    }
//    bool isPlaying() const noexcept { return count <= len; }
//    int len = 0, count = 0;
//};
//        
//class MouseDependentCountAnimation : public CountAnimation
//{
//public:
//    MouseDependentCountAnimation(int length, const Point2D& mousePos) noexcept
//        : CountAnimation(length), mouse(mousePos) {};
//    Point2D mouse;
//};
//
//// holds multiple, uniquely-IDed things
//template <class Thing>
//class OldMulti
//{
//    class IDedThing;
//public:
//    Thing* get(std::size_t idNumber)
//    {
//        auto index = std::find_if(std::begin(things), std::end(things),
//                                  [&](const auto& thing){return thing.idNum == idNumber;})
//                     - std::begin(things);
//        if (index < things.size())
//            return &things[index];
//        else
//            return nullptr;
//    }
//    std::vector<IDedThing>& getVector() noexcept
//    {
//        return things;
//    }
//    template <class... Args>
//    void add(std::size_t idNumber, Args&&... args)
//    {
//        remove(idNumber);
//        things.emplace_back(idNumber, std::forward<Args>(args)...);
//    }
//    void remove(std::size_t idNumber)
//    {
//        things.erase(std::remove_if(std::begin(things), std::end(things),
//                                    [&](const auto& thing){return thing.idNum == idNumber;}),
//                     std::end(things));
//    }
//    std::size_t size()
//    {
//        return things.size();
//    }
//private:
//    class IDedThing : public Thing
//    {
//    public:
//        template <class... Args>
//        IDedThing(std::size_t idNumber, Args&&... args) : Thing(std::forward<Args>(args)...), idNum(idNumber) {}
//        std::size_t idNum = 0;
//    };
//    std::vector<IDedThing> things;
//};
//
//class Tabs : public TextBoxGroup
//{
//public:
//    enum Orientation { ABOVE, BELOW, LEFT, RIGHT };
//    Tabs();
//    Tabs(const std::vector<std::string>& tabTxts, const Box& bounds, Orientation orient);
//    void draw(float aspectRatio, const Point2D& mousePos);
//    void draw(float aspectRatio, const Point2D& mousePos, bool mouseOverEnabled);
//    int mouseClicked();
//    void setSelectedTab(int tab, bool enableAnimation) noexcept;
//    int mouseExitAnimationLength = 20;
//    int mouseExitAnimationCount = mouseExitAnimationLength;
//    int mousePressAnimationLength = mouseExitAnimationLength/2;
//    int mousePressAnimationCount = mousePressAnimationLength;
//    Color color {1.0, 1.0, 1.0, 1.0};
//    float backgroundTransparency[2] {0.4, 0.2};
//private:
//    Orientation orientation {ABOVE};
//    const float gapFactor = 0.1;
//    int selectedTab = 0;
//    int mouseOverTab = -1;
//    OldMulti<MouseDependentCountAnimation> mouseExitAnimations;
//    OldMulti<MouseDependentCountAnimation> mouseClickAnimations;
//    Point2D mousePrev;
//};
//
//class RadioOption
//{
//public:
//    RadioOption();
//    RadioOption(const TextBox& title, const TextBoxGroup& options);
//    void draw(float aspectRatio, const Point2D& mousePos);
//    int mouseClicked();
//    void setSelectedOption(int option) noexcept;
//    const Box& getBoundary() const noexcept { return boundary; }
//    int getSelectedOption() const noexcept { return selectedOption; }
//    int getMouseOverOption() const noexcept { return mouseOverOption; }
//    int mouseExitAnimationLength = 20;
//    int mouseExitAnimationCount = mouseExitAnimationLength;
//    int mousePressAnimationLength = mouseExitAnimationLength/2;
//    int mousePressAnimationCount = mousePressAnimationLength;
//    Color color {102.0/255.0, 1.0, 102.0/255.0};
//    float backgroundTransparency[2] {0.4, 0.1};
//protected:
//    void drawAllButOptionsText(float aspectRatio, const Point2D& mousePos);
//    Box boundary;
//    TextBox title;
//    TextBoxGroup options;
//    int selectedOption = 0;
//    int mouseOverOption = -1;
//    OldMulti<MouseDependentCountAnimation> mouseExitAnimations;
//    OldMulti<MouseDependentCountAnimation> mouseClickAnimations;
//    Point2D mousePrev;
//};
//        
//class RadioOptionWithAutoDetect : public RadioOption
//{
//public:
//    RadioOptionWithAutoDetect();
//    RadioOptionWithAutoDetect(const RadioOption& radioOption, int autoDetectOpt, int autoDetectedOpt)
//        : RadioOption(radioOption), autoDetectOption(autoDetectOpt), autoDetectedOption(autoDetectedOpt) {}
//    void draw(float aspectRatio, const Point2D& mousePos);
//    int autoDetectOption = 0;
//    int autoDetectedOption = 0;
//private:
//};
//
//class TextInputRestrictor
//{
//public:
//    //InputRestrictor(std::string& txt) : text(txt) {}
//    virtual ~TextInputRestrictor() {}
//    virtual bool insert(std::string& text, int& cursorPosition, const std::string& textToInsert) = 0;
//    virtual void validateInput(std::string& text) = 0;
//protected:
//    //std::string& text;
//};
//    
//class DecimalNumberRestrictor : public TextInputRestrictor
//{
//public:
//    DecimalNumberRestrictor(float min, float max, int maxDecimalPlaces);
//    bool insert(std::string& text, int& cursorPosition, const std::string& textToInsert) override;
//    void validateInput(std::string& text) override;
//private:
//    int numDigitsLeftOfDecimal(float decimalNumber);
//    //using InputRestrictor::text;
//    long double min = 0, max = 0;
//    int maxDecimalPlaces = 0;
//};
//        
//class EditableTextBox : public TextBox
//{
//public:
//    EditableTextBox();
//    EditableTextBox(const std::string& text, const Box& bounds);
//    void draw(float aspectRatio, const Point2D& mousePos);
//    bool mouseClicked();
//    bool mouseDoubleClicked();
//    bool mouseDragged(const Point2D& mouseDownPosition, const Point2D& mouseCurrentPosition);
//    bool keyPressed(const std::string& key);
//    void setTextInputRestrictor(std::unique_ptr<TextInputRestrictor> ir);
//    const std::string& getText() const noexcept;
//    
//    Color color {1.0, 0.0, 0.0};
//    
//protected:
//    bool inputText(const std::string& str);
//    float getCursorPositionX() const;
//    int getClosestCursorPosition(float x) const;
//    Box getHighlightBox() const;
//    void initAnimations() noexcept;
//    
//    using TextBox::text;
//    bool selected = false;
//    bool mouseOver = false;
//    bool mouseDragging = false;
//    int cursorPosition = 0;
//    Point2D mousePrev;
//    std::array<int, 2> highlightedText {0, 0};
//
//    int animationLength = 10;
//    CountAnimation selectAnimation {animationLength};
//    CountAnimation mouseOverAnimation {animationLength};
//    CountAnimation cursorAnimation {animationLength*3};
//    
//    std::unique_ptr<TextInputRestrictor> inputRestrictor;
//};
//        
//class Slider
//{
//public:
//    Slider() {}
//    
//protected:
//    
//};
//        
//void glColour(const Colour& color);
//void glColor(const Color& color);
//void glColor(const Color& color, float alpha);
//void drawGradient(const Point2D& centerPoint, const Color& centerColor, const std::vector<Point2D>& boundaryPoints, const Color& boundaryColor);
//void removeLastWhitespace(std::string& str);
//        
////static constexpr float stroke_scale = 0.00035;
////static constexpr float topMax1 = 119.05*stroke_scale;
//// GLUT_STROKE_ROMAN text sizing
////static constexpr float topMax = 119.05;
////static constexpr float bottomMax = 33.33;
//        
//// DLCached stuff didn't work out as well as I'd like, plus it doesn't seem that necessary
////// put polymorphic drawing functionality into a non-member func so we can use it with DLCached
////static void drawText(float aspectRatio, DrawableTextBox& dt)
////{
////    dt.drawText(aspectRatio);
////}
////// ... along with this for cached drawing support
////static bool needsRedraw(float aspectRatio, const DrawableTextBox& dt)
////{
////    return aspectRatio == dt.aspect;
////}
////
////template <class Thing, class... DrawArgs>
////class DLCached : public Thing
////{
////public:
////    template <class... ThingArgs>
////    DLCached(const std::function<void(DrawArgs...)>& drawFunc,
////             const std::function<bool(DrawArgs...)>& cacheNeedsResetFunc,
////             ThingArgs&&... args)
////        : Thing(std::forward<ThingArgs>(args)...), doDraw(drawFunc), cacheNeedsReset(cacheNeedsResetFunc)
////    {}
////    DLCached() = default;
////    DLCached(const DLCached& other) = default;
////    DLCached(DLCached&& other) = default;
////    DLCached& operator=(const DLCached& other) = default;
////    DLCached& operator=(DLCached&& other) = default;
//////    // need copy ctor for doing: thing = makeCached<Thing>(...);
//////    DLCached(const DLCached& other)
//////        : doDraw(other.doDraw), cacheNeedsReset(other.cacheNeedsReset), displayList(other.displayList)
//////    {}
////    void resetCache() noexcept { displayList = 0; }
////    void drawCached(DrawArgs&&... args)
////    {
////        if (cacheNeedsReset(args...))
////        //if (cacheNeedsReset(std::forward<DrawArgs>(args)...))
////            resetCache();
////        if (displayList == 0)
////        {
////            glDeleteLists(displayList, 1);
////            displayList = glGenLists(1);
////            glNewList(displayList, GL_COMPILE_AND_EXECUTE);
////            doDraw(args...);
////            //doDraw(std::forward<DrawArgs>(args)...);
////            glEndList();
////        }
////        else
////            glCallList(displayList);
////    }
////private:
////    std::function<void(DrawArgs...)> doDraw;
////    std::function<bool(DrawArgs...)> cacheNeedsReset;
////    GLuint displayList {0};
////};
////
//////using DLCachedTextBox = DLCached<TextBox, float, DrawableTextBox&>;
//////template<class... Args>
//////DLCachedTextBox makeDLCachedTextBox(Args&&... args)
//////{
//////    return {drawText, needsRedraw, std::forward<Args>(args)...};
//////}
//////
//////DLCachedTextBox cachedTB = makeDLCachedTextBox("hello marsha", Box{1,-1,-1,1});
////
////template<class Thing>
////using Cached = DLCached<Thing, float, DrawableTextBox&>;
////
////template<class Thing, class... Args>
////Cached<Thing> makeCached(Args&&... args)
////{
////    return {drawText, needsRedraw, std::forward<Args>(args)...};
////}
////
//////// ex:
//////auto cachedTB = makeCached<TextBox>("hello marsha", Box{1,-1,-1,1});
//////auto cachedTBG = makeCached<TextBoxGroup>(std::vector<std::string>{"a","b"}, Box{1,-1,-1,1}, 2, 1);
//////auto cachedMLTB = makeCached<MultiLineTextBox>("blah blah blah...", Box{1,-1,-1,1});

#endif /* TextUtils_h */
