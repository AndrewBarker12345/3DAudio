/*
 TextUtils.cpp
 
 An assortment of text GUI functionality and other GUI building blocks.

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

#include "TextUtils.h"
#include <iostream>
#include <fstream>
#include <numeric>

void findAndReplace(std::string& str,
                    const std::string& find,
                    const std::string& replace) noexcept
{
    std::string::size_type n;
    while ((n = str.find(find)) != std::string::npos) {
        str.replace(n, find.length(), replace);
    }
}

std::string getLeadingWhitespace(const std::string& str) noexcept
{
    if (str.length() > 0 && str[0] == ' ')
        return str.substr(0, std::distance(std::begin(str),
                                           std::find_if_not(std::begin(str) + 1, std::end(str),
                                                            [](const auto& c){return c == ' ';})));
    return std::string();
}

std::string getTrailingWhitespace(const std::string& str) noexcept
{
    auto copy = str;
    std::reverse(std::begin(copy), std::end(copy));
    return getLeadingWhitespace(copy);
}

int removeFirstWhitespaceIfLessThan(std::string& str,
                                    const int spacesCount)
{
    int numSpacesRemoved;
    const auto firstNonWhitespace = std::find_if(std::begin(str), std::end(str), [](const auto& c){return c != ' ';});
    if ((numSpacesRemoved = std::distance(std::begin(str), firstNonWhitespace)) < spacesCount)
        str.erase(std::begin(str), firstNonWhitespace);
    else
        numSpacesRemoved = 0;
    return numSpacesRemoved;
}

int removeLastWhitespaceIfLessThan(std::string& str,
                                   const int spacesCount)
{
    int numSpacesRemoved;
    std::reverse(std::begin(str), std::end(str));
    numSpacesRemoved = removeFirstWhitespaceIfLessThan(str, spacesCount);
    std::reverse(std::begin(str), std::end(str));
    return numSpacesRemoved;
}

void removeFirstWhitespace(std::string& str) noexcept
{
    str.erase(std::begin(str), std::find_if(std::begin(str), std::end(str), [](const auto& c){return c != ' ';}));
}

void removeLastWhitespace(std::string& str) noexcept
{
    std::reverse(std::begin(str), std::end(str));
    removeFirstWhitespace(str);
    std::reverse(std::begin(str), std::end(str));
}

std::vector<std::string> parseStringIntoWords(const std::string& text)
{
    std::vector<std::string> words;
    int begin = 0;
    for (int i = 0; i < text.size(); ++i) {
        if (text[i] == '\n' || text[i] == '\r') { // newline
            if (i > begin) // add any non-whitespace word before the newline
                words.emplace_back(text.substr(begin, i-begin));
//            else if (words.size() > 0 && words.back().length() > 0 && words.back()[0] == ' ')
//                words.pop_back(); // remove any whitespace before the newline
            words.emplace_back("\n"); // add the newline as a word
            begin = i+1;
        } else if (text[i] == ' ') { // if we found the ending of a non-whitespace word
            if (i > begin) // don't skip loading first word if it is white space
                words.emplace_back(text.substr(begin, i-begin));
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
    return words;
}

void glColour(Colour color)
{
    glColor4f(color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue(), color.getFloatAlpha());
    currentGLColor = color;
}



MouseDependentAnimation::MouseDependentAnimation(const float durationInSeconds) noexcept
    : Animation(durationInSeconds)
{
}



//Box::Box(const Point<float>& pt,
//         const float radius) noexcept
//    : top (pt.y + radius),
//      bottom (pt.y - radius),
//      left (pt.x - radius),
//      right (pt.x + radius)
//{
//}
//
//bool Box::operator==(const Box& box) const noexcept
//{
//    return top == box.top && bottom == box.bottom && left == box.left && right == box.right;
//}
//
//bool Box::operator!=(const Box& box) const noexcept
//{
//    return top != box.top || bottom != box.bottom || left != box.left || right != box.right;
//}
//
//void Box::draw() const
//{
//    glBegin(GL_LINE_LOOP);
//    glVertex2f(left, top);
//    glVertex2f(right, top);
//    glVertex2f(right, bottom);
//    glVertex2f(left, bottom);
//    glEnd();
//}
//
//void Box::drawFill() const
//{
//    glBegin(GL_QUADS);
//    glVertex2f(left, top);
//    glVertex2f(right, top);
//    glVertex2f(right, bottom);
//    glVertex2f(left, bottom);
//    glEnd();
//}
//
//float Box::getWidth() const noexcept
//{
//    return right - left;
//}
//
//float Box::getHeight() const noexcept
//{
//    return top - bottom;
//}
//
//std::vector<Point<float>> Box::getBoundaryPoints() const noexcept
//{
//    return {{left, top}, {right, top}, {right, bottom}, {left, bottom}};
//}
//
//bool Box::contains(const Point<float>& pt) const noexcept
//{
//    return left <= pt.x && pt.x <= right && bottom <= pt.y && pt.y <= top;
//}
//    
//bool Box::overlaps(const Box& box) const noexcept
//{
//    return ! (bottom > box.top || top < box.bottom || left > box.right || right < box.left);
//}
//    
//void Box::crop(const Box& box) noexcept
//{
//    if (top > box.top)
//        top = box.top;
//    if (bottom < box.bottom)
//        bottom = box.bottom;
//    if (left < box.left)
//        left = box.left;
//    if (right > box.right)
//        right = box.right;
//}
//    
//Box combined(const Box& b1, const Box& b2) noexcept
//{
//    return { std::max(b1.top, b2.top), std::min(b1.bottom, b2.bottom),
//             std::min(b1.left, b2.left), std::max(b1.right, b2.right) };
//}
//
//Box getScaled(const Box& b, const float hScale, const float vScale) noexcept
//{
//    const float dWidth  = b.getWidth()  * 0.5 * (1.0 - hScale);
//    const float dHeight = b.getHeight() * 0.5 * (1.0 - vScale);
//    const float top     = b.top    - dHeight;
//    const float bottom  = b.bottom + dHeight;
//    const float left    = b.left   + dWidth;
//    const float right   = b.right  - dWidth;
//    return {top, bottom, left, right};
//}


    
int normalizedToPixels(const float normalizedLength,
                       const int windowLength) noexcept
{
    return std::round(windowLength * normalizedLength * 0.5f);
}
    
Point<int> normalizedToPixels(const Point<float>& normalizedPoint,
                              const int windowWidth,
                              const int windowHeight) noexcept
{
    return {static_cast<int>(std::round(      (normalizedPoint.getX() + 1) * 0.5f   * windowWidth)),
            static_cast<int>(std::round((1 - ((normalizedPoint.getY() + 1) * 0.5f)) * windowHeight))};
}

Rectangle<int> normalizedToPixels(const Box& normalizedRect,
                                  const int windowWidth,
                                  const int windowHeight) noexcept
{
    return {normalizedToPixels({normalizedRect.getLeft(),  normalizedRect.getTop()},    windowWidth, windowHeight),
            normalizedToPixels({normalizedRect.getRight(), normalizedRect.getBottom()}, windowWidth, windowHeight)};
}
    
    
float pixelsToNormalized(const float lengthInPixels,
                         const int windowLengthPixels) noexcept
{
    return 2.0f * lengthInPixels / windowLengthPixels;
}
    
Point<float> pixelsToNormalized(const Point<int>& ptInPixels,
                                const int windowWidth,
                                const int windowHeight) noexcept
{
    return {pixelsToNormalized(ptInPixels.x, windowWidth) - 1,
            1 - pixelsToNormalized(ptInPixels.y, windowHeight)};
}

float normalizedXYConvert(const float fromLengthNormalized,
                          const float fromWindowLengthInPixels,
                          const float toWindowLengthInPixels) noexcept
{
    const auto px = normalizedToPixels(fromLengthNormalized, fromWindowLengthInPixels);
    return pixelsToNormalized(px, toWindowLengthInPixels);
}
    
    
    
//void OpenGLWindow::checkResized(const int w,
//                                const int h) noexcept
//{
//    if (w != width || h != height) {
//        resized = true;
//        width  = w;
//        height = h;
//    }
//}
//
//void OpenGLWindow::saveResized() noexcept
//{
//    resized = false;
//}

    
    
Texture::Texture(const Box& boundary) noexcept
    : boundary(boundary), redrawToTexture(true)
{
}
    
Texture::Texture(const Texture& copyFrom) noexcept
    : boundary(copyFrom.boundary), redrawToTexture(true)
{
}

Texture& Texture::operator=(const Texture& copyFrom) noexcept
{
    if (&copyFrom != this) {
        boundary = copyFrom.boundary;
        redrawToTexture = true;
    }
    return *this;
}
    
    

void TextLook::draw(const std::string& text,
                    Texture& texture,
                    OpenGLWindow& window,
                    const float otherFontSize,
                    const float otherHorizontalPad,
                    std::vector<std::string>* const getLines,
                    Font* const getFont,
                    const int numSpacesForTab,
                    const float yScrollOffsetPercent,
                    const float xScrollOffsetPercent,
                    float* const longestLineWidth) const
{
    if (window.resized || texture.redrawToTexture) {
        const float whichFontSize = relativeScale * (otherFontSize > 0 ? otherFontSize : fontSize);
        const float whichHorizontalPad = otherHorizontalPad > 0 ? otherHorizontalPad : horizontalPad;
        const Box boundary = texture.boundary.scaled(relativeScale, relativeScale);
        const Box paddedBoundary = boundary.scaled(whichHorizontalPad, verticalPad);
        const int imageWidth  = normalizedToPixels(boundary.width(),  window.width);
        const int imageHeight = normalizedToPixels(boundary.height(), window.height);
        const int paddedWidth  = normalizedToPixels(paddedBoundary.width(),  window.width);
        const int paddedHeight = normalizedToPixels(paddedBoundary.height(), window.height);
        Image image (Image::ARGB, std::max(imageWidth, 1), std::max(imageHeight, 1), true); // avoid crash if image area happens to be 0 for some reason
        Graphics g (image);
        g.setColour(color);
//        const auto linesAndFont = getLinesAndFont(text, texture.boundary, window.width, window.height);
//        const auto lines = linesAndFont.first;
//        const auto font = linesAndFont.second;
        if (multiLine) {
            Font font = getFontWithSize(whichFontSize);
            std::vector<std::string> lines;
            if (whichFontSize > 0) {
                font.setHeight(whichFontSize);
                lines = getLinesWithFontSize(parseStringIntoWords(text), wrap ? paddedWidth : -1, whichFontSize);
            } else {
                const auto linesAndMaxFontSize = getLinesAndMaxFontSize(parseStringIntoWords(text), paddedWidth, paddedHeight);
                lines = linesAndMaxFontSize.first;
                font.setHeight(linesAndMaxFontSize.second);
            }
            if (getLines) *getLines = lines;
            if (getFont) *getFont = font;
            for (auto& l : lines) // remove any newline characters from end of lines
                findAndReplace(l, "\n", "");
            // if left justified, remove all leading whitespace less than a tab
            if (just.testFlags(Justification::left))
                for (auto& l : lines)
                    removeFirstWhitespaceIfLessThan(l, numSpacesForTab);
            // if right justified, remove all trailing whitespace less than a tab
            else if (just.testFlags(Justification::right))
                for (auto& l : lines)
                    removeLastWhitespaceIfLessThan(l, numSpacesForTab);
            float longestLineWidthInPixels = 0;
            if (longestLineWidth) {
                for (const auto& l : lines)
                    if (longestLineWidthInPixels < font.getStringWidthFloat(l))
                        longestLineWidthInPixels = font.getStringWidthFloat(l);
                *longestLineWidth = pixelsToNormalized(longestLineWidthInPixels, window.width);
            }
            g.setFont(font);
            // the linePad is only relevant when we have two or more lines
            const float whichLinePad = lines.size() > 1 ? linePad : 1;
            const float totalTextHeightInPixels = std::max((int)lines.size(), 1)
                                                * font.getHeight() / whichLinePad
                                                - (1 - whichLinePad) * (lines.size() > 1 ? font.getHeight() / whichLinePad : 0);
            // vertical justification needs to take into account all lines, default is centered
            float vertJustOffset = 0.5f * (paddedHeight - totalTextHeightInPixels);
            if (just.getOnlyVerticalFlags() == Justification::top)
                vertJustOffset = 0.0f;
            else if (just.getOnlyVerticalFlags() == Justification::bottom)
                vertJustOffset *= 2.0f;
            const float xOffsetBase = 0.5f * (imageWidth - paddedWidth) + horizontalOffset * font.getHeight()
                                      - xScrollOffsetPercent * (longestLineWidthInPixels - paddedWidth);
            // no vertical justification if scrolling is enabled
            float yOffset = 0.5f * (imageHeight - paddedHeight)
                            + (yScrollOffsetPercent >= 0 ? -yScrollOffsetPercent * (totalTextHeightInPixels - paddedHeight) : vertJustOffset);
            for (const auto& line : lines) {
                // if horizontally centered we need to handle leading/trailing whitespace spacing cuz juce's drawText() below centers the text as if there were equal amounts leading and trailing whitespace
                float xOffsetAdjust = 0;
                if (just.testFlags(Justification::horizontallyCentred))
                    xOffsetAdjust = std::round(0.5f * (font.getStringWidthFloat(getLeadingWhitespace(line))
                                                       - font.getStringWidthFloat(getTrailingWhitespace(line))));
                g.drawText(line, std::round(xOffsetBase + xOffsetAdjust), std::round(yOffset), std::max(paddedWidth, (int)longestLineWidthInPixels), font.getHeight(), just, false);
                yOffset += font.getHeight() / whichLinePad;
            }
        } else { // single-line text
            Font font = getFontWithSize(paddedHeight);
            float longestLineWidthInPixels = 0;
            if (whichFontSize > 0) {
                font.setHeight(whichFontSize);
                longestLineWidthInPixels = font.getStringWidthFloat(text);
            } else {
                const float stringWidth = font.getStringWidthFloat(text);
                const float scale = std::min(paddedWidth / stringWidth, 1.0f);
                font.setHeight(paddedHeight * scale);
                longestLineWidthInPixels = stringWidth * scale;
            }
            if (getLines) *getLines = {text};
            if (getFont) *getFont = font;
            if (longestLineWidth) *longestLineWidth = pixelsToNormalized(longestLineWidthInPixels, window.width);
            g.setFont(font);
            float xOffset = std::round(0.5f * (imageWidth - paddedWidth) + horizontalOffset * font.getHeight())
                            - xScrollOffsetPercent * (longestLineWidthInPixels - paddedWidth);
            // if horizontally centered we need to handle leading/trailing whitespace spacing cuz juce's drawText() below centers the text as if there were equal amounts leading and trailing whitespace
            if (just.testFlags(Justification::horizontallyCentred))
                xOffset += std::round(0.5f * (font.getStringWidthFloat(getLeadingWhitespace(text))
                                              - font.getStringWidthFloat(getTrailingWhitespace(text))));
            const int yOffset = std::round(0.5f * (imageHeight - paddedHeight)
                                            + (yScrollOffsetPercent >= 0 ? -yScrollOffsetPercent * (font.getHeight() - paddedHeight) : 0));
            g.drawText(text, std::round(xOffset), yOffset, std::max(paddedWidth, (int)longestLineWidthInPixels), paddedHeight, just, false);
        }
        for (const auto& e : effects)
            e.apply(image, g, 1, 1);
        const Point<int> textureTopLeft = normalizedToPixels({boundary.getLeft(), boundary.getTop()},
                                                            window.width, window.height);
        texture.texture.release(); // make sure to free any previous texture memory
        texture.texture.loadImage(image);
        texture.clip = {textureTopLeft, {textureTopLeft.getX() + image.getWidth(),
                                         textureTopLeft.getY() + image.getHeight()}};
        texture.pos  = {textureTopLeft, {textureTopLeft.getX() + texture.texture.getWidth(),
                                         textureTopLeft.getY() + texture.texture.getHeight()}};
        texture.redrawToTexture = false;
    }
    texture.texture.bind();
    window.context->copyTexture(texture.clip, texture.pos, window.width, window.height, false);
    texture.texture.unbind();
    // need to reactivate alpha color stuff after drawing a texture for some reason
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // this does the trick
    //    glEnable(GL_BLEND);                          // ... but these might be necessary also
    //    glClearColor(0.0,0.0,0.0,0.0);
}

Font TextLook::getFont(const std::string& text,
                       const Box& boundary,
                       const int windowWidth,
                       const int windowHeight,
                       const float otherHorizontalPad) const
{
    if (fontSize < 0) {
        const float whichHorizontalPad = otherHorizontalPad > 0 ? otherHorizontalPad : horizontalPad;
        const float w = normalizedToPixels(boundary.width()  * relativeScale * whichHorizontalPad, windowWidth);
        const float h = normalizedToPixels(boundary.height() * relativeScale * verticalPad, windowHeight);
        const Font font = getFontWithSize(h);
        if (boundary.width() == std::numeric_limits<float>::infinity())
            return font;
        else if (multiLine)
            return getLinesAndMaxFontSize(parseStringIntoWords(text), w, h).second;
        else
            return getFontWithSize(h * std::min(w / font.getStringWidthFloat(text), 1.0f));
    }
    return getFontWithSize(fontSize * relativeScale);
}
    
std::pair<std::vector<std::string>, float>
    TextLook::getLinesAndMaxFontSize(const std::vector<std::string>& words,
                                     const float boxWidth,
                                     const float boxHeight) const
{
    std::vector<std::string> lines;
    const Font font = getFontWithSize(boxHeight);
    // compute a first order approx of the maximum text size
    float totalLength = 0;
    std::vector<float> wordLengths (words.size());
    for (int i = 0; i < words.size(); ++i) {
        wordLengths[i] = font.getStringWidthFloat(words[i]);
        totalLength += wordLengths[i];
    }
    if (totalLength == 0) // if the text passed in is empty, we are done
        return {lines, boxHeight};
    const float initArea = totalLength / linePad;
    float textScale = std::min(1.0f, std::sqrt(boxWidth / initArea));
TRY_AGAIN: // scale back further still if the text cannot be contained in the specified box height-wise
    lines.clear();
    const int numLines = linePad / textScale;
    float textLength = 0;
    int lineCount = 1;
    for (int i = 0; i < words.size(); ++i) {
        textLength += /*font.getStringWidthFloat(words[i])*/ wordLengths[i] * textScale; // WOW!!! huge performance boost!
        if (textLength > boxWidth || words[i] == "\n") { // if we run over the bounday's width or hit a newline character, start a new line
            if (words[i] == "\n") { // add newline word to end of line
                while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
                    lines.emplace_back("");
                lines[lineCount-1] += words[i];
            }
            if (++lineCount > numLines) { // micro-adjust scale until text fits height-wise
                textScale *= 0.9999f;
                lines.clear();
                goto TRY_AGAIN;
            }
            textLength = 0;
            if (words[i] != "\n") // resume with last non-newline word that didn't fit onto the previous line
                --i;
        } else {
            while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
                lines.emplace_back("");
            lines[lineCount-1] += words[i];
        }
    }
    if (lines.size() > 0 && lines.back().size() > 0 && lines.back().back() == '\n') // if the last line's last character is a newline, tag on an empty line
        lines.emplace_back("");
    return {lines, boxHeight * textScale};
}

std::vector<std::string> TextLook::getLinesWithFontSize(const std::vector<std::string>& words,
                                                        const float boxWidth,
                                                        const float fontSize) const
{
    std::vector<std::string> lines;
    const Font font = getFontWithSize(fontSize);
    float textLength = 0;
    int lineCount = 1;
    int numWordsOnLine = 0;
    for (int i = 0; i < words.size(); ++i) {
        textLength += font.getStringWidthFloat(words[i]);
        ++ numWordsOnLine;
        if ((boxWidth > 0 && textLength > boxWidth) || words[i] == "\n") { // negative boxWidth implies there is no wrapping of text by word width wise
            const bool addWord = words[i] == "\n" || numWordsOnLine == 1; // add newline word to end of line, or if the only word on the line is too big to fit on the line add it to the line anyways to avoid an infinite loop
            if (addWord) {
                while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
                    lines.emplace_back("");
                lines[lineCount-1] += words[i];
                if (words[i] != "\n" && numWordsOnLine == 1 && i+1 < words.size() && words[i+1] == "\n") // if a newline follows the big (non-newline) word, add it to the end of the line also
                    lines[lineCount-1] += words[++i];
            }
            ++ lineCount;
            textLength = 0;
            if (! addWord) // resume with last non-newline word that didn't fit onto the previous line (if we didn't add it above)
                -- i;
            numWordsOnLine = 0;
        } else {
            while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
                lines.emplace_back("");
            lines[lineCount-1] += words[i];
        }
    }
    if (lines.size() > 0 && lines.back().size() > 0 && lines.back().back() == '\n') // if the last line's last character is a newline, tag on an empty line
        lines.emplace_back("");
    return lines;
}
    
//std::vector<std::string> TextLook::getLines(const std::string& text,
//                                            const float boxWidth,
//                                            const float fontSize) const
//{
//    std::vector<std::string> lines;
//    const auto words = parseStringIntoWords(text);
//    const auto font = getFont(fontSize);
//    float textLength = 0;
//    int lineCount = 1;
//    for (int i = 0; i < words.size(); ++i) {
//        textLength += font.getStringWidthFloat(words[i]);
//        if (textLength > boxWidth || words[i] == "\n") {
////            if (lines.size() > lineCount-1) // if we jump to a new line, the previous line's last whitespace is discarded
////                removeLastWhitespace(lines[lineCount-1]);
//            ++lineCount;
//            textLength = 0;
//            if (words[i].length() > 0 && words[i][0] != ' ' && words[i] != "\n")
//                --i;
//        } else {
//            while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
//                lines.emplace_back("");
//            lines[lineCount-1] += words[i];
//        }
//    }
//    return lines;
//}
    
//Font TextLook::getFont(/*const float size*/) const
//{
//    Font font;
//    if (fontSize < 0) {
//        if (multiLine) {
//            const auto linesAndMaxFontSize = getLinesAndMaxFontSize(parseStringIntoWords(text),
//                                                                    paddedWidth,
//                                                                    paddedHeight);
//            lines = linesAndMaxFontSize.first;
//            font = getFont(linesAndMaxFontSize.second);
//        } else {
//            
//        }
//    } else {
//        font = getFontWithSize(fontSize);
//    }
//    return font;
//}

Font TextLook::getFontWithSize(const float size) const
{
    Font font = Font(fontName, size/* * relativeScale*/, fontStyle);
    font.setExtraKerningFactor(kerning);
    return font;
}

std::pair<std::vector<std::string>, Font> TextLook::getLinesAndFont(const std::string& text,
                                                                    const Box& boundary,
                                                                    const int windowWidth,
                                                                    const int windowHeight,
                                                                    const float otherFontSize,
                                                                    const float otherHorizontalPad) const
{
    std::vector<std::string> lines;
    Font font;
    const float whichFontSize = relativeScale * (otherFontSize > 0 ? otherFontSize : fontSize);
    const float whichHorizontalPad = otherHorizontalPad > 0 ? otherHorizontalPad : horizontalPad;
    const int w = normalizedToPixels(boundary.width()  * relativeScale * whichHorizontalPad,  windowWidth);
    const int h = normalizedToPixels(boundary.height() * relativeScale * verticalPad,        windowHeight);
    if (multiLine) {
        if (whichFontSize < 0) {
            const auto linesAndMaxFontSize = getLinesAndMaxFontSize(parseStringIntoWords(text), w, h);
            lines = linesAndMaxFontSize.first;
            font = getFontWithSize(linesAndMaxFontSize.second);
        } else {
            lines = getLinesWithFontSize(parseStringIntoWords(text), wrap ? w : -1, whichFontSize);
            font = getFontWithSize(whichFontSize);
        }
    } else {
        // remove any newline characters if not in multiline mode
        std::string textNoNewlines = text;
        findAndReplace(textNoNewlines, "\n", "");
        lines.emplace_back(textNoNewlines);
        if (whichFontSize < 0) {
            font = getFontWithSize(h);
            const float size = h * std::min(w / font.getStringWidthFloat(textNoNewlines), 1.0f);
            font = getFontWithSize(size);
        } else
            font = getFontWithSize(whichFontSize);
    }
    return {lines, font};
}

bool TextLook::operator == (const TextLook& other) const noexcept
{
    return fontName == other.fontName &&
           fontStyle == other.fontStyle &&
           just == other.just &&
           color == other.color &&
           fontSize == other.fontSize &&
           horizontalPad == other.horizontalPad &&
           verticalPad == other.verticalPad &&
           kerning == other.kerning &&
           horizontalOffset == other.horizontalOffset &&
           effects == other.effects &&
           relativeScale == other.relativeScale &&
           linePad == other.linePad &&
           multiLine == other.multiLine &&
           wrap == other.wrap;
}

bool TextLook::operator != (const TextLook& other) const noexcept
{
    return fontName != other.fontName ||
           fontStyle != other.fontStyle ||
           just != other.just ||
           color != other.color ||
           fontSize != other.fontSize ||
           horizontalPad != other.horizontalPad ||
           verticalPad != other.verticalPad ||
           kerning != other.kerning ||
           horizontalOffset != other.horizontalOffset ||
           effects != other.effects ||
           relativeScale != other.relativeScale ||
           linePad != other.linePad ||
           multiLine != other.multiLine ||
           wrap != other.wrap;
}



TextLook blend(const TextLook& look1,
               const TextLook& look2,
               const float alpha)
{
    // copy over everything from the ending look
    TextLook blendedLook = look2;
    // interpolate the values that make sense to interpolate
    blendedLook.color = look1.color.interpolatedWith(look2.color, alpha);
    if (look2.fontSize >= 0 && look1.fontSize >= 0)
        blendedLook.fontSize = look1.fontSize + alpha * (look2.fontSize - look1.fontSize);
    blendedLook.horizontalPad = look1.horizontalPad + alpha * (look2.horizontalPad - look1.horizontalPad);
    blendedLook.verticalPad = look1.verticalPad + alpha * (look2.verticalPad - look1.verticalPad);
    blendedLook.kerning = look1.kerning + alpha * (look2.kerning - look1.kerning);
    blendedLook.horizontalOffset = look1.horizontalOffset + alpha * (look2.horizontalOffset - look1.horizontalOffset);
    blendedLook.effects.clear();
    blendedLook.effects = blend(look1.effects, look2.effects, alpha);
//    if (look1.effectsBehindText != look2.effectsBehindText) {
//        if (look2.effectsBehindText)
//            blendedLook.effectsBehindText = look1.effectsBehindText;
//        else
//    }
    blendedLook.linePad = look1.linePad + alpha * (look2.linePad - look1.linePad);
    blendedLook.relativeScale = look1.relativeScale + alpha * (look2.relativeScale - look1.relativeScale);
    return blendedLook;
}
    
    

TextBox::TextBox() noexcept
    : text(""), texture(Box()), look(nullptr)
{
}

TextBox::TextBox(const std::string& text,
                 const Box& boundary,
                 TextLook* look) noexcept
    : text(text), texture(boundary), look(look)
{
}

void TextBox::draw(OpenGLWindow& window,
                   const float otherFontSize,
                   const float otherHorizontalPad,
                   std::vector<std::string>* const getLines,
                   Font* const getFont,
                   int numSpacesForTab,
                   const float yScrollOffset,
                   const float xScrollOffset,
                   float* const longestLineWidth)
{
    look->draw(text, texture, window, otherFontSize, otherHorizontalPad, getLines, getFont, numSpacesForTab, yScrollOffset, xScrollOffset, longestLineWidth);
}

//void TextBox::draw(OpenGLWindow& window)
//{
//    //if (look != nullptr)
//    look->draw(text, texture, window);
//}

void TextBox::setText(const std::string& newText) noexcept
{
    if (text != newText) {
        text = newText;
        texture.redrawToTexture = true;
    }
}

std::string TextBox::getText() const noexcept
{
    return text;
}

float TextBox::getTextLengthVerticallyConstrainedOnly(const int windowWidth,
                                                      const int windowHeight) const
{
    auto boundary = getBoundary();
    boundary.setRight(std::numeric_limits<float>::infinity());
    return getLook()->getFont(getText(), boundary, windowWidth, windowHeight).getStringWidthFloat(getText());
}

float TextBox::getTextLength(const int windowWidth,
                             const int windowHeight) const
{
    return getFont(windowWidth, windowHeight).getStringWidthFloat(getText());
}

bool TextBox::setBoundary(const Box& newBoundary) noexcept
{
    if (texture.boundary != newBoundary) {
    // would be nice to not have to do all the drawing work again if only the position of boundary changes...
//        if (texture.boundary.width() != newBoundary.width() ||
//            texture.boundary.height() != newBoundary.height())
        texture.redrawToTexture = true;
        texture.boundary = newBoundary;
        return true;
    }
    return false;
}

Box TextBox::getBoundary() const noexcept
{
    //const auto scale = look->relativeScale;
    return texture.boundary;//getScaled(texture.boundary, scale, scale);
}

void TextBox::setLook(TextLook& newLook) noexcept
{
    if (look == nullptr) {
        look = &newLook;
        texture.redrawToTexture = true;
    } else {
        if (*look != newLook) {
            *look = newLook;
            texture.redrawToTexture = true;
        }
    }
}

void TextBox::setLook(TextLook* newLook) noexcept
{
    //if (look != newLook || *look != *newLook) {
    look = newLook;
    texture.redrawToTexture = true;
    //}
}

const TextLook* TextBox::getLook() const noexcept
{
    return look;
}
    
void TextBox::repaint() noexcept
{
    texture.redrawToTexture = true;
}

bool TextBox::needsRedraw() const noexcept
{
    return texture.redrawToTexture;
}
    
