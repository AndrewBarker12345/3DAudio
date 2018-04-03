//
//  DrawInterpolator.h
//
//  Created by Andrew Barker on 9/13/16.
//
//

#ifndef DrawInterpolator_h
#define DrawInterpolator_h

#include "../JuceLibraryCode/JuceHeader.h"

#include <cmath>
#include <vector>

#include "OpenGL.h"
#include "Interpolator.h"
#include "StackArray.h"
#include "Box.h"
#include "Multi.h"


class InterpolatorLook
{
public:
    enum Dimensionality { TWO_D, THREE_D };
    enum LineType { CONTINUOUS, DASHED, DOTTED };
    template <class T>
    InterpolatorLook (const Interpolator<T>* interp, Dimensionality dim) noexcept;
    float begin;
    float end;
    Dimensionality drawingMode;
    std::vector<int> dimensionsToDraw;
    int numVertices;
    LineType lineType;
    float lineSize;
    Colour beginColor;
    Colour endColor;
    float numColorCycles;
    float colorCyclePhase; // 0 to 1
};

template <class T>
InterpolatorLook::InterpolatorLook (const Interpolator<T>* interp,
                                    const Dimensionality drawingMode) noexcept
    : begin (interp->getInputRange()[0]),
      end (interp->getInputRange()[1]),
      drawingMode (drawingMode),
      dimensionsToDraw (((drawingMode == TWO_D) ? std::vector<int>{0, 1}
                                                : std::vector<int>{0, 1, 2})),
      numVertices (100),
      lineType (CONTINUOUS),
      lineSize (2),
      beginColor (Colour::fromFloatRGBA(1, 0, 0, 1)),
      endColor (Colour::fromFloatRGBA(0, 0, 1, 1)),
      numColorCycles (1.5),
      colorCyclePhase (0)
{
}

static GLenum getGLMode(const InterpolatorLook& look) noexcept
{
    switch (look.lineType) {
        case InterpolatorLook::CONTINUOUS:
            return GL_LINE_STRIP;
        case InterpolatorLook::DASHED:
            return GL_LINES;
        case InterpolatorLook::DOTTED:
            return GL_POINTS;
    };
}

static Colour getInterpolatedColor(const Colour beginColor,
                                   const Colour endColor,
                                   const int numCycles,
                                   const float phase,
                                   const float position) noexcept
{
    cauto dColor = std::fmod(2 * ((numCycles - 0.5f) * position + phase), 2.0f);
    cauto color = dColor < 1 ? beginColor.interpolatedWith(endColor, dColor)
                             : endColor.interpolatedWith(beginColor, dColor - 1);
    return color;
}

static void glInterpolatedColor(const InterpolatorLook& look,
                                const float position)
{
    glColour(getInterpolatedColor(look.beginColor, look.endColor, look.numColorCycles, look.colorCyclePhase, position));
}

template <class T>
void draw(const ParametricInterpolator<T>* interp,
          const InterpolatorLook& look)
{
    cauto inputRange = interp->getInputRange();
    cauto begin = std::max(inputRange[0], look.begin);
    cauto percentOfEnd = (interp->getType() == InterpolatorType::CLOSED_PARAMETRIC && interp->getNumPoints() == 2) ? 0.5f : 0.9999999f;
    cauto end = std::min(inputRange[1] * percentOfEnd, look.end);
    cauto length = end - begin;
    cauto interval = length / look.numVertices;
    
    GLboolean prevAntiAliasing;
    glGetBooleanv(GL_LINE_SMOOTH, &prevAntiAliasing);
    glEnable(GL_LINE_SMOOTH); // enable antialiasing
    glInterpolatedColor(look, 0);
    GLfloat prevGLSize;
    if (look.lineType == InterpolatorLook::DOTTED) {
        glGetFloatv(GL_POINT_SIZE, &prevGLSize);
        glPointSize(look.lineSize);
    } else {
        glGetFloatv(GL_LINE_WIDTH, &prevGLSize);
        glLineWidth(look.lineSize);
    }
    cauto glMode = getGLMode(look);
    
    glBegin(glMode);
    cauto numDimensions = interp->getNumDimensions();
    STACK_ARRAY(float, pt, numDimensions - 1);
    for (auto t = begin; t < end; t += interval) {
        if (interp->pointAt(t, pt)) {
            if (look.drawingMode == InterpolatorLook::TWO_D)
                glVertex2f(pt[0], pt[1]);
            else
                glVertex3f(pt[0], pt[1], pt[2]);
        }
        glInterpolatedColor(look, (t - begin) / length);
    }
    glEnd();
    
    if (look.lineType == InterpolatorLook::DOTTED)
        glPointSize(prevGLSize);
    else
        glLineWidth(prevGLSize);
    if (prevAntiAliasing == GL_FALSE)
        glDisable(GL_LINE_SMOOTH); // disable antialiasing
}