//std::pair<std::vector<std::string>, Font>
//    TextBox::getTextByLineAndFont(const int windowWidth,
//                                  const int windowHeight) const
//{
//    std::vector<std::string> lines;
//    Font font;
//    if (look->multiLine) {
//        if (look->fontSize < 0) {
//            const Box paddedBoundary = getScaled(texture.boundary, look->horizontalPad, look->verticalPad);
//            const int paddedWidth  = myLengthToStandard(paddedBoundary.getWidth(),  windowWidth);
//            const int paddedHeight = myLengthToStandard(paddedBoundary.getHeight(), windowHeight);
//            
//            const auto linesAndMaxFontSize = look->getLinesAndMaxFontSize(parseStringIntoWords(text),
//                                                                          paddedWidth, paddedHeight);
//            
//            lines = linesAndMaxFontSize.first;
//            font = getFont(linesAndMaxFontSize.second * relativeScale);//Font(fontName, linesAndMaxFontSize.second * relativeScale, fontStyle);
//        } else {
//            lines = getLinesWithFontSize(parseStringIntoWords(text), boundary.getWidth(), fontSize);
//            font = Font(fontName, fontSize * relativeScale, fontStyle);
//        }
//    } else {
//        lines.emplace_back(text);
//        if (fontSize < 0) {
//            const float w = myLengthToStandard(boundary.getWidth() * horizontalPad, windowWidth);
//            const float h = myLengthToStandard(boundary.getHeight() * verticalPad, windowHeight);
//            font = Font(fontName, h, fontStyle);
//            font.setExtraKerningFactor(kerning);
//            const float size = h * std::min(w / font.getStringWidthFloat(text), 1.0f) * relativeScale;
//            font.setSizeAndStyle(size, fontStyle, 1, kerning);
//        } else
//            font = Font(fontName, fontSize * relativeScale, fontStyle);
//    }
//    return {lines, font};
//}
    
Font TextBox::getFont(const int windowWidth,
                      const int windowHeight) const
{
    return look->getFont(text, texture.boundary, windowWidth, windowHeight);
}
    
   
    
void makeFontsSameSize(std::vector<TextBox>& textBoxes,
                       const int windowWidth,
                       const int windowHeight)
{
    // set all looks to autosize mode
    for (auto& x : textBoxes) {
        auto look = *x.getLook();
        look.fontSize = -1;
        x.setLook(look);
    }
    // find min font size
    auto min = -1.0f;
    for (const auto& x : textBoxes) {
        const auto size = x.getFont(windowWidth, windowHeight).getHeight();
        if (min < 0)
            min = size;
        else if (min > size)
            min = size;
    }
    // ... and set all to have the min size
    for (auto& x : textBoxes) {
        auto look = *x.getLook();
        look.fontSize = min;
        x.setLook(look);
    }
}
    
void centerAndRepositionTextBoxes(TextBox& a,
                                  TextBox& b,
                                  const Box& boundary,
                                  const int windowWidth,
                                  const int windowHeight,
                                  const int numSpacesBetween)
{
    std::vector<TextBox> boxes {a, b};
    makeFontsSameSize(boxes, windowWidth, windowHeight);
    const auto font = a.getFont(windowWidth, windowHeight); // assuming a and b have same font
    //const auto fontB = b.getFont(windowWidth, windowHeight);
    const auto widthA = pixelsToNormalized(font.getStringWidthFloat(a.getText()) / a.getLook()->horizontalPad, windowWidth);
    const auto widthB = pixelsToNormalized(font.getStringWidthFloat(b.getText()) / b.getLook()->horizontalPad, windowWidth);
    std::string spaces;
    for (int i = 0; i < numSpacesBetween; ++i)
        spaces += " ";
    const auto widthSpaces = pixelsToNormalized(font.getStringWidthFloat(spaces), windowWidth);
//    //const auto totalStringWidth = widthA + widthB + widthSpaces;
//    const auto totalStringWidth = pixelsToNormalized(widthA + widthB + widthSpaces, windowWidth);
    auto left = boundary.getLeft() + 0.5f * (boundary.width() - widthA - widthB - widthSpaces);
    auto right = left + widthA;
    a.setBoundary({boundary.getTop(), boundary.getBottom(), left, right});
    left = right + widthSpaces;
    right = left + widthB;
    b.setBoundary({boundary.getTop(), boundary.getBottom(), left, right});
}
    
    
    
TextBoxGroup::TextBoxGroup(const std::vector<std::string>& texts,
                           const int numRows,
                           const Box& boundary,
                           TextLook* look,
                           const bool horizontallyCompress) noexcept
    : numRows(numRows), boundary(boundary), fontSize(-1),
      horizontallyCompress(horizontallyCompress)
{
    textBoxes.resize(texts.size(), {"", {1, -1, -1, 1}, nullptr});
    const int numColumns = getNumColumns(); // need textBoxes to be sized before finding numColumns
    //textBoxes.reserve(texts.size());
    if (horizontallyCompress && numRows == 1 && !look->multiLine) {
        std::vector<float> lengths (numColumns);
        float totalLength = 0;
        Font font (look->fontName, 12, look->fontStyle);
        font.setExtraKerningFactor(look->kerning);
        for (int c = 0; c < numColumns; ++c) {
            lengths[c] = font.getStringWidthFloat(texts[c]);
            totalLength += lengths[c];
        }
        Box b {boundary.getTop(), boundary.getBottom(), boundary.getLeft(), boundary.getLeft()};
        for (int c = 0; c < numColumns; ++c) {
            b.setRight(b.getRight() + lengths[c] / totalLength * boundary.width());
            //textBoxes.emplace_back(texts[c], b, look);
            textBoxes[c] = {texts[c], b, look};
            b.setLeft(b.getRight());
        }
    } else {
        const float dW = boundary.width() / numColumns;
        const float dH = boundary.height() / numRows;
        Box b {boundary.getTop(), boundary.getTop() - dH, boundary.getLeft(), boundary.getLeft() + dW};
        for (int r = 0; r < numRows; ++r) {
            for (int c = 0; c < numColumns; ++c) {
                //textBoxes.emplace_back(texts[r*numColumns+c], b, look);
                textBoxes[r*numColumns+c] = {texts[r*numColumns+c], b, look};
                b.setLeft(b.getLeft() + dW);
                b.setRight(b.getRight() + dW);
            }
            b.setLeft(boundary.getLeft());
            b.setRight(b.getLeft() + dW);
            b.setTop(b.getTop() - dH);
            b.setBottom(b.getBottom() - dH);
        }
    }
}
    
//TextBoxGroup::ScalableTextBox::ScalableTextBox(const std::string& text,
//                                               const Box& boundary,
//                                               TextLook* look) noexcept
//    : TextBox(text, boundary, look), scale(1)
//{
//}
//
//void TextBoxGroup::ScalableTextBox::draw(OpenGLWindow& window,
//                                         const float fontSize,
//                                         const float horizontalPad)
//{
//    //if (look != nullptr) {
//    if (scale == 1)
//        look->draw(text, texture, window, fontSize, horizontalPad);
//    else
//        look->draw(text, texture, window, -1, horizontalPad);
//    //}
//}
    
void TextBoxGroup::draw(OpenGLWindow& window)
{
//    float horizontalPad = -1;
//    const bool compress = canHorizontallyCompress();
//    const auto calcNHPW = [&](){
//        if (compress) {
//            float tot = 0;
//            for (const auto& tb : textBoxes)
//                tot += tb.getLook()->horizontalPad;
//            return boundary.width() * (1 - tot / textBoxes.size());
//        } else
//            return 0.0f;
//    };
//    const float netHorizontalPadWidth = calcNHPW();
//    const int numColumns = getNumColumns();
    if (window.resized || fontSize < 0) {
        float size, minSize;
        for (int i = 0; i < textBoxes.size(); ++i) {
//            if (compress)
//                horizontalPad = 1 - netHorizontalPadWidth / (textBoxes[i].getBoundary().width() * numColumns);
            size = textBoxes[i].getLook()->getFont(textBoxes[i].getText(), textBoxes[i].getBoundary(),
                                                   window.width, window.height/*, horizontalPad*/).getHeight();
            if (size < minSize || i == 0)
                minSize = size;
        }
        fontSize = minSize;
    }
    for (auto& textBox : textBoxes) {
//        if (compress)
//            horizontalPad = 1 - netHorizontalPadWidth / (textBox.getBoundary().width() * numColumns);
        textBox.draw(window, fontSize/*, horizontalPad*/);
//        glColor3f(1, 1, 1);
//        textBox.getBoundary().drawOutline();
    }
}

bool TextBoxGroup::canHorizontallyCompress() const noexcept
{
    bool anyMultiLine = false;
    for (const auto& textBox : textBoxes)
        anyMultiLine |= textBox.getLook()->multiLine;
    return horizontallyCompress && numRows == 1 && !anyMultiLine;
}
    
void TextBoxGroup::setText(const int index,
                           const std::string& text) noexcept
{
    textBoxes[index].setText(text);
    fontSize = -1;
}

std::string TextBoxGroup::getText(const int index) const noexcept
{
    return textBoxes[index].getText();
}

std::vector<std::string> TextBoxGroup::getTexts() const noexcept
{
    std::vector<std::string> texts (textBoxes.size());
    for (int i = 0; i < textBoxes.size(); ++i)
        texts[i] = textBoxes[i].getText();
    return texts;
}

const std::vector<TextBox>& TextBoxGroup::getTextBoxes() const noexcept
{
    return textBoxes;
}

void TextBoxGroup::setBoundary(const Box& newBoundary,
                               TextLook* look,
                               const bool setLooks) noexcept
{
    boundary = newBoundary;
    if (setLooks && look)
        for (auto& textBox : textBoxes)
            textBox.setLook(look);
    const int numColumns = getNumColumns();
    if (canHorizontallyCompress() && look) {
        const auto texts = getTexts();
        std::vector<float> lengths (numColumns);
        float totalLength = 0;
        const auto font = look->getFontWithSize(12);
        for (int c = 0; c < numColumns; ++c) {
            lengths[c] = font.getStringWidthFloat(texts[c]);
            totalLength += lengths[c];
        }
        Box b {boundary.getTop(), boundary.getBottom(), boundary.getLeft(), boundary.getLeft()};
        for (int c = 0; c < numColumns; ++c) {
            b.setRight(b.getRight() + lengths[c] / totalLength * boundary.width());
            textBoxes[c].setBoundary(b);
            b.setLeft(b.getRight());
        }
    } else {
        const float dW = boundary.width() / numColumns;
        const float dH = boundary.height() / numRows;
        Box b {boundary.getTop(),  boundary.getTop() - dH, boundary.getLeft(), boundary.getLeft() + dW};
        for (int r = 0; r < numRows; ++r) {
            for (int c = 0; c < numColumns; ++c) {
                textBoxes[r*numColumns+c].setBoundary(b);
                b.setLeft(b.getLeft() + dW);
                b.setRight(b.getRight() + dW);
            }
            b.setLeft(boundary.getLeft());
            b.setRight(b.getLeft() + dW);
            b.setTop(b.getTop() - dH);
            b.setBottom(b.getBottom() - dH);
        }
    }
}
    
Box TextBoxGroup::getBoundary() const noexcept
{
    return boundary;
}

//void TextBoxGroup::setScale(const int index,
//                            const float scale) noexcept
//{
//    textBoxes[index].setBoundary(getScaled(textBoxes[index].getBoundary(), scale, scale));
//    textBoxes[index].scale = scale;
//}
    
std::vector<Box> TextBoxGroup::getSubBoundaries() const noexcept
{
    std::vector<Box> subBoundaries (textBoxes.size());
    for (int i = 0; i < textBoxes.size(); ++i)
        subBoundaries[i] = textBoxes[i].getBoundary();
//        subBoundaries[i] = getScaled(textBoxes[i].getBoundary(),
//                                     1.0/textBoxes[i].getLook()->relativeScale,
//                                     1.0/textBoxes[i].getLook()->relativeScale);
    return subBoundaries;
}

void TextBoxGroup::setLook(TextLook* newLook,
                           const bool resizeFont) noexcept
{
    for (auto& textBox : textBoxes)
        textBox.setLook(newLook);
    if (resizeFont)
        fontSize = -1;
}
    
void TextBoxGroup::setLook(const int index,
                           TextLook* look,
                           const bool resizeFont) noexcept
{
    textBoxes[index].setLook(look);
    if (resizeFont)
        fontSize = -1;
}
    
const TextLook* TextBoxGroup::getLook(const int index) const noexcept
{
    return textBoxes[index].getLook();
}

int TextBoxGroup::getNumRows() const noexcept
{
    return numRows;
}

int TextBoxGroup::getNumColumns() const noexcept
{
    if (numRows > 0)
        return std::ceil( ((float)textBoxes.size()) / numRows );
    else
        return 0;
}

int TextBoxGroup::getSize() const noexcept
{
    return numRows * getNumColumns();
}
    
//bool TextBoxGroup::getHorizontallyCompress() const noexcept
//{
//    return horizontallyCompress;
//}
    
               

GLTextButton::GLTextButton(const std::string& text,
                           const Box& boundary,
                           const TextLook& look) noexcept
    : textBox(text, boundary, &textLook), textLook(look)
{
}

void GLTextButton::draw(OpenGLWindow& window, const Point<float>& mousePosition)
{
    
    const auto boundary = textBox.getBoundary();
    
    // draw dark shading so any background objects are visible, but darkened
    glColor4f(0.0, 0.0, 0.0, 0.85);
    boundary.drawFill();
    
    if (boundary.contains(mousePosition) && mouseOverEnabled) {
        if (!mouseOver)
            mouseOverAnimation.restart();
        mouseOver = true;
    } else {
        if (mouseOver)
            mouseOverAnimation.restart();
        mouseOver = false;
    }
    
    const auto color = textBox.getLook()->color;
    const auto pressAnimationWasPlaying = pressAnimation.isPlaying();
    if (pressAnimationWasPlaying) {
        const float alphaFactor = 1 - pressAnimation.getProgress();
        glColour(color.withAlpha(0.8f * alphaFactor));
        boundary.drawFill();
        pressAnimation.advance(window.frameRate);
    }
    
    if (mouseOver || mouseOverAnimation.isPlaying()) {
        const float alphaFactor = mouseOver ? mouseOverAnimation.getProgress()
                                  : 1 - mouseOverAnimation.getProgress();
        glColour(color.withAlpha(0.4f * alphaFactor));
        boundary.drawFill();
        // mouse enter animation twice as fast as mouse exit
        mouseOverAnimation.advance(window.frameRate * (mouseOver ? 0.5 : 1));
    }
    
    if (showsState && buttonDown) {
        glColour(color.withAlpha(0.2f));
        boundary.drawFill();
    }
    
    if (pressAnimationWasPlaying && !pressAnimation.isPlaying())
        changeTextLook();
    
    textBox.draw(window);
    
    if (drawBoundary) {
        if ((showsState && buttonDown) || pressAnimation.isPlaying())
            glLineWidth(2.0);
        glColour(color);
        boundary.drawOutline();
        glLineWidth(1.0);
    }
}

bool GLTextButton::mouseClicked() noexcept
{
    if (mouseOver)
        press();
    return mouseOver;
}

void GLTextButton::press() noexcept
{
    buttonDown = !buttonDown;
    pressAnimation.restart();
    changeTextLook();
}

bool GLTextButton::isDown() const noexcept
{
    return buttonDown;
}

bool GLTextButton::isMouseOver() const noexcept
{
    return mouseOver;
}

void GLTextButton::setBoundary(const Box& newBoundary) noexcept
{
    textBox.setBoundary(newBoundary);
}

Box GLTextButton::getBoundary() const noexcept
{
    return textBox.getBoundary();
}
    
void GLTextButton::setColor(const Colour& newColor) noexcept
{
    textLook.color = newColor;
    textBox.setLook(&textLook);
}
    
Colour GLTextButton::getColor() const noexcept
{
    return textLook.color;
}

void GLTextButton::setText(const std::string& newText) noexcept
{
    textBox.setText(newText);
}

std::string GLTextButton::getText() const noexcept
{
    return textBox.getText();
}
    
void GLTextButton::setTextLook(const TextLook& newLook) noexcept
{
    textLook = newLook;
    textBox.setLook(&textLook);
}

TextLook GLTextButton::getTextLook() const noexcept
{
    return textLook;
}
    
TextBox& GLTextButton::getTextBox() noexcept
{
    return textBox;
}

const Animation& GLTextButton::getPressAnimation() const noexcept
{
    return pressAnimation;
}

const Animation& GLTextButton::getMouseOverAnimation() const noexcept
{
    return mouseOverAnimation;
}
    
void GLTextButton::changeTextLook() noexcept
{
    if ((showsState && buttonDown) || pressAnimation.isPlaying())
        textLook.fontStyle = Font::FontStyleFlags::bold;
    else
        textLook.fontStyle = Font::FontStyleFlags::plain;
    textBox.setLook(&textLook);
}
    
    
    
GLTextTabs::GLTextTabs(const std::vector<std::string>& texts,
                       const Box& boundary,
                       const Orientation orientation,
                       const TextLook& look,
                       const Box& windowBoundary) noexcept
    : textBoxGroup(texts, (orientation == ABOVE || orientation == BELOW) ? 1 : texts.size(),
                   boundary, &textLook),
      textLook(look), selectedTextLook(look), boundary(boundary), windowBoundary(windowBoundary)
{
    // make some space for the bold font and slanted tab edges
    if (orientation == ABOVE || orientation == BELOW) {
        textLook.horizontalPad = textLook.horizontalPad * 0.96 - 0.5 * gapFactor;
    } else { // if (orientation == LEFT || orientation == RIGHT)
        textLook.horizontalPad *= 0.96;
        textLook.verticalPad -= 0.5 * gapFactor;
    }
    textBoxGroup.setLook(&textLook);
    setBoundary(boundary); // set textBoxGroup's boundary with tab bar end gaps taken into account
    // selected tab's font in bold
    selectedTextLook.fontStyle = Font::FontStyleFlags::bold;
    textBoxGroup.setLook(selected, &selectedTextLook);
}

void GLTextTabs::draw(OpenGLWindow& window, const Point<float>& mousePosition)
{
    // compute dimensions of tabs for below
    const Box boundary = getBoundary();
    const float widthOrHeight = (orientation == ABOVE || orientation == BELOW)
                                ? boundary.width() : boundary.height();
    const int numTabs = textBoxGroup.getTexts().size();
    const float gapSpace = widthOrHeight * gapFactor / (numTabs + 2);
    const float halfGap = gapSpace * 0.5;
    const float tabSpace = widthOrHeight * (1 - gapFactor) / numTabs;
    const int prevMouseOver = mouseOver;
    
    // detect which tab mouse is over, if any
    if (boundary.contains(mousePosition) && mouseOverEnabled) {
        float lowA, highA, b1, b2, mouseA, mouseB, alpha, baseB, sign;
        if (orientation == ABOVE || orientation == BELOW) {
            lowA = boundary.getBottom();
            highA = boundary.getTop();
            mouseA = mousePosition.y;
            mouseB = mousePosition.x;
            baseB = boundary.getLeft() + halfGap;
            sign = +1;
        } else {
            lowA = boundary.getLeft();
            highA = boundary.getRight();
            mouseA = mousePosition.x;
            mouseB = mousePosition.y;
            baseB = boundary.getTop() - halfGap;
            sign = -1;
        }
        if (orientation == ABOVE || orientation == RIGHT)
            alpha = (mouseA - lowA) / (highA - lowA);
        else
            alpha = (highA - mouseA) / (highA - lowA);
        for (int i = 0; i < numTabs; ++i) {
            if (i != selected) {
                const float b = baseB + sign * i * (tabSpace + gapSpace);
                if (i == 0)
                    b1 = b + sign * alpha * gapSpace;
                else
                    b1 = b + sign * gapSpace * (0.5 + std::abs(alpha - 0.5));
                if (i+1 == selected)
                    b2 = b + sign * (tabSpace + gapSpace * (1.5 - std::abs(alpha - 0.5)));
                else
                    b2 = b + sign * (tabSpace + gapSpace * (2 - alpha));
                if (std::min(b1, b2) <= mouseB && mouseB <= std::max(b1, b2)) {
                    mouseOver = i;
                    goto MOUSE_OVER_TAB;
                }
            }
        }
    }
    // mouse is not currently over a tab if we get here
    mouseOver = -1;
    
MOUSE_OVER_TAB:
    
    // if mouse exits or enters over an unselected tab, add/refresh the mouse over animations
    if (prevMouseOver != mouseOver) {
        if (mouseOver >= 0 && mouseOver != selected) { // mouse enter animation 2x mouse exit
            mouseOverAnimations.add(mouseOver, mouseOverAnimationDuration * 0.5, true);
//            mouseOverAnimations[mouseOver] = Animation(mouseOverAnimationDuration * 0.5);
//            mouseOverAnimations[mouseOver].restart();
        }
        if (prevMouseOver >= 0 && prevMouseOver != selected) {
            mouseOverAnimations.add(prevMouseOver, mouseOverAnimationDuration, true);
//            mouseOverAnimations[prevMouseOver] = Animation(mouseOverAnimationDuration);
//            mouseOverAnimations[prevMouseOver].restart();
        }
    }
    mousePrev = mousePosition;
    
    // draw dark shading over tab bar (so any background objects are visible, but darkened)
    glColor4f(0.0, 0.0, 0.0, 0.85);
    boundary.drawFill();
    
    const auto color = textLook.color;
    
    // draw some lines around the window boarder (if specifed) to help make it more visible
    if (windowBoundary.width() > 0) {
        glColour(color);
        glLineWidth(2);
        glBegin(GL_LINE_STRIP);
        glVertex2f(windowBoundary.getLeft(), windowBoundary.getTop());
        glVertex2f(windowBoundary.getLeft(), windowBoundary.getBottom());
        glVertex2f(windowBoundary.getRight(), windowBoundary.getBottom());
        glVertex2f(windowBoundary.getRight(), windowBoundary.getTop());
        glEnd();
        glLineWidth(1);
    }
    
    // draw selected tab
    const float offset = selected * (tabSpace + gapSpace) + halfGap;
    float in, out;
    glColour(color);
    glLineWidth(2.0);
    glBegin(GL_LINE_STRIP);
    if (orientation == ABOVE || orientation == BELOW) {
        float x = boundary.getLeft();
        if (orientation == ABOVE) {
            in = boundary.getBottom();
            out = boundary.getTop();
        } else {
            in = boundary.getTop();
            out = boundary.getBottom();
        }
        glVertex2f(x, in);     x += offset;
        glVertex2f(x, in);
        glEnd();
        glEnable(GL_LINE_SMOOTH); // fuckin antialias just the non-vert/horiz lines
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, in);     x += gapSpace;
        glVertex2f(x, out);
        glEnd();
        glDisable(GL_LINE_SMOOTH);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, out);    x += tabSpace;
        glVertex2f(x, out);
        glEnd();
        glEnable(GL_LINE_SMOOTH);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, out);    x += gapSpace;
        glVertex2f(x, in);
        glEnd();
        glDisable(GL_LINE_SMOOTH);
        glBegin(GL_LINE_STRIP);
        glVertex2f(x, in);
        glVertex2f(boundary.getRight(), in);
    } else {
        float y = boundary.getTop();
        if (orientation == LEFT) {
            in = boundary.getRight();
            out = boundary.getLeft();
        } else {
            in = boundary.getLeft();
            out = boundary.getRight();
        }
        glVertex2f(in, y);     y -= offset;
        glVertex2f(in, y);     y -= gapSpace;
        glVertex2f(out, y);    y -= tabSpace;
        glVertex2f(out, y);    y -= gapSpace;
        glVertex2f(in, y);
        glVertex2f(in, boundary.getBottom());
    }
    glEnd();
    glLineWidth(1.0);
    
    // draw nonselected tabs
    const float mid = (in + out) * 0.5;
    float across;
    if (orientation == ABOVE || orientation == BELOW)
        across = boundary.getLeft() + halfGap;
    else
        across = boundary.getTop() - halfGap;
    Animation* mouseOverAnimation;
    for (int i = 0; i < numTabs; ++i) {
        // draw some shading over the hovered over tab
//        if (mouseOverAnimations.find(i) != mouseOverAnimations.end())
//            mouseOverAnimation = &mouseOverAnimations[i];
//        else
//            mouseOverAnimation = nullptr;
        mouseOverAnimation = mouseOverAnimations.get(i);
        if (i != selected && (i == mouseOver || mouseOverAnimation != nullptr)) {
            float alpha = 0.4;
            if (mouseOverAnimation != nullptr) {
                if (i == mouseOver)
                    alpha *= mouseOverAnimation->getProgress();
                else
                    alpha *= 1 - mouseOverAnimation->getProgress();
                mouseOverAnimation->advance(window.frameRate);
                if (! mouseOverAnimation->isPlaying())
                    mouseOverAnimations.remove(i);
                    //mouseOverAnimations.erase(i);
            }
            glColour(color.withAlpha(alpha));
            if (orientation == ABOVE || orientation == BELOW) {
                if (i == 0) {
                    if (i+1 == selected) {
                        glBegin(GL_QUADS);
                        glVertex2f(across, in);  across += gapSpace;
                        glVertex2f(across, out); across += tabSpace;
                        glVertex2f(across, out);
                        glVertex2f(across, in);
                        glEnd();
                        glBegin(GL_TRIANGLES);
                        glVertex2f(across, in);
                        glVertex2f(across, out); across += halfGap;
                        glVertex2f(across, mid);
                        glEnd();    across -= tabSpace + 1.5*gapSpace;
                    } else {
                        glBegin(GL_QUADS);
                        glVertex2f(across, in);  across += gapSpace;
                        glVertex2f(across, out); across += tabSpace;
                        glVertex2f(across, out); across += gapSpace;
                        glVertex2f(across, in);
                        glEnd();    across -= tabSpace + 2*gapSpace;
                    }
                } else {
                    if (i+1 == selected) {
                        across -= halfGap;
                        glBegin(GL_QUADS);
                        glVertex2f(across, mid); across += halfGap;
                        glVertex2f(across, out); across += tabSpace;
                        glVertex2f(across, out); across += halfGap;
                        glVertex2f(across, mid);
                        glVertex2f(across, mid); across -= halfGap;
                        glVertex2f(across, in);  across -= tabSpace;
                        glVertex2f(across, in);  across -= halfGap;
                        glVertex2f(across, mid);
                        across += halfGap;
                        glEnd();
                    } else {
                        across -= halfGap;
                        glBegin(GL_TRIANGLES);
                        glVertex2f(across, mid); across += halfGap;
                        glVertex2f(across, out);
                        glVertex2f(across, in);
                        glEnd();
                        glBegin(GL_QUADS);
                        glVertex2f(across, in);
                        glVertex2f(across, out); across += tabSpace;
                        glVertex2f(across, out); across += gapSpace;
                        glVertex2f(across, in);
                        glEnd();
                        across -= tabSpace + gapSpace;
                    }
                }
            } else { // if (orientation == LEFT || orientation == RIGHT)
                if (i == 0) {
                    if (i + 1 == selected) {
                        glBegin(GL_QUADS);
                        glVertex2f(in,  across); across -= gapSpace;
                        glVertex2f(out, across); across -= tabSpace;
                        glVertex2f(out, across);
                        glVertex2f(in,  across);
                        glEnd();
                        glBegin(GL_TRIANGLES);
                        glVertex2f(in,  across);
                        glVertex2f(out, across); across -= halfGap;
                        glVertex2f(mid, across);
                        glEnd();    across += tabSpace + 1.5*gapSpace;
                    } else {
                        glBegin(GL_QUADS);
                        glVertex2f(in,  across); across -= gapSpace;
                        glVertex2f(out, across); across -= tabSpace;
                        glVertex2f(out, across); across -= gapSpace;
                        glVertex2f(in,  across);
                        glEnd();    across += tabSpace + 2*gapSpace;
                    }
                } else {
                    if (i+1 == selected) {
                        across += halfGap;
                        glBegin(GL_QUADS);
                        glVertex2f(mid, across); across -= halfGap;
                        glVertex2f(out, across); across -= tabSpace;
                        glVertex2f(out, across); across -= halfGap;
                        glVertex2f(mid, across);
                        glVertex2f(mid, across); across += halfGap;
                        glVertex2f(in,  across); across += tabSpace;
                        glVertex2f(in,  across); across += halfGap;
                        glVertex2f(mid, across);
                        across -= halfGap;
                        glEnd();
                    } else {
                        across += halfGap;
                        glBegin(GL_TRIANGLES);
                        glVertex2f(mid, across); across -= halfGap;
                        glVertex2f(out, across);
                        glVertex2f(in,  across);
                        glEnd();
                        glBegin(GL_QUADS);
                        glVertex2f(in,  across);
                        glVertex2f(out, across); across -= tabSpace;
                        glVertex2f(out, across); across -= gapSpace;
                        glVertex2f(in,  across);
                        glEnd();
                        across += tabSpace + gapSpace;
                    }
                }
            }
        }
        // draw mouse click animation
        if (i == selected && selectAnimation.isPlaying()) {
            float prevAcross = across;
            if (i > 0) {
                if (orientation == ABOVE || orientation == BELOW)
                    across -= halfGap;
                else
                    across += halfGap;
            }
            const float prog = selectAnimation.getProgress();
            const float alpha = 0.8 * (1 - 0.5*prog);
            selectAnimation.advance(window.frameRate);
            if (orientation == ABOVE || orientation == BELOW) {
                const float mX = selectAnimation.mouse.x;
                const float midAIn = prog * across + (1 - prog) * mX;
                const float midAOut = prog * (across + gapSpace) + (1 - prog) * mX;
                const float midBIn = prog * (across + 2*gapSpace + tabSpace) + (1 - prog) * mX;
                const float midBOut = prog * (across + gapSpace + tabSpace) + (1 - prog) * mX;
                glBegin(GL_QUADS);
                glColour(color.withAlpha(alpha));
                glVertex2f(across, in);  across += gapSpace;
                glVertex2f(across, out);
                glColour(color.withAlpha(0.0f));
                glVertex2f(midAOut, out);
                glVertex2f(midAIn, in);
                glVertex2f(midBIn, in);
                glVertex2f(midBOut, out); across += tabSpace;
                glColour(color.withAlpha(alpha));
                glVertex2f(across, out);  across += gapSpace;
                glVertex2f(across, in);
                glEnd();
                glLineWidth(2);
                glBegin(GL_LINES);
                glColour(color);
                glVertex2f(across, in);
                glColour(color.withAlpha(0.0f));
                glVertex2f(midBIn, in);
                glVertex2f(midAIn, in);   across -= tabSpace + 2*gapSpace;
                glColour(color);
                glVertex2f(across, in);
                glEnd();
                glLineWidth(1);
            } else { // if (orientation == LEFT || orientation == RIGHT)
                const float mY = selectAnimation.mouse.y;
                const float midAIn = prog * across + (1 - prog) * mY;
                const float midAOut = prog * (across - gapSpace) + (1 - prog) * mY;
                const float midBIn = prog * (across - 2*gapSpace - tabSpace) + (1 - prog) * mY;
                const float midBOut = prog * (across - gapSpace - tabSpace) + (1 - prog) * mY;
                glBegin(GL_QUADS);
                glColour(color.withAlpha(alpha));
                glVertex2f(in, across);  across -= gapSpace;
                glVertex2f(out, across);
                glColour(color.withAlpha(0.0f));
                glVertex2f(out, midAOut);
                glVertex2f(in,  midAIn);
                glVertex2f(in,  midBIn);
                glVertex2f(out, midBOut); across -= tabSpace;
                glColour(color.withAlpha(alpha));
                glVertex2f(out, across);  across -= gapSpace;
                glVertex2f(in, across);
                glEnd();
                glLineWidth(2);
                glBegin(GL_LINES);
                glColour(color);
                glVertex2f(in, across);
                glColour(color.withAlpha(0.0f));
                glVertex2f(in, midBIn);
                glVertex2f(in, midAIn);   across += tabSpace + 2*gapSpace;
                glColour(color);
                glVertex2f(in, across);
                glEnd();
                glLineWidth(1);
            }
            across = prevAcross;
        }
        // draw the normal, unselected tab
        if (i != selected) {
            glColour(color);
            glEnable(GL_LINE_SMOOTH);
            glBegin(GL_LINE_STRIP);
            if (orientation == ABOVE || orientation == BELOW) {
                if (i == 0) {
                    glVertex2f(across, in); across += gapSpace;
                } else {
                    across -= halfGap;
                    glVertex2f(across, mid);
                    across += halfGap;
                }
                glVertex2f(across, out);
                glEnd();
                glDisable(GL_LINE_SMOOTH); // fuckin antialias just the non-vert/horiz lines
                glBegin(GL_LINE_STRIP);
                glVertex2f(across, out); across += tabSpace;
                glVertex2f(across, out);
                glEnd();
                glEnable(GL_LINE_SMOOTH);
                glBegin(GL_LINE_STRIP);
                glVertex2f(across, out);
                if (i+1 < numTabs && i+1 == selected) {
                    across += halfGap;
                    glVertex2f(across, mid);
                } else {
                    across += gapSpace;
                    glVertex2f(across, in);
                }
            } else {
                if (i == 0) {
                    glVertex2f(in, across); across -= gapSpace;
                } else {
                    across += halfGap;
                    glVertex2f(mid, across);
                    across -= halfGap;
                }
                glVertex2f(out, across);    across -= tabSpace;
                glVertex2f(out, across);
                if (i+1 < numTabs && i+1 == selected) {
                    across -= halfGap;
                    glVertex2f(mid, across);
                } else {
                    across -= gapSpace;
                    glVertex2f(in, across);
                }
            }
            glEnd();
            glDisable(GL_LINE_SMOOTH);
        } else { // skip over selected tab cuz we already drew it above
            if (orientation == ABOVE || orientation == BELOW)
                across += tabSpace + ((selected == 0) ? 2 : 1.5) * gapSpace;
            else
                across -= tabSpace + ((selected == 0) ? 2 : 1.5) * gapSpace;
        }
    }
    
    // draw the text labels, selected in bold
    textBoxGroup.draw(window);
}