template <class T>
void draw(const FunctionalInterpolator<T>* interp,
          const InterpolatorLook& look)
{
    const auto pts = interp->getPoints();
    const auto inputRange = interp->getInputRange();
    const auto begin = std::max(inputRange[0], look.begin);
    const auto end = std::min(inputRange[1], look.end);
    const float interval = (end - begin) / look.numVertices;
    if (interval <= 0.0000001f)
        return; // avoid inf loop below
    const int numDimensions = interp->getNumDimensions();
    STACK_ARRAY(float, pt, numDimensions-1);
    float x = begin;
    int splineIndex = 0;
    int prevSplineIndex = 0;
    bool dontReset = false;
    bool doAgain = true;
    GLboolean prevAntiAliasing;
    glGetBooleanv(GL_LINE_SMOOTH, &prevAntiAliasing);
    glEnable(GL_LINE_SMOOTH); // enable antialiasing
    glInterpolatedColor(look, 0);
    GLfloat prevGLSize;
    if (look.lineType == InterpolatorLook::DOTTED) {
        glGetFloatv(GL_POINT_SIZE, &prevGLSize);
        glPointSize(look.lineSize);
    } else {
        glGetFloatv(GL_LINE_WIDTH, &prevGLSize);
        glLineWidth(look.lineSize);
    }
    const auto glMode = getGLMode(look);
    glBegin(glMode);
AGAIN: // man this got complicated...
    int count = 0;
    while (x <= end) {
        if (++count > look.numVertices) // trying to avoid inf loop at all costs
            break;
        // only draw the dotted line over the portions that are not open/empty segments,
        if (interp->pointAtSmart(x, pt, splineIndex)) {
            if (prevSplineIndex+2 <= splineIndex ? pts[prevSplineIndex+1][0] != pts[splineIndex][0] : true) {
                if (look.drawingMode == InterpolatorLook::TWO_D)
                    glVertex2f(x, pt[look.dimensionsToDraw[1]-1]);
                else
                    glVertex3f(x, pt[look.dimensionsToDraw[1]-1], pt[look.dimensionsToDraw[2]-1]);
            } else if (!dontReset) { // avoiding drawing anything between 2 or more pts with the same x val
                glEnd();
                glBegin(glMode);
                dontReset = false;
            }
            x += interval;
            // draw verticies at any points that would otherwise get skipped over
            int lastSplineIndex = -1;
            for (int i = 1; splineIndex+i < pts.size() && x > pts[splineIndex+i][0]; ++i) {
                lastSplineIndex = splineIndex + i;
                if (pts[lastSplineIndex][0] != pts[lastSplineIndex-1][0]
                    && interp->getSplineShape(lastSplineIndex-1) != SplineShape::EMPTY) {
                    if (look.drawingMode == InterpolatorLook::TWO_D)
                        glVertex2f(pts[lastSplineIndex][0],
                                   pts[lastSplineIndex][look.dimensionsToDraw[1]]);
                    else
                        glVertex3f(pts[lastSplineIndex][0],
                                   pts[lastSplineIndex][look.dimensionsToDraw[1]],
                                   pts[lastSplineIndex][look.dimensionsToDraw[2]]);
                } else {
                    glEnd();
                    glBegin(glMode);
                }
            }
            if (lastSplineIndex >= 0) {
                if (look.drawingMode == InterpolatorLook::TWO_D)
                    glVertex2f(pts[lastSplineIndex][0],
                               pts[lastSplineIndex][look.dimensionsToDraw[1]]);
                else
                    glVertex3f(pts[lastSplineIndex][0],
                               pts[lastSplineIndex][look.dimensionsToDraw[1]],
                               pts[lastSplineIndex][look.dimensionsToDraw[2]]);
                dontReset = true;
            }
        } else {
            // finish drawing any unfinished GL_LINE segments
            if (look.drawingMode == InterpolatorLook::TWO_D)
                glVertex2f(pts[splineIndex][0],
                           pts[splineIndex][look.dimensionsToDraw[1]]);
            else
                glVertex3f(pts[splineIndex][0],
                           pts[splineIndex][look.dimensionsToDraw[1]],
                           pts[splineIndex][look.dimensionsToDraw[2]]);
            // gotta reset the gl line drawing state so we don't possibly draw a single line segment across an open segment
            glEnd();
            glBegin(glMode);
            if (splineIndex + 1 < pts.size())
                x = pts[splineIndex + 1][0]; // skip the loop to the begining of the next segment since this one is either open or spatially nonexistant in x
            else
                x = end; // goto last iteration of while loop
        }
        glInterpolatedColor(look, (x - begin) / (end - begin));
        prevSplineIndex = splineIndex;
    }
    if (doAgain && x - interval < end // make sure the last x point is the end of specified range
        && (pts.back()[0] <= look.end ? pts[pts.size()-2][0] < pts.back()[0] : true)) {
        x = end;
        doAgain = false;
        goto AGAIN;
    }
    glEnd();
    if (look.lineType == InterpolatorLook::DOTTED)
        glPointSize(prevGLSize);
    else
        glLineWidth(prevGLSize);
    if (prevAntiAliasing == GL_FALSE)
        glDisable(GL_LINE_SMOOTH); // disable antialiasing
}