int GLTextTabs::mouseClicked() noexcept
{
    if (mouseOver != -1)
        setSelected(mouseOver);
    return mouseOver;
}

void GLTextTabs::setSelected(const int newSelected,
                             const bool enableAnimation) noexcept
{
    if (enableAnimation && selected != newSelected) {
        selectAnimation.restart();
        if (mouseOver >= 0 && newSelected == mouseOver)
            selectAnimation.mouse = mousePrev;
        else { // find midpoint of tab for animation
            const int numTabs = textBoxGroup.getTexts().size();
            if (orientation == ABOVE || orientation == BELOW) {
                const float gapSpace = getBoundary().width() * gapFactor / (numTabs + 2);
                const float tabSpace = getBoundary().width() * (1 - gapFactor) / numTabs;
                const float begin = getBoundary().getLeft()
                                    + ((newSelected == 0) ? gapSpace * 0.5 : 0)
                                    + newSelected * (tabSpace + gapSpace);
                selectAnimation.mouse.x = begin + 0.5 * (2*gapSpace + tabSpace);
            } else {
                const float gapSpace = getBoundary().height() * gapFactor / (numTabs + 2);
                const float tabSpace = getBoundary().height() * (1 - gapFactor) / numTabs;
                const float begin = getBoundary().getTop()
                                    - ((newSelected == 0) ? gapSpace * 0.5 : 0)
                                    - newSelected * (tabSpace + gapSpace);
                selectAnimation.mouse.y = begin - 0.5 * (2*gapSpace + tabSpace);
            }
        }
    }
    textBoxGroup.setLook(selected, &textLook);
    selected = newSelected;
    textBoxGroup.setLook(selected, &selectedTextLook);
}

int GLTextTabs::getSelected() const noexcept
{
    return selected;
}

void GLTextTabs::setBoundary(const Box& newBoundary) noexcept
{
    boundary = newBoundary;
    if (orientation == ABOVE || orientation == BELOW) {
        const float gapSpace = boundary.width() * gapFactor / (textBoxGroup.getNumColumns() + 2);
        textBoxGroup.setBoundary(newBoundary.scaled(1 - 2 * gapSpace / boundary.width(), 1));
    } else {
        const float gapSpace = boundary.height()* gapFactor / (textBoxGroup.getNumRows() + 2);
        textBoxGroup.setBoundary(newBoundary.scaled(1, 1 - 2 * gapSpace / boundary.height()));
    }
}

Box GLTextTabs::getBoundary() const noexcept
{
    return boundary;
}
    
bool GLTextTabs::isMouseOver() const noexcept
{
    return mouseOver > -1;
}

TextLook GLTextTabs::getSelectedTextLook() const noexcept
{
    return selectedTextLook;
}
    
    
GLTextRadioButton::GLTextRadioButton(const TextBoxGroup& optionsIn,
                                     const int autoDetect) noexcept
    : normalLook(const_cast<TextLook*>(optionsIn.getLook(0))),
      options(optionsIn),
      autoDetect(autoDetect)
{
}

void GLTextRadioButton::draw(OpenGLWindow& window,
	const Point<float>& mousePosition)
{
	// draw dark shading so any background objects are visible, but darkened
	glColor4f(0.0, 0.0, 0.0, 0.85);
	options.getBoundary().drawFill();

	setMouseOver(mousePosition);

	
	for (int i = 0; i < mouseOverAnimations.getVector().size(); ++i) {
		auto& x = mouseOverAnimations.getVector()[i];
	//for (auto& x : mouseOverAnimations.getVector()) {
		x.thing.advance(window.frameRate);
		if (!x.thing.isPlaying()) {
			if (mouseOver == x.idNum) {
				if (selected == autoDetect && mouseOver == autoDetected)
					options.setLook(x.idNum, mouseOverAutoDetectLook);
				else
					options.setLook(x.idNum, mouseOverLook);
			}
			else {
				//                if (selected == autoDetect && x.idNum == autoDetected)
				//                    options.setLook(x.idNum, selectedLook);
				//                else
				options.setLook(x.idNum, normalLook);
			}
			mouseOverAnimations.remove(x.idNum);
		}
		else {
			if (mouseOver == x.idNum) {
				if (selected == autoDetect && mouseOver == autoDetected)
					x.thing.look = blend(*normalLook/*selectedLook*/, *mouseOverAutoDetectLook, x.thing.getProgress());
				else
					x.thing.look = blend(*normalLook, *mouseOverLook, x.thing.getProgress());
			}
			else {
				if (selected == autoDetect && x.idNum == autoDetected)
					x.thing.look = blend(*mouseOverAutoDetectLook, *normalLook/*selectedLook*/, x.thing.getProgress());
				else
					x.thing.look = blend(*mouseOverLook, *normalLook, x.thing.getProgress());
			}
			options.setLook(x.idNum, &x.thing.look);
		}
	}
	
    if (selectAnimation.isPlaying()) {
        selectAnimation.look = blend(*selectAnimationBeginLook, *selectedLook, selectAnimation.getProgress());
        selectAnimation.advance(window.frameRate);
        options.setLook(selected, &selectAnimation.look);
    }
        
    options.draw(window);
}

int GLTextRadioButton::mouseClicked() noexcept
{
    if (mouseOver >= 0)
        setSelected(mouseOver);
    return mouseOver;
}

void GLTextRadioButton::setSelected(const int newSelected,
                                    const bool enableAnimation) noexcept
{
    if (selected != newSelected
        && selectedLook != nullptr
        && selectAnimationBeginLook != nullptr) {
        if (selected == autoDetect)
            options.setLook(autoDetected, normalLook);
        options.setLook(selected, normalLook);
        if (newSelected == autoDetect)
            options.setLook(autoDetected, normalLook/*selectedLook*/);
//        options.setLook(newSelected, selectedLook);
        if (enableAnimation)
            selectAnimation.restart();
    }
    selected = newSelected;
}
    
int GLTextRadioButton::getSelected() const noexcept
{
    return selected;
}

void GLTextRadioButton::setMouseOver(const Point<float>& mousePosition) noexcept
{
    const int prevMouseOver = mouseOver;
    if (options.getBoundary().contains(mousePosition)) {
        int i = 0;
        for (const auto& b : options.getSubBoundaries()) {
            if (b.contains(mousePosition)) {
                mouseOver = i;
                goto MOUSE_OVER_OPTION;
            }
            ++i;
        }
    }
    mouseOver = -1;
    
MOUSE_OVER_OPTION:
    
    // adjust the font look if mouse changes hovered over option
    if (mouseOver != prevMouseOver && mouseOverLook != nullptr) {
        if (mouseOver >= 0 && mouseOver != selected)
            mouseOverAnimations.add(mouseOver, mouseOverAnimationDuration, true);
        if (prevMouseOver >= 0 && prevMouseOver != selected)
            mouseOverAnimations.add(prevMouseOver, mouseOverAnimationDuration * 6, true);
//        if (prevMouseOver != selected) {
//            if (autoDetect == selected && prevMouseOver == autoDetected && selectedLook != nullptr)
//                options.setLook(prevMouseOver, selectedLook);
//            else
//                options.setLook(prevMouseOver, normalLook);
//        }
//        if (mouseOver != selected) {
//            options.setLook(mouseOver, mouseOverLook);
//        }
    }
}
    
int GLTextRadioButton::getMouseOver() const noexcept
{
    return mouseOver;
}

void GLTextRadioButton::setAutoDetected(const int newAutoDetected) noexcept
{
    if (autoDetected != newAutoDetected) {
        if (selected == autoDetect && selectedLook != nullptr) {
            options.setLook(autoDetected, normalLook);
            options.setLook(newAutoDetected, selectedLook);
        }
        autoDetected = newAutoDetected;
    }
}
    
int GLTextRadioButton::getAutoDetected() const noexcept
{
    return autoDetected;
}

bool GLTextRadioButton::autoDetectSelected() const noexcept
{
    return selected == autoDetect;
}

void GLTextRadioButton::setNormalLook(TextLook* look) noexcept
{
    for (int i = 0; i < options.getSize(); ++i)
        if (i != selected && i != mouseOver && (autoDetect >= 0 ? i != autoDetected : true))
            options.setLook(i, look);
    normalLook = look;
}

void GLTextRadioButton::setSelectedLook(TextLook* look, TextLook* newSelectAnimationBeginLook) noexcept
//void GLTextRadioButton::setSelectedLook(TextLook* look) noexcept
{
    options.setLook(selected, look);
//    if (selected == autoDetect)
//        options.setLook(autoDetected, look);
    selectedLook = look;
    selectAnimationBeginLook = newSelectAnimationBeginLook;
}

void GLTextRadioButton::setMouseOverLook(TextLook* look) noexcept
{
    if (mouseOver >= 0)
        options.setLook(mouseOver, look);
    mouseOverLook = look;
}
    
void GLTextRadioButton::setMouseOverAutoDetectLook(TextLook* look) noexcept
{
    if (selected == autoDetect && mouseOver == autoDetected)
        options.setLook(mouseOver, look);
    mouseOverAutoDetectLook = look;
}

void GLTextRadioButton::setFontSize(const float newFontSize) noexcept
{
    normalLook->fontSize = newFontSize;
    setNormalLook(normalLook);
    
    selectedLook->fontSize = newFontSize;
    selectAnimationBeginLook->fontSize = newFontSize;
    setSelectedLook(selectedLook, selectAnimationBeginLook);
    
    mouseOverLook->fontSize = newFontSize;
    setMouseOverLook(mouseOverLook);
    
    mouseOverAutoDetectLook->fontSize = newFontSize;
    setMouseOverLook(mouseOverAutoDetectLook);
}

Box GLTextRadioButton::getBoundary() const noexcept
{
    return options.getBoundary();
}

void GLTextRadioButton::setBoundary(const Box& b) noexcept
{
    options.setBoundary(b, normalLook);
}

const std::vector<TextBox>& GLTextRadioButton::getTextBoxes() const noexcept
{
    return options.getTextBoxes();
}



GLTitledRadioButton::GLTitledRadioButton(const TextBox& title,
                                         const GLTextRadioButton& options) noexcept
    :  GLTextRadioButton(options), title(title)
{
}

void GLTitledRadioButton::draw(OpenGLWindow& window,
                               const Point<float>& mousePosition)
{
    // draw dark shading so any background objects are visible, but darkened
    glColor4f(0.0, 0.0, 0.0, 0.85);
    title.getBoundary().drawFill();
    
    title.draw(window);
    GLTextRadioButton::draw(window, mousePosition);
}
    
    

void drawThickenedLine(const float a,
                 const float b,
                 const float mid,
                 const float delta,
                 const float aspect,
                 const Colour innerColor,
                 const Colour outerColor,
                 const bool horizontal)
{
    if (horizontal) {
        cauto horizDelta = delta * aspect;
        glBegin(GL_QUADS);
        glColour(outerColor);
        glVertex2f(a - horizDelta, mid + delta);
        glVertex2f(b + horizDelta, mid + delta);
        glColour(innerColor);
        glVertex2f(b, mid);
        glVertex2f(a, mid);
        glColour(outerColor);
        glVertex2f(a - horizDelta, mid - delta);
        glVertex2f(b + horizDelta, mid - delta);
        glColour(innerColor);
        glVertex2f(b, mid);
        glVertex2f(a, mid);
        glEnd();
        glBegin(GL_TRIANGLES);
        glColour(outerColor);
        glVertex2f(a - horizDelta, mid + delta);
        glVertex2f(a - horizDelta, mid - delta);
        glColour(innerColor);
        glVertex2f(a, mid);
        glColour(outerColor);
        glVertex2f(b + horizDelta, mid + delta);
        glVertex2f(b + horizDelta, mid - delta);
        glColour(innerColor);
        glVertex2f(b, mid);
        glEnd();
    } else {
        cauto vertDelta = delta / aspect;
        glBegin(GL_QUADS);
        glColour(outerColor);
        glVertex2f(mid + delta, a - vertDelta);
        glVertex2f(mid + delta, b + vertDelta);
        glColour(innerColor);
        glVertex2f(mid, b);
        glVertex2f(mid, a);
        glColour(outerColor);
        glVertex2f(mid - delta, a - vertDelta);
        glVertex2f(mid - delta, b + vertDelta);
        glColour(innerColor);
        glVertex2f(mid, b);
        glVertex2f(mid, a);
        glEnd();
        glBegin(GL_TRIANGLES);
        glColour(outerColor);
        glVertex2f(mid + delta, a - vertDelta);
        glVertex2f(mid - delta, a - vertDelta);
        glColour(innerColor);
        glVertex2f(mid, a);
        glColour(outerColor);
        glVertex2f(mid + delta, b + vertDelta);
        glVertex2f(mid - delta, b + vertDelta);
        glColour(innerColor);
        glVertex2f(mid, b);
        glEnd();
    }
}
    
    

GLSlider::GLSlider() noexcept
    : mouseOver(false), mouseOverEnabled(true)
{
}

void GLSlider::setup(const Box& newBoundary,
                     const TextBox& newTitle,
                     const EditableTextBox& newValueText,
                     const Colour newColor) noexcept
{
    boundary = newBoundary;
    title = newTitle;
    valueText = newValueText;
    color = newColor;
}

GLSlider::GLSlider(const Box& boundary,
                   const TextBox& title,
                   const EditableTextBox& newValueText,
                   /*const std::unique_ptr<TextInputRestrictor> ir,*/
                   const Colour color) noexcept
    : boundary(boundary), title(title), valueText(newValueText), color(color), value(0.5f), mouseOver(false), mouseOverEnabled(true)
{
    //valueText.setTextInputRestrictor(ir);
}

void GLSlider::draw(OpenGLWindow& w,
                    const Point<float>& mousePosition,
                    const bool newMouseOverEnabled)
{
    window = &w;
    mouseOverEnabled = newMouseOverEnabled;
    cauto prevMouseOver = mouseOver;
    mouseOver = mouseOverEnabled && isMouseOver(mousePosition) && !valueText.getMouseOver();
    if (mouseOver != prevMouseOver)
        mouseOverAnimation.restart(mouseOver ? 0.25f : 0.5f);
    // set the text to display the appropriate value
    if (mouseOver && !valueText.getSelected()) {
        cauto mouseOverValue = getValue(mousePosition);
        auto mouseOverValueStr = std::to_string(mouseOverValue);
        if (getTextInputRestrictor())
            mouseOverValueStr += getTextInputRestrictor()->getUnits();
        if (valueText.getText() != mouseOverValueStr)
            setTextValue(mouseOverValue);
    } else if (prevMouseOver && !mouseOver && !valueText.getSelected()) {
        auto sliderValueStr = std::to_string(value);
        if (getTextInputRestrictor())
            sliderValueStr += getTextInputRestrictor()->getUnits();
        if (valueText.getText() != sliderValueStr)
            setTextValue(value);
    }

    
//    // draw dark shading so any background objects are visible, but darkened
//    glColor4f(0.0, 0.0, 0.0, 0.85);
//    boundary.drawFill();
    cauto aspect = window->getAspect();
    glColour(color);
    cauto horizontal = boundary.width() >= boundary.height();
    if (horizontal) {
        glBegin(GL_LINES);
        cauto mid = boundary.getBottom() + 0.5f * boundary.height();
        glVertex2f(boundary.getLeft(), mid);
        glVertex2f(boundary.getRight(), mid);
        auto val = boundary.getLeft() + getNormalizedValue() * boundary.width();
        if (interpolator)
            val = interpolator->getValue(val, boundary.getLeft(), boundary.getRight());
        cauto dev = 0.3f * boundary.height();
        glVertex2f(val, mid - dev);
        glVertex2f(val, mid + dev);
        glEnd();
        if (setValueAnimation.isPlaying()) {
            drawThickenedLine(mid - dev, mid + dev, val, pixelsToNormalized(4, window->width), aspect,
                        color.withMultipliedAlpha(setValueAnimation.getProgress()),
                        color.withMultipliedAlpha(1 - setValueAnimation.getProgress()), false);
            setValueAnimation.advance(window->frameRate);
        }
        if (mouseOver || mouseOverAnimation.isPlaying()) {
            cauto alphaFactor = mouseOver ? mouseOverAnimation.getProgress() : 1 - mouseOverAnimation.getProgress();
            cauto delta = pixelsToNormalized(3, window->height);
            cauto innerColor = color.withMultipliedAlpha(alphaFactor * 0.75f);
            cauto outerColor = color.withAlpha(0.1f);
            drawThickenedLine(boundary.getLeft(), boundary.getRight(), mid, delta, aspect, innerColor, outerColor, true);
            if (mouseOver) {
                glLineWidth(3);
                glColour(color.withMultipliedAlpha(0.5f));
                glBegin(GL_LINES);
                auto x = mousePosition.x;
                if (x < boundary.getLeft())
                    x = boundary.getLeft();
                else if (x > boundary.getRight())
                    x = boundary.getRight();
                glVertex2f(x, boundary.getBottom());
                glVertex2f(x, boundary.getTop());
                glEnd();
                glLineWidth(1);
            }
            if (mouseOverAnimation.isPlaying())
                mouseOverAnimation.advance(window->frameRate);
        }
    } else {
        glBegin(GL_LINES);
        cauto mid = boundary.getLeft() + 0.5f * boundary.width();
        glVertex2f(mid, boundary.getBottom());
        glVertex2f(mid, boundary.getTop());
        auto val = boundary.getBottom() + getNormalizedValue() * boundary.height();
        if (interpolator)
            val = interpolator->getValue(val, boundary.getBottom(), boundary.getTop());
        cauto dev = 0.3f * boundary.width();
        glVertex2f(mid - dev, val);
        glVertex2f(mid + dev, val);
        glEnd();
        if (setValueAnimation.isPlaying()) {
            drawThickenedLine(mid - dev, mid + dev, val, pixelsToNormalized(4, window->height), aspect,
                        color.withMultipliedAlpha(setValueAnimation.getProgress()),
                        color.withMultipliedAlpha(1-setValueAnimation.getProgress()), true);
            setValueAnimation.advance(window->frameRate);
        }
        if (mouseOver || mouseOverAnimation.isPlaying()) {
            cauto alphaFactor = mouseOver ? mouseOverAnimation.getProgress() : 1 - mouseOverAnimation.getProgress();
            cauto delta = pixelsToNormalized(3, window->width);
            cauto innerColor = color.withMultipliedAlpha(alphaFactor * 0.5f);
            cauto outerColor = color.withAlpha(0.1f);
            drawThickenedLine(boundary.getBottom(), boundary.getTop(), mid, delta, aspect, innerColor, outerColor, false);
            if (mouseOver) {
                glLineWidth(3);
                glColour(color.withMultipliedAlpha(0.5f));
                glBegin(GL_LINES);
                auto y = mousePosition.y;
                if (y < boundary.getBottom())
                    y = boundary.getBottom();
                else if (y > boundary.getTop())
                    y = boundary.getTop();
                glVertex2f(boundary.getLeft(), y);
                glVertex2f(boundary.getRight(), y);
                glEnd();
                glLineWidth(1);
            }
            if (mouseOverAnimation.isPlaying())
                mouseOverAnimation.advance(window->frameRate);
        }
    }
    title.draw(w);
    valueText.draw(w, mousePosition, mouseOverEnabled);
}

bool GLSlider::mouseClicked(const Point<float>& mousePosition) noexcept
{
    if (mouseOverEnabled) {
        if (valueText.mouseClicked()) {
            return true;
        } else if (mouseOver /*isMouseOver(mousePosition)*/) {
            setValue(getValue(mousePosition));
//            value = getValue(mousePosition);
//            setValueAnimation.restart();
            return true;
        }
    }
    return false;
}
    
bool GLSlider::mouseDoubleClicked()
{
    return valueText.mouseDoubleClicked() || (mouseOver && mouseOverEnabled);
}

bool GLSlider::mouseDragged(const Point<float>& mouseDownPosition,
                            const Point<float>& mouseCurrentPosition)
{
    return valueText.mouseDragged(mouseDownPosition, mouseCurrentPosition);
}

bool GLSlider::mouseWheelMove(const float dx,
                              const float dy) noexcept
{
    return valueText.mouseWheelMove(dx, dy);
}
    
bool GLSlider::keyPressed(const std::string& key)
{
    const auto result = valueText.keyPressed(key);
    if (result) {
        if (key == "return" || key == "tab") {
            try {
                setValue(std::stof(valueText.getText()));
            } catch (...) {
                setValue(value);
            }
        }
    }
    return result;
}
    
bool GLSlider::getMouseOver() const noexcept
{
    return valueText.getMouseOver() || mouseOver;
}

void GLSlider::setTextInputRestrictor(std::unique_ptr<DecimalNumberRestrictor> ir) noexcept
{
    valueText.setTextInputRestrictor(std::move(ir));
}

const DecimalNumberRestrictor* const GLSlider::getTextInputRestrictor() const noexcept
{
    return dynamic_cast<const DecimalNumberRestrictor* const>(valueText.getTextInputRestrictor());
}
    
void GLSlider::setInterpolator(const std::unique_ptr<NonlinearInterpolator<float>> interp) noexcept
{
    interpolator = interp->clone();
}

void GLSlider::setValue(const float newValue) noexcept
{
    if (value != newValue) {
        value = newValue;
        setValueAnimation.restart();
        setTextValue(value);
    }
}
    
void GLSlider::setTextValue(const float val) noexcept
{
    auto valueString = std::to_string(val);
    valueText.setText(valueString);
}
   
float GLSlider::getValue() const noexcept
{
    return value;
}

bool GLSlider::isMouseOver(const Point<float>& mousePosition) noexcept
{
    const bool horizontal = boundary.width() >= boundary.height();
    const auto endPad = pixelsToNormalized(4, horizontal ? window->width : window->height);
    const Box boundaryWithEndPad = horizontal
                                    ? Box{boundary.getTop(), boundary.getBottom(),
                                          boundary.getLeft() - endPad, boundary.getRight() + endPad}
                                    : Box{boundary.getTop() + endPad, boundary.getBottom() - endPad,
                                          boundary.getLeft(), boundary.getRight()};
    return boundaryWithEndPad.contains(mousePosition);
}
    
float GLSlider::getValue(const Point<float>& mousePosition) noexcept
{
    float val;
    const auto ir = getTextInputRestrictor();
    float min = 0, max = 1;
    if (ir) {
        min = ir->getMin();
        max = ir->getMax();
    }
    if (boundary.width() >= boundary.height())
        val = (mousePosition.x - boundary.getLeft()) / boundary.width();
    else
        val = (mousePosition.y - boundary.getBottom()) / boundary.height();
    if (val < 0)
        val = 0;
    else if (val > 1)
        val = 1;
    val = min + val * (max - min);
    if (interpolator)
        val = interpolator->getInverseValue(val, min, max);
    return val;
}

float GLSlider::getNormalizedValue() const noexcept
{
    float val = value;
//    float min = 0, max = 1;
    const auto ir = getTextInputRestrictor();
    if (ir)
        val = (value - ir->getMin()) / (ir->getMax() - ir->getMin());
//    if (interpolator)
//        val = interpolator->getValue(value, min, max);
//    else
//        val = value;
    return val;
}
    
void GLSlider::repaint() noexcept
{
    title.repaint();
    valueText.repaint();
}

void GLSlider::makeTitleAndValueFontsSameSize(const int windowWidth,
                                              const int windowHeight)
{
    //if (window && window->context) { // yah... pointers... problems...
        std::vector<TextBox> titleAndValue = {title, valueText};
        makeFontsSameSize(titleAndValue, windowWidth, windowHeight);
        //makeFontsSameSize(titleAndValue, window->width, window->height);
    //}
}
    
void GLSlider::horizontallyCenterAndRepositionText(const Box& titleBoundary,
                                                   const Box& valueBoundary,
                                                   const int windowWidth,
                                                   const int windowHeight,
                                                   const int numSpacesBetween)
{
    title.setBoundary(titleBoundary);
    valueText.setBoundary(valueBoundary);
    const Box centeringBoundary = titleBoundary.combinedWith(valueBoundary);
    std::vector<TextBox> boxes {title, valueText};
    makeFontsSameSize(boxes, windowWidth, windowHeight);
    const auto font = title.getFont(windowWidth, windowHeight); // assuming same font
    std::string valueStr;
    const auto ir = getTextInputRestrictor();
    if (ir) {
        const auto numDigits = ir->getMaxNumDigits() + ir->getUnits().size() + 1;
        for (int i = 0; i < numDigits; ++i)
            valueStr += "0";
    } else
        valueStr = valueText.getText();
    const auto widthA = pixelsToNormalized(font.getStringWidthFloat(title.getText()) / title.getLook()->horizontalPad, windowWidth);
    const auto widthB = pixelsToNormalized(font.getStringWidthFloat(valueStr) / valueText.getLook()->horizontalPad, windowWidth);
    std::string spaces;
    for (int i = 0; i < numSpacesBetween; ++i)
        spaces += " ";
    const auto widthSpaces = pixelsToNormalized(font.getStringWidthFloat(spaces), windowWidth);
    auto left = centeringBoundary.getLeft() + 0.5f * (centeringBoundary.width() - widthA - widthB - widthSpaces);
    auto right = left + widthA;
    title.setBoundary({centeringBoundary.getTop(), centeringBoundary.getBottom(), left, right});
    left = right + widthSpaces;
    right = left + widthB;
    valueText.setBoundary({centeringBoundary.getTop(), centeringBoundary.getBottom(), left, right});
}
    
TextBox& GLSlider::getTitleTextBox() noexcept
{
    return title;
}

EditableTextBox& GLSlider::getValueTextBox() noexcept
{
    return valueText;
}

void GLSlider::setSliderBoundary(const Box& b) noexcept
{
    boundary = b;
}

void GLSlider::setColor(Colour newColor) noexcept
{
    color = newColor;
    auto titleLook = *title.getLook();
    titleLook.color = newColor;
    title.setLook(titleLook);
    auto valueLook = *valueText.getLook();
    valueLook.color = newColor;
    valueText.setLook(valueLook);
    valueText.repaint(); // force repaint in case title and valueText share the same FontLook data
}

Colour GLSlider::getColor() const noexcept
{
    return color;
}
    
    
    
DecimalNumberRestrictor::DecimalNumberRestrictor(const float min,
                                                 const float max,
                                                 const int maxDecimalPlaces,
                                                 const std::string& units)
    : min(min), max(max), maxDecimalPlaces(maxDecimalPlaces), units(units)
{
}

TextInputRestrictor* DecimalNumberRestrictor::clone() const
{
    return new DecimalNumberRestrictor(*this);
}
    