class PointLook
{
public:
    float radius;
    float mouseOverRadius;
    Colour color;
    Colour selectedColor;
    float animationDuration;
};

class DrawablePointState
{
public:
    // need copy of these for each interp...
    Multi<Animation> mouseOverAnimations;
    Multi<Animation> selectAnimations;
    std::vector<bool> prevMouseOvers;
    std::vector<bool> prevSelecteds;
};

static void drawCircleOutline(const Point<float> position,
                              const float radius,
                              const int segments,
                              const float windowWidth,
                              const float windowHeight)
{
//    GLboolean prevAntiAliasing;
//    glGetBooleanv(GL_LINE_SMOOTH, &prevAntiAliasing);
//    glEnable(GL_LINE_SMOOTH); // enable antialiasing
    glBegin(GL_LINE_LOOP);
    cauto aspect = windowHeight / windowWidth;
    for (int n = 0; n <= segments; ++n) {
        cauto t = 2 * M_PI * (float)n / segments;
        glVertex2f(position.x + aspect * radius * std::sin(t),
                   position.y + radius * std::cos(t));
    }
    glEnd();
//    if (prevAntiAliasing == GL_FALSE)
//        glDisable(GL_LINE_SMOOTH); // disable antialiasing
}

static void drawCircle(const Point<float> position,
                       const float radius,
                       const int segments,
                       const float windowWidth,
                       const float windowHeight)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(position.x, position.y);
    cauto aspect = windowHeight / windowWidth;
    for (int n = 0; n <= segments; ++n) {
        cauto t = 2 * M_PI * (float)n / segments;
        glVertex2f(position.x + aspect * radius * std::sin(t),
                   position.y + radius * std::cos(t));
    }
    glEnd();
    GLboolean antiAliasing;
    glGetBooleanv(GL_LINE_SMOOTH, &antiAliasing);
    if (antiAliasing) {
        GLfloat prevLineWidth;
        glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
        glLineWidth(1);
        drawCircleOutline(position, radius, segments, windowWidth, windowHeight);
        glLineWidth(prevLineWidth);
    }
}