bool DecimalNumberRestrictor::insert(std::string& text,
                                     int& cursorIndex,
                                     const std::string& textToInsert) const noexcept
{
    // this function is intended to only insert one character at a time, but a string is passed in for programatic convienience with EditableTextBox::keyPressed.  handling one character at a time simplifies the logic.
    if (textToInsert.length() == 1 && cursorIndex <= text.size()) {
        if (textToInsert == "-") { // negative sign
            if (cursorIndex == 0 && min < 0) {
                text.insert(cursorIndex++, "-");
                return true;
            }
            return false;
        } else if (textToInsert == ".") { // decimal point
            if (text.find(".") == std::string::npos && maxDecimalPlaces > 0) {
                text.insert(cursorIndex++, ".");
                return true;
            }
            return false;
        } else { // anything but "-" or "."
            int textToInsertIntValue = -1;
            try {
                textToInsertIntValue = std::stoi(textToInsert);
            } catch (...) { // couldn't convert to integer
                return false;
            }
            if (0 <= textToInsertIntValue && textToInsertIntValue <= 9) { // numeric digit 0-9
                text.insert(cursorIndex++, textToInsert);
                return true;
//                const int decimalIndex = text.find(".");
//                // text contains a "."
//                if (decimalIndex != std::string::npos) {
//                    // cursor is left of "."
//                    if (cursorIndex <= decimalIndex) {
//                        int negativeSignDigit = 0;
//                        if (text.length() > 0 && text[0] == '-')
//                            negativeSignDigit = 1;
//                        if (text.length() - negativeSignDigit - (text.length()-(decimalIndex+1)+1) < numDigitsLeftOfDecimal(max)) {
//                            text.insert(cursorIndex++, textToInsert);
//                            return true;
//                        }
//                        return false;
//                    } else { // cursor is right of "."
//                        if (text.length() - (decimalIndex+1) < maxDecimalPlaces) {
//                            text.insert(cursorIndex++, textToInsert);
//                            return true;
//                        }
//                        return false;
//                    }
//                } else { // text doesn't contain a "."
//                    text.insert(cursorIndex++, textToInsert);
//                    return true;
////                    int negativeSignDigit = 0;
////                    if (text.length() > 0 && text[0] == '-')
////                        negativeSignDigit = 1;
////                    if (text.length() - negativeSignDigit < numDigitsLeftOfDecimal(max)) {
////                        text.insert(cursorIndex++, textToInsert);
////                        return true;
////                    }
////                    return false;
//                }
            } else { // not 0-9
                return false;
            }
        }
    }
    return false;
}

void DecimalNumberRestrictor::validateInput(std::string& text) const noexcept
{
    try {
        auto value = std::stold(text); // using long double here to avoid having any digits be rounded
        auto fixedToBounds = false;
        if (value < min) {
            value = min;
            fixedToBounds = true;
        } else if (value > max) {
            value = max;
            fixedToBounds = true;
        }
        text = StrFuncs::roundedFloatString(value, maxDecimalPlaces);
//        const auto bvstr = std::to_string(value);
//        const auto tmp = bvstr.substr(0, (maxDecimalPlaces > 0) ? (bvstr.find(".") + maxDecimalPlaces + 1) : bvstr.length());
//        const auto original = text.substr(0, tmp.length());
//        if (!fixedToBounds)
//            text = original; // make sure that digits don't get rounded
//        else
//            text = tmp;
        text += units;
    } catch (...) {}
}
    
float DecimalNumberRestrictor::getMin() const noexcept
{
    return min;
}
    
float DecimalNumberRestrictor::getMax() const noexcept
{
    return max;
}
    
int DecimalNumberRestrictor::getMaxNumDigits() const noexcept
{
    if (max == std::numeric_limits<long double>::infinity())
        return std::numeric_limits<int>::infinity();
    else
        return numDigitsLeftOfDecimal(max) + 1 + maxDecimalPlaces;
}

std::string DecimalNumberRestrictor::getUnits() const noexcept
{
    return units;
}

int DecimalNumberRestrictor::numDigitsLeftOfDecimal(const float decimalNumber) const
{
    int digits = 0;
    float positiveNumber = std::abs(decimalNumber);
    while (positiveNumber >= 1.0) {
        positiveNumber *= 0.1;
        ++digits;
    }
    return digits;
}



EditableTextBox::EditableTextBox() noexcept
{
    //initAnimations();
}

EditableTextBox::EditableTextBox(const TextBox& textBox, OpenGLWindow* w) noexcept
    : TextBox        (textBox),
      highlightColor (Colours::black.withAlpha(0.0f)),
      window         (w)
{
    //    if (inputRestrictor)
    //    inputRestrictor->validateInput(text);
    //    TextBox(text, bounds);
    //initAnimations();
}

void EditableTextBox::draw(OpenGLWindow& w,
                           const Point<float>& mousePosition,
                           const bool mouseOverEnabled)
{
    window = &w; // update our pointer to the gl window
    if (window->resized) {
        moveXScroll(0); // do bounds checking on xy scroll offsets
        moveYScroll(0);
        cursorPosition.needsUpdate = true;
        highlightBoxesNeedUpdate = true;
    }
    
    if (mouseDragging) // move the scrollOffsets if the mouse is dragged and held above/below/left/right of text box's boundary
        mouseDragged(prevMouseDownPosition, mousePosition, true);
    
    const auto boundary = getBoundary();
    if (mouseOverEnabled) {
        if (boundary.contains(mousePosition)) {
            if (!mouseOver)
                mouseOverAnimation.restart();
            mouseOver = true;
        } else {
            if (mouseOver)
                mouseOverAnimation.restart();
            mouseOver = false;
        }
    } else {
        mouseOver = false;
    }
    mousePrev = mousePosition;

//    // draw dark shading so any background objects are visible, but darkened
//    glColor4f(0.0, 0.0, 0.0, 0.85);
//    boundary.drawFill();
    
    const auto yScrollOffsetPercent = verticalScrollingEnabled() ? scrollOffsetY / getMaxScrollOffsetY() : -1;
    const auto xScrollOffsetPercent = horizontalScrollingEnabled() ? scrollOffsetX / getMaxScrollOffsetX() : 0;
    TextBox::draw(*window, -1, -1, &lines, &font, tabKeySpaces, yScrollOffsetPercent, xScrollOffsetPercent, &longestLineWidth);
    
    const auto color = getLook()->color;
    
    if (mouseOver || mouseOverAnimation.isPlaying()) {
        const float alpha = mouseOver ? mouseOverAnimation.getProgress()
                                      : 1 - mouseOverAnimation.getProgress();
        mouseOverAnimation.advance(window->frameRate);
        glColour(color.withAlpha(alpha));
        boundary.drawOutline();
    }
    
    if (selected || selectAnimation.isPlaying()) {
        const float glowWidth = pixelsToNormalized(4, window->width);
        const float glowHeight = pixelsToNormalized(4, window->height);
        const float alpha = selected ? selectAnimation.getProgress()
                                     : 1 - selectAnimation.getProgress();
        selectAnimation.advance(window->frameRate);
        glBegin(GL_QUADS);
        glColour(color.withAlpha(alpha));
        glVertex2f(boundary.getLeft(), boundary.getTop());
        glVertex2f(boundary.getRight(), boundary.getTop());
        glColour(color.withAlpha(0.0f));
        glVertex2f(boundary.getRight() + glowWidth, boundary.getTop() + glowHeight);
        glVertex2f(boundary.getLeft() - glowWidth, boundary.getTop() + glowHeight);
        
        glColour(color.withAlpha(alpha));
        glVertex2f(boundary.getRight(), boundary.getTop());
        glVertex2f(boundary.getRight(), boundary.getBottom());
        glColour(color.withAlpha(0.0f));
        glVertex2f(boundary.getRight() + glowWidth, boundary.getBottom() - glowHeight);
        glVertex2f(boundary.getRight() + glowWidth, boundary.getTop() + glowHeight);
        
        glColour(color.withAlpha(alpha));
        glVertex2f(boundary.getRight(), boundary.getBottom());
        glVertex2f(boundary.getLeft(), boundary.getBottom());
        glColour(color.withAlpha(0.0f));
        glVertex2f(boundary.getLeft() - glowWidth, boundary.getBottom() - glowHeight);
        glVertex2f(boundary.getRight() + glowWidth, boundary.getBottom() - glowHeight);
        
        glColour(color.withAlpha(alpha));
        glVertex2f(boundary.getLeft(), boundary.getBottom());
        glVertex2f(boundary.getLeft(), boundary.getTop());
        glColour(color.withAlpha(0.0f));
        glVertex2f(boundary.getLeft() - glowWidth, boundary.getTop() + glowHeight);
        glVertex2f(boundary.getLeft() - glowWidth, boundary.getBottom() - glowHeight);
        glEnd();
    }
    
    // draw the highlighted region of the text
    if (highlightedText[1] - highlightedText[0] > 0) {
        if (highlightBoxesNeedUpdate)
            getHighlightBoxes();
        glColour(getHighlightColor() /*color.withAlpha(0.3f)*/);
        for (const auto& b : highlightBoxes)
            b.drawFill();
//        const Box b = getHighlightBox(window.width, window.height);
//        glColour(color.withAlpha(0.3f));
//        glBegin(GL_QUADS);
//        glVertex2f(b.left, b.bottom);
//        glVertex2f(b.left, b.top);
//        glVertex2f(b.right, b.top);
//        glVertex2f(b.right, b.bottom);
//        glEnd();
    } /*else*/ if (selected) { // if the text box is selected for editing
        // draw cursor
        if (cursorAnimation.getProgress() < 0.5) {
            drawCursor();
//            glColour(color);
//            const float x = getCursorPositionX(window.width, window.height);
//            const float h = boundary.top - boundary.bottom;
//            const float mid = boundary.bottom + 0.5*h;
//            glBegin(GL_LINES);
//            glVertex2f(x, mid - 0.4*h);
//            glVertex2f(x, mid + 0.4*h);
//            glEnd();
        }
        cursorAnimation.advance(window->frameRate);
        // restart cursor animation so it continues to blink
        if (!cursorAnimation.isPlaying())
            cursorAnimation.restart();
    }
    // draw scrollers
    glColour(color.withAlpha(0.8f));
    glLineWidth(3);
    glBegin(GL_LINES);
    if (verticalScrollingEnabled())
        drawVerticalScrollbar();
    if (horizontalScrollingEnabled())
        drawHorizontalScrollbar();
    glEnd();
    glLineWidth(1);
    history.advanceTimer(1.0f / window->frameRate);
}

bool EditableTextBox::mouseClicked()
{
    if (highlightedText[1] - highlightedText[0] > 0)
        prevHighlightedText = highlightedText; // hacky as shit, but enables what we want for double clicking behavior
    if (mouseOver) {
        if (!selected) {
            gainFocus();
        } else {
            if (!mouseDragging) {
                highlightedText = {0, 0};
                highlightBoxesNeedUpdate = true;
            }
            cursorPosition.setIndex(getClosestCursorPosition(mousePrev).index);
            cursorAnimation.restart();
            selected = true;
        }
    } else if (mouseDragging) { // mouse is not in the boundary, but we are dragging to select
        selected = true;
    } else {
        releaseFocus();
    }
    prevMouseDownCursorIndex = -1;
    mouseDragging = false;
    return selected;//mouseOver;
}

bool EditableTextBox::mouseDoubleClicked()
{
    if (mouseOver) {
        if (doubleClickSeparator.empty()) { // no separator defined so highlight all text
            highlightedText = {0, static_cast<int>(text.length())};
        } else { // separator defined
            highlightedText = prevHighlightedText; // need remember the highlighted text just before double click b/c the single click event resets it
            const auto cp = getClosestCursorPosition(mousePrev);
            int i = cp.index;
            if (mousePrev.x < cp.x)
                i = std::max(0, cp.index - 1);
            if (highlightedText[1] - highlightedText[0] == 0
             || highlightedText[1] - highlightedText[0] == text.length()) { // there is no highlighted text or all the text is highlighted, then highlight around the mouse between separators on both sides
             A:
                auto begin = text.rfind(doubleClickSeparator, i);
                auto end = text.find(doubleClickSeparator, i);
                if (doubleClickSeparator.find(text[i]) != doubleClickSeparator.npos) { // double clicking over a separator
                    begin = text.find_last_not_of(doubleClickSeparator, i);
                    end = text.find_first_not_of(doubleClickSeparator, i);
                    highlightedText = {(int)(begin != text.npos ? begin + 1 : 0), (int)(end != text.npos ? end : text.length())};
                } else // double clicking over non-separator text
                    highlightedText = {(int)(begin != text.npos ? begin + doubleClickSeparator.length() : 0), (int)(end != text.npos ? end : text.length())};
            } else { // part of the text is highlighted
                if (highlightedText[0] <= i && i <= highlightedText[1]) { // clicking over already highlighted text
                    if (highlightedText[1] - highlightedText[0] == lines[cp.lineIndex].length()) { // clicking over a full highlighted line
                        highlightedText = {0, static_cast<int>(text.length())};
                    } else { // clicking over less than a highlighted line
                        const auto lineBegin = cp.index - cp.indexWithinLine;
                        const auto lineEnd = lineBegin + lines[cp.lineIndex].length();
                        highlightedText = {lineBegin, static_cast<int>(lineEnd)};
                    }
                } else { // clicking over non-highlighted text
                    goto A;
                }
            }
        }
        highlightBoxesNeedUpdate = true;
        cursorPosition.setIndex(highlightedText[1]);
        return true;
    }
    return false;
}

bool EditableTextBox::mouseDragged(const Point<float>& mouseDownPosition,
                                   const Point<float>& mouseCurrentPosition,
                                   const bool adjustScroll)
{
    if (getBoundary().contains(mouseDownPosition)) {
        const int down = mouseDownPosition == prevMouseDownPosition ? // this can be expensive to recompute if it don't need to be
                         prevMouseDownCursorIndex : getClosestCursorPosition(mouseDownPosition).index;
        if (down >= 0 && mouseDownPosition != mouseCurrentPosition) {
            selected = true;
            prevMouseDownPosition = mouseDownPosition;
            prevMouseDownCursorIndex = down;
            if (adjustScroll) {
                const float normScrollFactor = 10 / window->frameRate;
                const auto paddedBoundary = getBoundary().scaled(getLook()->horizontalPad, getLook()->verticalPad);
                float dx = 0, dy = 0;
                if (mouseCurrentPosition.x < paddedBoundary.getLeft())
                    dx = normScrollFactor * (paddedBoundary.getLeft() - mouseCurrentPosition.x);
                else if (mouseCurrentPosition.x > paddedBoundary.getRight())
                    dx = normScrollFactor * (paddedBoundary.getRight() - mouseCurrentPosition.x);
                if (mouseCurrentPosition.y > paddedBoundary.getTop())
                    dy = normScrollFactor * (mouseCurrentPosition.y - paddedBoundary.getTop());
                else if (mouseCurrentPosition.y < paddedBoundary.getBottom())
                    dy = normScrollFactor * (mouseCurrentPosition.y - paddedBoundary.getBottom());
                if (dx != 0 || dy != 0)
                    mouseWheelMove(dx, dy, false);
            }
            const int current = getClosestCursorPosition(mouseCurrentPosition).index;
            cursorPosition.setIndex(current);
            cursorAnimation.restart();
            highlightedText[0] = std::min(down, current);
            highlightedText[1] = std::max(down, current);
            highlightBoxesNeedUpdate = true;
            mouseDragging = true;
            return true;
        }
    }
    //highlightedText = {0, 0}; // this is screwing up double clicking on already highlighted text...
    //highlightBoxesNeedUpdate = true;
    return false;
}
    
bool EditableTextBox::mouseWheelMove(const float dx,
                                     const float dy,
                                     const bool moveOneDimension) noexcept
{
    if (mouseOver || selected) {
        if (moveOneDimension) {
            if (std::abs(dx) > std::abs(dy))
                moveXScroll(dx);
            else
                moveYScroll(dy);
        } else {
            moveXScroll(dx);
            moveYScroll(dy);
        }
        texture.redrawToTexture = true;
        cursorPosition.needsUpdate = true;
        highlightBoxesNeedUpdate = true;
        return true;
    }
    return false;
}

bool EditableTextBox::keyPressed(const std::string& key)
{
//    std::ofstream stream;
//    stream.open("Debugger.txt");
//    stream << key << std::endl;
//    stream.close();
    if (selected) {
        std::string tempKey;
        auto textChanged = false;
        const auto prevFont = (extendToFitSameSizeFont/* && window*/) ? std::make_unique<Font>(getFont(window->width, window->height)) : nullptr;
        //const auto prevHPadSpacing = normalizedToPixels(getBoundary().width() * (1 - getLook()->horizontalPad), window->width);
        // special keys have a longer description
        if (key.length() > 1) {
            if (key == "cursor left") {
//                if (highlightedText[1] - highlightedText[0] > 0)
//                    cursorPosition.setIndex(highlightedText[0]);
//                else
                    cursorPosition.setIndex(std::max(cursorPosition.index - 1, 0));
                highlightedText = {0, 0};
                highlightBoxesNeedUpdate = true;
            } else if (key == "cursor right") {
//                if (highlightedText[1] - highlightedText[0] > 0)
//                    cursorPosition.setIndex(highlightedText[1]);
//                else
                    cursorPosition.setIndex(std::min(cursorPosition.index + 1, static_cast<int>(text.length())));
                highlightedText = {0, 0};
                highlightBoxesNeedUpdate = true;
            } else if (key == "cursor up") {
                const Point<float> lineUp = {cursorPosition.x, (cursorPosition.yLow + cursorPosition.yHigh) * 0.5f
                                                               + getLineHeight()};
                if (cursorPosition.lineIndex > 0)
                    cursorPosition.setIndex(getClosestCursorPosition(lineUp).index);
                highlightedText = {0, 0};
                highlightBoxesNeedUpdate = true;
            } else if (key == "cursor down") {
                const Point<float> lineDown = {cursorPosition.x, (cursorPosition.yLow + cursorPosition.yHigh) * 0.5f
                                                                 - getLineHeight()};
                if (cursorPosition.lineIndex < lines.size() - 1)
                    cursorPosition.setIndex(getClosestCursorPosition(lineDown).index);
                highlightedText = {0, 0};
                highlightBoxesNeedUpdate = true;
            } else if (key == "shift + cursor left") {
                const int cursorIndex = std::max(cursorPosition.index - 1, 0);
                if (highlightedText[1] - highlightedText[0] == 0) {
                    highlightedText[0] = cursorIndex;
                    highlightedText[1] = cursorPosition.index;
                } else if (highlightedText[0] == cursorPosition.index)
                    highlightedText[0] = cursorIndex;
                else if (highlightedText[1] == cursorPosition.index)
                    highlightedText[1] = cursorIndex;
                cursorPosition.setIndex(cursorIndex);
                highlightBoxesNeedUpdate = true;
            } else if (key == "shift + cursor right") {
                const int cursorIndex = std::min(cursorPosition.index + 1, static_cast<int>(text.length()));
                if (highlightedText[1] - highlightedText[0] == 0) {
                    highlightedText[0] = cursorPosition.index;
                    highlightedText[1] = cursorIndex;
                } else if (highlightedText[0] == cursorPosition.index)
                    highlightedText[0] = cursorIndex;
                else if (highlightedText[1] == cursorPosition.index)
                    highlightedText[1] = cursorIndex;
                cursorPosition.setIndex(cursorIndex);
                highlightBoxesNeedUpdate = true;
            } else if (key == "shift + cursor up") {
                const Point<float> lineUp = {cursorPosition.x, (cursorPosition.yLow + cursorPosition.yHigh) * 0.5f
                                                               + getLineHeight()};
                int cursorIndex = cursorPosition.index;
                if (cursorPosition.lineIndex > 0)
                    cursorIndex = getClosestCursorPosition(lineUp).index;
//                if (lineUp.y < getBoundary().top - 0.5f * getBoundary().getHeight() * (1 - getLook()->verticalPad))
//                    cursorIndex = getClosestCursorPosition(lineUp);
                if (highlightedText[1] - highlightedText[0] == 0) {
                    highlightedText[0] = cursorIndex;
                    highlightedText[1] = cursorPosition.index;
                } else if (highlightedText[0] == cursorPosition.index) {
                    highlightedText[0] = cursorIndex;
                } else if (highlightedText[1] == cursorPosition.index) {
                    highlightedText[1] = cursorIndex;
                }
                const int temp = highlightedText[0];
                highlightedText[0] = std::min(highlightedText[0], highlightedText[1]);
                highlightedText[1] = std::max(highlightedText[1], temp);
                highlightBoxesNeedUpdate = true;
                cursorPosition.setIndex(cursorIndex);
//                cursorAnimation.restart();
            } else if (key == "shift + cursor down") {
                const Point<float> lineDown = {cursorPosition.x, (cursorPosition.yLow + cursorPosition.yHigh) * 0.5f
                                                                 - getLineHeight()};
                int cursorIndex = cursorPosition.index;
                if (cursorPosition.lineIndex < lines.size() - 1)
                    cursorIndex = getClosestCursorPosition(lineDown).index;
//                if (lineDown.y > getBoundary().bottom + 0.5f * getBoundary().getHeight() * (1 - getLook()->verticalPad))
//                    cursorIndex = getClosestCursorPosition(lineDown);
                if (highlightedText[1] - highlightedText[0] == 0) {
                    highlightedText[0] = cursorPosition.index;
                    highlightedText[1] = cursorIndex;
                } else if (highlightedText[0] == cursorPosition.index) {
                    highlightedText[0] = cursorIndex;
                } else if (highlightedText[1] == cursorPosition.index) {
                    highlightedText[1] = cursorIndex;
                }
                const int temp = highlightedText[0];
                highlightedText[0] = std::min(highlightedText[0], highlightedText[1]);
                highlightedText[1] = std::max(highlightedText[1], temp);
                highlightBoxesNeedUpdate = true;
                cursorPosition.setIndex(cursorIndex);
//                cursorAnimation.restart();
            } else if (((String)key).equalsIgnoreCase("ctrl + x") || ((String)key).equalsIgnoreCase("command + x")) {
                if (highlightedText[1] - highlightedText[0] > 0) {
                    history.pushBack({text, cursorPosition.index, highlightedText});
                    SystemClipboard::copyTextToClipboard(text.substr(highlightedText[0], highlightedText[1] - highlightedText[0]));
                    text.erase(std::begin(text) + highlightedText[0], std::begin(text) + highlightedText[1]);
                    cursorPosition.setIndex(highlightedText[0]);
//                    cursorAnimation.restart();
                    highlightedText = {0, 0};
                    highlightBoxesNeedUpdate = true;
                    textChanged = true;
                }
            } else if (((String)key).equalsIgnoreCase("ctrl + c") || ((String)key).equalsIgnoreCase("command + c")) {
                if (highlightedText[1] - highlightedText[0] > 0)
                    SystemClipboard::copyTextToClipboard(text.substr(highlightedText[0], highlightedText[1] - highlightedText[0]));
            } else if (((String)key).equalsIgnoreCase("ctrl + v") || ((String)key).equalsIgnoreCase("command + v")) {
                const auto pastedText = SystemClipboard::getTextFromClipboard().toStdString();
                if (pastedText.size() > 0) {
                    history.pushBack({text, cursorPosition.index, highlightedText});
                    textChanged = true;
                }
                if (highlightedText[1] - highlightedText[0] > 0) {
                    text.erase(std::begin(text) + highlightedText[0], std::begin(text) + highlightedText[1]);
                    cursorPosition.setIndex(highlightedText[0]);
                    highlightedText = {0, 0};
                    highlightBoxesNeedUpdate = true;
                }
//                for (const auto& c : pastedText)
//                    inputText(std::to_string(c));
                text.insert(cursorPosition.index, pastedText);
                cursorPosition.setIndex(cursorPosition.index + pastedText.size());

//                cursorAnimation.restart();
            } else if (((String)key).equalsIgnoreCase("ctrl + a") || ((String)key).equalsIgnoreCase("command + a")) {
                highlightedText[0] = 0;
                highlightedText[1] = text.size();
                highlightBoxesNeedUpdate = true;
            } else if (((String)key).equalsIgnoreCase("ctrl + z") || ((String)key).equalsIgnoreCase("command + z")) {
                if (history.getCurrent() != nullptr && history.getCurrent()->text != text)
                    history.pushBack({text, cursorPosition.index, highlightedText}); // save state if necessary before undoing
                auto state = history.undo();
                if (state != nullptr && text != state->text) {
                    text = state->text;
                    cursorPosition.setIndex(state->cursorIndex);
                    highlightedText = state->highlightedText;
                    highlightBoxesNeedUpdate = true;
                    textChanged = true;
                } else
                    return true;
            } else if (((String)key).equalsIgnoreCase("ctrl + shift + z") || ((String)key).equalsIgnoreCase("shift + command + z")) {
                auto state = history.redo();
                if (state != nullptr && text != state->text) {
                    text = state->text;
                    cursorPosition.setIndex(state->cursorIndex);
                    highlightedText = state->highlightedText;
                    highlightBoxesNeedUpdate = true;
                    textChanged = true;
                } else
                    return true;
            } else if (key == "return") {
                if (returnKeyReleasesFocus || !getLook()->multiLine)
                    releaseFocus();
                else {
                    tempKey = "\n";
                    goto INPUT_TEXT;
                }
            } else if (key == "tab") {
                if (tabKeySpaces <= 0 || !getLook()->multiLine)
                    releaseFocus();
                else {
                    for (int i = 0; i < tabKeySpaces; ++i)
                        tempKey += " ";
                    goto INPUT_TEXT;
                }
//            } else if (key.find("return") != std::string::npos
//                       || key.find("tab") != std::string::npos) {
//                if (inputRestrictor)
//                    inputRestrictor->validateInput(text);
//                highlightedText = {0, 0};
//                selectAnimation.restart();
//                selected = false;
            } else if (key == "delete" || key == "backspace") {
                if (highlightedText[1] - highlightedText[0] > 0) {
                    history.pushBack({text, cursorPosition.index, highlightedText});
                    text.erase(std::begin(text) + highlightedText[0], std::begin(text) + highlightedText[1]);
                    cursorPosition.setIndex(highlightedText[0]);
                    highlightedText = {0, 0};
                    highlightBoxesNeedUpdate = true;
                    textChanged = true;
                } else if (cursorPosition.index > 0) {
                    if (history.timerExpired())
                        history.pushBack({text, cursorPosition.index, highlightedText});
                    history.resetTimer();
                    cursorPosition.setIndex(cursorPosition.index-1);
                    text.erase(std::begin(text) + cursorPosition.index);
                    //inputText("");
                    textChanged = true;
                }
//                cursorAnimation.restart();
            } else if (key == "spacebar") {
                tempKey = " ";
                goto INPUT_TEXT;
            } else if (key.find("shift") != std::string::npos) {
                tempKey = key.substr(key.length()-1, key.length());
                goto INPUT_TEXT;
            }
        } else { // key.length() <= 1, therefore its a 'normal' character
            tempKey = key;
        INPUT_TEXT:
            const auto hTxt = highlightedText;
            const auto txt = text;
            const auto cPos = cursorPosition.index;
            if (highlightedText[1] - highlightedText[0] > 0) {
                text.erase(std::begin(text) + highlightedText[0], std::begin(text) + highlightedText[1]);
                cursorPosition.setIndex(highlightedText[0]);
                highlightedText = {0, 0};
                highlightBoxesNeedUpdate = true;
            }
            if (!inputText(tempKey)) { // if input fails, don't delete highlighted text
                text = txt;
                highlightedText = hTxt;
                highlightBoxesNeedUpdate = true;
                cursorPosition.setIndex(cPos);
            } else { // input succeeded
                textChanged = true;
                if (history.timerExpired()) // if input succeeds and the undo timer is expired, add on the state just before the edit to the history
                    history.pushBack({txt, cPos, hTxt});
            }
            history.resetTimer();
        }
        cursorAnimation.restart();
        if (textChanged) { // if text is changed need to get the updated lines, font and longestLineWidth, b/c scrollingEnabled() depends on these
            texture.redrawToTexture = true; // also only redraw to texture if necessary
            if (prevFont) { // adjust box right side / hpad? to fit previous text width
                lines = {text};
                font = *prevFont;
                auto b = getBoundary();
                longestLineWidth = pixelsToNormalized(prevFont->getStringWidthFloat(text), window->width);
                b.setRight(b.getLeft() + longestLineWidth + pixelsToNormalized(4, window->width));
                setBoundary(b);
            } else {
                const auto linesAndFont = getLook()->getLinesAndFont(text, getBoundary(), window->width, window->height);
                lines = linesAndFont.first;
                font = linesAndFont.second;
                float newLongestLineWidth = 0;
                for (const auto& l : lines)
                    if (newLongestLineWidth < font.getStringWidthFloat(l))
                        newLongestLineWidth = font.getStringWidthFloat(l);
                longestLineWidth = pixelsToNormalized(newLongestLineWidth, window->width);
            }
        }
        if (cursorPosition.needsUpdate && (verticalScrollingEnabled() || horizontalScrollingEnabled())) { // adjust scroll offsets so that the cursor is in view
            cursorPosition.set(getCursorPosition()); // and update the cursor position
            float dx = 0, dy = 0;
            const auto paddedBoundary = getBoundary().scaled(getLook()->horizontalPad, getLook()->verticalPad);
            if (cursorPosition.x < paddedBoundary.getLeft())
                dx = paddedBoundary.getLeft() - cursorPosition.x;
            else if (cursorPosition.x > paddedBoundary.getRight())
                dx = paddedBoundary.getRight() - cursorPosition.x;
            if (cursorPosition.yHigh > paddedBoundary.getTop())
                dy = cursorPosition.yHigh - paddedBoundary.getTop();
            else if (cursorPosition.yLow < paddedBoundary.getBottom())
                dy = cursorPosition.yLow - paddedBoundary.getBottom();
            mouseWheelMove(dx, dy, false); // important to do for bounds checking even if dx and dy = 0
        }
        return true;
    }
    return false;
}

void EditableTextBox::setTextInputRestrictor(const std::unique_ptr<TextInputRestrictor> ir)
{
    inputRestrictor = ir->clone();
    if (inputRestrictor)
        inputRestrictor->validateInput(text);
}

void EditableTextBox::setTextInputRestrictor(TextInputRestrictor* ir)
{
    inputRestrictor = ir;
    if (inputRestrictor)
        inputRestrictor->validateInput(text);
}
    
const TextInputRestrictor* const EditableTextBox::getTextInputRestrictor() const noexcept
{
    return inputRestrictor.get();
}

TextInputRestrictor* EditableTextBox::getTextInputRestrictor() noexcept
{
    return inputRestrictor.get();
}

const std::string& EditableTextBox::getText() const noexcept
{
    return text;
}
    
bool EditableTextBox::getMouseOver() const noexcept
{
    return mouseOver;
}
    
bool EditableTextBox::getSelected() const noexcept
{
    return selected;
}

bool EditableTextBox::getMouseDragging() const noexcept
{
    return mouseDragging;
}
    
void EditableTextBox::setText(std::string& text) noexcept
{
    if (inputRestrictor)
        inputRestrictor->validateInput(text);
    TextBox::setText(text);
}

void EditableTextBox::setBoundary(const Box& newBoundary) noexcept
{
    auto b = newBoundary;
    if (extendToFitSameSizeFont && b.width() < minimumWidth) {
        if (horizontalAnchor == HorizontalAnchor::LEFT)
            b.setRight(b.getLeft() + minimumWidth);
        else if (horizontalAnchor == HorizontalAnchor::CENTER) {
            cauto centerX = b.centerX();
            b.setLeft(centerX - 0.5f * minimumWidth);
            b.setRight(centerX + 0.5f * minimumWidth);
        } else // right anchored
            b.setLeft(b.getRight() - minimumWidth);
    }
    if (TextBox::setBoundary(b)) {
        highlightBoxesNeedUpdate = true;
        cursorPosition.needsUpdate = true;
    }
}

bool EditableTextBox::inputText(const std::string& str)
{
    bool result = true;
    if (inputRestrictor) {
        int i = cursorPosition.index;
        result = inputRestrictor->insert(text, i, str);
        cursorPosition.setIndex(i);
    } else {
        text.insert(cursorPosition.index, str);
        cursorPosition.setIndex(cursorPosition.index + str.length());
    }
    return result;
}
    
void EditableTextBox::gainFocus() noexcept
{
    //if (highlightedText[1] - highlightedText[0] == 0) {
        highlightedText = {0, static_cast<int>(text.length())};
        highlightBoxesNeedUpdate = true;
        cursorPosition.setIndex(text.length());
    //}
    selectAnimation.restart();
    cursorAnimation.restart();
    selected = true;
    minimumWidth = getBoundary().width();
}
    
void EditableTextBox::releaseFocus()
{
    if (inputRestrictor) {
        inputRestrictor->validateInput(text);
        repaint();
    }
    highlightedText = {0, 0};
    highlightBoxesNeedUpdate = true;
    if (selected)
        selectAnimation.restart();
    selected = false;
    if (releaseFocusClearsHistory)
        history.clear();
    minimumWidth = 0;
}

void EditableTextBox::finishAnimations() noexcept
{
    selectAnimation.finish();
    mouseOverAnimation.finish();
}

const Animation& EditableTextBox::getSelectAnimation() const noexcept
{
    return selectAnimation;
}

const OpenGLWindow* EditableTextBox::getWindow() const noexcept
{
    return window;
}

void EditableTextBox::setHightColor(Colour newHighlightColor) noexcept
{
    highlightColor = newHighlightColor;
    repaint();
}

Colour EditableTextBox::getHighlightColor() const noexcept
{
    if (highlightColor == Colours::black.withAlpha(0.0f))
        return getLook()->color.withAlpha(0.3f);
    else
        return highlightColor;
}

EditableTextBox::CursorPosition EditableTextBox::getClosestCursorPosition(const Point<float>& point,
                                                                          const float otherHorizontalPad) const
{
    std::vector<int> indecies (text.size() + 1);
    std::iota(std::begin(indecies), std::end(indecies), 0);
    auto it = std::lower_bound(std::begin(indecies), std::end(indecies), point,
                               [&](const int index, const Point<float>& pt) {
                                   bool result = false;
                                   const auto c = getCursorPosition(index, otherHorizontalPad);
                                   const float delta = 0.25f * (c.yHigh - c.yLow) / getLook()->linePad;
                                   const float yLow  = c.yLow  - delta;
                                   const float yHigh = c.yHigh + delta;
                                   if (yLow <= pt.y && pt.y < yHigh) {
                                       if (c.x < pt.x)
                                           result = true;
                                   } else if (yLow > pt.y)
                                       result = true;
                                   return result;
                               });
    if (it == std::end(indecies))
        return getCursorPosition(text.size());//text.size();
    if (*it > 0)
        --it;
    const auto first = getCursorPosition(*it);
    const auto second = (*it + 1 <= text.size() && point.y <= getBoundary().getTop()) ? getCursorPosition(*it + 1) : CursorPosition();
    if (second.index >= 0) { // we have a second valid cursor position to consider
        const auto dxFirst  = std::abs(point.x - first.x);
        const auto dxSecond = std::abs(point.x - second.x);
        if (dxSecond == dxFirst) { // x values are equal (as in the case of boundary cursor positions being on seperate lines), so look at y
            if (second.yLow <= point.y && point.y < second.yHigh)
                return second;//*it + 1;
            else
                return first;//*it;
        } else { // select closest position based on dx
            if (dxSecond < dxFirst)
                return second;//*it + 1;
            else
                return first;//*it;
        }
    } else
        return first;//*it;
//    if (dSecond >= 0 && dSecond < dFirst)
//        return *it + 1;
//    else
//        return *it;
}

EditableTextBox::CursorPosition
    EditableTextBox::getCursorPosition(const int otherCursorIndex,
                                       const float otherHorizontalPad) const
{
    const int whichCursorIndex = otherCursorIndex >= 0 ? otherCursorIndex : cursorPosition.index;
    int lineBeginCharacterIndex = 0;
    int lineIndex = 0;
    std::string line;
    for (int i = 0; i < lines.size(); ++i) {
        if (whichCursorIndex > lineBeginCharacterIndex + lines[i].length()
            || (whichCursorIndex == lineBeginCharacterIndex + lines[i].length()
                && i+1 < lines.size() && (lines[i].back() == ' ' || lines[i].back() == '\n'))) {
            lineBeginCharacterIndex += lines[i].length();
        }
        else {
            line = lines[i];
            lineIndex = i;
            // remove any newline characers from end of lines, because it messes with the cursor position computation below
            findAndReplace(line, "\n", "");
            break;
        }
    }
    const int indexWithinLine = whichCursorIndex - lineBeginCharacterIndex;
    int indexWithinLineAdjust = 0;
    if (getLook()->just.testFlags(Justification::left))
        indexWithinLineAdjust = removeFirstWhitespaceIfLessThan(line, tabKeySpaces);
    else if (getLook()->just.testFlags(Justification::right))
        removeLastWhitespaceIfLessThan(line, tabKeySpaces);
    const float whichHorizontalPad = otherHorizontalPad > 0 ? otherHorizontalPad : getLook()->horizontalPad;
    const float xStart = getBoundary().getLeft() + 0.5f * (1 - whichHorizontalPad) * getBoundary().width();
    float justOffsetX = 0.0f; // left justified
    if (getLook()->just.testFlags(Justification::horizontallyCentred))
        justOffsetX = 0.5f * (getBoundary().width() * getLook()->horizontalPad - pixelsToNormalized(font.getStringWidthFloat(line), window->width));
    else if (getLook()->just.testFlags(Justification::right))
        justOffsetX = getBoundary().width() * getLook()->horizontalPad - pixelsToNormalized(font.getStringWidthFloat(line), window->width);
    float xScrollAdjust = 0;
    if (horizontalScrollingEnabled()) {
        if (getLook()->just.testFlags(Justification::left))
            xScrollAdjust = -scrollOffsetX;
        else if (getLook()->just.testFlags(Justification::horizontallyCentred))
            xScrollAdjust = 0.5f * getMaxScrollOffsetX() - scrollOffsetX;
        else // right justified
            xScrollAdjust = getMaxScrollOffsetX() - scrollOffsetX;
    }
    const float x = xStart + justOffsetX + pixelsToNormalized(font.getStringWidthFloat(line.substr(0, std::max(indexWithinLine - indexWithinLineAdjust, 0))), window->width)
                    + xScrollAdjust;
    float yLow, yHigh;
    const float imageHeight = getBoundary().height() * getLook()->relativeScale;
    const float paddedHeight = getBoundary().height() * getLook()->relativeScale * getLook()->verticalPad;
    const float fontHeight = pixelsToNormalized(font.getHeight(), window->height);
    if (getLook()->multiLine) {
        const float whichLinePad = lines.size() > 1 ? getLook()->linePad : 1; // the linePad is only relevant when we have two or more lines
        // vertical justification needs to take into account all lines, default is centered
        float vertJustOffset = 0.5f * (paddedHeight - getTotalTextHeight());
        if (getLook()->just.getOnlyVerticalFlags() == Justification::top)
            vertJustOffset = 0.0f;
        else if (getLook()->just.getOnlyVerticalFlags() == Justification::bottom)
            vertJustOffset *= 2.0f;
        const float yOffset = 0.5f * (imageHeight - paddedHeight) + (verticalScrollingEnabled() ? -scrollOffsetY : vertJustOffset);
        yHigh = getBoundary().getTop() - yOffset - lineIndex * fontHeight / whichLinePad;
    } else {
        float vertJustOffset = 0.5f * (paddedHeight - fontHeight);
        if (getLook()->just.getOnlyVerticalFlags() == Justification::top)
            vertJustOffset = 0.0f;
        else if (getLook()->just.getOnlyVerticalFlags() == Justification::bottom)
            vertJustOffset *= 2.0f;
        const float yOffset = 0.5f * (imageHeight - paddedHeight) + vertJustOffset;
        yHigh = getBoundary().getTop() - yOffset;
    }
    yLow = yHigh - fontHeight;
	CursorPosition cPos;
	cPos.x = x;
	cPos.yLow = yLow;
	cPos.yHigh = yHigh;
	cPos.index = whichCursorIndex;
	cPos.lineIndex = lineIndex;
	cPos.indexWithinLine = indexWithinLine;
	return cPos;// fuckin MSVC compiler... { x, yLow, yHigh, whichCursorIndex, lineIndex, indexWithinLine };
}
    
void EditableTextBox::drawCursor()
{
    if (cursorPosition.needsUpdate)
        cursorPosition.set(getCursorPosition());
    const auto b = getBoundary();
    if (cursorPosition.yLow < b.getTop() && cursorPosition.yHigh > b.getBottom() &&
        cursorPosition.x > b.getLeft() && cursorPosition.x < b.getRight()) {
        glColour(getLook()->color);
        glBegin(GL_LINES);
        glVertex2f(cursorPosition.x, std::max(cursorPosition.yLow, b.getBottom()));
        glVertex2f(cursorPosition.x, std::min(cursorPosition.yHigh, b.getTop()));
        glEnd();
    }
}
    
void EditableTextBox::getHighlightBoxes()
{
    highlightBoxes.clear();
    if (highlightedText[1] - highlightedText[0] > 0) {
        const CursorPosition begin = getCursorPosition(highlightedText[0]);
        const CursorPosition end = getCursorPosition(highlightedText[1]);
        if (begin.lineIndex != end.lineIndex) {
            const int numBoxes = end.lineIndex - begin.lineIndex + 1;
            for (int i = 0; i < numBoxes; ++i) {
                if (i == 0) { // first box
                    std::string line = lines[begin.lineIndex];
                    int adjust = 0;
                    if (getLook()->just.testFlags(Justification::left))
                        adjust = removeFirstWhitespaceIfLessThan(line, tabKeySpaces);
                    const std::string highlightedPartOfLine = line.substr(std::max(begin.indexWithinLine - adjust, 0), std::string::npos);
                    const float endX = begin.x + pixelsToNormalized(font.getStringWidthFloat(highlightedPartOfLine), window->width);
                    highlightBoxes.emplace_back(begin.yHigh, begin.yLow, begin.x, std::min(endX, getBoundary().getRight()));
                } else if (i < numBoxes-1) { // middle box(es)
                    int cursorIndexOfLineBegin = 0;
                    bool adjust = false;
                    for (int l = 0; l < i + begin.lineIndex; ++l) {
                        cursorIndexOfLineBegin += lines[l].size();
                        const int leadingWhitespaceLength = getLeadingWhitespace(lines[l+1]).size();
                        if (l == i + begin.lineIndex - 1 && leadingWhitespaceLength > 0) {
                            //if (getLook()->just.testFlags(Justification::left))
                                cursorIndexOfLineBegin += 1;
                            if (leadingWhitespaceLength >= tabKeySpaces)
                                adjust = true;
                        }
                    }
                    CursorPosition lineBegin = getCursorPosition(cursorIndexOfLineBegin);
                    if (adjust)
                        lineBegin.x -= pixelsToNormalized(font.getStringWidthFloat(" "), window->width);
                    std::string line = lines[lineBegin.lineIndex];
                    if (getLook()->just.testFlags(Justification::left))
                        removeFirstWhitespaceIfLessThan(line, tabKeySpaces);
                    const float endX = lineBegin.x + pixelsToNormalized(font.getStringWidthFloat(line/*lines[lineBegin.lineIndex]*/), window->width);
                    highlightBoxes.emplace_back(lineBegin.yHigh, lineBegin.yLow, lineBegin.x, std::min(endX, getBoundary().getRight()));
                } else { // last box
                    std::string line = lines[end.lineIndex];
                    int adjust = 0;
                    if (getLook()->just.testFlags(Justification::left))
                        adjust = removeFirstWhitespaceIfLessThan(line, tabKeySpaces);
                    const std::string highlightedPartOfLine = line.substr(0, std::max(end.indexWithinLine - adjust, 0));
                    const float beginX = end.x - pixelsToNormalized(font.getStringWidthFloat(highlightedPartOfLine), window->width);
                    highlightBoxes.emplace_back(end.yHigh, end.yLow, beginX, end.x);
                }
            }
        } else {
            highlightBoxes.emplace_back(begin.yHigh, begin.yLow, begin.x, end.x);
        }
        // trim to what fits inside the boundary
        const auto boundary = getBoundary();
        Box b;
        for (int i = 0; i < highlightBoxes.size(); ++i) {
            if (highlightBoxes[i].overlaps(boundary))
                highlightBoxes[i].crop(boundary);
            else
                highlightBoxes.erase(highlightBoxes.begin() + i--);
        }
    }
    highlightBoxesNeedUpdate = false;
}
    
bool EditableTextBox::verticalScrollingEnabled() const noexcept
{
    return (mouseOver || selected)
        && (getTotalTextHeight() > getBoundary().height() /* * getLook()->verticalPad*/);
    // single-line text was having this enabled when it shouldn't ^
}
    
bool EditableTextBox::horizontalScrollingEnabled() const noexcept
{
    return (mouseOver || selected)
        && (longestLineWidth > getBoundary().width() * getLook()->horizontalPad);
}
    
void EditableTextBox::drawVerticalScrollbar() const
{
    const auto pixelsFromEdge = 4;
    const auto normHeightPad = pixelsToNormalized(pixelsFromEdge, window->height);
    const auto x = getBoundary().getRight() - pixelsToNormalized(pixelsFromEdge, window->width);
    const auto totalTextHeight = getTotalTextHeight();
    const auto availableHeight = getBoundary().height() - 2 * normHeightPad;
    const auto ratio = availableHeight / totalTextHeight;
    const auto yTop = getBoundary().getTop() - normHeightPad - scrollOffsetY * ratio;
    const auto h = availableHeight - getMaxScrollOffsetY() * ratio;
    glVertex2f(x, yTop);
    glVertex2f(x, yTop - h);
}

void EditableTextBox::drawHorizontalScrollbar() const
{
    const auto pixelsFromEdge = 4;
    const auto normWidthPad = pixelsToNormalized(pixelsFromEdge, window->width);
    const auto y = getBoundary().getBottom() + pixelsToNormalized(pixelsFromEdge, window->height);
    const auto availableWidth = getBoundary().width() - 2 * normWidthPad;
    const auto ratio = availableWidth / longestLineWidth;
    const auto xLeft = getBoundary().getLeft() + normWidthPad + scrollOffsetX * ratio;
    const auto l = availableWidth - getMaxScrollOffsetX() * ratio;
    glVertex2f(xLeft,     y);
    glVertex2f(xLeft + l, y);
}

float EditableTextBox::getTotalTextHeight() const noexcept
{
    const auto h = pixelsToNormalized(font.getHeight(), window->height);
    const auto numLines = std::max(static_cast<int>(lines.size()), 1); // we have at least one line, even if the text box is empty
    if (numLines > 1)
        return numLines * h / getLook()->linePad - h / getLook()->linePad * (1 - getLook()->linePad);
    else // numLines == 1
        return h;
}

float EditableTextBox::getMaxScrollOffsetY() const noexcept
{
    return std::max(getTotalTextHeight() - getBoundary().height() * getLook()->verticalPad, 0.0f);
}
   
//std::array<float, 2> EditableTextBox::getScrollOffsetXBounds() const noexcept
//{
//    if (getLook()->just.testFlags(Justification::left))
//        return {0.0f, getMaxScrollOffsetX()};
//    else if (getLook()->just.testFlags(Justification::horizontallyCentred))
//        return {-0.5f * getMaxScrollOffsetX(), 0.5f * getMaxScrollOffsetX()};
//    else // right justified
//        return {-getMaxScrollOffsetX(), 0.0f};
//}
    
float EditableTextBox::getMaxScrollOffsetX() const noexcept
{
    return std::max(longestLineWidth - getBoundary().width() * getLook()->horizontalPad, 0.0f);
}
    
//float EditableTextBox::getNormalizedScrollOffset(const float offset) const noexcept
//{
//    return offset * getBoundary().getHeight() * getLook()->verticalPad / getTotalTextHeight();
//}
    
float EditableTextBox::getPaddedTop() const noexcept
{
    return getBoundary().getTop() - 0.5f * getBoundary().height() * (1 - getLook()->verticalPad);
}

float EditableTextBox::getPaddedBottom() const noexcept
{
    return getBoundary().getBottom() + 0.5f * getBoundary().height() * (1 - getLook()->verticalPad);
}
    
float EditableTextBox::getLineHeight() const noexcept
{
    return (cursorPosition.yHigh - cursorPosition.yLow) / getLook()->linePad;
}

void EditableTextBox::moveXScroll(const float dx) noexcept
{
    scrollOffsetX -= dx;
    if (scrollOffsetX < 0)
        scrollOffsetX = 0;
    else if (scrollOffsetX > getMaxScrollOffsetX())
        scrollOffsetX = getMaxScrollOffsetX();
   
}
    
void EditableTextBox::moveYScroll(const float dy) noexcept
{
    scrollOffsetY -= dy;
    if (scrollOffsetY < 0)
        scrollOffsetY = 0;
    else if (scrollOffsetY > getMaxScrollOffsetY())
        scrollOffsetY = getMaxScrollOffsetY();
}
//void EditableTextBox::undo()
//{
//    
//}
//
//void EditableTextBox::redo()
//{
//
//}
    
//Box EditableTextBox::getHighlightBox(const int windowWidth,
//                                     const int windowHeight) const
//{
//    const auto xStart = getBoundary().left + 0.5f * (1 - getLook()->horizontalPad) * getBoundary().getWidth();
//    const auto font = getFont(windowWidth, windowHeight);
//    return { getBoundary().top, getBoundary().bottom,
//        xStart + standardLengthToMine(font.getStringWidthFloat(text.substr(0, highlightedText[0])), windowWidth),
//        xStart + standardLengthToMine(font.getStringWidthFloat(text.substr(0, highlightedText[1])), windowWidth) };
////    auto posAndScale = getBeginPositionAndScale();
////    const float xStart = posAndScale[0];
////    const float yStart = posAndScale[1];
////    const float scale = posAndScale[2];
////    return { yStart + font->topMax() * scale,
////        yStart - font->bottomMax() * scale,
////        xStart + font->length(text.substr(0, highlightedText[0])) * scale * aspect,
////        xStart + font->length(text.substr(0, highlightedText[1])) * scale * aspect };
//}

//void EditableTextBox::initAnimations() noexcept
//{
//    selectAnimation.count = selectAnimation.len;
//    mouseOverAnimation.count = mouseOverAnimation.len;
//    cursorAnimation.count = cursorAnimation.len;
//}

    
    