template <class T>
std::vector<Point<T>> convertPoints(const std::vector<std::vector<T>>& points,
                                    const int dimX = 0,
                                    const int dimY = 1)
{
    std::vector<Point<T>> pts (points.size());
    for (int i = 0; i < points.size(); ++i) {
        pts[i].x = points[i][dimX];
        pts[i].y = points[i][dimY];
    }
    return pts;
}

template <class T>
void drawPoints2D(const std::vector<SelectablePoint<T>>& points,
                  const PointLook& look,
                  DrawablePointState& state,
                  const Point<float>& mousePosition,
                  const OpenGLWindow& window,
                  const Box& selectBox,
                  const Box& antiSelectBox,
                  int& mouseOverPointIndex,
                  bool mouseOverEnabled = true,
                  const float viewWidth = 1.0,
                  const std::array<float, 2>& range = {0, 0})
{
    GLfloat prevLineWidth;
    glGetFloatv(GL_LINE_WIDTH, &prevLineWidth);
    GLboolean prevAntiAliasing;
    glGetBooleanv(GL_LINE_SMOOTH, &prevAntiAliasing);
    glEnable(GL_LINE_SMOOTH); // enable antialiasing
    mouseOverPointIndex = -1;
    auto resized = false;
    if (state.prevMouseOvers.size() != points.size()) {
        state.prevMouseOvers.resize(points.size(), false);
        resized = true;
    }
    if (state.prevSelecteds.size() != points.size()) {
        state.prevSelecteds.resize(points.size(), false);
        resized = true;
    }
    Colour color;
    float radius;
    for (int i = 0; i < points.size(); ++i) {
        cauto x = points[i].point[0];
        cauto y = points[i].point[1];
        if (range[0] <= x && x <= range[1]) {
            radius = look.radius;
            cauto yRadius = pixelsToNormalized(look.mouseOverRadius, window.height);
            cauto xRadius = pixelsToNormalized(look.mouseOverRadius, window.width) * viewWidth;// radius * viewWidth * window.getAspect();
            const Box b { y + yRadius, y - yRadius,
                          x - xRadius, x + xRadius };
            cauto mouseOver = mouseOverEnabled &&
                ((mouseOverPointIndex == -1 && b.contains(mousePosition)) || selectBox.contains({x, y})) &&
                !antiSelectBox.contains({x, y});
            if (mouseOver) {
                auto animation = state.mouseOverAnimations.get(i);
                if (state.prevMouseOvers[i] && animation) {
                    animation->advance(window.frameRate);
                    radius = look.radius + (look.mouseOverRadius - look.radius) * animation->getProgress();
                    color = look.color.interpolatedWith(look.selectedColor, animation->getProgress());
                    glColour(color);
                    if (!animation->isPlaying())
                        state.mouseOverAnimations.remove(i);
                } else if (!state.prevMouseOvers[i]){
                    state.mouseOverAnimations.add(i, look.animationDuration, true);
                    color = look.color;
                    radius = look.radius;
                } else {
                    color = look.selectedColor;
                    radius = look.mouseOverRadius;
                }
                state.prevMouseOvers[i] = true;
                mouseOverPointIndex = i;
            } else {
                if (state.prevMouseOvers[i] && !resized)
                    state.mouseOverAnimations.add(i, look.animationDuration, true);
                auto animation = state.mouseOverAnimations.get(i);
                if (animation) {
                    animation->advance(window.frameRate);
                    radius = look.mouseOverRadius + (look.radius - look.mouseOverRadius) * animation->getProgress();
                    color = look.selectedColor.interpolatedWith(look.color, animation->getProgress());
                    if (!animation->isPlaying())
                        state.mouseOverAnimations.remove(i);
                } else {
                    radius = look.radius;
                    color = look.color;
                }
            }
            if (points[i].selected) {
                if (!state.prevSelecteds[i] /*&& !resized*/)
                    state.selectAnimations.add(i, look.animationDuration, true);
                state.prevSelecteds[i] = true;
                auto rad = 2 * radius;
                auto lineWidth = 1.5f;
                auto animation = state.selectAnimations.get(i);
                if (animation) {
                    cauto progress = animation->getProgress();
                    lineWidth = 10 - (10 - lineWidth) * progress;// 15 - 13.5f * progress;
                    color = look.color.interpolatedWith(look.selectedColor, progress).withAlpha(progress);
                    glColour(color);
                    rad = (2 - progress) * radius;
                    animation->getProgress();
                    animation->advance(window.frameRate);
                    if (!animation->isPlaying())
                        state.selectAnimations.remove(i);
                } else {
                    radius -= 1;//look.radius - 1;
                    rad = radius + 1.5f;
                    color = look.selectedColor;
                    glColour(color);
                }
                glLineWidth(lineWidth);
                drawCircleOutline({x, y}, pixelsToNormalized(rad, window.height), 12, window.width / viewWidth, window.height);
            } else {
                if (state.prevSelecteds[i] && !resized)
                    state.selectAnimations.add(i, look.animationDuration, true);
                state.prevSelecteds[i] = false;
                auto rad = 2 * radius;
                auto lineWidth = 1.5f;
                auto animation = state.selectAnimations.get(i);
                if (animation) {
                    cauto progress = animation->getProgress();
                    color = look.selectedColor.interpolatedWith(look.color, progress).withAlpha(1 - progress);
                    glColour(color);
                    rad = (1 + progress) * radius;
                    animation->getProgress();
                    animation->advance(window.frameRate);
                    if (!animation->isPlaying())
                        state.selectAnimations.remove(i);
                    glLineWidth(lineWidth + (10 - lineWidth) * progress);
                    drawCircleOutline({x, y}, pixelsToNormalized(rad, window.height), 12, window.width / viewWidth, window.height);
                }
            }
            if (!mouseOver)
                state.prevMouseOvers[i] = false;
            glColour(color.withAlpha(1.0f));
            drawCircle({x, y}, pixelsToNormalized(radius, window.height), 12, window.width / viewWidth, window.height);
        }
    }
    if (prevAntiAliasing == GL_FALSE)
        glDisable(GL_LINE_SMOOTH); // disable antialiasing
    
    glDisable(GL_POLYGON_SMOOTH);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glLineWidth(prevLineWidth);
}

template <class T>
void draw(const Interpolator<T>* interp,
          const InterpolatorLook& look)
{
    if (!interp)
        return;
    if (interp->getPoints().size() < 2) // no path to draw
        return;
    if (interp->getType() == InterpolatorType::FUNCTIONAL) {
        draw(dynamic_cast<const FunctionalInterpolator<T>* const>(interp), look);
    } else { // parametric
        draw(dynamic_cast<const ParametricInterpolator<T>* const>(interp), look);
    }
}

template <class T>
void draw(const Interpolator<T>* interp,
          const InterpolatorLook& look,
          GLuint& displayList)
{
    if (displayList != 0) {
        glCallList(displayList);
        return;
    } else {
        glDeleteLists(displayList, 1);
        displayList = glGenLists(1);
        glNewList(displayList, GL_COMPILE_AND_EXECUTE);
        draw(interp, look);
        glEndList();
    }
}

#endif /* DrawInterpolator_h */