//std::pair<std::vector<std::string>, float>
//    TexturedText::getLinesAndMaxFontSize(const std::vector<std::string>& words,
//                                         const float boxWidth,
//                                         const float boxHeight)
//{
//    // compute a first order approx of the maximum text size
//    Font font (fontName, boxHeight, fontStyle);
//    font.setExtraKerningFactor(kerningFactor);
//    float totalLength = 0.0;
//    for (const auto& w : words)
//        totalLength += font.getStringWidthFloat(w);
//    const float textHeight = font.getHeight();
//    const float initArea = totalLength * textHeight / linePadFactor;
//    float textScale = std::sqrt(boxWidth * boxHeight / initArea);
//    
//    //int iters = 0;
//TRY_AGAIN: // scale back further still if the text cannot be contained in the specified box height-wise
//    //iters ++;
//    std::vector<std::string> lines;
//    const int numLines = boxHeight / (textHeight / linePadFactor * textScale);
//    float textLength = 0;
//    int lineCount = 1;
//    for (int i = 0; i < words.size(); ++i) {
//        textLength += font.getStringWidthFloat(words[i]) * textScale;
//        if (textLength > boxWidth || words[i] == "\n") {
//            if (lines.size() > lineCount-1) // if we jump to a new line, the previous line's last whitespace is discarded
//                removeLastWhitespace(lines[lineCount-1]);
//            ++lineCount;
//            if (lineCount > numLines) { // micro-adjust scale until text fits height-wise
//                textScale *= 0.95;
//                lines.clear();
//                goto TRY_AGAIN;
//            }
//            textLength = 0;
//            if (words[i].length() > 0 && words[i][0] != ' ' && words[i] != "\n")
//                --i;
//        } else {
//            while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
//                lines.emplace_back("");
//            lines[lineCount-1] += words[i];
//        }
//    }
//    //lines.emplace_back(std::to_string(iters));
//    return {lines, textHeight * textScale};
//}
//    
//std::vector<std::string>
//    TexturedText::getLinesWithFontSize(const std::vector<std::string>& words,
//                                       const float boxWidth,
//                                       const float fontSize)
//{
//    Font font (fontName, fontSize, fontStyle);
//    font.setExtraKerningFactor(kerningFactor);
//    std::vector<std::string> lines;
//    float textLength = 0;
//    int lineCount = 1;
//    for (int i = 0; i < words.size(); ++i) {
//        textLength += font.getStringWidthFloat(words[i]);
//        if (textLength > boxWidth || words[i] == "\n") {
//            if (lines.size() > lineCount-1) // if we jump to a new line, the previous line's last whitespace is discarded
//                removeLastWhitespace(lines[lineCount-1]);
//            ++lineCount;
//            textLength = 0;
//            if (words[i].length() > 0 && words[i][0] != ' ' && words[i] != "\n")
//                --i;
//        } else {
//            while (lineCount-1 >= lines.size()) // prevent array out of bounds crashing
//                lines.emplace_back("");
//            lines[lineCount-1] += words[i];
//        }
//    }
//    return lines;
//}
//
//void TexturedText::imageToTexture(const Image& image,
//                                  const Point<int>& textureTopLeft)
//{
//    texture.release(); // make sure to free any previous texture memory
//    texture.loadImage(image);
//    clip = {textureTopLeft, {textureTopLeft.getX() + image.getWidth(),
//                             textureTopLeft.getY() + image.getHeight()}};
//    pos  = {textureTopLeft, {textureTopLeft.getX() + texture.getWidth(),
//                             textureTopLeft.getY() + texture.getHeight()}};
//    redrawToTexture = false;
//}
//    
//TexturedText::TexturedText(const Box& boundary) noexcept
//    : boundary(boundary)
//{
//}
//    
//TexturedText::~TexturedText()
//{
//}
//    
//void TexturedText::draw(OpenGLWindow& glWindow)
//{
//    if (glWindow.resized || redrawToTexture)
//    {
//        if (multiLine)
//            drawMultiLine(glWindow.width, glWindow.height);
//        else
//            drawSingleLine(glWindow.width, glWindow.height);
//    }
//    
//    texture.bind();
//    glWindow.context->copyTexture(clip, pos, glWindow.width, glWindow.height, false);
//    texture.unbind();
//    
//    // need to reactivate alpha color stuff after drawing a texture for some reason
//    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); // this does the trick
//    //    glEnable(GL_BLEND);                          // ... but these might be necessary also
//    //    glClearColor(0.0,0.0,0.0,0.0);
//}
//    
//void TexturedText::setFont(const String& newFontName) noexcept
//{
//    if (fontName != newFontName) {
//        fontName = newFontName;
//        redrawToTexture = true;
//    }
//}
//
//String TexturedText::getFont() const noexcept
//{
//    return fontName;
//}
//
//void TexturedText::setFontStyle(const Font::FontStyleFlags newFontStyle) noexcept
//{
//    if (fontStyle != newFontStyle) {
//        fontStyle = newFontStyle;
//        redrawToTexture = true;
//    }
//}
//
//Font::FontStyleFlags TexturedText::getFontStyle() const noexcept
//{
//    return fontStyle;
//}
//
//void TexturedText::setJustification(const Justification newJust) noexcept
//{
//    if (just != newJust) {
//        just = newJust;
//        redrawToTexture = true;
//    }
//}
//
//Justification TexturedText::getJustification() const noexcept
//{
//    return just;
//}
//
//void TexturedText::setKerning(const float newKerningFactor) noexcept
//{
//    if (kerningFactor != newKerningFactor) {
//        kerningFactor = newKerningFactor;
//        redrawToTexture = true;
//    }
//}
//
//float TexturedText::getKerning() const noexcept
//{
//    return kerningFactor;
//}
//
//void TexturedText::setDrawMultiLine(const bool newDrawMultiLine) noexcept
//{
//    if (multiLine != newDrawMultiLine) {
//        multiLine = newDrawMultiLine;
//        redrawToTexture = true;
//    }
//}
//
//bool TexturedText::getDrawMultiLine() const noexcept
//{
//    return multiLine;
//}
//
//void TexturedText::setLinePadding(const float newLinePadFactor) noexcept
//{
//    if (linePadFactor != newLinePadFactor) {
//        linePadFactor = std::max(newLinePadFactor, 0.0f);
//        redrawToTexture = true;
//    }
//}
//
//float TexturedText::getLinePadding() const noexcept
//{
//    return linePadFactor;
//}
//
//void TexturedText::setHorizontalOffset(const float newHorizontalOffsetFactor) noexcept
//{
//    if (horizontalOffsetFactor != newHorizontalOffsetFactor) {
//        horizontalOffsetFactor = newHorizontalOffsetFactor;
//        redrawToTexture = true;
//    }
//}
//
//float TexturedText::getHorizontalOffset() const noexcept
//{
//    return horizontalOffsetFactor;
//}
//
//void TexturedText::setColor(const Colour& newColor) noexcept
//{
//    if (color != newColor) {
//        color = newColor;
//        redrawToTexture = true;
//    }
//}
//
//Colour TexturedText::getColor() const noexcept
//{
//    return color;
//}
//    
//void TexturedText::setBoundary(const Box& newBoundary) noexcept
//{
//    if (boundary != newBoundary) {
//        boundary = newBoundary;
//        redrawToTexture = true;
//    }
//}
//
//Box TexturedText::getBoundary() const noexcept
//{
//    return boundary;
//}
//
//void TexturedText::setBoundaryPadding(const float newHorizontalPad,
//                                      const float newVerticalPad) noexcept
//{
//    if (horizontalBoundaryPad != newHorizontalPad) {
//        horizontalBoundaryPad = std::min(std::max(newHorizontalPad, 0.0f), 1.0f);
//        redrawToTexture = true;
//    }
//    if (verticalBoundaryPad != newVerticalPad) {
//        verticalBoundaryPad = std::min(std::max(newVerticalPad, 0.0f), 1.0f);
//        redrawToTexture = true;
//    }
//}
//
//std::array<float, 2> TexturedText::getBoundaryPadding() const noexcept
//{
//    return {horizontalBoundaryPad, verticalBoundaryPad};
//}
//    
//    
//    
//OpenGLTextBox::OpenGLTextBox(const String& text,
//                             const Box& boundary) noexcept
//    : TexturedText(boundary), text(text)
//{
//}
//  
//void OpenGLTextBox::drawSingleLine(const int windowWidth,
//                                   const int windowHeight)
//{
//    const Box paddedBoundary = getScaled(boundary, horizontalBoundaryPad, verticalBoundaryPad);
//    const int imageWidth  = myLengthToStandard(paddedBoundary.getWidth(),  windowWidth);
//    const int imageHeight = myLengthToStandard(paddedBoundary.getHeight(), windowHeight);
//    Image image (Image::ARGB, imageWidth, imageHeight, true);
//    Graphics g (image);
//    g.setColour(color);
//    Font font (fontName, imageHeight, fontStyle);
//    font.setExtraKerningFactor(kerningFactor);
//    const float stringWidth = font.getStringWidthFloat(text);
//    const float scale = std::min(imageWidth / stringWidth, 1.0f);
//    font.setHeight(imageHeight * scale);
//    g.setFont(font);
//    const int xOffset = std::round(horizontalOffsetFactor * font.getHeight());
//    g.drawText(text, xOffset, 0, imageWidth, imageHeight, just);
//    if (effect != nullptr) {
//        effect->applyEffect(image, g, 1, 1);
//        if (effectBehind) {
//            g.setColour(color);
//            g.drawText(text, xOffset, 0, imageWidth, imageHeight, just);
//        }
//    }
////    if (glowOn) {
////        GlowEffect glow;
////        glow.setGlowProperties(glowRadius, glowColor);
////        glow.applyEffect(image, g, 1, 1);
////    }
////    if (glowOn && glowBehind) {
////        g.setColour(color);
////        g.drawText(text, xOffset, 0, imageWidth, imageHeight, just);
////    }
//    imageToTexture(image, myPointToStandard({paddedBoundary.left, paddedBoundary.top},
//                                            windowWidth, windowHeight));
//}
//
//void OpenGLTextBox::drawMultiLine(const int windowWidth,
//                                  const int windowHeight)
//{
//    const Box paddedBoundary = getScaled(boundary, horizontalBoundaryPad, verticalBoundaryPad);
//    const int imageWidth  = myLengthToStandard(paddedBoundary.getWidth(),  windowWidth);
//    const int imageHeight = myLengthToStandard(paddedBoundary.getHeight(), windowHeight);
//    
//    const auto linesAndFontSize = getLinesAndMaxFontSize(parseStringIntoWords(text.toStdString()),
//                                                         imageWidth, imageHeight);
//    const auto lines = linesAndFontSize.first;
//    const float fontSize = linesAndFontSize.second;
//    
//    // draw the now fitted text lines to the gl texture
//    Image image (Image::ARGB, imageWidth, imageHeight, true);
//    Graphics g (image);
//    g.setColour(color);
//    Font font (fontName, fontSize, fontStyle);
//    font.setExtraKerningFactor(kerningFactor);
//    g.setFont(font);
//    float yOffset = 0.5 * imageHeight / lines.size() * (1 - linePadFactor);
//    const int xOffset = std::round(horizontalOffsetFactor * font.getHeight());
//    for (const auto& line : lines) {
//        g.drawText(line, xOffset, yOffset, imageWidth, font.getHeight(), just);
//        yOffset += font.getHeight() / linePadFactor;
//    }
//    imageToTexture(image, myPointToStandard({paddedBoundary.left, paddedBoundary.top},
//                                            windowWidth, windowHeight));
//}
//    
//void OpenGLTextBox::setText(const String& newText) noexcept
//{
//    text = newText;
//    redrawToTexture = true;
//}
//    
//String OpenGLTextBox::getText() const noexcept
//{
//    return text;
//}
// 
//    
//    
//OpenGLTextBoxGroup::OpenGLTextBoxGroup(const StringArray& texts,
//                                       const Box& boundary,
//                                       const int numRows,
//                                       const int numColumns) noexcept
//: TexturedText(boundary), texts(texts), numRows(numRows), numColumns(numColumns)
//{
//    jassert(texts.size() == numRows * numColumns);
//}
//
//void OpenGLTextBoxGroup::drawBoundaries() const
//{
//    glColour(color);
//    boundary.draw();
//    glBegin(GL_LINES);
//    const float cellWidth = boundary.getWidth() / numColumns;
//    const float cellHeight = boundary.getHeight() / numRows;
//    float x, y;
//    for (int c = 1; c < numColumns; ++c) {
//        x = boundary.left + c * cellWidth;
//        glVertex2f(x, boundary.top);
//        glVertex2f(x, boundary.bottom);
//    }
//    for (int r = 1; r < numRows; ++r) {
//        y = boundary.top - r * cellHeight;
//        glVertex2f(boundary.left,  y);
//        glVertex2f(boundary.right, y);
//    }
//    glEnd();
//}
//    
//void OpenGLTextBoxGroup::drawSingleLine(const int windowWidth,
//                                        const int windowHeight)
//{
//    const int imageWidth  = myLengthToStandard(boundary.getWidth(),  windowWidth);
//    const int imageHeight = myLengthToStandard(boundary.getHeight(), windowHeight);
//    const float cellWidth  = static_cast<float>(imageWidth) / numColumns;
//    const float cellHeight = static_cast<float>(imageHeight) / numRows;
//    const float paddedCellWidth = cellWidth * horizontalBoundaryPad;
//    const float paddedCellHeight = cellHeight * verticalBoundaryPad;
//    Image image (Image::ARGB, imageWidth, imageHeight, true);
//    Graphics g (image);
//    g.setColour(color);
//    Font font (fontName, paddedCellHeight, fontStyle);
//    font.setExtraKerningFactor(kerningFactor);
//    float maxStringWidth = 0, stringWidth;
//    for (const auto& text : texts) {
//        stringWidth = font.getStringWidthFloat(text);
//        if (maxStringWidth < stringWidth)
//            maxStringWidth = stringWidth;
//    }
//    const float scale = std::min(paddedCellWidth / maxStringWidth, 1.0f);
//    font.setHeight(paddedCellHeight * scale);
//    g.setFont(font);
//    SpecialFontAttributes* specialFontAttr;
//    int xOffset, yOffset;
//    for (int r = 0; r < numRows; ++r) {
//        yOffset = std::round(cellHeight * (r + 0.5 * (1 - verticalBoundaryPad)));
//        for (int c = 0; c < numColumns; ++c) {
//            specialFontAttr = specialFonts.get(r*numColumns + c);
//            if (specialFontAttr != nullptr) {
//                const float normalWidth = font.getStringWidthFloat(texts[r*numColumns + c]);
//                const float normalHeight = font.getHeight();
//                font.setStyleFlags(specialFontAttr->fontStyle);
//                font.setHeight(font.getHeight() * specialFontAttr->relativeSize);
//                const float widthRatio = font.getStringWidthFloat(texts[r*numColumns + c]) / normalWidth;
//                const float heightRatio = font.getHeight() / normalHeight;
//                g.setFont(font);
//                g.setColour(specialFontAttr->color);
//                yOffset = std::round(cellHeight * (r + 0.5 * (1 - verticalBoundaryPad))
//                                     - paddedCellHeight * 0.5 * (heightRatio - 1));
//                xOffset = std::round(horizontalOffsetFactor * font.getHeight()
//                                     + cellWidth * (c + 0.5 * (1 - horizontalBoundaryPad))
//                                     - paddedCellWidth * 0.5 * (widthRatio - 1));
//                g.drawText(texts[r*numColumns + c], xOffset, yOffset,
//                           std::round(paddedCellWidth * widthRatio),
//                           std::round(paddedCellHeight * heightRatio),
//                           just);
//                font.setStyleFlags(fontStyle);
//                font.setHeight(normalHeight);
//                g.setFont(font);
//                g.setColour(color);
//                yOffset = std::round(cellHeight * (r + 0.5 * (1 - verticalBoundaryPad)));
//            } else {
//                xOffset = std::round(horizontalOffsetFactor * font.getHeight()
//                                     + cellWidth * (c + 0.5 * (1 - horizontalBoundaryPad)));
//                g.drawText(texts[r*numColumns + c], xOffset, yOffset,
//                           std::round(paddedCellWidth), std::round(paddedCellHeight), just);
//            }
//        }
//    }
//    imageToTexture(image, myPointToStandard({boundary.left, boundary.top},
//                                            windowWidth, windowHeight));
//}
//
//void OpenGLTextBoxGroup::drawMultiLine(const int windowWidth,
//                                       const int windowHeight)
//{
//    const int imageWidth  = myLengthToStandard(boundary.getWidth(),  windowWidth);
//    const int imageHeight = myLengthToStandard(boundary.getHeight(), windowHeight);
//    const float cellWidth  = static_cast<float>(imageWidth)  / numColumns;
//    const float cellHeight = static_cast<float>(imageHeight) / numRows;
//    const float paddedCellWidth = cellWidth * horizontalBoundaryPad;
//    const float paddedCellHeight = cellHeight * verticalBoundaryPad;
//    
//    // find the font size that will fit the largest text box to its cell
//    std::vector<std::vector<std::string>> cellsOfWords;
//    cellsOfWords.reserve(texts.size());
//    for (const auto& text : texts)
//        cellsOfWords.emplace_back(parseStringIntoWords(text.toStdString()));
//    float minFontSize = paddedCellHeight, fontSize;
//    for (const auto& words : cellsOfWords) {
//        fontSize = getLinesAndMaxFontSize(words, paddedCellWidth, paddedCellHeight).second;
//        if (minFontSize > fontSize)
//            minFontSize = fontSize;
//    }
//    
//    // vertical justification needs to take into account all lines, default is centered
//    std::function<float(int, float)> vertJustOffset = [&](int numLines, float fontHeight)
//        { return 0.5 * (paddedCellHeight - fontHeight * linePadFactor * numLines); };
//    if (just.getOnlyVerticalFlags() == Justification::top)
//        vertJustOffset = [](int, float){ return 0; };
//    else if (just.getOnlyVerticalFlags() == Justification::bottom)
//        vertJustOffset = [&](int numLines, float fontHeight)
//            { return paddedCellHeight - fontHeight * linePadFactor * numLines; };
//    
//    // draw the now fitted text lines to the gl texture
//    Image image (Image::ARGB, imageWidth, imageHeight, true);
//    Graphics g (image);
//    g.setColour(color);
//    Font font (fontName, minFontSize, fontStyle);
//    font.setExtraKerningFactor(kerningFactor);
//    g.setFont(font);
//    SpecialFontAttributes* specialFontAttr;
//    float xOffset, yOffset;
//    for (int r = 0; r < numRows; ++r) {
//        for (int c = 0; c < numColumns; ++c) {
//            const auto lines = getLinesWithFontSize(cellsOfWords[r*numColumns + c],
//                                                    paddedCellWidth, minFontSize);
//            specialFontAttr = specialFonts.get(r*numColumns + c);
//            if (specialFontAttr != nullptr) {
//                const float normalWidth = font.getStringWidthFloat(texts[r*numColumns + c]);
//                font.setStyleFlags(specialFontAttr->fontStyle);
//                font.setHeight(font.getHeight() * specialFontAttr->relativeSize);
//                const float widthRatio = font.getStringWidthFloat(texts[r*numColumns + c]) / normalWidth;
//                g.setFont(font);
//                g.setColour(specialFontAttr->color);
//                yOffset = cellHeight * (r + 0.5 * ((1 - linePadFactor) / lines.size()
//                                                   + (1 - verticalBoundaryPad)))
//                          + vertJustOffset(lines.size(), font.getHeight());
//                for (const auto& line : lines) {
//                    xOffset = cellWidth * (c + 0.5 * (1 - horizontalBoundaryPad))
//                              + horizontalOffsetFactor * font.getHeight()
//                              - paddedCellWidth * 0.5 * (widthRatio - 1);
//                    g.drawText(line, std::round(xOffset), std::round(yOffset),
//                               std::round(paddedCellWidth * widthRatio), std::round(font.getHeight()), just);
//                    yOffset += font.getHeight() / linePadFactor;
//                }
//                font.setStyleFlags(fontStyle);
//                font.setHeight(minFontSize);
//                g.setFont(font);
//                g.setColour(color);
//            } else {
//                yOffset = cellHeight * (r + 0.5 * ((1 - linePadFactor) / lines.size()
//                                                   + (1 - verticalBoundaryPad)))
//                          + vertJustOffset(lines.size(), minFontSize);
//                for (const auto& line : lines) {
//                    xOffset = cellWidth * (c + 0.5 * (1 - horizontalBoundaryPad))
//                              + horizontalOffsetFactor * font.getHeight();
//                    g.drawText(line, std::round(xOffset), std::round(yOffset),
//                               std::round(paddedCellWidth), std::round(font.getHeight()), just);
//                    yOffset += font.getHeight() / linePadFactor;
//                }
//            }
//        }
//    }
//    imageToTexture(image, myPointToStandard({boundary.left, boundary.top},
//                                            windowWidth, windowHeight));
//}
//
//void OpenGLTextBoxGroup::setTexts(const StringArray& newTexts,
//                                  const int newNumRows,
//                                  const int newNumColumns) noexcept
//{
//    jassert (newTexts.size() == newNumRows * newNumColumns);
//    texts           = newTexts;
//    numRows         = newNumRows;
//    numColumns      = newNumColumns;
//    redrawToTexture = true;
//}
//
//StringArray OpenGLTextBoxGroup::getTexts() const noexcept
//{
//    return texts;
//}
//    
//int OpenGLTextBoxGroup::getNumRows() const noexcept
//{
//    return numRows;
//}
//
//int OpenGLTextBoxGroup::getNumColumns() const noexcept
//{
//    return numColumns;
//}
//    
//void OpenGLTextBoxGroup::addSpecialFont(const int idNum,
//                                        const SpecialFontAttributes& specialFont) noexcept
//{
//    specialFonts.add(idNum, specialFont);
//    redrawToTexture = true;
//}
//    
//void OpenGLTextBoxGroup::removeSpecialFont(const int idNum) noexcept
//{
//    specialFonts.remove(idNum);
//    redrawToTexture = true;
//}
//    
//void OpenGLTextBoxGroup::setSpecialFont(const int idNum,
//                                        const SpecialFontAttributes& specialFont) noexcept
//{
//    auto special = specialFonts.get(idNum);
//    if (special != nullptr) {
//        *special = specialFont;
//        redrawToTexture = true;
//    }
//}
//    
//const OpenGLTextBoxGroup::SpecialFontAttributes* OpenGLTextBoxGroup::getSpecialFont(const int idNum) const noexcept
//{
//    return specialFonts.get(idNum);
//}
//
//    
//    
//OpenGLTextButton::OpenGLTextButton(const String& text,
//                                   const Box& boundary) noexcept
//    : textBox(text, boundary), color(textBox.getColor())
//{
//}
//    
//void OpenGLTextButton::draw(OpenGLWindow& glWindow,
//                            const Point<float>& mousePos)
//{
//    const Box boundary = textBox.getBoundary();
//    if (boundary.contains(mousePos) && mouseOverEnabled) {
//        if (!mouseOver)
//            mouseOverAnimation.restart();
//        mouseOver = true;
//    } else {
//        if (mouseOver)
//            mouseOverAnimation.restart();
//        mouseOver = false;
//    }
//    
//    const bool pressAnimationWasPlaying = pressAnimation.isPlaying();
//    if (pressAnimationWasPlaying) {
//        const float alphaFactor = 1.0 - pressAnimation.getProgress();
//        glColour(color.withAlpha(0.8f * alphaFactor));
//        boundary.drawFill();
//        pressAnimation.advance(glWindow.frameRate);
//    }
//    
//    if (mouseOver || mouseOverAnimation.isPlaying()) {
//        const float alphaFactor = mouseOver ? mouseOverAnimation.getProgress()
//                                      : 1.0 - mouseOverAnimation.getProgress();
//        glColour(color.withAlpha(0.4f * alphaFactor));
//        boundary.drawFill();
//        // mouse enter animation twice as fast as mouse exit
//        mouseOverAnimation.advance(glWindow.frameRate * (mouseOver ? 0.5 : 1));
//    }
//    
//    if ((showsState && buttonDown) || pressAnimation.isPlaying())
//        glLineWidth(2.0);
//
//    if (!showsState && pressAnimationWasPlaying
//                    && !pressAnimation.isPlaying())
//        textBox.setFontStyle(Font::FontStyleFlags::plain);
//    
//    glColour(color);
//    boundary.draw();
//    textBox.draw(glWindow);
//    
//    glLineWidth(1.0);
//}
//    
//bool OpenGLTextButton::mouseClicked() noexcept
//{
//    if (mouseOver)
//        press();
//    return mouseOver;
//}
//
//void OpenGLTextButton::press() noexcept
//{
//    buttonDown = !buttonDown;
//    if (buttonDown || !showsState)
//        textBox.setFontStyle(Font::FontStyleFlags::bold);
//    else
//        textBox.setFontStyle(Font::FontStyleFlags::plain);
//    pressAnimation.restart();
//}
//
//bool OpenGLTextButton::isDown() const noexcept
//{
//    return buttonDown;
//}
//
//bool OpenGLTextButton::isMouseOver() const noexcept
//{
//    return mouseOver;
//}
//    
//void OpenGLTextButton::setBoundary(const Box& newBoundary) noexcept
//{
//    textBox.setBoundary(newBoundary);
//}
//
//Box OpenGLTextButton::getBoundary() const noexcept
//{
//    return textBox.getBoundary();
//}
//
//const Animation& OpenGLTextButton::getPressAnimation() const noexcept
//{
//    return pressAnimation;
//}
//
//const Animation& OpenGLTextButton::getMouseOverAnimation() const noexcept
//{
//    return mouseOverAnimation;
//}
//    
//
//        
//OpenGLTextTabs::OpenGLTextTabs(const StringArray& texts,
//                               const Box& boundary,
//                               const Orientation orientation) noexcept
//    : textBoxGroup(texts, boundary,
//                   (orientation == ABOVE || orientation == BELOW) ? 1 : texts.size(),
//                   (orientation == ABOVE || orientation == BELOW) ? texts.size() : 1),
//      color(textBoxGroup.getColor()),
//      orientation(orientation)
//{
//    // make some space for the bold font and slanted tab edges
//    const auto padding = textBoxGroup.getBoundaryPadding();
//    if (orientation == ABOVE || orientation == BELOW) {
//        textBoxGroup.setBoundaryPadding(padding[0]*0.96 - 0.5 * gapFactor, padding[1]);
//    } else { // if (orientation == LEFT || orientation == RIGHT)
//        textBoxGroup.setBoundaryPadding(padding[0]*0.96, padding[1] - 0.5 * gapFactor);
//    }
//    setBoundary(boundary); // set textBoxGroup's boundary with tab bar end gaps taken into account
//    textBoxGroup.addSpecialFont(selected, {}); // selected tab's font in bold
//}
//
//void OpenGLTextTabs::draw(OpenGLWindow& glWindow,
//                          const Point<float>& mousePos)
//{
//    // compute dimensions of tabs for below
//    const Box boundary = getBoundary();
//    const float widthOrHeight = (orientation == ABOVE || orientation == BELOW)
//                                ? boundary.getWidth() : boundary.getHeight();
//    const int numTabs = textBoxGroup.getTexts().size();
//    const float gapSpace = widthOrHeight * gapFactor / (numTabs + 2);
//    const float halfGap = gapSpace * 0.5;
//    const float tabSpace = widthOrHeight * (1 - gapFactor) / numTabs;
//    const int prevMouseOver = mouseOver;
//    
//    // detect which tab mouse is over, if any
//    if (boundary.contains(mousePos) && mouseOverEnabled) {
//        float lowA, highA, b1, b2, mouseA, mouseB, alpha, baseB, sign;
//        if (orientation == ABOVE || orientation == BELOW) {
//            lowA = boundary.bottom;
//            highA = boundary.top;
//            mouseA = mousePos.y;
//            mouseB = mousePos.x;
//            baseB = boundary.left + halfGap;
//            sign = +1;
//        } else {
//            lowA = boundary.left;
//            highA = boundary.right;
//            mouseA = mousePos.x;
//            mouseB = mousePos.y;
//            baseB = boundary.top - halfGap;
//            sign = -1;
//        }
//        if (orientation == ABOVE || orientation == RIGHT)
//            alpha = (mouseA - lowA) / (highA - lowA);
//        else
//            alpha = (highA - mouseA) / (highA - lowA);
//        for (int i = 0; i < numTabs; ++i) {
//            if (i != selected) {
//                const float b = baseB + sign * i * (tabSpace + gapSpace);
//                if (i == 0)
//                    b1 = b + sign * alpha * gapSpace;
//                else
//                    b1 = b + sign * gapSpace * (0.5 + std::abs(alpha - 0.5));
//                if (i+1 == selected)
//                    b2 = b + sign * (tabSpace + gapSpace * (1.5 - std::abs(alpha - 0.5)));
//                else
//                    b2 = b + sign * (tabSpace + gapSpace * (2 - alpha));
//                if (std::min(b1, b2) <= mouseB && mouseB <= std::max(b1, b2)) {
//                    mouseOver = i;
//                    goto MOUSE_OVER_TAB;
//                }
//            }
//        }
//    }
//    // mouse is not currently over a tab if we get here
//    mouseOver = -1;
//    
//MOUSE_OVER_TAB:
//    
//    // if mouse exits or enters over an unselected tab, add/refresh the mouse over animations
//    if (prevMouseOver != mouseOver) {
//        if (mouseOver >= 0 && mouseOver != selected) { // mouse enter animation 2x mouse exit
//            mouseOverAnimations.add(mouseOver, mouseOverAnimationDuration * 0.5);
//            mouseOverAnimations.get(mouseOver)->restart();
//        }
//        if (prevMouseOver >= 0 && prevMouseOver != selected) {
//            mouseOverAnimations.add(prevMouseOver, mouseOverAnimationDuration);
//            mouseOverAnimations.get(prevMouseOver)->restart();
//        }
//    }
//    mousePrev = mousePos;
//    
//    // draw dark shading over tab bar (so any background objects are visible, but darkened)
//    glColor4f(0.0, 0.0, 0.0, 0.85);
//    boundary.drawFill();
//    
//    // draw selected tab
//    const float offset = selected * (tabSpace + gapSpace) + halfGap;
//    float in, out;
//    glColour(color);
//    glLineWidth(2.0);
//    glBegin(GL_LINE_STRIP);
//    if (orientation == ABOVE || orientation == BELOW) {
//        float x = boundary.left;
//        if (orientation == ABOVE) {
//            in = boundary.bottom;
//            out = boundary.top;
//        } else {
//            in = boundary.top;
//            out = boundary.bottom;
//        }
//        glVertex2f(x, in);     x += offset;
//        glVertex2f(x, in);     x += gapSpace;
//        glVertex2f(x, out);    x += tabSpace;
//        glVertex2f(x, out);    x += gapSpace;
//        glVertex2f(x, in);
//        glVertex2f(boundary.right, in);
//    } else {
//        float y = boundary.top;
//        if (orientation == LEFT) {
//            in = boundary.right;
//            out = boundary.left;
//        } else {
//            in = boundary.left;
//            out = boundary.right;
//        }
//        glVertex2f(in, y);     y -= offset;
//        glVertex2f(in, y);     y -= gapSpace;
//        glVertex2f(out, y);    y -= tabSpace;
//        glVertex2f(out, y);    y -= gapSpace;
//        glVertex2f(in, y);
//        glVertex2f(in, boundary.bottom);
//    }
//    glEnd();
//    glLineWidth(1.0);
//    
//    // draw nonselected tabs
//    const float mid = (in + out) * 0.5;
//    float across;
//    if (orientation == ABOVE || orientation == BELOW)
//        across = boundary.left + halfGap;
//    else
//        across = boundary.top - halfGap;
//    Animation* mouseOverAnimation;
//    for (int i = 0; i < numTabs; ++i) {
//        // draw some shading over the hovered over tab
//        mouseOverAnimation = mouseOverAnimations.get(i);
//        if (i != selected && (i == mouseOver || mouseOverAnimation != nullptr)) {
//            float alpha = 0.4;
//            if (mouseOverAnimation != nullptr) {
//                if (i == mouseOver)
//                    alpha *= mouseOverAnimation->getProgress();
//                else
//                    alpha *= 1 - mouseOverAnimation->getProgress();
//                mouseOverAnimation->advance(glWindow.frameRate);
//                if (! mouseOverAnimation->isPlaying())
//                    mouseOverAnimations.remove(i);
//            }
//            glColour(color.withAlpha(alpha));
//            if (orientation == ABOVE || orientation == BELOW) {
//                if (i == 0) {
//                    if (i+1 == selected) {
//                        glBegin(GL_QUADS);
//                        glVertex2f(across, in);  across += gapSpace;
//                        glVertex2f(across, out); across += tabSpace;
//                        glVertex2f(across, out);
//                        glVertex2f(across, in);
//                        glEnd();
//                        glBegin(GL_TRIANGLES);
//                        glVertex2f(across, in);
//                        glVertex2f(across, out); across += halfGap;
//                        glVertex2f(across, mid);
//                        glEnd();    across -= tabSpace + 1.5*gapSpace;
//                    } else {
//                        glBegin(GL_QUADS);
//                        glVertex2f(across, in);  across += gapSpace;
//                        glVertex2f(across, out); across += tabSpace;
//                        glVertex2f(across, out); across += gapSpace;
//                        glVertex2f(across, in);
//                        glEnd();    across -= tabSpace + 2*gapSpace;
//                    }
//                } else {
//                    if (i+1 == selected) {
//                        across -= halfGap;
//                        glBegin(GL_QUADS);
//                        glVertex2f(across, mid); across += halfGap;
//                        glVertex2f(across, out); across += tabSpace;
//                        glVertex2f(across, out); across += halfGap;
//                        glVertex2f(across, mid);
//                        glVertex2f(across, mid); across -= halfGap;
//                        glVertex2f(across, in);  across -= tabSpace;
//                        glVertex2f(across, in);  across -= halfGap;
//                        glVertex2f(across, mid);
//                        across += halfGap;
//                        glEnd();
//                    } else {
//                        across -= halfGap;
//                        glBegin(GL_TRIANGLES);
//                        glVertex2f(across, mid); across += halfGap;
//                        glVertex2f(across, out);
//                        glVertex2f(across, in);
//                        glEnd();
//                        glBegin(GL_QUADS);
//                        glVertex2f(across, in);
//                        glVertex2f(across, out); across += tabSpace;
//                        glVertex2f(across, out); across += gapSpace;
//                        glVertex2f(across, in);
//                        glEnd();
//                        across -= tabSpace + gapSpace;
//                    }
//                }
//            } else { // if (orientation == LEFT || orientation == RIGHT)
//                if (i == 0) {
//                    if (i + 1 == selected) {
//                        glBegin(GL_QUADS);
//                        glVertex2f(in,  across); across -= gapSpace;
//                        glVertex2f(out, across); across -= tabSpace;
//                        glVertex2f(out, across);
//                        glVertex2f(in,  across);
//                        glEnd();
//                        glBegin(GL_TRIANGLES);
//                        glVertex2f(in,  across);
//                        glVertex2f(out, across); across -= halfGap;
//                        glVertex2f(mid, across);
//                        glEnd();    across += tabSpace + 1.5*gapSpace;
//                    } else {
//                        glBegin(GL_QUADS);
//                        glVertex2f(in,  across); across -= gapSpace;
//                        glVertex2f(out, across); across -= tabSpace;
//                        glVertex2f(out, across); across -= gapSpace;
//                        glVertex2f(in,  across);
//                        glEnd();    across += tabSpace + 2*gapSpace;
//                    }
//                } else {
//                    if (i+1 == selected) {
//                        across += halfGap;
//                        glBegin(GL_QUADS);
//                        glVertex2f(mid, across); across -= halfGap;
//                        glVertex2f(out, across); across -= tabSpace;
//                        glVertex2f(out, across); across -= halfGap;
//                        glVertex2f(mid, across);
//                        glVertex2f(mid, across); across += halfGap;
//                        glVertex2f(in,  across); across += tabSpace;
//                        glVertex2f(in,  across); across += halfGap;
//                        glVertex2f(mid, across);
//                        across -= halfGap;
//                        glEnd();
//                    } else {
//                        across += halfGap;
//                        glBegin(GL_TRIANGLES);
//                        glVertex2f(mid, across); across -= halfGap;
//                        glVertex2f(out, across);
//                        glVertex2f(in,  across);
//                        glEnd();
//                        glBegin(GL_QUADS);
//                        glVertex2f(in,  across);
//                        glVertex2f(out, across); across -= tabSpace;
//                        glVertex2f(out, across); across -= gapSpace;
//                        glVertex2f(in,  across);
//                        glEnd();
//                        across += tabSpace + gapSpace;
//                    }
//                }
//            }
//        }
//        // draw mouse click animation
//        if (i == selected && selectAnimation.isPlaying()) {
//            float prevAcross = across;
//            if (i > 0) {
//                if (orientation == ABOVE || orientation == BELOW)
//                    across -= halfGap;
//                else
//                    across += halfGap;
//            }
//            const float prog = selectAnimation.getProgress();
//            const float alpha = 0.8 * (1 - 0.5*prog);
//            selectAnimation.advance(glWindow.frameRate);
//            if (orientation == ABOVE || orientation == BELOW) {
//                const float mX = selectAnimation.mouse.x;
//                const float midAIn = prog * across + (1 - prog) * mX;
//                const float midAOut = prog * (across + gapSpace) + (1 - prog) * mX;
//                const float midBIn = prog * (across + 2*gapSpace + tabSpace) + (1 - prog) * mX;
//                const float midBOut = prog * (across + gapSpace + tabSpace) + (1 - prog) * mX;
//                glBegin(GL_QUADS);
//                glColour(color.withAlpha(alpha));
//                glVertex2f(across, in);  across += gapSpace;
//                glVertex2f(across, out);
//                glColour(color.withAlpha(0.0f));
//                glVertex2f(midAOut, out);
//                glVertex2f(midAIn, in);
//                glVertex2f(midBIn, in);
//                glVertex2f(midBOut, out); across += tabSpace;
//                glColour(color.withAlpha(alpha));
//                glVertex2f(across, out);  across += gapSpace;
//                glVertex2f(across, in);
//                glEnd();
//                glLineWidth(2);
//                glBegin(GL_LINES);
//                glColour(color);
//                glVertex2f(across, in);
//                glColour(color.withAlpha(0.0f));
//                glVertex2f(midBIn, in);
//                glVertex2f(midAIn, in);   across -= tabSpace + 2*gapSpace;
//                glColour(color);
//                glVertex2f(across, in);
//                glEnd();
//                glLineWidth(1);
//            } else { // if (orientation == LEFT || orientation == RIGHT)
//                const float mY = selectAnimation.mouse.y;
//                const float midAIn = prog * across + (1 - prog) * mY;
//                const float midAOut = prog * (across - gapSpace) + (1 - prog) * mY;
//                const float midBIn = prog * (across - 2*gapSpace - tabSpace) + (1 - prog) * mY;
//                const float midBOut = prog * (across - gapSpace - tabSpace) + (1 - prog) * mY;
//                glBegin(GL_QUADS);
//                glColour(color.withAlpha(alpha));
//                glVertex2f(in, across);  across -= gapSpace;
//                glVertex2f(out, across);
//                glColour(color.withAlpha(0.0f));
//                glVertex2f(out, midAOut);
//                glVertex2f(in,  midAIn);
//                glVertex2f(in,  midBIn);
//                glVertex2f(out, midBOut); across -= tabSpace;
//                glColour(color.withAlpha(alpha));
//                glVertex2f(out, across);  across -= gapSpace;
//                glVertex2f(in, across);
//                glEnd();
//                glLineWidth(2);
//                glBegin(GL_LINES);
//                glColour(color);
//                glVertex2f(in, across);
//                glColour(color.withAlpha(0.0f));
//                glVertex2f(in, midBIn);
//                glVertex2f(in, midAIn);   across += tabSpace + 2*gapSpace;
//                glColour(color);
//                glVertex2f(in, across);
//                glEnd();
//                glLineWidth(1);
//            }
//            across = prevAcross;
//        }
//        // draw the normal, unselected tab
//        if (i != selected) {
//            glColour(color);
//            glBegin(GL_LINE_STRIP);
//            if (orientation == ABOVE || orientation == BELOW) {
//                if (i == 0) {
//                    glVertex2f(across, in); across += gapSpace;
//                } else {
//                    across -= halfGap;
//                    glVertex2f(across, mid);
//                    across += halfGap;
//                }
//                glVertex2f(across, out);    across += tabSpace;
//                glVertex2f(across, out);
//                if (i+1 < numTabs && i+1 == selected) {
//                    across += halfGap;
//                    glVertex2f(across, mid);
//                } else {
//                    across += gapSpace;
//                    glVertex2f(across, in);
//                }
//            } else {
//                if (i == 0) {
//                    glVertex2f(in, across); across -= gapSpace;
//                } else {
//                    across += halfGap;
//                    glVertex2f(mid, across);
//                    across -= halfGap;
//                }
//                glVertex2f(out, across);    across -= tabSpace;
//                glVertex2f(out, across);
//                if (i+1 < numTabs && i+1 == selected) {
//                    across -= halfGap;
//                    glVertex2f(mid, across);
//                } else {
//                    across -= gapSpace;
//                    glVertex2f(in, across);
//                }
//            }
//            glEnd();
//        } else { // skip over selected tab cuz we already drew it above
//            if (orientation == ABOVE || orientation == BELOW)
//                across += tabSpace + ((selected == 0) ? 2 : 1.5) * gapSpace;
//            else
//                across -= tabSpace + ((selected == 0) ? 2 : 1.5) * gapSpace;
//        }
//    }
//    
//    // draw the text labels, selected in bold
//    glColour(color);
//    textBoxGroup.draw(glWindow);
//}
//
//int OpenGLTextTabs::mouseClicked() noexcept
//{
//    if (mouseOver != -1) {
//        setSelected(mouseOver);
//    }
//    return mouseOver;
//}
//
//void OpenGLTextTabs::setSelected(const int newSelected,
//                                 const bool enableAnimation) noexcept
//{
//    textBoxGroup.removeSpecialFont(selected);
//    textBoxGroup.addSpecialFont(newSelected, {});
//    if (enableAnimation && selected != newSelected) {
//        selectAnimation.restart();
//        if (mouseOver >= 0 && newSelected == mouseOver)
//            selectAnimation.mouse = mousePrev;
//        else { // find midpoint of tab for animation
//            const int numTabs = textBoxGroup.getTexts().size();
//            if (orientation == ABOVE || orientation == BELOW) {
//                const float gapSpace = getBoundary().getWidth() * gapFactor / (numTabs + 2);
//                const float tabSpace = getBoundary().getWidth() * (1 - gapFactor) / numTabs;
//                const float begin = getBoundary().left
//                                    + ((newSelected == 0) ? gapSpace * 0.5 : 0)
//                                    + newSelected * (tabSpace + gapSpace);
//                selectAnimation.mouse.x = begin + 0.5 * (2*gapSpace + tabSpace);
//            } else {
//                const float gapSpace = getBoundary().getHeight() * gapFactor / (numTabs + 2);
//                const float tabSpace = getBoundary().getHeight() * (1 - gapFactor) / numTabs;
//                const float begin = getBoundary().top
//                                    - ((newSelected == 0) ? gapSpace * 0.5 : 0)
//                                    - newSelected * (tabSpace + gapSpace);
//                selectAnimation.mouse.y = begin - 0.5 * (2*gapSpace + tabSpace);
//            }
//        }
//    }
//    selected = newSelected;
//}
//    
//int OpenGLTextTabs::getSelected() const noexcept
//{
//    return selected;
//}
//    
//void OpenGLTextTabs::setBoundary(const Box& newBoundary) noexcept
//{
//    boundary = newBoundary;
//    const int numTabs = textBoxGroup.getTexts().size();
//    if (orientation == ABOVE || orientation == BELOW) {
//        const float gapSpace = boundary.getWidth() * gapFactor / (numTabs + 2);
//        textBoxGroup.setBoundary(getScaled(newBoundary, 1 - 2 * gapSpace / boundary.getWidth(), 1));
//    } else {
//        const float gapSpace = boundary.getHeight() * gapFactor / (numTabs + 2);
//        textBoxGroup.setBoundary(getScaled(newBoundary, 1, 1 - 2 * gapSpace / boundary.getHeight()));
//    }
//}
//
//Box OpenGLTextTabs::getBoundary() const noexcept
//{
//    return boundary;
//}
//    
//    
//    
//OpenGLTextRadioButton::OpenGLTextRadioButton(const StringArray& texts,
//                                             const Box& boundary,
//                                             const int numRows,
//                                             const int numColumns,
//                                             const int autoDetect) noexcept
//    : options(texts, boundary, numRows, numColumns),
//      //color(options.getColor()),
//      autoDetect(autoDetect)
//{
//    options.addSpecialFont(selected, selectedFont);
//    mouseOverFont.color = selectedFont.color.withAlpha(0.7f);
//}
//    
//void OpenGLTextRadioButton::draw(OpenGLWindow& glWindow, const Point<float>& mousePos)
//{
//    // see if mouse is hovering over any of the options and which one if so
//    setMouseOver(mousePos);
//    // draw the radio options
//    options.draw(glWindow);
//}
//
//int OpenGLTextRadioButton::mouseClicked() noexcept
//{
//    if (mouseOver >= 0)
//        setSelected(mouseOver);
//    return mouseOver;
//}
//    
//void OpenGLTextRadioButton::setSelected(const int newSelected) noexcept
//{
//    if (selected != newSelected) {
//        if (selected == autoDetect)
//            options.removeSpecialFont(autoDetected);
//        options.removeSpecialFont(selected);
//        selected = newSelected;
//        if (selected == autoDetect)
//            options.addSpecialFont(autoDetected, selectedFont);
//        options.addSpecialFont(selected, selectedFont);
//    }
//}
//
//int OpenGLTextRadioButton::getSelected() const noexcept
//{
//    return selected;
//}
//    
//void OpenGLTextRadioButton::setMouseOver(const Point<float>& mousePos) noexcept
//{
//    const Box& boundary = options.getBoundary();
//    const int prevMouseOver = mouseOver;
//    if (boundary.contains(mousePos)) {
//        const int numColumns = options.getNumColumns();
//        const int numRows = options.getNumRows();
//        const float dW = boundary.getWidth() / numColumns;
//        const float dH = boundary.getHeight() / numRows;
//        Box b = {boundary.top,  boundary.top - dH,
//            boundary.left, boundary.left + dW};
//        for (int r = 0; r < numRows; ++r) {
//            for (int c = 0; c < numColumns; ++c) {
//                if (b.contains(mousePos)) {
//                    mouseOver = r * numColumns + c;
//                    goto MOUSE_OVER_OPTION;
//                }
//                b.left  += dW;
//                b.right += dW;
//            }
//            b.left  = boundary.left;
//            b.right = b.left + dW;
//            b.top    -= dH;
//            b.bottom -= dH;
//        }
//    }
//    mouseOver = -1;
//    
//MOUSE_OVER_OPTION:
//    
//    // adjust the font look if mouse changes hovered over option
//    if (mouseOver != prevMouseOver) {
//        if (prevMouseOver != selected) {
//            if (autoDetect > 0 && prevMouseOver == autoDetected)
//                options.setSpecialFont(prevMouseOver, selectedFont);
//            else
//                options.removeSpecialFont(prevMouseOver);
//        }
//        if (mouseOver != selected)
//            options.addSpecialFont(mouseOver, mouseOverFont);
//    }
//}
//
//int OpenGLTextRadioButton::getMouseOver() const noexcept
//{
//    return mouseOver;
//}
//    
//void OpenGLTextRadioButton::setAutoDetected(const int newAutoDetected) noexcept
//{
//    if (autoDetected != newAutoDetected) {
//        if (selected == autoDetect) {
//            options.removeSpecialFont(autoDetected);
//            options.addSpecialFont(newAutoDetected, selectedFont);
//        }
//        autoDetected = newAutoDetected;
//    }
//}
//
//int OpenGLTextRadioButton::getAutoDetected() const noexcept
//{
//    return autoDetected;
//}
//    
//void OpenGLTextRadioButton::setColor(const Colour& color) noexcept
//{
//    options.setColor(color);
//    selectedFont.color = color;
//    options.setSpecialFont(selected, selectedFont);
//    if (selected == autoDetect)
//        options.setSpecialFont(autoDetected, selectedFont);
//    mouseOverFont.color = color.withAlpha(color.getFloatAlpha() * 0.7f);
//    options.setSpecialFont(mouseOver, mouseOverFont);
//}
//    
//    
//    
//TitledRadioButton::TitledRadioButton(const String& titleText,
//                                     const Box& titleBoundary,
//                                     const StringArray& optionsTexts,
//                                     const Box& optionsBoundary,
//                                     const int optionsRows,
//                                     const int optionsColumns,
//                                     const int autoDetect) noexcept
//    : OpenGLTextRadioButton(optionsTexts, optionsBoundary, optionsRows, optionsColumns, autoDetect),
//      title(titleText, titleBoundary)
//{
//}
//    
//void TitledRadioButton::draw(OpenGLWindow& glWindow, const Point<float>& mousePos)
//{
//    OpenGLTextRadioButton::draw(glWindow, mousePos);
//    title.draw(glWindow);
//}
//    
//void TitledRadioButton::setColor(const Colour& color) noexcept
//{
//    OpenGLTextRadioButton::setColor(color);
//    title.setColor(color);
//}
    
    
    
//// ****************************************************************************************************
//float GLUTRoman::length(const std::string& str) const
//{
//    // 1.01 is b/c glutStrokeLength() seems to be slightly off
//    return glutStrokeLength(GLUT_STROKE_ROMAN, (const unsigned char*)(str.c_str())) * 1.01;
//}
//
//// from: https://www.opengl.org/resources/libraries/glut/spec3/node78.html
//float GLUTRoman::topMax() const { return 119.05; }
//float GLUTRoman::bottomMax() const { return 33.33; }
//
//void GLUTRoman::drawString(const std::string& str, const Point2D& pos, float aspect, float scale) const
//{
//    glPushMatrix();
//    glTranslatef(pos.x, pos.y, 0);
//    glScalef(aspect*scale, scale, 1.0);
//    for (int i = 0; i < str.length(); ++i)
//        glutStrokeCharacter(GLUT_STROKE_ROMAN, str[i]);
//    glPopMatrix();
//}
//
//    
//    
//TextBox::TextBox(const std::string& txt, const Box& bounds)
//    : text(txt) 
//{
//    boundary = bounds;
//}
//
//void TextBox::drawText(const float aspectRatio)
//{
//    aspect = aspectRatio;
////    const float boxWidth  = boundary.right - boundary.left;
////    const float boxHeight = boundary.top - boundary.bottom;
////    const float textLength = font->length(text) * aspect;
////    const float textHeight = font->totalHeight();
////    const float hScale = boxWidth  / textLength * hPad;
////    const float vScale = boxHeight / textHeight * vPad;
////    const float scale = std::min(hScale, vScale);
////    float xOffset, yOffset;
////    switch (hJust) {
////        case Just::TOP_LEFT:
////            xOffset = 0.5*(1.0-hPad) * boxWidth;
////            break;
////        case Just::CENTER:
////            xOffset = 0.5 * (boxWidth - scale * textLength);
////            break;
////        case Just::BOTTOM_RIGHT:
////            xOffset = (hPad + 0.5*(1.0-hPad)) * boxWidth - scale * textLength;
////            break;
////    }
////    switch (vJust) {
////        case Just::TOP_LEFT:
////            yOffset = (vPad + 0.5*(1.0-vPad)) * boxHeight - scale * textHeight;
////            break;
////        case Just::CENTER:
////            yOffset = 0.5 * (boxHeight - scale * textHeight);
////            break;
////        case Just::BOTTOM_RIGHT:
////            yOffset = 0.5*(1.0-vPad) * boxHeight;
////            break;
////    }
////    const float xPos = boundary.left + xOffset;
////    const float yPos = boundary.bottom + scale*font->bottomMax() + yOffset;
////    font->drawString(text, {xPos, yPos}, aspect, scale);
//    auto posAndScale = getBeginPositionAndScale();
//    font->drawString(text, {posAndScale[0], posAndScale[1]}, aspect, posAndScale[2]);
//}
//
//std::array<float, 3> TextBox::getBeginPositionAndScale() const noexcept
//{
//    const float boxWidth  = boundary.right - boundary.left;
//    const float boxHeight = boundary.top - boundary.bottom;
//    const float textLength = font->length(text) * aspect;
//    const float textHeight = font->totalHeight();
//    const float hScale = boxWidth  / textLength * hPad;
//    const float vScale = boxHeight / textHeight * vPad;
//    const float scale = std::min(hScale, vScale);
//    float xOffset, yOffset;
//    switch (hJust) {
//        case Just::TOP_LEFT:
//            xOffset = 0.5*(1.0-hPad) * boxWidth;
//            break;
//        case Just::CENTER:
//            xOffset = 0.5 * (boxWidth - scale * textLength);
//            break;
//        case Just::BOTTOM_RIGHT:
//            xOffset = (hPad + 0.5*(1.0-hPad)) * boxWidth - scale * textLength;
//            break;
//    }
//    switch (vJust) {
//        case Just::TOP_LEFT:
//            yOffset = (vPad + 0.5*(1.0-vPad)) * boxHeight - scale * textHeight;
//            break;
//        case Just::CENTER:
//            yOffset = 0.5 * (boxHeight - scale * textHeight);
//            break;
//        case Just::BOTTOM_RIGHT:
//            yOffset = 0.5*(1.0-vPad) * boxHeight;
//            break;
//    }
//    const float xStart = boundary.left + xOffset;
//    const float yStart = boundary.bottom + scale*font->bottomMax() + yOffset;
//    return {xStart, yStart, scale};
//}
//    
//    
//    
//TextBoxGroup::TextBoxGroup(const std::vector<std::string>& txts, const Box& bounds, int numRows, int numColumns)
//    : texts(txts), rows(numRows), columns(numColumns)
//{
//    assert(texts.size() == rows * columns);
//    boundary = bounds;
//}
//
//void TextBoxGroup::drawText(const float aspectRatio)
//{
//    drawText(aspectRatio, {}, [](bool){});
//}
//
//void TextBoxGroup::drawText(const float aspectRatio,
//                            const std::vector<int>& specialCells,
//                            const std::function<void(bool)>& special)
//{
//    if (aspectRatio != aspect || displayList == 0) {
//    aspect = aspectRatio;
//    const float boundaryWidth  = boundary.right - boundary.left;
//    const float cellWidth = boundaryWidth / columns;
//    const float boundaryHeight = boundary.top - boundary.bottom;
//    const float cellHeight = boundaryHeight / rows;
//    float maxTextLength = 0;
//    float* textLengths = static_cast<float*>(alloca(texts.size() * sizeof(float)));
//    int i = 0;
//    for (const auto& line : texts) {
//        textLengths[i] = font->length(line) * aspect;
//        if (textLengths[i] > maxTextLength)
//            maxTextLength = textLengths[i];
//        ++i;
//    }
//    const float textHeight = font->totalHeight();
//    const float hScale = cellWidth  / maxTextLength * hPad;
//    const float vScale = cellHeight / textHeight * vPad;
//    const float scale = std::min(hScale, vScale);
//    i = 0;
//    float xOffset, yOffset;
//    switch (vJust) {
//        case Just::TOP_LEFT:
//            yOffset = (vPad + 0.5*(1.0-vPad)) * cellHeight - scale * textHeight;
//            break;
//        case Just::CENTER:
//            yOffset = 0.5 * (cellHeight - scale * textHeight);
//            break;
//        case Just::BOTTOM_RIGHT:
//            yOffset = 0.5*(1.0-vPad) * cellHeight;
//            break;
//    }
//    yOffset += scale * font->bottomMax();
//    glDeleteLists(displayList, 1);
//    displayList = glGenLists(1);
//    glNewList(displayList, GL_COMPILE_AND_EXECUTE);
//    for (int r = 1; r <= rows; ++r) {
//        for (int c = 0; c < columns; ++c) {
////    for (int c = 0; c < columns; ++c) {
////        for (int r = 1; r <= rows; ++r) {
//            switch (hJust) {
//                case Just::TOP_LEFT:
//                    xOffset = 0.5*(1.0-hPad) * cellWidth;
//                    break;
//                case Just::CENTER:
//                    xOffset = 0.5 * (cellWidth - scale * textLengths[i]);
//                    break;
//                case Just::BOTTOM_RIGHT:
//                    xOffset = (hPad + 0.5*(1.0-hPad)) * cellWidth - scale * textLengths[i];
//                    break;
//            }
//            const Point2D pos {boundary.left + c * cellWidth + xOffset,
//                               boundary.top - (r * cellHeight - yOffset)};
//            const bool doSpecial = std::find(std::begin(specialCells), std::end(specialCells), (r-1)*columns+c/*(r-1)+c*rows*/) != std::end(specialCells);
//            if (doSpecial)
//                special(true);
//            font->drawString(texts[i], pos, aspect, scale);
//            if (doSpecial)
//                special(false);
//            ++i;
//        }
//    }
//        glEndList();
//    }
//    else
//        glCallList(displayList);
//}
//
//void TextBoxGroup::drawBoundaries() const
//{
//    boundary.draw();
//    const float boundaryWidth  = boundary.right - boundary.left;
//    const float cellWidth = boundaryWidth / columns;
//    const float boundaryHeight = boundary.top - boundary.bottom;
//    const float cellHeight = boundaryHeight / rows;
//    float tmp;
//    glBegin(GL_LINES);
//    for (int r = 1; r <= rows; ++r) {
//        tmp = boundary.bottom + r * cellHeight;
//        glVertex2f(boundary.left,  tmp);
//        glVertex2f(boundary.right, tmp);
//    }
//    for (int c = 1; c <= columns; ++c) {
//        tmp = boundary.left + c * cellWidth;
//        glVertex2f(tmp, boundary.bottom);
//        glVertex2f(tmp, boundary.top);
//    }
//    glEnd();
//}
//    
//int TextBoxGroup::getNumRows() const noexcept
//{
//    return rows;
//}
//int TextBoxGroup::getNumColumns() const noexcept
//{
//    return columns;
//}
//
//int TextBoxGroup::getSize() const noexcept
//{
//    return texts.size();
//}
//
//    
//
//MultiLineTextBox::MultiLineTextBox(const std::string& text, const Box& bounds)
//{
//    boundary = bounds;
//    hJust = Just::TOP_LEFT; // horizontally left justified seems like a more fitting default than centered.
//    loadText(text);
//}
//
//// parses string of text into words separated by white space.  the white spaces are saved into words as well to preserve the length of the white space.
//void MultiLineTextBox::loadText(const std::string& text)
//{
//    words.clear();
//    int begin = 0;
//    for (int i = 0; i < text.size(); ++i) {
//        if (text[i] == ' ') { // if we found the ending of a non-whitespace word
//            if (i > begin) { // don't skip loading first word if it is white space
//                words.emplace_back(text.substr(begin, i-begin));
//            }
//            for (int j = i+1; j <= text.size(); ++j) { // load white space words
//                if (j < text.size() && text[j] != ' ') {
//                    words.emplace_back(text.substr(i, j-i));
//                    begin = j;
//                    i = j-1;
//                    j = text.size(); // exit for j loop
//                } else if (j == text.size()) { // load last word if it is white space
//                    words.emplace_back(text.substr(i, text.size()-i));
//                    i = j-1;
//                }
//            }
//        } else if (i == text.size()-1) { // load last non-whitespace word
//            words.emplace_back(text.substr(begin, text.size()-begin));
//        }
//    }
//}
//
//void MultiLineTextBox::drawText(const float aspectRatio)
//{
//    aspect = aspectRatio;
//
//    float totalLength = 0.0;
//    for (const auto& w : words)
//        totalLength += font->length(w);
//    totalLength *= aspect;
//
//    const float textHeight = font->totalHeight();
//    const float linePad = 0.9;
//    const float initArea = totalLength * textHeight/linePad;
//    float textScale = (boundary.top - boundary.bottom) * vPad
//                        * (boundary.right - boundary.left) * hPad
//                        / initArea;
//    textScale = 1.1*std::sqrt(textScale); // this is a good first order (over)approximation
//    
//    // scale back further still if the text cannot be contained in the specified box height-wise
//TRY_AGAIN:
//    std::vector<std::string> lines;
//    const int numLines = (boundary.top - boundary.bottom) * vPad / (textHeight/linePad*textScale);
//    const float horizontalScale = aspect * textScale;
//    float textLength = 0;
//    int lineCount = 1;
//    for (int i = 0; i < words.size(); ++i) {
//        textLength += font->length(words[i]) * horizontalScale;
//        if (textLength > (boundary.right-boundary.left)*hPad) {
//            if (lines.size() > lineCount-1)
//                removeLastWhitespace(lines[lineCount-1]);
//            ++lineCount;
//            if (lineCount > numLines) {
//                // micro-adjust scale until text fits height-wise
//                textScale *= 0.99;
//                lines.clear();
//                goto TRY_AGAIN;
//            }
//            textLength = 0;
//            if (words[i][0] != ' ')
//                --i;
//        } else {
//            if (lineCount-1 >= lines.size())
//                lines.emplace_back("");
//            lines[lineCount-1] += words[i];
//        }
//    }
//    
//    const float hPadGap = (boundary.right - boundary.left) * (1.0 - hPad) * 0.5;
//    const float vPadGap = (boundary.top - boundary.bottom) * (1.0 - vPad) * 0.5;
//    float offsetY = boundary.top - vPadGap;
//    switch (vJust) {
//        case Just::TOP_LEFT:
//            //offsetY -= 0;
//            break;
//        case Just::CENTER:
//            offsetY -= 0.5*((boundary.top-boundary.bottom)*vPad - lines.size()*textScale*textHeight/linePad);
//            break;
//        case Just::BOTTOM_RIGHT:
//            offsetY -= ((boundary.top-boundary.bottom)*vPad - lines.size()*textScale*textHeight/linePad);
//            break;
//    }
//
//    float offsetX;
//    offsetY += textScale * font->bottomMax() / linePad;
//    for (const auto& line : lines) {
//        offsetY -= textScale * textHeight / linePad;
//        offsetX = boundary.left + hPadGap;
//        switch (hJust) {
//            case Just::TOP_LEFT:
//                //offsetX += 0;
//                break;
//            case Just::CENTER:
//                offsetX += 0.5*((boundary.right-boundary.left)*hPad - font->length(line) * aspect * textScale);
//                break;
//            case Just::BOTTOM_RIGHT:
//                offsetX += (boundary.right-boundary.left)*hPad - font->length(line) * aspect * textScale;
//                break;
//        }
//        font->drawString(line, {offsetX, offsetY}, aspect, textScale);
//    }
//}
//
//    
//
//TextBoxButton::TextBoxButton()
//{
//}
//
//TextBoxButton::TextBoxButton(const std::string& txt, const Box& bounds)
//    : TextBox(txt, bounds)
//{
//}
//
//void TextBoxButton::setClickCallback(const std::function<void()>& clickCallback) noexcept
//{
//    doClickCallback = clickCallback;
//}
//    
//bool TextBoxButton::mouseClicked()
//{
//    if (mouseOverButton)
//        press();
//    return mouseOverButton;
//}
//
//void TextBoxButton::press()
//{
//    buttonPressed = ! buttonPressed;
//    mousePressAnimationCount = 0;
//    doClickCallback();
//}
//    
//bool TextBoxButton::isPressed() const noexcept
//{
//    return buttonPressed;
//}
//
//bool TextBoxButton::isMouseOver() const noexcept
//{
//    return mouseOverButton;
//}
//    
//void TextBoxButton::draw(const float aspectRatio, const Point2D& mousePos)
//{
//    draw(aspectRatio, mousePos, true);
//}
//    
//void TextBoxButton::draw(float aspectRatio, const Point2D& mousePos, bool mouseOverEnabled)
//{
//    mouseOverButton = boundary.contains(mousePos) && mouseOverEnabled;
//    if (mouseOverButton) {
//        drawGradient(mousePos,
//                     {buttonColor.red, buttonColor.green, buttonColor.blue, backgroundTransparency[0]},
//                     boundary.getBoundaryPoints(),
//                     {buttonColor.red, buttonColor.green, buttonColor.blue, backgroundTransparency[1]});
//        mousePrev = mousePos;
//        mouseExitAnimationCount = 0;
//    } else if (mouseExitAnimationCount < mouseExitAnimationLength) {
//        const float factor = ((float)(mouseExitAnimationLength-mouseExitAnimationCount++))/mouseExitAnimationLength;
//        drawGradient(mousePrev,
//                     {buttonColor.red, buttonColor.green, buttonColor.blue, backgroundTransparency[0]*factor},
//                     boundary.getBoundaryPoints(),
//                     {buttonColor.red, buttonColor.green, buttonColor.blue, backgroundTransparency[1]*factor});
//    }
//    if (mousePressAnimationCount < mousePressAnimationLength) {
//        const float factor = ((float)(mousePressAnimationLength-mousePressAnimationCount))/mousePressAnimationLength;
//        if (mouseOverButton) {
//            drawGradient(mousePrev,
//                         {buttonColor.red, buttonColor.green, buttonColor.blue, 2.0f*backgroundTransparency[0]*factor},
//                         boundary.getBoundaryPoints(),
//                         {buttonColor.red, buttonColor.green, buttonColor.blue, 2.0f*backgroundTransparency[1]*factor});
//        } else {
//            glColor({buttonColor.red, buttonColor.green, buttonColor.blue, 2.0f*backgroundTransparency[0]*factor});
//            glBegin(GL_QUADS);
//            glVertex2f(boundary.left, boundary.top);
//            glVertex2f(boundary.left, boundary.bottom);
//            glVertex2f(boundary.right, boundary.bottom);
//            glVertex2f(boundary.right, boundary.top);
//            glEnd();
//        }
//        ++mousePressAnimationCount;
//    }
//    glColor(buttonColor);
//    if ((showsState && buttonPressed) || mousePressAnimationCount < mousePressAnimationLength) {
//        glLineWidth(2.0);
//    }
//    boundary.draw(); //drawBoundary();
//    drawText(aspectRatio);
//    glLineWidth(1.0);
//}
// 
//    
//
//Tabs::Tabs()
//{
//}
//    
//Tabs::Tabs(const std::vector<std::string>& tabTxts, const Box& bounds, Orientation orient)
//    :TextBoxGroup(tabTxts, bounds,
//                  (orient == ABOVE || orient == BELOW) ? 1 : tabTxts.size(),
//                  (orient == ABOVE || orient == BELOW) ? tabTxts.size() : 1), orientation(orient)
//{
//    if (orientation == ABOVE || orientation == BELOW) {
//        hPad -= gapFactor;
//    } else { // if (orientation == LEFT || orientation == RIGHT)
//        vPad -= gapFactor;
//    }
//}
//    
//void Tabs::draw(const float aspectRatio, const Point2D& mousePos)
//{
//    draw(aspectRatio, mousePos, true);
//}
//    
//void Tabs::draw(const float aspectRatio, const Point2D& mousePos, const bool mouseOverEnabled)
//{
//    // compute dimensions of tabs for below
//    const float widthOrHeight = (orientation == ABOVE || orientation == BELOW)
//                                ? boundary.right-boundary.left : boundary.top-boundary.bottom;
//    const float gap = widthOrHeight * gapFactor / (texts.size()*3.0+1.0);
//    const float tab = widthOrHeight * (1.0-gapFactor) / texts.size();
//    const int prevMouseOverTab = mouseOverTab;
//    
//    // detect which tab mouse is over, if any
//    if (boundary.contains(mousePos) && mouseOverEnabled) {
//        float lowA, highA, b1, b2, mouseA, mouseB, alpha, baseB, sign;
//        if (orientation == ABOVE || orientation == BELOW) {
//            lowA = boundary.bottom;
//            highA = boundary.top;
//            mouseA = mousePos.y;
//            mouseB = mousePos.x;
//            baseB = boundary.left + gap;
//            sign = +1.0;
//        } else {
//            lowA = boundary.left;
//            highA = boundary.right;
//            mouseA = mousePos.x;
//            mouseB = mousePos.y;
//            baseB = boundary.top - gap;
//            sign = -1.0;
//        }
//        if (orientation == ABOVE || orientation == RIGHT)
//            alpha = (mouseA-lowA)/(highA-lowA);
//        else
//            alpha = (highA-mouseA)/(highA-lowA);
//        for (int i = 0; i < texts.size(); ++i) {
//            if (i != selectedTab) { // mouse can't hover over an already selected tab with any visual effect
//                const float b = baseB + sign*i*(tab+3.0*gap);
//                b1 = b + sign*alpha*gap;
//                b2 = b + sign*(gap+tab + (1.0-alpha)*gap);
//                if (std::min(b1, b2) <= mouseB && mouseB <= std::max(b1, b2)) {
//                    mouseOverTab = i;
//                    goto MOUSE_OVER_TAB;
//                }
//            }
//        }
//    }
//    // mouse is not currently over a tab if we get here
//    mouseOverTab = -1;
//    
//MOUSE_OVER_TAB:
//    
//    // if mouse just leaves a tab, add/refresh the mouse exit animation
//    if (prevMouseOverTab >= 0 && prevMouseOverTab != mouseOverTab) {
//        mouseExitAnimations.add(prevMouseOverTab, mouseExitAnimationLength, mousePrev);
//    }
//    mousePrev = mousePos;
//    
//    // draw dark shading over tab bar (so any background objects are visible, but dark
//    glColor4f(0.0, 0.0, 0.0, 0.85);
//    glBegin(GL_QUADS);
//    glVertex2f(boundary.left, boundary.bottom);
//    glVertex2f(boundary.left, boundary.top);
//    glVertex2f(boundary.right, boundary.top);
//    glVertex2f(boundary.right, boundary.bottom);
//    glEnd();
//    
//    // draw selected tab
//    const float offset = selectedTab*(tab+gap*3.0) + gap;
//    float in, out;
//    glColor(color);
//    glLineWidth(2.0);
//    glBegin(GL_LINE_STRIP);
//    if (orientation == ABOVE || orientation == BELOW) {
//        float x = boundary.left;
//        if (orientation == ABOVE) {
//            in = boundary.bottom;
//            out = boundary.top;
//        } else {
//            in = boundary.top;
//            out = boundary.bottom;
//        }
//        glVertex2f(x, in);     x += offset;
//        glVertex2f(x, in);     x += gap;
//        glVertex2f(x, out);    x += tab;
//        glVertex2f(x, out);    x += gap;
//        glVertex2f(x, in);
//        glVertex2f(boundary.right, in);
//    } else {
//        float y = boundary.top;
//        if (orientation == LEFT) {
//            in = boundary.right;
//            out = boundary.left;
//        } else {
//            in = boundary.left;
//            out = boundary.right;
//        }
//        glVertex2f(in, y);     y -= offset;
//        glVertex2f(in, y);     y -= gap;
//        glVertex2f(out, y);    y -= tab;
//        glVertex2f(out, y);    y -= gap;
//        glVertex2f(in, y);
//        glVertex2f(in, boundary.bottom);
//    }
//    glEnd();
//    glLineWidth(1.0);
//
//    // draw nonselected tabs
//    float across;
//    if (orientation == ABOVE || orientation == BELOW)
//        across = boundary.left + gap;
//    else
//        across = boundary.top - gap;
//    for (int i = 0; i < texts.size(); ++i) {
//        // draw some shading over the hovered over tab
//        if (i == mouseOverTab) {
//            std::vector<Point2D> boundaryPts(4);
//            if (orientation == ABOVE || orientation == BELOW)
//                boundaryPts = {{across, in}, {across+gap, out}, {across+tab+gap, out}, {across+2.0f*gap+tab, in}};
//            else
//                boundaryPts = {{in, across}, {out, across-gap}, {out, across-tab-gap}, {in, across-2.0f*gap-tab}};
//            drawGradient(mousePos,   {color.red, color.green, color.blue, backgroundTransparency[0]},
//                         boundaryPts, {color.red, color.green, color.blue, backgroundTransparency[1]});
//        } else {
//            // draw mouse click animations if any
//            auto clickAnimation = mouseClickAnimations.get(i);
//            if (clickAnimation) {
//                std::vector<Point2D> boundaryPts(4);
//                if (orientation == ABOVE || orientation == BELOW)
//                    boundaryPts = {{across, in}, {across+gap, out}, {across+tab+gap, out}, {across+2.0f*gap+tab, in}};
//                else
//                    boundaryPts = {{in, across}, {out, across-gap}, {out, across-tab-gap}, {in, across-2.0f*gap-tab}};
//                const float factor = 1.0 - clickAnimation->getProgress();
//                drawGradient(clickAnimation->mouse,
//                             {color.red, color.green, color.blue, 0.0},
//                             boundaryPts,
//                             {color.red, color.green, color.blue, 2.0f*backgroundTransparency[0]*factor});
//                glLineWidth(2.0);
//                glBegin(GL_LINES);
//                if (orientation == ABOVE || orientation == BELOW) {
//                    glColor(color);
//                    glVertex2f(across, in);
//                    glColor({color.red, color.green, color.blue, 0.0});
//                    glVertex2f(across + factor*(clickAnimation->mouse.x - across), in);
//                    //glVertex2f(across + factor*(gap+0.5*tab), in);
//                    //glVertex2f(across+2.0*gap+tab - factor*(gap+0.5*tab), in);
//                    const float end = across+2.0*gap+tab;
//                    glVertex2f(end - factor*(end - clickAnimation->mouse.x), in);
//                    glColor(color);
//                    glVertex2f(end, in);
//                } else {
//                    glColor(color);
//                    glVertex2f(in, across);
//                    glColor({color.red, color.green, color.blue, 0.0});
//                    glVertex2f(in, across - factor*(across - clickAnimation->mouse.y));
//                    const float end = across-2.0*gap-tab;
//                    glVertex2f(in, end + factor*(clickAnimation->mouse.x - end));
//                    glColor(color);
//                    glVertex2f(in, end);
//                }
//                glEnd();
//                glLineWidth(1.0);
//                if (factor <= 0) {
//                    mouseClickAnimations.remove(i);
//                    mouseExitAnimations.remove(i);
//                }
//            } else {
//                // draw mouse exit animations if any
//                auto exitAnimation = mouseExitAnimations.get(i);
//                if (exitAnimation) {
//                    std::vector<Point2D> boundaryPts(4);
//                    if (orientation == ABOVE || orientation == BELOW)
//                        boundaryPts = {{across, in}, {across+gap, out}, {across+tab+gap, out}, {across+2.0f*gap+tab, in}};
//                    else
//                        boundaryPts = {{in, across}, {out, across-gap}, {out, across-tab-gap}, {in, across-2.0f*gap-tab}};
//                    const float factor = 1.0 - exitAnimation->getProgress();
//                    drawGradient(exitAnimation->mouse,
//                                 {color.red, color.green, color.blue, backgroundTransparency[0]*factor},
//                                 boundaryPts,
//                                 {color.red, color.green, color.blue, backgroundTransparency[1]*factor});
//                    if (factor <= 0)
//                        mouseExitAnimations.remove(i);
//                }
//            }
//        }
//        // draw the normal, unselected tab
//        if (i != selectedTab) {
//            glColor(color);
//            glBegin(GL_LINE_STRIP);
//            if (orientation == ABOVE || orientation == BELOW) {
//                glVertex2f(across, in);     across += gap;
//                glVertex2f(across, out);    across += tab;
//                glVertex2f(across, out);    across += gap;
//                glVertex2f(across, in);     across += gap;
//            } else {
//                glVertex2f(in, across);     across -= gap;
//                glVertex2f(out, across);    across -= tab;
//                glVertex2f(out, across);    across -= gap;
//                glVertex2f(in, across);     across -= gap;
//            }
//            glEnd();
//        } else { // skip over selected tab cuz we already drew it above
//            if (orientation == ABOVE || orientation == BELOW)
//                across += tab + gap*3.0;
//            else
//                across -= tab + gap*3.0;
//        }
//    }
//    
//    // wishful OO thinking
////    for (auto& a : animations) {
////        a.draw();
////    }
//    
//    // need some scaling for the text to be centered properly with the endpoint gap segments
//    glPushMatrix();
//    if (orientation == ABOVE || orientation == BELOW) {
//        glTranslatef(+(boundary.left + 0.5*widthOrHeight), 0.0, 0.0);
//        glScalef(1.0-gap/widthOrHeight, 1.0, 1.0);
//        glTranslatef(-(boundary.left + 0.5*widthOrHeight), 0.0, 0.0);
//    } else {
//        glTranslatef(0.0, +(boundary.top - 0.5*widthOrHeight), 0.0);
//        glScalef(1.0, 1.0-gap/widthOrHeight, 1.0);
//        glTranslatef(0.0, -(boundary.top - 0.5*widthOrHeight), 0.0);
//    }
//    // and draw the text labels, selected in bold
//    glColor(color);
//    drawText(aspectRatio, {selectedTab}, [](const bool on){ on ? glLineWidth(2.0) : glLineWidth(1.0); });
//    glPopMatrix();
//}
//    
//int Tabs::mouseClicked()
//{
//    if (mouseOverTab != -1) {
//        setSelectedTab(mouseOverTab, true);
//    }
//    return mouseOverTab;
//}
//
//void Tabs::setSelectedTab(const int tabIndex, const bool enableAnimation) noexcept
//{
//    if (enableAnimation) {
//        if (mouseOverTab == tabIndex) {
//            Point2D pt;
//            switch (orientation) {
//                case ABOVE:
//                    pt = {mousePrev.x, boundary.bottom};
//                    break;
//                case BELOW:
//                    pt = {mousePrev.x, boundary.top};
//                    break;
//                case LEFT:
//                    pt = {boundary.right, mousePrev.y};
//                    break;
//                case RIGHT:
//                    pt = {boundary.left, mousePrev.y};
//                    break;
//            }
//            mouseClickAnimations.add(tabIndex, mousePressAnimationLength, pt);
//        } else {
//            const float widthOrHeight = (orientation == ABOVE || orientation == BELOW)
//                        ? boundary.right-boundary.left : boundary.top-boundary.bottom;
//            const float gap = widthOrHeight * gapFactor / (texts.size()*3.0+1.0);
//            const float tab = widthOrHeight * (1.0-gapFactor) / texts.size();
//            Point2D center;
//            switch (orientation) {
//                case ABOVE:
//                    center = {boundary.left + float(tabIndex+1)*gap + float(tabIndex+1)*(tab+2.0f*gap) - 0.5f*(tab+2.0f*gap), boundary.bottom};
//                    break;
//                case BELOW:
//                    center = {boundary.left + float(tabIndex+1)*gap + 0.5f*float(tabIndex)*(tab+2.0f*gap), boundary.top};
//                    break;
//                case LEFT:
//                    center = {boundary.right, boundary.top - ((tabIndex+1)*gap + 0.5f*(tabIndex*(tab+2.0f*gap)))};
//                    break;
//                case RIGHT:
//                    center = {boundary.left, boundary.top - ((tabIndex+1)*gap + 0.5f*(tabIndex*(tab+2.0f*gap)))};
//                    break;
//            }
//            mouseClickAnimations.add(tabIndex, mousePressAnimationLength, center);
//        }
//    }
//    selectedTab = tabIndex;
//}
//
//    
//    
//RadioOption::RadioOption()
//{
//}
//
//RadioOption::RadioOption(const TextBox& titleTB, const TextBoxGroup& optionsTBG)
//    : title(titleTB), options(optionsTBG)
//{
//    boundary = getMax(title.boundary, options.boundary);
//}
//
//void RadioOption::draw(const float aspectRatio, const Point2D& mousePos)
//{
//    drawAllButOptionsText(aspectRatio, mousePos);
//    // draw option text with the selected option in bold
//    options.drawText(aspectRatio, {selectedOption}, [](bool on){on ? glLineWidth(2.0) : glLineWidth(1.0);});
//}
//
//void RadioOption::drawAllButOptionsText(const float aspectRatio, const Point2D& mousePos)
//{
//    glColor(color);
//    //boundary.draw();
//    //title.boundary.draw();
//    title.drawText(aspectRatio);
//    //options.drawBoundaries();
//    
//    int prevMouseOverOption = mouseOverOption;
//    
//    const float w = options.boundary.right - options.boundary.left;
//    const float h = options.boundary.top - options.boundary.bottom;
//    const int numColums = options.getNumColumns();
//    const int numRows = options.getNumRows();
//    Box b = {options.boundary.top,  options.boundary.top - h / numRows,
//        options.boundary.left, options.boundary.left + w / numColums};
//    // see if mouse is hovering over any of the options and which one if so
//    if (options.boundary.contains(mousePos)) {
//        for (int r = 0; r < numRows; ++r) {
//            for (int c = 0; c < numColums; ++c) {
//                if (b.contains(mousePos)) {
//                    mouseOverOption = r * numColums + c;//c * numRows + r;
//                    goto MOUSE_OVER_OPTION;
//                }
//                b.left  += w / numColums;
//                b.right += w / numColums;
//            }
//            b.left  = options.boundary.left;
//            b.right = b.left + w / numColums;
//            b.top    -= h / numRows;
//            b.bottom -= h / numRows;
//        }
//    }
//    mouseOverOption = -1;
//    
//MOUSE_OVER_OPTION:
//    
//    // if mouse just leaves an option, add/refresh the mouse exit animation
//    if (prevMouseOverOption >= 0 && prevMouseOverOption != mouseOverOption && prevMouseOverOption != selectedOption) {
//        mouseExitAnimations.add(prevMouseOverOption, mouseExitAnimationLength, mousePrev);
//    }
//    mousePrev = mousePos;
//    
//    // draw mouse click animations
//    if (mouseClickAnimations.size() > 0) {
//        for (auto& a : mouseClickAnimations.getVector()) {
//            float tmp = ((float)a.idNum) / options.getNumColumns();
//            int r = tmp;
//            int c = std::round((tmp - std::floor(tmp)) * options.getNumColumns());
//            const float factor = 1.0 - a.getProgress();
//            const Box b {options.boundary.top  - r     * h/numRows,
//                options.boundary.top  - (r+1) * h/numRows,
//                options.boundary.left + c     * w/numColums,
//                options.boundary.left + (c+1) * w/numColums};
//            drawGradient(a.mouse, color.withAlpha(2.0*backgroundTransparency[0]*factor),
//                         b.getBoundaryPoints(), color.withAlpha(2.0*backgroundTransparency[1]*factor));
//        }
//        // remove expired mouse click animations
//        for (auto& a : mouseClickAnimations.getVector())
//            if (!a.isPlaying())
//                mouseClickAnimations.remove(a.idNum);
//    }
//    
//    // draw mouse exit animations
//    if (mouseExitAnimations.size() > 0) {
//        for (auto& a : mouseExitAnimations.getVector()) {
//            float tmp = ((float)a.idNum) / options.getNumColumns();
//            int r = tmp;
//            int c = std::round((tmp - std::floor(tmp)) * options.getNumColumns());
//            const float factor = 1.0 - a.getProgress();
//            const Box b {options.boundary.top  - r     * h/numRows,
//                options.boundary.top  - (r+1) * h/numRows,
//                options.boundary.left + c     * w/numColums,
//                options.boundary.left + (c+1) * w/numColums};
//            drawGradient(a.mouse, color.withAlpha(backgroundTransparency[0]*factor),
//                         b.getBoundaryPoints(), color.withAlpha(backgroundTransparency[1]*factor));
//        }
//        // remove expired mouse exit animations
//        for (auto& a : mouseExitAnimations.getVector())
//            if (!a.isPlaying())
//                mouseExitAnimations.remove(a.idNum);
//    }
//    
//    // draw mouse over option shading
//    if (mouseOverOption >= 0 && selectedOption != mouseOverOption) {
//        drawGradient(mousePos, color.withAlpha(backgroundTransparency[0]),
//                     b.getBoundaryPoints(), color.withAlpha(backgroundTransparency[1]));
//    }
//    glColor(color);
//}
//    
//int RadioOption::mouseClicked()
//{
//    if (mouseOverOption >= 0 && selectedOption != mouseOverOption) {
//        mouseClickAnimations.add(mouseOverOption, mousePressAnimationLength, mousePrev);
//        selectedOption = mouseOverOption;
//    }
//    return mouseOverOption;
//}
//    
//void RadioOption::setSelectedOption(const int option) noexcept
//{
//    if (0 <= option && option < options.getSize())
//        selectedOption = option;
//    
//}
//
//void RadioOptionWithAutoDetect::draw(float aspectRatio, const Point2D& mousePos)
//{
//    drawAllButOptionsText(aspectRatio, mousePos);
//    // draw option text with the selected option in bold
//    if (selectedOption == autoDetectOption)
//        options.drawText(aspectRatio, {selectedOption, autoDetectedOption}, [](bool on){on ? glLineWidth(2.0) : glLineWidth(1.0);});
//    else
//        options.drawText(aspectRatio, {selectedOption}, [](bool on){on ? glLineWidth(2.0) : glLineWidth(1.0);});
//}
//
//    
//    
//DecimalNumberRestrictor::DecimalNumberRestrictor(const float min, const float max,
//                                                 const int maxDecimalPlaces)
//    : min(min), max(max), maxDecimalPlaces(maxDecimalPlaces) {}
//    
//bool DecimalNumberRestrictor::insert(std::string& text, int& cursorPosition, const std::string& textToInsert)
//{
//    // this function is intended to only insert one character at a time, but a string is passed in for programatic convienience with EditableTextBox::keyPressed.  handling one character at a time simplifies the logic.
//    if (textToInsert.length() == 1) {
//        if (textToInsert == "-") { // negative sign
//            if (cursorPosition == 0 && min < 0) {
//                text.insert(cursorPosition++, "-");
//                return true;
//            }
//            return false;
//        } else if (textToInsert == ".") { // decimal point
//            if (text.find(".") == std::string::npos) {
//                text.insert(cursorPosition++, ".");
//                return true;
//            }
//            return false;
//        } else { // anything but "-" or "."
//            int textToInsertIntValue = -1;
//            try {
//                textToInsertIntValue = std::stoi(textToInsert);
//            } catch (...) { // couldn't convert to integer
//                return false;
//            }
//            if (0 <= textToInsertIntValue && textToInsertIntValue <= 9) { // numeric digit 0-9
//                const int decimalIndex = text.find(".");
//                // text contains a "."
//                if (decimalIndex != std::string::npos) {
//                    // cursor is left of "."
//                    if (cursorPosition <= decimalIndex) {
//                        int negativeSignDigit = 0;
//                        if (text.length() > 0 && text[0] == '-')
//                            negativeSignDigit = 1;
//                        if (text.length() - negativeSignDigit - (text.length()-(decimalIndex+1)+1) < numDigitsLeftOfDecimal(max)) {
//                            text.insert(cursorPosition++, textToInsert);
//                            return true;
//                        }
//                        return false;
//                    } else { // cursor is right of "."
//                        if (text.length() - (decimalIndex+1) < maxDecimalPlaces) {
//                            text.insert(cursorPosition++, textToInsert);
//                            return true;
//                        }
//                        return false;
//                    }
//                } else { // text doesn't contain a "."
//                    int negativeSignDigit = 0;
//                    if (text.length() > 0 && text[0] == '-')
//                        negativeSignDigit = 1;
//                    if (text.length() - negativeSignDigit < numDigitsLeftOfDecimal(max)) {
//                        text.insert(cursorPosition++, textToInsert);
//                        return true;
//                    }
//                    return false;
//                }
//            } else { // not 0-9
//                return false;
//            }
//        }
//    }
//    return false;
//}
//
//void DecimalNumberRestrictor::validateInput(std::string& text)
//{
//    try {
//        auto value = std::stold(text); // using long double here to avoid having any digits be rounded
//        if (value < min)
//            value = min;
//        else if (value > max)
//            value = max;
//        text = std::to_string(value);
//        text = text.substr(0, text.find(".") + maxDecimalPlaces + 1);
//    } catch (...) {
//        text = "";
//    }
//}
//    
//int DecimalNumberRestrictor::numDigitsLeftOfDecimal(const float decimalNumber)
//{
//    int digits = 0;
//    float positiveNumber = std::abs(decimalNumber);
//    while (positiveNumber >= 1.0) {
//        positiveNumber *= 0.1;
//        ++digits;
//    }
//    return digits;
//}
// 
//    
//    
//EditableTextBox::EditableTextBox()
//{
//    initAnimations();
//}
//    
//EditableTextBox::EditableTextBox(const std::string& text, const Box& bounds)
//    : TextBox(text, bounds)
//{
////    if (inputRestrictor)
////    inputRestrictor->validateInput(text);
////    TextBox(text, bounds);
//    initAnimations();
//}
//    
//void EditableTextBox::draw(const float aspectRatio, const Point2D& mousePos)
//{
//    if (boundary.contains(mousePos)) {
//        if (!mouseOver)
//            mouseOverAnimation.count = 0;
//        mouseOver = true;
//    } else {
//        if (mouseOver)
//            mouseOverAnimation.count = 0;
//        mouseOver = false;
//    }
//    mousePrev = mousePos;
//    
//    glColor(color);
//    drawText(aspectRatio);
//    
//    if (mouseOver || mouseOverAnimation.isPlaying()) {
//        const float alpha = mouseOver ? mouseOverAnimation.getProgress()
//                                      : 1.0 - mouseOverAnimation.getProgress();
//        glColor(color.withAlpha(alpha));
//        boundary.draw();
//    }
//    
//    if (selected || selectAnimation.isPlaying()) {
//        const float glow = std::min(boundary.right - boundary.left,
//                                    boundary.top - boundary.bottom) * 0.075;
//        const float alpha = selected ? selectAnimation.getProgress()
//                                     : 1.0 - selectAnimation.getProgress();
//        glBegin(GL_QUADS);
//        glColor(color.withAlpha(alpha));
//        glVertex2f(boundary.left, boundary.top);
//        glVertex2f(boundary.right, boundary.top);
//        glColor(color.withAlpha(0.0));
//        glVertex2f(boundary.right + glow*aspect, boundary.top + glow);
//        glVertex2f(boundary.left - glow*aspect, boundary.top + glow);
//        
//        glColor(color.withAlpha(alpha));
//        glVertex2f(boundary.right, boundary.top);
//        glVertex2f(boundary.right, boundary.bottom);
//        glColor(color.withAlpha(0.0));
//        glVertex2f(boundary.right + glow*aspect, boundary.bottom - glow);
//        glVertex2f(boundary.right + glow*aspect, boundary.top + glow);
//        
//        glColor(color.withAlpha(alpha));
//        glVertex2f(boundary.right, boundary.bottom);
//        glVertex2f(boundary.left, boundary.bottom);
//        glColor(color.withAlpha(0.0));
//        glVertex2f(boundary.left - glow*aspect, boundary.bottom - glow);
//        glVertex2f(boundary.right + glow*aspect, boundary.bottom - glow);
//        
//        glColor(color.withAlpha(alpha));
//        glVertex2f(boundary.left, boundary.bottom);
//        glVertex2f(boundary.left, boundary.top);
//        glColor(color.withAlpha(0.0));
//        glVertex2f(boundary.left - glow*aspect, boundary.top + glow);
//        glVertex2f(boundary.left - glow*aspect, boundary.bottom - glow);
//        glEnd();
//    }
//    
//    // draw the highlighted region of the text
//    if (highlightedText[1] - highlightedText[0] > 0) {
//        const Box b = getHighlightBox();
//        glColor(color.withAlpha(0.3));
//        glBegin(GL_QUADS);
//        glVertex2f(b.left, b.bottom);
//        glVertex2f(b.left, b.top);
//        glVertex2f(b.right, b.top);
//        glVertex2f(b.right, b.bottom);
//        glEnd();
//    } else if (selected) { // if the text box is selected for editing
//        // draw cursor
//        if (cursorAnimation.getProgress() < 0.5) {
//            glColor(color);
//            const float x = getCursorPositionX();
//            const float h = boundary.top - boundary.bottom;
//            const float mid = boundary.bottom + 0.5*h;
//            glBegin(GL_LINES);
//            glVertex2f(x, mid - 0.4*h);
//            glVertex2f(x, mid + 0.4*h);
//            glEnd();
//        }
//        // restart cursor animation so it continues to blink
//        if (!cursorAnimation.isPlaying())
//            cursorAnimation.count = 0;
//    }
//}
//    
//bool EditableTextBox::mouseClicked()
//{
//    if (mouseOver) {
//        if (!selected) {
//            if (highlightedText[1] - highlightedText[0] == 0) {
//                highlightedText = {0, static_cast<int>(text.length())};
//                cursorPosition = text.length();
//            }
//            selectAnimation.count = 0;
//        } else {
//            if (!mouseDragging)
//                highlightedText = {0, 0};
//            cursorPosition = getClosestCursorPosition(mousePrev.x);
//        }
//        cursorAnimation.count = 0;
//        selected = true;
//    } else {
//        if (selected)
//            selectAnimation.count = 0;
//        highlightedText = {0, 0};
//        selected = false;
//    }
//    mouseDragging = false;
//    return selected;
//}
//    
//bool EditableTextBox::mouseDoubleClicked()
//{
//    if (mouseOver) {
//        highlightedText = {0, static_cast<int>(text.length())};
//        cursorPosition = text.length();
//        return true;
//    }
//    return false;
//}
//
//bool EditableTextBox::mouseDragged(const Point2D& mouseDownPosition,
//                                   const Point2D& mouseCurrentPosition)
//{
//    if (boundary.contains(mouseCurrentPosition)) {
//        const int down    = getClosestCursorPosition(mouseDownPosition.x);
//        const int current = getClosestCursorPosition(mouseCurrentPosition.x);
//        highlightedText[0] = std::min(down, current);
//        highlightedText[1] = std::max(down, current);
//        mouseDragging = true;
//        return true;
//    }
//    highlightedText = {0, 0};
//    return false;
//}
//    
//bool EditableTextBox::keyPressed(const std::string& key)
//{
//    if (selected) {
//        std::string tempKey;
//        // special keys have a longer description
//        if (key.length() > 1) {
//            if (key == "cursor left") {
//                if (highlightedText[1] - highlightedText[0] > 0)
//                    cursorPosition = highlightedText[0];
//                else
//                    cursorPosition = std::max(cursorPosition - 1, 0);
//                highlightedText = {0, 0};
//                cursorAnimation.count = 0;
//            } else if (key == "cursor right") {
//                if (highlightedText[1] - highlightedText[0] > 0)
//                    cursorPosition = highlightedText[1];
//                else
//                    cursorPosition = std::min(cursorPosition + 1, static_cast<int>(text.length()));
//                highlightedText = {0, 0};
//                cursorAnimation.count = 0;
//            } else if (key == "shift + cursor left") {
//                if (highlightedText[1] - highlightedText[0] == 0)
//                    highlightedText[0] = highlightedText[1] = cursorPosition;
//                if (highlightedText[1] == cursorPosition)
//                    highlightedText[0] = std::max(highlightedText[0] - 1, 0);
//                else
//                    highlightedText[1] = std::max(highlightedText[1] - 1, 0);
//            } else if (key == "shift + cursor right") {
//                if (highlightedText[1] - highlightedText[0] == 0)
//                    highlightedText[0] = highlightedText[1] = cursorPosition;
//                if (highlightedText[0] == cursorPosition)
//                    highlightedText[1] = std::min(highlightedText[1] + 1, static_cast<int>(text.length()));
//                else
//                    highlightedText[0] = std::min(highlightedText[0] + 1, static_cast<int>(text.length()));
//            } else if (key.find("return") != std::string::npos
//                       || key.find("tab") != std::string::npos) {
//                if (inputRestrictor)
//                    inputRestrictor->validateInput(text);
//                highlightedText = {0, 0};
//                selectAnimation.count = 0;
//                selected = false;
//            } else if (key.find("delete") != std::string::npos
//                       || key.find("backspace") != std::string::npos) {
//                if (highlightedText[1] - highlightedText[0] > 0) {
//                    text.erase(std::begin(text) + highlightedText[0], std::begin(text) + highlightedText[1]);
//                    cursorPosition = highlightedText[0];
//                    highlightedText = {0, 0};
//                } else if (cursorPosition > 0) {
//                    text.erase(std::begin(text) + --cursorPosition);
//                }
//            } else if (key.find("spacebar") != std::string::npos) {
//                tempKey = " ";
//                goto INPUT_TEXT;
//            } else if (key.find("shift") != std::string::npos) {
//                tempKey = key.substr(key.length()-1, key.length());
//                goto INPUT_TEXT;
//            }
//        } else { // key.length() <= 1, therefore its a 'normal' character
//        INPUT_TEXT:
//            tempKey = key;
//            auto hTxt = highlightedText;
//            auto txt = text;
//            auto cPos = cursorPosition;
//            if (highlightedText[1] - highlightedText[0] > 0) {
//                text.erase(std::begin(text) + highlightedText[0], std::begin(text) + highlightedText[1]);
//                cursorPosition = highlightedText[0];
//                highlightedText = {0, 0};
//            }
//            if (!inputText(tempKey)) { // if input fails, don't delete highlighted text
//                text = txt;
//                highlightedText = hTxt;
//                cursorPosition = cPos;
//            }
//        }
//        return true;
//    }
//    return false;
//}
//    
//void EditableTextBox::setTextInputRestrictor(std::unique_ptr<TextInputRestrictor> ir)
//{
//    inputRestrictor = std::move(ir);
//    if (inputRestrictor)
//        inputRestrictor->validateInput(text);
//}
//    
//const std::string& EditableTextBox::getText() const noexcept
//{
//    return text;
//}
//    
//bool EditableTextBox::inputText(const std::string& str)
//{
//    if (inputRestrictor) {
//        return inputRestrictor->insert(text, cursorPosition, str);
//    } else {
//        text.insert(cursorPosition++, str);
//        return true;
//    }
//}
//
//int EditableTextBox::getClosestCursorPosition(const float x) const
//{
//    auto posAndScale = getBeginPositionAndScale();
//    const float xStart = posAndScale[0];
//    const float scale = posAndScale[2];
//    float x_i = xStart;
//    float x_ip1;
//    for (int i = 1; i <= text.length(); ++i) {
//        x_ip1 = xStart + font->length(text.substr(0, i)) * scale * aspect;
//        if (x_ip1 > x) {
//            if (x_ip1 - x < x - x_i)
//                return i;
//            else
//                return i-1;
//        }
//        x_i = x_ip1;
//    }
//    return text.length();
//}
//    
//float EditableTextBox::getCursorPositionX() const
//{
//    auto posAndScale = getBeginPositionAndScale();
//    const float xStart = posAndScale[0];
//    const float scale = posAndScale[2];
//    return xStart + font->length(text.substr(0, cursorPosition)) * scale * aspect;
//}
//
//Box EditableTextBox::getHighlightBox() const
//{
//    auto posAndScale = getBeginPositionAndScale();
//    const float xStart = posAndScale[0];
//    const float yStart = posAndScale[1];
//    const float scale = posAndScale[2];
//    return { yStart + font->topMax() * scale,
//             yStart - font->bottomMax() * scale,
//        xStart + font->length(text.substr(0, highlightedText[0])) * scale * aspect,
//        xStart + font->length(text.substr(0, highlightedText[1])) * scale * aspect };
//}
//    
//void EditableTextBox::initAnimations() noexcept
//{
//    selectAnimation.count = selectAnimation.len;
//    mouseOverAnimation.count = mouseOverAnimation.len;
//    cursorAnimation.count = cursorAnimation.len;
//}
//
//    
//
//void drawGradient(const Point2D& centerPoint,
//                  const Color& centerColor,
//                  const std::vector<Point2D>& boundaryPoints,
//                  const Color& boundaryColor)
//{
//    glBegin(GL_TRIANGLE_FAN);
//    glColor(centerColor);
//    glVertex2f(centerPoint.x, centerPoint.y);
//    glColor(boundaryColor);
//    for (const auto& pt : boundaryPoints)
//        glVertex2f(pt.x, pt.y);
//    glVertex2f(boundaryPoints[0].x, boundaryPoints[0].y);
//    glEnd();
//}
//    
//void removeLastWhitespace(std::string& str)
//{
//    std::reverse(std::begin(str), std::end(str));
//    str.erase(std::begin(str), std::find_if(std::begin(str), std::end(str), [](const auto& c){return c != ' ';}));
//    std::reverse(std::begin(str), std::end(str));
//}
//
//void glColour(const Colour& color)
//{
//    glColor4f(color.getFloatRed(), color.getFloatGreen(), color.getFloatBlue(), color.getFloatAlpha());
//}
//    
//void glColor(const Color& color)
//{
//    glColor4f(color.red, color.green, color.blue, color.alpha);
//}
//
//void glColor(const Color& color, const float alpha)
//{
//    glColor4f(color.red, color.green, color.blue, alpha);
//}
