//
//  SoundSource.cpp
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

#include "SoundSource.h"
#include "Functions.h"
#include <string>

// fuckin C++ man
template <class T_SRC, class T_DEST>
std::unique_ptr<T_DEST> unique_cast(std::unique_ptr<T_SRC> &&src)
{
    if (!src) return std::unique_ptr<T_DEST>();
    // Throws a std::bad_cast() if this doesn't work out
    T_DEST *dest_ptr = &dynamic_cast<T_DEST &>(*src.get());
    src.release();
    return std::unique_ptr<T_DEST>(dest_ptr);
}

SoundSource::SoundSource()
{
    const std::vector<std::vector<float>> noPts {};
    path = std::move(unique_cast<Interpolator<float>, ParametricInterpolator<float>>(InterpolatorFactory(InterpolatorType::CLOSED_PARAMETRIC, noPts)));
    path->addListener(&pathListener);
    pathPos.addListener(&pathPosListener);
}

SoundSource::SoundSource(const std::array<float,3>& xyz) : SoundSource()
{
    // set position to the one passed in (after bounds checking)
    setPosXYZ(xyz);
}

SoundSource::SoundSource(const SoundSource& source)
{
    // non dynamically allocated data can be shallow copied
    posRAE = source.posRAE;
    sourceMuted = source.sourceMuted;
    eleDir = source.eleDir;
    sourceSelected = source.sourceSelected;
    
    pathPos = source.pathPos;
    pathPos.removeListeners();
    pathPosListener = source.pathPosListener;
    pathPos.addListener(&pathPosListener);
    
    pathListener = source.pathListener;
    // need to deep copy pointers that are not null
    if (source.path.get() != nullptr) {
        path = std::move(source.path->clone());
        path->removeListeners();
        path->addListener(&pathListener);
    }
}

//SoundSource& SoundSource::operator=(const SoundSource& source)
//{
//    // check for self-assignment
//    if (this == &source)
//        return *this;
//    // deallocate previous stuff
//    path.release();
//    // non dynamically allocated data can be shallow copied
//    posRAE = source.posRAE;
//    sourceMuted = source.sourceMuted;
//    eleDir = source.eleDir;
//    sourceSelected = source.sourceSelected;
//    pathPos = source.pathPos;
//    pathListener = source.pathListener;
//    pathPosListener = source.pathPosListener;
//    // need to deep copy pointers that are not null
//    if (source.path.get() != nullptr)
//        path = source.path->clone();
//    return *this;
//}

SoundSource& SoundSource::operator=(const SoundSource& source)
{
    // check for self-assignment
    if (this == &source)
        return *this;
    
    // deallocate previous stuff
    // (see below)
    
    // non dynamically allocated data can be shallow copied
    posRAE = source.posRAE;
    sourceMuted = source.sourceMuted;
    eleDir = source.eleDir;
    sourceSelected = source.sourceSelected;
    if (source.pathPosListener.changed) {
        pathPos = source.pathPos;
        pathPos.removeListeners();
        pathPosListener.changed = false;
        pathPos.addListener(&pathPosListener);
    }
    //pathListener.changed = false;
    //pathPosListener.changed = false;
    
    // need to deep copy pointers that are not null
    if (source.path.get() != nullptr && source.pathListener.changed) {
//        // deallocate previous stuff
//        path.release(); // necessary ?
        path = std::move(source.path->clone());
        path->removeListeners();
        pathListener.changed = false;
        path->addListener(&pathListener);
    }
    return *this;
}

SoundSource::SoundSource(XmlElement *sourceXML)
{
    // restore saved position
    posRAE[0] = sourceXML->getDoubleAttribute("rad");
    posRAE[1] = sourceXML->getDoubleAttribute("azi");
    posRAE[2] = sourceXML->getDoubleAttribute("ele");
    // restore saved interpolaters
    //path = unique_cast<Interpolator<float>, ParametricInterpolator<float>>(getInterpolator(*(sourceXML->getChildElement(0))));
    path = std::move(unique_cast<Interpolator<float>, ParametricInterpolator<float>>(getInterpolator(*(sourceXML->getChildElement(0)))));
    pathPos = dynamic_cast<FunctionalInterpolator<float>&>(*getInterpolator(*(sourceXML->getChildElement(1))));
    pathListener.changed = true;
    pathPosListener.changed = true;
    path->addListener(&pathListener);
    pathPos.addListener(&pathPosListener);
}

XmlElement* SoundSource::getXML() const
{
    // xml element for the whole interpolator object
    XmlElement *sourceXML = new XmlElement("SOUNDSOURCE");
    // store the basic position stuff
    sourceXML->setAttribute("rad", posRAE[0]);
    sourceXML->setAttribute("azi", posRAE[1]);
    sourceXML->setAttribute("ele", posRAE[2]);
    // store the path and pathPos interps
    if (path.get() != nullptr)
        sourceXML->addChildElement(getXML(*path));
    else // to put a dummy XmlElement in so that the indexing for path and pathPos are always sourceXML->getChildElement(0) and sourceXML->getChildElement(1), respectively
        sourceXML->addChildElement(new XmlElement("THERE_IS_NO_PATH"));
    sourceXML->addChildElement(getXML(pathPos));
    return sourceXML;
}

XmlElement* SoundSource::getXML(const Interpolator<float>& interp) const
{
    // xml element for the whole interpolator object
    XmlElement *interpXML = new XmlElement("INTERPOLATOR");
    // store the interp type
    interpXML->setAttribute("type", static_cast<int>(interp.getType()));
    // store the data points in xml
    XmlElement ptsXML ("POINTS");
    // store spline types in xml
    XmlElement splinesXML ("SPLINES");
    std::vector<std::vector<float>> pts = interp.getPoints();
    for (int i = 0; i < pts.size(); ++i)
    {
        XmlElement ptXML ("POINT");
        for (int j = 0; j < pts[i].size(); ++j)
            ptXML.setAttribute(Identifier("pt" + std::to_string(j)), pts[i][j]);
        ptsXML.addChildElement(new XmlElement(ptXML));
        
        XmlElement splineXML ("SPLINE");
        splineXML.setAttribute("spline_type", static_cast<int>(interp.getSplineShape(i)));
        splinesXML.addChildElement(new XmlElement(splineXML));
    }
    interpXML->addChildElement(new XmlElement(ptsXML));
    interpXML->addChildElement(new XmlElement(splinesXML));
    return interpXML;
}

std::unique_ptr<Interpolator<float>> SoundSource::getInterpolator(const XmlElement& interpXML) const
{
    if (interpXML.getTagName().equalsIgnoreCase("INTERPOLATOR"))
    {
        //InterpolatorType type = (InterpolatorType)interpXML.getIntAttribute("pathType");
        InterpolatorType/*int*/ type = static_cast<InterpolatorType>(interpXML.getIntAttribute("type"));
        // first child element should be the xml element containing all dataPts
        XmlElement *xmlPoints = interpXML.getFirstChildElement();
        // next child element contains all the spline segment types, if they got saved ...
        XmlElement *xmlSplines = interpXML.getChildElement(1);
        const int numPts = xmlPoints->getNumChildElements();
        std::vector<std::vector<float>> points (numPts);
        int numSplines;
        switch (type)
        {
            case InterpolatorType::CLOSED_PARAMETRIC:
                numSplines = numPts;
                break;
            case InterpolatorType::OPEN_PARAMETRIC:
                numSplines = std::max(numPts-1, 0);
                break;
            case InterpolatorType::FUNCTIONAL:
                numSplines = std::max(numPts-1, 0);
                break;
        }
        std::vector<SplineShape> splines (numSplines, SplineShape::CUBIC); // spline types default to CUBIC if that data wasn't found in XML
        for (int i = 0; i < numPts; ++i)
        {
            const int dim = xmlPoints->getChildElement(i)->getNumAttributes();
            std::vector<float> pt (dim);
            for (int j = 0; j < dim; ++j)
                pt[j] = xmlPoints->getChildElement(i)->getDoubleAttribute(Identifier("pt" + std::to_string(j)));
            points[i] = pt;
            if (i < numSplines && xmlSplines != nullptr) // keeping support for XML presets that didn't save spline segment types
                splines[i] = static_cast<SplineShape>(xmlSplines->getChildElement(i)->getIntAttribute("spline_type", static_cast<int>(SplineShape::CUBIC)));
        }
        return InterpolatorFactory<float>(type, points, splines);
    }
    else
        return nullptr;
}

void SoundSource::boundsCheckRAE(std::array<float,3>& rae, float& eleDirection) noexcept
{
	float stackrae[3] = { rae[0], rae[1], rae[2] };
	boundsCheckRAE(stackrae, eleDirection);
	rae[0] = stackrae[0]; rae[1] = stackrae[1]; rae[2] = stackrae[2];
}

void SoundSource::boundsCheckRAE(float (&rae)[3], float& eleDirection) noexcept
{
    // bounds checking for the setting the source's position
    while (rae[2] < 0)
        rae[2] += 2.0*M_PI;
    while (rae[2] > 2.0*M_PI)
        rae[2] -= 2.0*M_PI;
    // now eleTemp must be between 0 and 2pi, just have to make sure it is btw 0 and pi now
    if (rae[2] > M_PI)
    {
        rae[2] = M_PI-(rae[2]-M_PI);
        rae[1] = M_PI+rae[1];
        eleDirection *= -1;
    }
    while (rae[1] > 2.0*M_PI)
        rae[1] -= 2.0*M_PI;
    while (rae[1] < 0)
        rae[1] += 2.0*M_PI;
    // no moving source inside the head (rad = 0.1meters), or more than 50m away
    if (rae[0] < distanceBegin)
        rae[0] = distanceBegin;
	else if (rae[0] > 50)
		rae[0] = 50;
}

void SoundSource::boundsCheckXYZ(std::array<float, 3>& xyz)
{
    std::array<float, 3> rae;
    XYZtoRAE(&xyz[0], &rae[0]);
    boundsCheckRAE(rae, eleDir);
    RAEtoXYZ(&rae[0], &xyz[0]);
}

void SoundSource::setPosRAE(std::array<float, 3>& rae)
{
    boundsCheckRAE(rae, eleDir);
    posRAE = rae;
}

std::array<float, 3> SoundSource::getPosRAE() const
{
    return posRAE;
}

void SoundSource::setEleDir(const float newEleDir)
{
    eleDir = newEleDir;
}

float SoundSource::getEleDir() const
{
    return eleDir;
}

void SoundSource::setPosXYZ(const std::array<float, 3>& xyz)
{
    std::array<float, 3> rae;
    XYZtoRAE(&xyz[0], &rae[0]);
    boundsCheckRAE(rae, eleDir);
    posRAE = rae;
}

void SoundSource::setPosXYZ(const float* xyz)
{
    float rae[3];
    XYZtoRAE(xyz, rae);
    boundsCheckRAE(rae, eleDir);
    posRAE[0] = rae[0];
    posRAE[1] = rae[1];
    posRAE[2] = rae[2];
}

std::array<float, 3> SoundSource::getPosXYZ() const
{
    std::array<float, 3> xyz;
    RAEtoXYZ(&posRAE[0], &xyz[0]);
    return xyz;
}

bool SoundSource::setParametricPosition(const float posSec, int& prevPathPosIndex, const float parametricPositionFromDAW)
{
    bool setPosFromPath = false;
    if (path.get() != nullptr && path->getNumPoints() > 1) // only makes sense to set the sources position on its path when the source is moving(playing) and actually on a valid path with at least two points
    {
        float y;
        if (pathPos.getNumPoints()) // if pathPos has points, use that
        {
            if (pathPos.pointAtSmart(posSec, &y, prevPathPosIndex)) // if pathPos exist over current time
            {
            I_LOVE_GOTO:
                sourceMuted = false;
                float xyz[4]; // need 4, and not 3 b/c we stored the eleDir for each point in the 4th dim and we get stack corruption if we don't make room for it here
                float range[2];
                path->getInputRangeQuick(range);
                // the 0.999999 scaling here is to prevent the case when a pt-pt interp would suddenly jump back to the begining of the path if the y value is exactly equal to 1.0
                if (y == y && path->pointAt(y * range[1] * 0.999999f, xyz)) // also make sure y is not a nan
                {
                    setPosXYZ(xyz);
                    setPosFromPath = true;
                }
            }
            else
                sourceMuted = true;
        }
        else // if no pathPos points, use the plugin parameters from with the DAW
        {
            y = parametricPositionFromDAW;
            goto I_LOVE_GOTO;
        }
    }
    return setPosFromPath;
}

void SoundSource::setPositionUpdate(const std::array<float,3>& newPosRAE, const bool newMuted)
{
    posRAE = newPosRAE;
    sourceMuted = newMuted;
}

void SoundSource::setSourceSelected(const bool newSourceSelected) noexcept
{
    sourceSelected = newSourceSelected;
    if (newSourceSelected)
        eleDir = 1; // reset ele dir so all selected sources move the same direction in ele
    // was trying to do keep symetry in ele angle for the selected sources, but need to do that by considering all selected sources positions...
//    if (0 <= posRAE[1] && posRAE[1] < M_PI_2)
//        eleDir = 1.0;
//    else if (M_PI_2 <= posRAE[1] && posRAE[1] < M_PI)
//        eleDir = -1.0;
//    else if (M_PI <= posRAE[1] && posRAE[1] < 3*M_PI_2)
//        eleDir = 1.0;
//    else if (3*M_PI_2 <= posRAE[1] && posRAE[1] < 2*M_PI)
//        eleDir = -1.0;
}

bool SoundSource::getSourceSelected() const noexcept
{
    return sourceSelected;
}

void SoundSource::setSourceMuted(const bool newMutedState)
{
    sourceMuted = newMutedState;
}

bool SoundSource::getSourceMuted() const
{
    return sourceMuted;
}

void SoundSource::addPathPoint()
{
    const std::array<float,3> current = getPosXYZ();
    if (path.get() == nullptr)
    {
        std::vector<std::vector<float>> points;
        points.push_back({current[0], current[1], current[2], 1.0});
        path = std::move(std::unique_ptr<ParametricInterpolator<float>>(new ClosedParametricInterpolator<float>(points)));
        path->addListener(&pathListener);
    }
    else
        path->addPoint({current[0], current[1], current[2], 1.0}); // last value is the elevation direction for the path point
}

void SoundSource::addPathPoint(std::array<float,3>& xyz)
{
    boundsCheckXYZ(xyz);
    if (path.get() == nullptr)
    {
        std::vector<std::vector<float>> points;
        points.push_back({xyz[0], xyz[1], xyz[2], 1.0});
        path = std::move(std::unique_ptr<ParametricInterpolator<float>>(new ClosedParametricInterpolator<float>(points)));
        path->addListener(&pathListener);
    }
    else
        path->addPoint({xyz[0], xyz[1], xyz[2], 1.0}); // last value is the elevation direction for the path point
}

int SoundSource::deleteSelectedPathPoints()
{
    if (path.get() != nullptr)
        return path->deleteSelectedPoints();
    else
        return 0;
}

void SoundSource::setPathType(const int pathType)
{
    if (path.get() != nullptr && path->getType() != (InterpolatorType)pathType)
    {
        if ((InterpolatorType)pathType == InterpolatorType::OPEN_PARAMETRIC)
            path = std::move(std::unique_ptr<ParametricInterpolator<float>>(new OpenParametricInterpolator<float>(dynamic_cast<ClosedParametricInterpolator<float>&>(*path))));
        else if ((InterpolatorType)pathType == InterpolatorType::CLOSED_PARAMETRIC)
            path = std::move(std::unique_ptr<ParametricInterpolator<float>>(new ClosedParametricInterpolator<float>(dynamic_cast<OpenParametricInterpolator<float>&>(*path))));
    }
}

//std::unique_ptr<ParametricInterpolator<float>> SoundSource::getPath() const
//{
//    if (path)
//        return path->clone();
//    else
//        return nullptr;
//}

const ParametricInterpolator<float>* const SoundSource::getPathPtr() const
{
    return (const ParametricInterpolator<float>* const) path.get();
}

ParametricInterpolator<float>* const SoundSource::getPathPtrMutable()
{
    return (ParametricInterpolator<float>* const) path.get();
}


//FunctionalInterpolator<float> SoundSource::getPathPos() const
//{
//    return pathPos;
//}

FunctionalInterpolator<float>* SoundSource::getPathPosPtr() noexcept
{
    return &pathPos;
}

const FunctionalInterpolator<float>* const SoundSource::getPathPosPtr() const noexcept
{
    return (const FunctionalInterpolator<float>* const) &pathPos;
}

void SoundSource::doneUpdatingPath() noexcept
{
    pathListener.changed = false;
}

void SoundSource::setPathChanged(const bool changed) noexcept
{
    pathListener.changed = changed;
}

void SoundSource::doneUpdatingPathPos() noexcept
{
    pathPosListener.changed = false;
}

void SoundSource::setPathPosChanged(const bool changed) noexcept
{
    pathPosListener.changed = changed;
}

bool SoundSource::moveSelectedPathPointsXYZ(const float dX, const float dY, const float dZ)
{
    // NOTE: the bounds checking for not allowing a path point inside the head was removed
    if (path.get() != nullptr)
    {
        const int numMoved = path->moveSelectedPoints({dX,dY,dZ});
        // return value signals weather or not the source(s) should be moved too
        return (numMoved > 0 && numMoved != path->getNumPoints());
    }
    else
        return false;
}

bool SoundSource::moveSelectedPathPointsRAE(const float dRad, const float dAzi, const float dEle)
{
    // normally we wouldn't move points of an interp like this, but the continuous elevation and RAE coordinate system is not supported by Interpolators
    if (path.get() != nullptr)
    {
        std::vector<std::vector<float>> selPts = path->getSelectedPoints();
        std::array<float,3> rae {0};
        std::array<float,3> xyz {0};
        std::vector<int> selPtsIndicies = path->getSelectedPointIndices();
        for (int i = 0; i < selPts.size(); ++i)
        {
            float selpt[3] = {selPts[i][0], selPts[i][1], selPts[i][2]};
            XYZtoRAE(&selpt[0], &rae[0]);
            rae[0] *= dRad;
            rae[1] += dAzi;
            rae[2] += selPts[i][3]*dEle;
            boundsCheckRAE(rae, selPts[i][3]);
            RAEtoXYZ(&rae[0], &xyz[0]);
            path->setPointPosition({xyz[0], xyz[1], xyz[2], selPts[i][3]}, selPtsIndicies[i]);
        }
        if (selPts.size() > 0)
            path->recalcSplines();
        return (selPts.size() > 0 && selPts.size() != path->getNumPoints());
    }
    else
        return false;
}

void SoundSource::setAllPathPointsSelected(const bool selectedState)
{
    if (path.get() != nullptr)
    {
        path->setAllPointsSelected(selectedState);
        if (selectedState)
        {// reset ele dir so all path points move same direction in ele
            int i = 0;
            for (auto& pt : path->getPoints())
            {
                pt[3] = 1.0;
                path->setPointPosition(pt, i++);
            }
        }
    }
}

void SoundSource::setPathPointSelected(const int ptIndex, const bool selectedState)
{
    if (path)
    {
        path->setPointSelected(ptIndex, selectedState);
        if (selectedState)
        {// reset ele dir so all path points move same direction in ele
            std::vector<float> resetEleDirPt = path->getPoint(ptIndex);
            resetEleDirPt[3] = 1.0;
            path->setPointPosition(resetEleDirPt, ptIndex);
        }
    }
}

bool SoundSource::getPathPointSelected(const int ptIndex) const
{
    if (path)
        return path->getPointSelected(ptIndex);
    else
        return false;
}

void SoundSource::setPathAutomationPointSelected(const int ptIndex, const bool selectedState)
{
    pathPos.setPointSelected(ptIndex, selectedState);
}

bool SoundSource::getPathAutomationPointSelected(const int ptIndex) const
{
    return pathPos.getPointSelected(ptIndex);
}

int SoundSource::moveSelectedPathAutomationPoints(const float dx, const float dy)
{
    return pathPos.moveSelectedPoints({dx, dy});
}

//std::vector<int> SoundSource::moveSelectedPathAutomationPointsWithReorderingInfo(const float dx, const float dy)
//{
//    return pathPos.moveSelectedPointsWithReorderingInfo({dx, dy});
//}

bool SoundSource::deleteSelectedAutomationPoints()
{
    return pathPos.deleteSelectedPoints() > 0;
}

void SoundSource::setAllPathAutomationPointsSelected(const bool selectedState)
{
    pathPos.setAllPointsSelected(selectedState);
}

void SoundSource::addPathAutomationPoint(const float x, const float y)
{
    pathPos.addPoint({x, y});
}

int SoundSource::copySelectedPathPoints()
{
    if (path.get() != nullptr)
        return path->copySelectedPoints();
    else
        return 0;
}

int SoundSource::copySelectedPathAutomationPoints()
{
    return pathPos.copySelectedPoints();
}

std::vector<std::vector<float>> SoundSource::getPathPoints() const
{
    if (path)
        return path->getPoints();
    else
        return std::vector<std::vector<float>>();
}

int SoundSource::getNumPathPoints() const
{
    if (path.get() != nullptr)
        return path->getNumPoints();
    else
        return 0;
}

int SoundSource::getNumPathAutomationPoints() const
{
    return pathPos.getNumPoints();
}

int SoundSource::getNumSelectedPathPoints() const
{
    if (path.get() != nullptr)
        return path->getNumSelectedPoints();
    else
        return 0;
}

int SoundSource::getNumSelectedPathAutomationPoints() const
{
    return pathPos.getNumSelectedPoints();
}

std::vector<bool> SoundSource::getSelectedPathPoints() const
{
    if (path.get() != nullptr)
        return path->getPointsSelected();
    else
        return std::vector<bool>();
}

std::vector<bool> SoundSource::getSelectedPathAutomationPoints() const
{
    return pathPos.getPointsSelected();
}

bool SoundSource::setSelectedPathAutomationPointsSegmentType(const int newSegType)
{
    return pathPos.setSelectedSplinesType((SplineShape)newSegType) > 0;
}


/***** PlayableSoundSource *****/
PlayableSoundSource::PlayableSoundSource()
{
    interpolateHRIR(&posRAE[0], &HRIR[0]);
    HRIRScaling[0] = HRIRScaling[1] = 0;//= HRIRScaling[2] = HRIRScaling[3] = 0;
    for (int n = 0; n < numTimeSteps; ++n) {
        HRIRScaling[0] += std::abs(HRIR[             n]);
        //HRIRScaling[2] += std::abs(HRIR[             n]);
        HRIRScaling[1] += std::abs(HRIR[numTimeSteps+n]);
        //HRIRScaling[3] += std::abs(HRIR[numTimeSteps+n]);
    }
    HRIRScaling[0] = 1.0/HRIRScaling[0];
    HRIRScaling[1] = 1.0/HRIRScaling[1];
    // hoping this (init of HRIRs at construction) might fix the random fuzz issue with moving sources, it did seem to work...
    for (int n = 0; n < numTimeSteps; ++n)
    {
        HRIR[             n] *= HRIRScaling[0];
        HRIR[numTimeSteps+n] *= HRIRScaling[1];
        HRIRs[2*numTimeSteps+n] = HRIRs[             n] = HRIR[             n];// * HRIRScaling[0];
        //HRIRs[2*numTimeSteps+n] = HRIR[             n];// * HRIRScaling[0];
        HRIRs[3*numTimeSteps+n] = HRIRs[numTimeSteps+n] = HRIR[numTimeSteps+n];// * HRIRScaling[1];
        //HRIRs[3*numTimeSteps+n] = HRIR[numTimeSteps+n];// * HRIRScaling[1];
    }
    HRIRScaling[2] = HRIRScaling[0] = 1.0/HRIRScaling[0];
    HRIRScaling[3] = HRIRScaling[1] = 1.0/HRIRScaling[1];
}

PlayableSoundSource::~PlayableSoundSource()
{
    //inputs.clear();
    //for (int i = 0; i < inputs.size(); ++i)
    //   inputs[i].~_Input_();

    //delete[] hqHRIRs;
    //delete[] hqHRIRScaling;
    //delete[] temp;
}

void PlayableSoundSource::advancePosition() noexcept
{
    if (prevHRIRChange && dopplerOn)
    {
        HRIRChange = true;
        float currentXYZ[3];
        RAEtoXYZ(&posRAE[0], currentXYZ);
        float prevXYZ[3];
        RAEtoXYZ(&pprevRAE[0], prevXYZ);
        float newXYZ[3] = { currentXYZ[0] + currentXYZ[0]-prevXYZ[0],
                            currentXYZ[1] + currentXYZ[1]-prevXYZ[1],
                            currentXYZ[2] + currentXYZ[2]-prevXYZ[2] };
        prevRAE = posRAE; // need this so that pprevRAE can get properly updated in processAudio() if advancePosition() gets called two or more buffers in a row
        XYZtoRAE(newXYZ, &posRAE[0]);
    }
}

void PlayableSoundSource::updateFromSoundSource(const SoundSource& source) noexcept
{
    if (posRAE != source.posRAE)
    {
        HRIRChange = true;
        posRAE = source.posRAE;
    }
//    // SMOOTH TRANSITION
//    if (posRAE != source.posRAE && !HRIRChange)
//    {
//        HRIRChange = true;
//        prevRAE = posRAE;
//        posRAE = source.posRAE;
//        currentTransitionTime = nextTransitionTime;//std::min(nextTransitionTime, maxTransitionTime);
//        nextTransitionTime = 0;
//    }
    sourceMuted = source.sourceMuted;
}

std::array<float,3> PlayableSoundSource::getPosRAE() const noexcept
{
    return posRAE;
}

void PlayableSoundSource::allocateForMaxBufferSize(const int N_max)
{
    Nmax = N_max;
	inputBuffer.resize(Nmax * (std::ceil(float(numTimeSteps - 1) / float(Nmax)) + 1), 0.0f);
	inputBufferInPos = 0;
	inputBufferOutPos = 0;
	const int maxNumHRIRs = (Nmax >> 1) + 1; // new hrir position for each 2 samples seems more than sufficient...
	hqHRIRs.resize(maxNumHRIRs * 2 * numTimeSteps, 0);
	hqHRIRScaling.resize(maxNumHRIRs * 2, 0);
    //inputs.resize(std::ceil((float)(numTimeSteps-1)/((float)Nmax)) + 1);
    //for (auto& input : inputs)
    //    input.setSize(Nmax);
    //newInputIndex = 0;
    if (dopplerOn)
    {
        //doppler[0].free();
        //doppler[1].free();
        doppler[0].allocate(dopplerMaxDistance, Nmax, 0.1f/*dopplerSpeedOfSound*/);
        doppler[1].allocate(dopplerMaxDistance, Nmax, 0.1f/*dopplerSpeedOfSound*/);
    }
}

//void PlayableSoundSource::setRealTime(const bool isRealTime) noexcept
//{
//    realTime = isRealTime;
//}
//
//bool PlayableSoundSource::getRealTime() const noexcept
//{
//    return realTime;
//}

void PlayableSoundSource::setDopplerOn(const bool newDopplerOn, const float newSpeedOfSound)
{
    const bool speedOfSoundChanged = (newSpeedOfSound != dopplerSpeedOfSound);
    dopplerSpeedOfSound = newSpeedOfSound;
	doppler[0].setSpeedOfSound(dopplerSpeedOfSound);
	doppler[1].setSpeedOfSound(dopplerSpeedOfSound);
	if (newDopplerOn != dopplerOn) {
		if (newDopplerOn) {
			doppler[0].allocate(dopplerMaxDistance, Nmax, 0.1f);
			doppler[1].allocate(dopplerMaxDistance, Nmax, 0.1f);
		} else {
			doppler[0].free();
			doppler[1].free();
		}
		// reset processing state of sound source
		for (auto& i : inputs)
			i.clear();
		newInputIndex = 0;
		HRIRChange = false;
		prevRAE = posRAE;
	}
    //if (newDopplerOn != dopplerOn || speedOfSoundChanged)
    //{
    //    if (newDopplerOn || speedOfSoundChanged)
    //    {
    //        doppler[0].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
    //        doppler[1].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
    //    }
    //    else
    //    {
    //        doppler[0].free();
    //        doppler[1].free();
    //    }
    //    // reset processing state of sound source
    //    for (auto& i : inputs)
    //        i.clear();
    //    newInputIndex = 0;
    //    HRIRChange = false;
    //    prevRAE = posRAE;
    //}
    dopplerOn = newDopplerOn;
}

void PlayableSoundSource::setDopplerSampleRate(const float sampleRate) noexcept
{
    doppler[0].setSampleRate(sampleRate);
    doppler[1].setSampleRate(sampleRate);
}

void PlayableSoundSource::setSourceMuted(const bool newMutedState) noexcept
{
    sourceMuted = newMutedState;
}

bool PlayableSoundSource::getSourceMuted() const noexcept
{
    return sourceMuted;
}

void PlayableSoundSource::resetProcessingState() noexcept
{
    if (dopplerOn)
    {
        doppler[0].reset();
        doppler[1].reset();
    }
    for (auto& i : inputs)
        i.clear();
    newInputIndex = 0;
    HRIRChange = false;
    prevRAE = posRAE;
}

void PlayableSoundSource::processAudio(const float* in, const int N, float* out, const bool realTime)
{
    float* whichHRIRs = nullptr;
    float* whichHRIRScaling = nullptr;
    // if we had an HRIRChange update we gotta interpolate that hrir data for the blended output
    if (HRIRChange) {
        if (realTime) {
            // to process in realtime, need to limit the number of blending positions to something that can be reasonably computed
            numHRIRs = 2; // must be 2 in order to get away with only making one new interpolateHRIR() call
            whichHRIRs = &HRIRs[0];
            whichHRIRScaling = &HRIRScaling[0];
            // end of the positional interps (only one that needs computation for realtime)
            interpolateHRIR(&posRAE[0], &HRIRs[2*numTimeSteps]);
            // this pre-convolution normalization is required to get rid of the crackling in the quiet ear for close sources due to floating point addition inaccuracy
            HRIRScaling[2] = HRIRScaling[3] = 0;
            for (int n = 0; n < numTimeSteps; ++n) {
                HRIRScaling[2] += std::abs(HRIRs[2*numTimeSteps+n]);
                HRIRScaling[3] += std::abs(HRIRs[3*numTimeSteps+n]);
            }
            HRIRScaling[2] = 1.0/HRIRScaling[2];
            HRIRScaling[3] = 1.0/HRIRScaling[3];
            for (int n = 0; n < numTimeSteps; ++n) {
                HRIRs[2*numTimeSteps+n] *= HRIRScaling[2];
                HRIRs[3*numTimeSteps+n] *= HRIRScaling[3];
            }
            HRIRScaling[2] = 1.0/HRIRScaling[2];
            HRIRScaling[3] = 1.0/HRIRScaling[3];
		}
		else {
			// for non-realtime processing, we can go crazy and have each output sample be processed with a different blending position for nice smooth audio despite potentially fast moving source
			// (note the N+1 instead of N because the blend widths are calculated L = 2N/(numHRIRs-1) and we want L = 2.0 in this case)
			numHRIRs = (N >> 2) + 1; // new hrir position for each 2 samples seems more than sufficient...
			//const int newNumHRIRs = (N >> 1/*HRIRInterpQuality*/) + 1; // new hrir position for each 2 samples seems more than sufficient...
			//if (newNumHRIRs != numHRIRs) {
			//	numHRIRs = newNumHRIRs;
			//	delete[] hqHRIRs;
			//	hqHRIRs = new float[numHRIRs*2*numTimeSteps];
			//	delete[] hqHRIRScaling;
			//	hqHRIRScaling = new float[numHRIRs*2];
			//}
			whichHRIRs = &hqHRIRs[0];
            whichHRIRScaling = &hqHRIRScaling[0];
            const int lastHRIR = numHRIRs-1;
            // end of the positional interps (only one that needs computation for realtime)
            interpolateHRIR(&posRAE[0], &hqHRIRs[lastHRIR*2*numTimeSteps]);
            // pre-convolution normalization
            hqHRIRScaling[lastHRIR*2] = hqHRIRScaling[lastHRIR*2+1] = 0;
            for (int n = 0; n < numTimeSteps; ++n) {
                hqHRIRScaling[lastHRIR*2  ] += std::abs(hqHRIRs[ lastHRIR*2   *numTimeSteps+n]);
                hqHRIRScaling[lastHRIR*2+1] += std::abs(hqHRIRs[(lastHRIR*2+1)*numTimeSteps+n]);
            }
            hqHRIRScaling[lastHRIR*2  ] = 1.0/hqHRIRScaling[lastHRIR*2  ];
            hqHRIRScaling[lastHRIR*2+1] = 1.0/hqHRIRScaling[lastHRIR*2+1];
            for (int n = 0; n < numTimeSteps; ++n) {
                hqHRIRs[ lastHRIR*2   *numTimeSteps+n] *= hqHRIRScaling[lastHRIR*2  ];
                hqHRIRs[(lastHRIR*2+1)*numTimeSteps+n] *= hqHRIRScaling[lastHRIR*2+1];
            }
            hqHRIRScaling[lastHRIR*2  ] = 1.0/hqHRIRScaling[lastHRIR*2  ];
            hqHRIRScaling[lastHRIR*2+1] = 1.0/hqHRIRScaling[lastHRIR*2+1];
            hqHRIRScaling[0] = HRIRScaling[0]; // load the first hrir pos scaling factors
            hqHRIRScaling[1] = HRIRScaling[1];
            // number of interps minus the endpoints which have already been interped!
            const int numInterps = numHRIRs-2;
            // positional data for blending
            float posXYZ[3];
            float pos_RAE[3];
            float xyzCurrent[3];
            RAEtoXYZ(&prevRAE[0], &xyzCurrent[0]);
            float xyzNext[3];
            RAEtoXYZ(&posRAE[0], &xyzNext[0]);
            const float oneOverNumInterpsP1 = 1.0/(numInterps+1);
            const float factorX = oneOverNumInterpsP1 * (xyzNext[0]-xyzCurrent[0]);
            const float factorY = oneOverNumInterpsP1 * (xyzNext[1]-xyzCurrent[1]);
            const float factorZ = oneOverNumInterpsP1 * (xyzNext[2]-xyzCurrent[2]);
            // interpolate positions and hrirs for those positions
            for (int i = 1; i <= numInterps; ++i) {
                // interpolate intermediate positions in xyz land
                posXYZ[0] = i * factorX + xyzCurrent[0];
                posXYZ[1] = i * factorY + xyzCurrent[1];
                posXYZ[2] = i * factorZ + xyzCurrent[2];
                // convert back to spherical
                XYZtoRAE(&posXYZ[0], &pos_RAE[0]);
                interpolateHRIR(pos_RAE, &hqHRIRs[i*2*numTimeSteps]);
                // pre-convolution normalization
                hqHRIRScaling[i*2] = hqHRIRScaling[i*2+1] = 0;
                for (int n = 0; n < numTimeSteps; ++n) {
                    hqHRIRScaling[i*2  ] += std::abs(hqHRIRs[ i*2   *numTimeSteps+n]);
                    hqHRIRScaling[i*2+1] += std::abs(hqHRIRs[(i*2+1)*numTimeSteps+n]);
                }
                hqHRIRScaling[i*2  ] = 1.0/hqHRIRScaling[i*2  ];
                hqHRIRScaling[i*2+1] = 1.0/hqHRIRScaling[i*2+1];
                for (int n = 0; n < numTimeSteps; ++n) {
                    hqHRIRs[ i*2   *numTimeSteps+n] *= hqHRIRScaling[i*2  ];
                    hqHRIRs[(i*2+1)*numTimeSteps+n] *= hqHRIRScaling[i*2+1];
                }
                hqHRIRScaling[i*2  ] = 1.0/hqHRIRScaling[i*2  ];
                hqHRIRScaling[i*2+1] = 1.0/hqHRIRScaling[i*2+1];
            }
        }
        // load the "current" hrir into the blended HRIRs and make "current" hrir the one for the next position, think that screwy stuff with the HRIR data is causing those rare fuzzes when the sources moves, still not sure what to do to fix it...
//        for (int ch = 0; ch < 2; ++ch) {
//            for (int n = 0; n < numTimeSteps; ++n) {
//                whichHRIRs[ch*numTimeSteps+n] = HRIR[ch*numTimeSteps+n];
//                HRIR[ch*numTimeSteps+n] = whichHRIRs[((numHRIRs-1)*2+ch)*numTimeSteps+n];
//            }
//        }
        for (int n = 0; n < numTimeSteps; ++n) {
            // ch 0
            whichHRIRs[             n] = HRIR      [             n];
            HRIR      [             n] = whichHRIRs[((numHRIRs-1)*2)  *numTimeSteps+n];
            // ch 1
            whichHRIRs[numTimeSteps+n] = HRIR      [numTimeSteps+n];
            HRIR      [numTimeSteps+n] = whichHRIRs[((numHRIRs-1)*2+1)*numTimeSteps+n];
        }
        // advance positional state
        pprevRAE = prevRAE;
        prevRAE = posRAE;
    } // end if HRIRChange
    // load the current input
	for (int n = 0; n < N; ++n) {
		inputBuffer[inputBufferInPos] = in[n];
		inputBufferInPos = (inputBufferInPos + 1) % inputBuffer.size();
	}

//	// old input inserting
//    inputs[newInputIndex].load(in, N);
//    const int numInputs = inputs.size();
//    int buflengths = 0;
//    const int begin = (newInputIndex+1) % numInputs;
//    const int end = newInputIndex;
//    const int beginInputIndex = newInputIndex;
//    int inputsToProcess = 1;
//    for (int i = begin; i != end; i = (i+1) % numInputs) {
//        ++inputsToProcess;
//        buflengths += inputs[i].N;
//        if (inputs[i].N+numTimeSteps-1 <= buflengths) {
//            break;
//        }
//    }
//    //const int numInputsToProcess = inputsToProcess;
//    if (--newInputIndex < 0)
//        newInputIndex = numInputs - 1;
////    inputs.insert(0, new Input(in, N));
////    // remove any previous buffers that are no longer needed...
////    int buflengths = 0;
////    for (int i = 1; i < inputs.size(); ++i) {
////        buflengths += inputs[i]->N;
////        if (inputs[i]->N+numTimeSteps-1 <= buflengths) {
////            delete inputs[i];
////            inputs.remove(i);
////            --i;
////        }
////    }
	
    // allocate final output array
    STACK_ARRAY(float, yfinal, N);
    // process for each ear
    for (int ch = 0; ch < 2; ++ch) {
        // zero fill output array
        for (int n = 0; n < N; ++n)
            yfinal[n] = 0;
            
        // blending hrirs in this buffer
        if (HRIRChange) {
            //// init array of outputs for each hrir chunk and each needed previous input's tail
            //const int N_max = Nmax;
            //float* ytemp = nullptr;
            //const int tempSize = numInputs*numHRIRs*(N_max+numTimeSteps-1);
            //STACK_ARRAY(float, y, realTime ? tempSize : 0);
            //if (realTime) {
            //    ytemp = &y[0];
            //} else {  // using new here to allocate on the heap instead of the stack which will likely overflow for numHRIRs = big number in the hq hrir interpolated non-realtime case
            //    if (prevTempSize != tempSize) {
            //        prevTempSize = tempSize;
            //        delete[] temp;
            //        temp = new float[tempSize];
            //    }
            //    ytemp = &temp[0];
            //}
            //// length of blending region (in samples) that blends between two consecutive interped hrirs
            //const float L = ((float)N) / ((float)(numHRIRs-1));
            //int beginIndex = 0, endIndex, thing, beginIndex2, endIndex2;
            //for (int k = 0, i = beginInputIndex; k < (const int)inputsToProcess; i = (i+1) % numInputs, ++k) {
            //    if (i != beginInputIndex)
            //        beginIndex += inputs[i].N;
            //    // if the tail for this previous input goes up to or past the end of this current buffer
            //    thing = inputs[i].N+numTimeSteps-1 - (N+beginIndex);
            //    if (thing >= 0)
            //        endIndex = beginIndex + N-1;
            //    else // otherwise the tail for this previous input ends somewhere in this current buffer
            //        endIndex = beginIndex + N-1 + thing;
            //    // loop through each hrir and do the positional convolution chunks
            //    for (int j = 0; j < numHRIRs-1; ++j) {
            //        // indexing for the hrir section of interest in terms of the respective input's indexing
            //        beginIndex2 = std::floor( ((float)j)*L + beginIndex );
            //        endIndex2 = std::ceil( ((float)(j+1))*L + beginIndex );
            //        // make sure to not go past the end of the convolution
            //        if (endIndex2 > endIndex)
            //            endIndex2 = endIndex;
            //        // make sure we even need to do the convolution for this positional hrir chunk
            //        if (beginIndex2 <= endIndex2) {
            //            convolve(&inputs[i].input[0], inputs[i].N,
            //                     &whichHRIRs[ ( j  *2+ch)*numTimeSteps], numTimeSteps,
            //                     &ytemp[(i*numHRIRs+  j  )*(N_max+numTimeSteps-1)]/*y[i][j]*/  , beginIndex2, endIndex2);
            //            convolve(&inputs[i].input[0], inputs[i].N,
            //                     &whichHRIRs[((j+1)*2+ch)*numTimeSteps], numTimeSteps,
            //                     &ytemp[(i*numHRIRs+(j+1))*(N_max+numTimeSteps-1)]/*y[i][j+1]*/, beginIndex2, endIndex2);
            //        }
            //    }
            //}
            //int j, c1, c2, offset;
            //for (int n = 0; n < N; ++n) {
            //    // which hrir position chunk are we in
            //    j = std::floor( ((float)n) / L );
            //    // starts at 0.0 at multiples of L(frac) samples, goes up to ~< 1.0 at the sample before the next multiple
            //    c2 = (((float)n) - ((float)j)*L) / L;
            //    c1 = 1.0 - c2;
            //    offset = 0;
            //    for (int k = 0, i = beginInputIndex; k < (const int)inputsToProcess; i = (i+1) % numInputs, ++k) {
            //        if (i != beginInputIndex)
            //            offset += inputs[i].N;
            //        // don't want to exceed the bounds of the y[][][N+numtimesteps-1] array
            //        if (n+offset < inputs[i].N+numTimeSteps-1) {
            //            yfinal[n] += c1 * whichHRIRScaling[  j  *2+ch]
            //                            * ytemp[((i*numHRIRs+  j  )*(N_max+numTimeSteps-1))+(n+offset)]/*y[i][j][n+offset]*/
            //                       + c2 * whichHRIRScaling[(j+1)*2+ch]
            //                            * ytemp[((i*numHRIRs+(j+1))*(N_max+numTimeSteps-1))+(n+offset)]/*y[i][j+1][n+offset]*/;
            //        }
            //    }
            //}
			convolve(&inputBuffer[0], inputBufferOutPos, inputBuffer.size(),
					 &whichHRIRs[0], numTimeSteps, numHRIRs, &whichHRIRScaling[0], ch,
				     &yfinal[0], N);
            // advance the HRIR scaling stuff
            HRIRScaling[0] = whichHRIRScaling[(numHRIRs-1)*2];
            HRIRScaling[1] = whichHRIRScaling[(numHRIRs-1)*2+1];
        } else { // no blending to do in this buffer as we are stationary

			convolve(&inputBuffer[0], inputBufferOutPos, inputBuffer.size(), 
				     &HRIR[ch*numTimeSteps], numTimeSteps, HRIRScaling[ch],
				     &yfinal[0], N);

//            // do convolutions for all the inputs that are needed to render this buffers output
//            // note that begin and end indecies are refering to the previous buffer's indexing context, not the current buffer's
//            int beginIndex = 0, endIndex, thing;
//            //for (int i = 0; i < inputs.size(); ++i) {
//            for (int k = 0, i = beginInputIndex; k < (const int)inputsToProcess; i = (i+1) % numInputs, ++k) {
//                // intermediate output for each input buffer
//                STACK_ARRAY(float, y, inputs[i].N+numTimeSteps-1)
////            #ifdef WIN32
////                float *y = static_cast<float *>(alloca((inputs[i].N + numTimeSteps - 1) * sizeof(float)));
////            #else
////                float y[inputs[i].N+numTimeSteps-1];
////            #endif
//                //if (i != 0)
//                //    beginIndex += inputs[i]->N;
//                if (i != beginInputIndex)
//                    beginIndex += inputs[i].N;
//                // if the tail for this previous input goes up to or past the end of this current buffer
//                thing = inputs[i].N+numTimeSteps-1 - (N+beginIndex);
//                if (thing >= 0)
//                    endIndex = beginIndex + N-1;
//                else // otherwise the tail for this previous input ends somewhere in this current buffer
//                    endIndex = beginIndex + N-1 + thing;
//                // do the convolution
//                convolve(&inputs[i].input[0], inputs[i].N, &HRIR[ch*numTimeSteps], numTimeSteps, y, beginIndex, endIndex);
//                for (int n = beginIndex; n <= endIndex; ++n)
//                    yfinal[n-beginIndex] += y[n] * HRIRScaling[ch];
//            }
        }
        // apply doppler effect
        if (dopplerOn) {
            STACK_ARRAY(float, yDoppler, N)
            float sourceXYZ[3];
            RAEtoXYZ(&posRAE[0], sourceXYZ);
            float earXYZ[3];
			const float earRAE[3] {sphereRad, static_cast<float>(ch == 0 ? earAzimuth : -earAzimuth), earElevation};
			RAEtoXYZ(earRAE, earXYZ);
            const float dx = sourceXYZ[0] - earXYZ[0];
            const float dy = sourceXYZ[1] - earXYZ[1];
            const float dz = sourceXYZ[2] - earXYZ[2];
            const float earToSourceDistance = std::sqrt(dx*dx + dy*dy + dz*dz);
            if (earToSourceDistance > dopplerMaxDistance) {
                // shouldn't happen that often, so reallocing here when necessary shouldn't cause any big problems
                dopplerMaxDistance = earToSourceDistance * 2;
                doppler[0].allocate(dopplerMaxDistance, Nmax, 0.1f/*dopplerSpeedOfSound*/);
                doppler[1].allocate(dopplerMaxDistance, Nmax, 0.1f/*dopplerSpeedOfSound*/);
                //dopplerMaxDistanceChanged = true;
            }
            doppler[ch].process(earToSourceDistance, N, yfinal, yDoppler);
            // package each channel's output into one dual-channel array
			for (int n = 0; n < N; ++n)
				out[ch*N + n] += yDoppler[n];
		}
		else { // no doppler effect
			// package each channel's output into one dual-channel array
			for (int n = 0; n < N; ++n)
				out[ch*N + n] += yfinal[n];
        }
    } // end for each channel
	inputBufferOutPos = (inputBufferOutPos + N) % inputBuffer.size();
    prevHRIRChange = HRIRChange;
    // set the state of movement so that the next buffer is stationary, which may change if we get an updated position from the gl side
    HRIRChange = false;
}

// the global hrir data that gets one instance across multiple plugin instances, this just references the one instance defined in PluginProcessor.cpp
extern float***** HRIRdata;
extern float**** HRIRdataPoles;

// compacted (one azimuth side provided) with pole data version
void PlayableSoundSource::interpolateHRIR(const float* rae, float* hrir) const noexcept
{
    // get the inner + outer rad,azi,ele indicies that define the 3d region bounded by the hrtf/dvf sampling resolution that the source is currently located in
    const int innerRadiusIndex = std::max(0, std::min((int)((std::log(rae[0])-std::log(distanceBegin))/std::log(distanceEnd/distanceBegin)*(numDistanceSteps-1)), numDistanceSteps-2));//-1);
    const int outerRadiusIndex = innerRadiusIndex+1;// std::min(innerRadiusIndex+1, numDistanceSteps-1);
    
    const int lowerElevationIndex = std::max(0, std::min((int)std::floor(rae[2]/M_PI*numElevationSteps), numElevationSteps-1));
    const int upperElevationIndex = lowerElevationIndex+1;
    
    const float revAzi = std::fmod(4*M_PI-rae[1], 2*M_PI); // fix reversed azimuth indexing with hrir array's, this caused lowerAzimuthIndex = -1 without fmod
    int lowerAzimuthIndex = std::min((int)std::floor(revAzi/(2*M_PI)*numAzimuthSteps), numAzimuthSteps-1);
    int upperAzimuthIndex = (lowerAzimuthIndex+1) % numAzimuthSteps;
    
    // inner/outer surface radius values
    const float rIn  = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)innerRadiusIndex)/(numDistanceSteps-1));
    const float rOut = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)outerRadiusIndex)/(numDistanceSteps-1));
    
    // upper/lower azimuth values
    const float aP = ((float)(upperAzimuthIndex))*(2*M_PI)/numAzimuthSteps;
    const float aM = ((float)(lowerAzimuthIndex))*(2*M_PI)/numAzimuthSteps;
    
    // upper/lower elevation values
    const float eM = ((float)lowerElevationIndex)*M_PI/numElevationSteps;
    const float eP = ((float)upperElevationIndex)*M_PI/numElevationSteps;
    
    // for making close/far more loud/quiet
    const float intensity_factor = 0.1 / std::pow(rae[0], 0.5);
    
    const float mu3 = 0.5*intensity_factor*std::min((rae[0]-rIn)/(rOut-rIn), 1.0f); // scaled by 1/2*intensity_factor here instead of for each sample below
    
    // interpolate along azimuth edges
    const float mu1_01 = (rae[2]-eM)*numElevationSteps/M_PI; // should be btw 0 and 1
    const float mu1 = mu1_01 + 2;                            // should be btw 2 and 3
    const float nmu1 = std::abs(2.5-mu1); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
    
    // interpolate along elevation edges
    const float mu2_01 = (revAzi-aM)*numAzimuthSteps/(2.0*M_PI); // should be btw 0 and 1
    const float mu2 = mu2_01 + 2;                                // should be btw 2 and 3
    const float nmu2 = std::abs(2.5-mu2); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
    
    // variables for bounds wrapping surrounding data
    int uAzip1 = upperAzimuthIndex+1;
    int lAzim1 = lowerAzimuthIndex-1;
    int uElep1 = upperElevationIndex+1;
    int lElem1 = lowerElevationIndex-1;
    
    if (uAzip1 > numAzimuthSteps-1)
        uAzip1 -= numAzimuthSteps;
    if (lAzim1 < 0)
        lAzim1 += numAzimuthSteps;
    
    bool lElem1Flip = false, uElep1Flip = false; // ele's need bounds wrapping and if they wrap, the corresponding azi it is used with needs flipping
    if (uElep1 > numElevationSteps) {
        uElep1 = 2*numElevationSteps - uElep1;
        uElep1Flip = true;
    }
    if (lElem1 < 0) {
        lElem1 = -lElem1;
        lElem1Flip = true;
    }
    
    int nAzi2, nEle2;  // neighboring azi/ele indices
    bool nAziUp, nEleUp;
    if ((aP > aM ? aP-revAzi : 2.0*M_PI-revAzi) > revAzi-aM) {
        nAzi2 = lowerAzimuthIndex-2;
        nAziUp = false;
    } else {
        nAzi2 = upperAzimuthIndex+2;
        nAziUp = true;
    }
    if (eP-rae[2] > rae[2]-eM) {
        nEle2 = lowerElevationIndex-2;
        nEleUp = false;
    } else {
        nEle2 = upperElevationIndex+2;
        nEleUp = true;
    }
    
    if (nAzi2 > numAzimuthSteps-1)
        nAzi2 -= numAzimuthSteps;
    if (nAzi2 < 0)
        nAzi2 += numAzimuthSteps;
    
    bool nEleFlip = false; // ele's need bounds wrapping and if they wrap, the corresponding azi needs flipping
    if (nEle2 > numElevationSteps) {
        nEle2 = 2*numElevationSteps - nEle2;
        nEleFlip = true;
    }
    if (nEle2 < 0) {
        nEle2 = -nEle2;
        nEleFlip = true;
    }
    
    // for the compacted data with only one azimuth side provided, we gotta do some channel flipping
    int lAziBaseCh = 0, uAziBaseCh = 0;
    if (lowerAzimuthIndex > numAzimuthSteps/2) {
        lowerAzimuthIndex = numAzimuthSteps - lowerAzimuthIndex;// numAzimuthSteps/2 - (lowerAzimuthIndex - numAzimuthSteps/2);
        lAziBaseCh = 1;
    }
    if (upperAzimuthIndex > numAzimuthSteps/2) {
        upperAzimuthIndex = numAzimuthSteps - upperAzimuthIndex;
        uAziBaseCh = 1;
    }
    int lAzim1BaseCh = 0, uAzip1BaseCh = 0;
    if (lAzim1 > numAzimuthSteps/2) {
        lAzim1 = numAzimuthSteps - lAzim1;
        lAzim1BaseCh = 1;
    }
    if (uAzip1 > numAzimuthSteps/2) {
        uAzip1 = numAzimuthSteps - uAzip1;
        uAzip1BaseCh = 1;
    }
    int nAziBaseCh = 0;
    if (nAzi2 > numAzimuthSteps/2) {
        nAzi2 = numAzimuthSteps - nAzi2;
        nAziBaseCh = 1;
    }
    
    // i like things that are difficult to understand (see below, they follow the same pattern as the original interpolateHRIR())
    float **niRuAE1, **niRuAE2, **niRuAE3, **niRuAE4,
           **iRuAE1,  **iRuAE2,  **iRuAE3,  **iRuAE4,
          **niRlAE1, **niRlAE2, **niRlAE3, **niRlAE4,
           **iRlAE1,  **iRlAE2,  **iRlAE3,  **iRlAE4,
          **noRuAE1, **noRuAE2, **noRuAE3, **noRuAE4,
           **oRuAE1,  **oRuAE2,  **oRuAE3,  **oRuAE4,
          **noRlAE1, **noRlAE2, **noRlAE3, **noRlAE4,
           **oRlAE1,  **oRlAE2,  **oRlAE3,  **oRlAE4,
          **niRA1uE, **niRA2uE, **niRA3uE, **niRA4uE,
           **iRA1uE,  **iRA2uE,  **iRA3uE,  **iRA4uE,
          **niRA1lE, **niRA2lE, **niRA3lE, **niRA4lE,
           **iRA1lE,  **iRA2lE,  **iRA3lE,  **iRA4lE,
          **noRA1uE, **noRA2uE, **noRA3uE, **noRA4uE,
           **oRA1uE,  **oRA2uE,  **oRA3uE,  **oRA4uE,
          **noRA1lE, **noRA2lE, **noRA3lE, **noRA4lE,
           **oRA1lE,  **oRA2lE,  **oRA3lE,  **oRA4lE;
    
    // need these cuz bounds wrapped ele indecies can flip their channels or at least the order of the 1234 matters depending on n(Ele/Azi)Up
    int nuAE1BaseCh, nuAE2BaseCh, nuAE3BaseCh, nuAE4BaseCh,
        nlAE1BaseCh, nlAE2BaseCh, nlAE3BaseCh, nlAE4BaseCh,
         uAE1BaseCh,           /*uAE3BaseCh,*/  uAE4BaseCh,
         lAE1BaseCh,           /*lAE3BaseCh,*/  lAE4BaseCh,
        nA1uEBaseCh, nA2uEBaseCh, nA3uEBaseCh, nA4uEBaseCh,
        nA1lEBaseCh, nA2lEBaseCh, nA3lEBaseCh, nA4lEBaseCh;
//         A1uEBaseCh,  A2uEBaseCh,  A3uEBaseCh,  A4uEBaseCh;
    
    //const int uAzip1EleFlipped        = numAzimuthSteps-(uAzip1           +numAzimuthSteps/2);
    const int upperAziIndexEleFlipped = numAzimuthSteps-(upperAzimuthIndex+numAzimuthSteps/2);
    const int lowerAziIndexEleFlipped = numAzimuthSteps-(lowerAzimuthIndex+numAzimuthSteps/2);
    //const int lAzim1EleFlipped        = numAzimuthSteps-(lAzim1           +numAzimuthSteps/2);
    //const int nAzi2EleFlipped         = numAzimuthSteps-(nAzi2            +numAzimuthSteps/2);
    
    float mu1n;
    if (nEleUp) {
        mu1n = mu1 - 1;
        if (lowerElevationIndex == 0) {
            niRuAE1 = niRlAE1 = HRIRdataPoles[innerRadiusIndex][0];
            noRuAE1 = noRlAE1 = HRIRdataPoles[outerRadiusIndex][0];
            nuAE1BaseCh = nlAE1BaseCh = 0;
        } else {
            niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            nuAE1BaseCh = uAziBaseCh;
            nlAE1BaseCh = lAziBaseCh;
        }
        if (upperElevationIndex == numElevationSteps) {
            niRuAE2 = niRlAE2 = HRIRdataPoles[innerRadiusIndex][1];
            noRuAE2 = noRlAE2 = HRIRdataPoles[outerRadiusIndex][1];
            nuAE2BaseCh = nlAE2BaseCh = 0;
        } else {
            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            nuAE2BaseCh = uAziBaseCh;
            nlAE2BaseCh = lAziBaseCh;
        }
        if (uElep1Flip) {
            niRuAE3 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
            noRuAE3 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
            nuAE3BaseCh = (uAziBaseCh + 1) % 2;
            nlAE3BaseCh = (lAziBaseCh + 1) % 2;
        } else if (uElep1 == numElevationSteps) {
            niRuAE3 = niRlAE3 = HRIRdataPoles[innerRadiusIndex][1];
            noRuAE3 = noRlAE3 = HRIRdataPoles[outerRadiusIndex][1];
            nuAE3BaseCh = nlAE3BaseCh = 0;
        } else {
            niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1-1];
            noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1-1];
            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1-1];
            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1-1];
            nuAE3BaseCh = uAziBaseCh;
            nlAE3BaseCh = lAziBaseCh;
        }
        if (nEleFlip) {
            niRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
            noRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
            nuAE4BaseCh = (uAziBaseCh + 1) % 2;
            nlAE4BaseCh = (lAziBaseCh + 1) % 2;
        } else if (nEle2 == numElevationSteps) {
            niRuAE4 = niRlAE4 = HRIRdataPoles[innerRadiusIndex][1];
            noRuAE4 = noRlAE4 = HRIRdataPoles[outerRadiusIndex][1];
            nuAE4BaseCh = nlAE4BaseCh = 0;
        } else {
            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2-1];
            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2-1];
            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2-1];
            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2-1];
            nuAE4BaseCh = uAziBaseCh;
            nlAE4BaseCh = lAziBaseCh;
        }
    } else {
        mu1n = mu1 + 1;
        if (nEleFlip) {
            niRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
            noRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
            nuAE1BaseCh = (uAziBaseCh + 1) % 2;
            nlAE1BaseCh = (lAziBaseCh + 1) % 2;
        } else if (nEle2 == 0) {
            niRuAE1 = niRlAE1 = HRIRdataPoles[innerRadiusIndex][0];
            noRuAE1 = noRlAE1 = HRIRdataPoles[outerRadiusIndex][0];
            nuAE1BaseCh = nlAE1BaseCh = 0;
        } else {
            niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2-1];
            noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2-1];
            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2-1];
            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2-1];
            nuAE1BaseCh = uAziBaseCh;
            nlAE1BaseCh = lAziBaseCh;
        }
        if (lElem1Flip) {
            niRuAE2 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
            noRuAE2 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
            nuAE2BaseCh = (uAziBaseCh + 1) % 2;
            nlAE2BaseCh = (lAziBaseCh + 1) % 2;
        } else if (lElem1 == 0) {
            niRuAE2 = niRlAE2 = HRIRdataPoles[innerRadiusIndex][0];
            noRuAE2 = noRlAE2 = HRIRdataPoles[outerRadiusIndex][0];
            nuAE2BaseCh = nlAE2BaseCh = 0;
        } else {
            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1-1];
            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1-1];
            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1-1];
            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1-1];
            nuAE2BaseCh = uAziBaseCh;
            nlAE2BaseCh = lAziBaseCh;
        }
        if (lowerElevationIndex == 0) {
            niRuAE3 = niRlAE3 = HRIRdataPoles[innerRadiusIndex][0];
            noRuAE3 = noRlAE3 = HRIRdataPoles[outerRadiusIndex][0];
            nuAE3BaseCh = nlAE3BaseCh = 0;
        } else {
            niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            nuAE3BaseCh = uAziBaseCh;
            nlAE3BaseCh = lAziBaseCh;
        }
        if (upperElevationIndex == numElevationSteps) {
            niRuAE4 = niRlAE4 = HRIRdataPoles[innerRadiusIndex][1];
            noRuAE4 = noRlAE4 = HRIRdataPoles[outerRadiusIndex][1];
            nuAE4BaseCh = nlAE4BaseCh = 0;
        } else {
            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            nuAE4BaseCh = uAziBaseCh;
            nlAE4BaseCh = lAziBaseCh;
        }
    }
    if (lElem1Flip) {
        iRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
        oRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
        uAE1BaseCh = (uAziBaseCh + 1) % 2;
        lAE1BaseCh = (lAziBaseCh + 1) % 2;
    } else if (lElem1 == 0) {
        iRuAE1 = iRlAE1 = HRIRdataPoles[innerRadiusIndex][0];
        oRuAE1 = oRlAE1 = HRIRdataPoles[outerRadiusIndex][0];
        uAE1BaseCh = lAE1BaseCh = 0;
    } else {
        iRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1-1];
        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1-1];
        oRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1-1];
        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1-1];
        uAE1BaseCh = uAziBaseCh;
        lAE1BaseCh = lAziBaseCh;
    }
    if (lowerElevationIndex == 0) {
        iRuAE2 = iRlAE2 = HRIRdataPoles[innerRadiusIndex][0];
        oRuAE2 = oRlAE2 = HRIRdataPoles[outerRadiusIndex][0];
    } else {
        iRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
        iRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
        oRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
        oRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
    }
    if (upperElevationIndex == numElevationSteps) {
        iRuAE3 = iRlAE3 = HRIRdataPoles[innerRadiusIndex][1];
        oRuAE3 = oRlAE3 = HRIRdataPoles[outerRadiusIndex][1];
        //uAE3BaseCh = lAE3BaseCh = 0;
    } else {
        iRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
        iRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
        oRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
        oRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
        //uAE3BaseCh = uAziBaseCh;
        //lAE3BaseCh = lAziBaseCh;
    }
    if (uElep1Flip) {
        iRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
        oRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
        uAE4BaseCh = (uAziBaseCh + 1) % 2;
        lAE4BaseCh = (lAziBaseCh + 1) % 2;
    } else if (uElep1 == numElevationSteps) {
        iRuAE4 = iRlAE4 = HRIRdataPoles[innerRadiusIndex][1];
        oRuAE4 = oRlAE4 = HRIRdataPoles[outerRadiusIndex][1];
        uAE4BaseCh = lAE4BaseCh = 0;
    } else {
        iRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1-1];
        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1-1];
        oRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1-1];
        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1-1];
        uAE4BaseCh = uAziBaseCh;
        lAE4BaseCh = lAziBaseCh;
    }
    
    float mu2n;
    if (nAziUp) {
        mu2n = mu2 - 1;
        if (lowerElevationIndex == 0) {
            niRA1lE = niRA2lE = niRA3lE = niRA4lE = HRIRdataPoles[innerRadiusIndex][0];
            noRA1lE = noRA2lE = noRA3lE = noRA4lE = HRIRdataPoles[outerRadiusIndex][0];
            nA1lEBaseCh = nA2lEBaseCh = nA3lEBaseCh = nA4lEBaseCh = 0;
        } else {
            niRA1lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            niRA2lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            niRA3lE = HRIRdata[innerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
            niRA4lE = HRIRdata[innerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
            noRA1lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            noRA2lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            noRA3lE = HRIRdata[outerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
            noRA4lE = HRIRdata[outerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
            nA1lEBaseCh = lAziBaseCh;
            nA2lEBaseCh = uAziBaseCh;
            nA3lEBaseCh = uAzip1BaseCh;
            nA4lEBaseCh = nAziBaseCh;
        }
        if (upperElevationIndex == numElevationSteps) {
            niRA1uE = niRA2uE = niRA3uE = niRA4uE = HRIRdataPoles[innerRadiusIndex][1];
            noRA1uE = noRA2uE = noRA3uE = noRA4uE = HRIRdataPoles[outerRadiusIndex][1];
            nA1uEBaseCh = nA2uEBaseCh = nA3uEBaseCh = nA4uEBaseCh = 0;
        } else {
            niRA1uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            niRA2uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            niRA3uE = HRIRdata[innerRadiusIndex][uAzip1]           [upperElevationIndex-1];
            niRA4uE = HRIRdata[innerRadiusIndex][nAzi2]            [upperElevationIndex-1];
            noRA1uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            noRA2uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            noRA3uE = HRIRdata[outerRadiusIndex][uAzip1]           [upperElevationIndex-1];
            noRA4uE = HRIRdata[outerRadiusIndex][nAzi2]            [upperElevationIndex-1];
            nA1uEBaseCh = lAziBaseCh;
            nA2uEBaseCh = uAziBaseCh;
            nA3uEBaseCh = uAzip1BaseCh;
            nA4uEBaseCh = nAziBaseCh;
        }
    } else {
        mu2n = mu2 + 1;
        if (lowerElevationIndex == 0) { // NOTE; this is exact same as in nAziUp above
            niRA1lE = niRA2lE = niRA3lE = niRA4lE = HRIRdataPoles[innerRadiusIndex][0];
            noRA1lE = noRA2lE = noRA3lE = noRA4lE = HRIRdataPoles[outerRadiusIndex][0];
            nA1lEBaseCh = nA2lEBaseCh = nA3lEBaseCh = nA4lEBaseCh = 0;
        } else {
            niRA1lE = HRIRdata[innerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
            niRA2lE = HRIRdata[innerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
            niRA3lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            niRA4lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            noRA1lE = HRIRdata[outerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
            noRA2lE = HRIRdata[outerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
            noRA3lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
            noRA4lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
            nA1lEBaseCh = nAziBaseCh;
            nA2lEBaseCh = lAzim1BaseCh;
            nA3lEBaseCh = lAziBaseCh;
            nA4lEBaseCh = uAziBaseCh;
        }
        if (upperElevationIndex == numElevationSteps) { // NOTE; this is exact same as in nAziUp above
            niRA1uE = niRA2uE = niRA3uE = niRA4uE = HRIRdataPoles[innerRadiusIndex][1];
            noRA1uE = noRA2uE = noRA3uE = noRA4uE = HRIRdataPoles[outerRadiusIndex][1];
            nA1uEBaseCh = nA2uEBaseCh = nA3uEBaseCh = nA4uEBaseCh = 0;
        } else {
            niRA1uE = HRIRdata[innerRadiusIndex][nAzi2]            [upperElevationIndex-1];
            niRA2uE = HRIRdata[innerRadiusIndex][lAzim1]           [upperElevationIndex-1];
            niRA3uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            niRA4uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            noRA1uE = HRIRdata[outerRadiusIndex][nAzi2]            [upperElevationIndex-1];
            noRA2uE = HRIRdata[outerRadiusIndex][lAzim1]           [upperElevationIndex-1];
            noRA3uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
            noRA4uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
            nA1uEBaseCh = nAziBaseCh;
            nA2uEBaseCh = lAzim1BaseCh;
            nA3uEBaseCh = lAziBaseCh;
            nA4uEBaseCh = uAziBaseCh;
        }
    }
    if (lowerElevationIndex == 0) {
        iRA1lE = iRA2lE = iRA3lE = iRA4lE = HRIRdataPoles[innerRadiusIndex][0];
        oRA1lE = oRA2lE = oRA3lE = oRA4lE = HRIRdataPoles[outerRadiusIndex][0];
    } else {
        iRA1lE = HRIRdata[innerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
        iRA2lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
        iRA3lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
        iRA4lE = HRIRdata[innerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
        oRA1lE = HRIRdata[outerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
        oRA2lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
        oRA3lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
        oRA4lE = HRIRdata[outerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
    }
    if (upperElevationIndex == numElevationSteps) {
        iRA1uE = iRA2uE = iRA3uE = iRA4uE = HRIRdataPoles[innerRadiusIndex][1];
        oRA1uE = oRA2uE = oRA3uE = oRA4uE = HRIRdataPoles[outerRadiusIndex][1];
    } else {
        iRA1uE = HRIRdata[innerRadiusIndex][lAzim1]           [upperElevationIndex-1];
        iRA2uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
        iRA3uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
        iRA4uE = HRIRdata[innerRadiusIndex][uAzip1]           [upperElevationIndex-1];
        oRA1uE = HRIRdata[outerRadiusIndex][lAzim1]           [upperElevationIndex-1];
        oRA2uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
        oRA3uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
        oRA4uE = HRIRdata[outerRadiusIndex][uAzip1]           [upperElevationIndex-1];
//        A1uEBaseCh = lAzim1BaseCh;
//        A2uEBaseCh = lAziBaseCh;
//        A3uEBaseCh = uAziBaseCh;
//        A4uEBaseCh = uAzip1BaseCh;
    }
    
    // mu1's are for azi interp
    const float mu1_1 = mu1 - 1;
    const float mu1_2 = mu1 - 2;
    const float mu1_3 = mu1 - 3;
    const float mu1_4 = mu1 - 4;
    const float mu1n_1 = mu1n - 1;
    const float mu1n_2 = mu1n - 2;
    const float mu1n_3 = mu1n - 3;
    const float mu1n_4 = mu1n - 4;
    
    // mu2's are for ele interp
    const float mu2_1 = mu2 - 1;
    const float mu2_2 = mu2 - 2;
    const float mu2_3 = mu2 - 3;
    const float mu2_4 = mu2 - 4;
    const float mu2n_1 = mu2n - 1;
    const float mu2n_2 = mu2n - 2;
    const float mu2n_3 = mu2n - 3;
    const float mu2n_4 = mu2n - 4;
    
    const float oneminus_nmu1 = 1.0 - nmu1;
    const float oneminus_nmu2 = 1.0 - nmu2;
    const float oneminus_mu3 = 0.5*intensity_factor - mu3;//1.0 - mu3; // scaled by 1/2*intensity_factor here instead of for each sample below
    const float oneminus_mu1_01 = 1.0 - mu1_01;
    const float oneminus_mu2_01 = 1.0 - mu2_01;
    
    const float a1 = mu1_2 * mu1_3 * mu1_4 * -0.1666666666666666667;
    const float a2 = mu1_1 * mu1_3 * mu1_4 * 0.5;
    const float a3 = mu1_1 * mu1_2 * mu1_4 * -0.5;
    const float a4 = mu1_1 * mu1_2 * mu1_3 * 0.1666666666666666667;
    const float na1 = mu1n_2 * mu1n_3 * mu1n_4 * -0.1666666666666666667;
    const float na2 = mu1n_1 * mu1n_3 * mu1n_4 * 0.5;
    const float na3 = mu1n_1 * mu1n_2 * mu1n_4 * -0.5;
    const float na4 = mu1n_1 * mu1n_2 * mu1n_3 * 0.1666666666666666667;
    
    const float e1 = mu2_2 * mu2_3 * mu2_4 * -0.1666666666666666667;
    const float e2 = mu2_1 * mu2_3 * mu2_4 * 0.5;
    const float e3 = mu2_1 * mu2_2 * mu2_4 * -0.5;
    const float e4 = mu2_1 * mu2_2 * mu2_3 * 0.1666666666666666667;
    const float ne1 = mu2n_2 * mu2n_3 * mu2n_4 * -0.1666666666666666667;
    const float ne2 = mu2n_1 * mu2n_3 * mu2n_4 * 0.5;
    const float ne3 = mu2n_1 * mu2n_2 * mu2n_4 * -0.5;
    const float ne4 = mu2n_1 * mu2n_2 * mu2n_3 * 0.1666666666666666667;
    
    // intermediate interpolation values (in-region/nearby)(inner/outer)(azimuth/elevation)(plus/minus)
    float inAp, inAm, inEp, inEm, outAp, outAm, outEp, outEm, netIn, netOut;
    
    bool again = true;
    int hrirCh = 0;
AGAIN:
    for (int n = 0; n < numTimeSteps; ++n)
    {
        inAp = nmu1*(na1*niRuAE1[nuAE1BaseCh][n] + na2*niRuAE2[nuAE2BaseCh][n] + na3*niRuAE3[nuAE3BaseCh][n] + na4*niRuAE4[nuAE4BaseCh][n])
    + oneminus_nmu1*( a1* iRuAE1[ uAE1BaseCh][n] +  a2* iRuAE2[ uAziBaseCh][n] +  a3* iRuAE3[ uAziBaseCh][n] +  a4* iRuAE4[ uAE4BaseCh][n]);
        
        inAm = nmu1*(na1*niRlAE1[nlAE1BaseCh][n] + na2*niRlAE2[nlAE2BaseCh][n] + na3*niRlAE3[nlAE3BaseCh][n] + na4*niRlAE4[nlAE4BaseCh][n])
    + oneminus_nmu1*( a1* iRlAE1[ lAE1BaseCh][n] +  a2* iRlAE2[ lAziBaseCh][n] +  a3* iRlAE3[ lAziBaseCh][n] +  a4* iRlAE4[ lAE4BaseCh][n]);
        
        outAp = nmu1*(na1*noRuAE1[nuAE1BaseCh][n] + na2*noRuAE2[nuAE2BaseCh][n] + na3*noRuAE3[nuAE3BaseCh][n] + na4*noRuAE4[nuAE4BaseCh][n])
     + oneminus_nmu1*( a1* oRuAE1[ uAE1BaseCh][n] +  a2* oRuAE2[ uAziBaseCh][n] +  a3* oRuAE3[ uAziBaseCh][n] +  a4* oRuAE4[ uAE4BaseCh][n]);
        
        outAm = nmu1*(na1*noRlAE1[nlAE1BaseCh][n] + na2*noRlAE2[nlAE2BaseCh][n] + na3*noRlAE3[nlAE3BaseCh][n] + na4*noRlAE4[nlAE4BaseCh][n])
     + oneminus_nmu1*( a1* oRlAE1[ lAE1BaseCh][n] +  a2* oRlAE2[ lAziBaseCh][n] +  a3* oRlAE3[ lAziBaseCh][n] +  a4* oRlAE4[ lAE4BaseCh][n]);
        
        
        inEp = nmu2*(ne1*niRA1uE[ nA1uEBaseCh][n] + ne2*niRA2uE[nA2uEBaseCh][n] + ne3*niRA3uE[nA3uEBaseCh][n] + ne4*niRA4uE[ nA4uEBaseCh][n])
    + oneminus_nmu2*( e1* iRA1uE[lAzim1BaseCh][n] +  e2* iRA2uE[ lAziBaseCh][n] +  e3* iRA3uE[ uAziBaseCh][n] +  e4* iRA4uE[uAzip1BaseCh][n]);
        
        inEm = nmu2*(ne1*niRA1lE[ nA1lEBaseCh][n] + ne2*niRA2lE[nA2lEBaseCh][n] + ne3*niRA3lE[nA3lEBaseCh][n] + ne4*niRA4lE[ nA4lEBaseCh][n])
    + oneminus_nmu2*( e1* iRA1lE[lAzim1BaseCh][n] +  e2* iRA2lE[ lAziBaseCh][n] +  e3* iRA3lE[ uAziBaseCh][n] +  e4* iRA4lE[uAzip1BaseCh][n]);
        
        outEp = nmu2*(ne1*noRA1uE[ nA1uEBaseCh][n] + ne2*noRA2uE[nA2uEBaseCh][n] + ne3*noRA3uE[nA3uEBaseCh][n] + ne4*noRA4uE[ nA4uEBaseCh][n])
     + oneminus_nmu2*( e1* oRA1uE[lAzim1BaseCh][n] +  e2* oRA2uE[ lAziBaseCh][n] +  e3* oRA3uE[ uAziBaseCh][n] +  e4* oRA4uE[uAzip1BaseCh][n]);
        
        outEm = nmu2*(ne1*noRA1lE[ nA1lEBaseCh][n] + ne2*noRA2lE[nA2lEBaseCh][n] + ne3*noRA3lE[nA3lEBaseCh][n] + ne4*noRA4lE[ nA4lEBaseCh][n])
     + oneminus_nmu2*( e1* oRA1lE[lAzim1BaseCh][n] +  e2* oRA2lE[ lAziBaseCh][n] +  e3* oRA3lE[ uAziBaseCh][n] +  e4* oRA4lE[uAzip1BaseCh][n]);
        
        
        netIn  = (oneminus_mu1_01*inEm  + mu1_01*inEp)
               + (oneminus_mu2_01*inAm  + mu2_01*inAp);
        netOut = (oneminus_mu1_01*outEm + mu1_01*outEp)
               + (oneminus_mu2_01*outAm + mu2_01*outAp);
        
        hrir[hrirCh*numTimeSteps+n] = /*0.5*intensity_factor**/(mu3*netOut + oneminus_mu3*netIn);
    }
    if (again)
    {
        nuAE1BaseCh = (nuAE1BaseCh + 1) % 2;
        nuAE2BaseCh = (nuAE2BaseCh + 1) % 2;
        nuAE3BaseCh = (nuAE3BaseCh + 1) % 2;
        nuAE4BaseCh = (nuAE4BaseCh + 1) % 2;
        nlAE1BaseCh = (nlAE1BaseCh + 1) % 2;
        nlAE2BaseCh = (nlAE2BaseCh + 1) % 2;
        nlAE3BaseCh = (nlAE3BaseCh + 1) % 2;
        nlAE4BaseCh = (nlAE4BaseCh + 1) % 2;
         uAE1BaseCh = ( uAE1BaseCh + 1) % 2;
         //uAE3BaseCh = ( uAE3BaseCh + 1) % 2;
         uAE4BaseCh = ( uAE4BaseCh + 1) % 2;
         lAE1BaseCh = ( lAE1BaseCh + 1) % 2;
         //lAE3BaseCh = ( lAE3BaseCh + 1) % 2;
         lAE4BaseCh = ( lAE4BaseCh + 1) % 2;
        nA1uEBaseCh = (nA1uEBaseCh + 1) % 2;
        nA2uEBaseCh = (nA2uEBaseCh + 1) % 2;
        nA3uEBaseCh = (nA3uEBaseCh + 1) % 2;
        nA4uEBaseCh = (nA4uEBaseCh + 1) % 2;
        nA1lEBaseCh = (nA1lEBaseCh + 1) % 2;
        nA2lEBaseCh = (nA2lEBaseCh + 1) % 2;
        nA3lEBaseCh = (nA3lEBaseCh + 1) % 2;
        nA4lEBaseCh = (nA4lEBaseCh + 1) % 2;
//         A1uEBaseCh = ( A1uEBaseCh + 1) % 2;
//         A2uEBaseCh = ( A2uEBaseCh + 1) % 2;
//         A3uEBaseCh = ( A3uEBaseCh + 1) % 2;
//         A4uEBaseCh = ( A4uEBaseCh + 1) % 2;
        lAzim1BaseCh = (lAzim1BaseCh + 1) % 2;
          lAziBaseCh = (  lAziBaseCh + 1) % 2;
          uAziBaseCh = (  uAziBaseCh + 1) % 2;
        uAzip1BaseCh = (uAzip1BaseCh + 1) % 2;
        hrirCh = 1;
        again = false;
        goto AGAIN; // i love the goto
    }
}

//// PRE CONCURRENTRESOURCE
////
////  SoundSource.cpp
////
////  Created by Andrew Barker on 7/2/14.
////
////
//
//#include "SoundSource.h"
//#include "Functions.h"
//
//// fuckin C++ man
//template <class T_SRC, class T_DEST>
//std::unique_ptr<T_DEST> unique_cast(std::unique_ptr<T_SRC> &&src)
//{
//    if (!src) return std::unique_ptr<T_DEST>();
//    // Throws a std::bad_cast() if this doesn't work out
//    T_DEST *dest_ptr = &dynamic_cast<T_DEST &>(*src.get());
//    src.release();
//    return std::unique_ptr<T_DEST>(dest_ptr);
//}
//
//SoundSource::SoundSource(const std::array<float,3>& xyz)
//{
//    // set position to the one passed in (after bounds checking)
//    setPosXYZ(xyz);
//}
//
//SoundSource::SoundSource(const SoundSource& source)
//{
//    // non dynamically allocated data can be shallow copied
//    posRAE = source.posRAE;
//    sourceMuted = source.sourceMuted;
//    eleDir = source.eleDir;
//    sourceSelected = source.sourceSelected;
//    pathPos = source.pathPos;
//    // need to deep copy pointers that are not null
//    if (source.path.get() != nullptr)
//        path = source.path->clone();
//}
//
//SoundSource& SoundSource::operator=(const SoundSource& source)
//{
//    // check for self-assignment
//    if (this == &source)
//        return *this;
//    // deallocate previous stuff
//    path.release();
//    // non dynamically allocated data can be shallow copied
//    posRAE = source.posRAE;
//    sourceMuted = source.sourceMuted;
//    eleDir = source.eleDir;
//    sourceSelected = source.sourceSelected;
//    pathPos = source.pathPos;
//    // need to deep copy pointers that are not null
//    if (source.path.get() != nullptr)
//        path = source.path->clone();
//    return *this;
//}
//
////bool SoundSource::isSameAs(const SoundSource& source)
////{
////    // compare paths:  are points same, are all spline types same, (and for path only is open/closedness the same)
////    return (posRAE == source.posRAE);
////}
//
//SoundSource::SoundSource(XmlElement *sourceXML)
//{
//    // restore saved position
//    posRAE[0] = sourceXML->getDoubleAttribute("rad");
//    posRAE[1] = sourceXML->getDoubleAttribute("azi");
//    posRAE[2] = sourceXML->getDoubleAttribute("ele");
//    // restore saved interpolaters
//    path = unique_cast<Interpolator<float>, ParametricInterpolator<float>>(getInterpolator(*(sourceXML->getChildElement(0))));
//    //path = std::move(unique_cast<Interpolator<float>, ParametricInterpolator<float>>(getInterpolator(*(sourceXML->getChildElement(0)))));
//    pathPos = dynamic_cast<FunctionalInterpolator<float>&>(*getInterpolator(*(sourceXML->getChildElement(1))));
//}
//
//XmlElement* SoundSource::getXML() const
//{
//    // xml element for the whole interpolator object
//    XmlElement *sourceXML = new XmlElement("SOUNDSOURCE");
//    // store the basic position stuff
//    sourceXML->setAttribute("rad", posRAE[0]);
//    sourceXML->setAttribute("azi", posRAE[1]);
//    sourceXML->setAttribute("ele", posRAE[2]);
//    // store the path and pathPos interps
//    if (path.get() != nullptr)
//        sourceXML->addChildElement(getXML(*path));
//    else // need to put a dummy XmlElement in so that the indexing for path and pathPos are always sourceXML->getChildElement(0) and sourceXML->getChildElement(1), respectively
//        sourceXML->addChildElement(new XmlElement("THERE_IS_NO_PATH"));
//    sourceXML->addChildElement(getXML(pathPos));
//    return sourceXML;
//}
//
//XmlElement* SoundSource::getXML(const Interpolator<float>& interp) const
//{
//    // xml element for the whole interpolator object
//    XmlElement *interpXML = new XmlElement("INTERPOLATOR");
//    // store the interp type
//    interpXML->setAttribute("type", interp.type);
//    // store the data points in xml
//    XmlElement ptsXML ("POINTS");
//    std::vector<std::vector<float>> pts = interp.getPoints();
//    for (int i = 0; i < pts.size(); ++i)
//    {
//        XmlElement ptXML ("POINT");
//        for (int j = 0; j < pts[i].size(); ++j)
//            ptXML.setAttribute(Identifier("pt" + std::to_string(j)), pts[i][j]);
//        ptsXML.addChildElement(new XmlElement(ptXML));
//    }
//    interpXML->addChildElement(new XmlElement(ptsXML));
//    return interpXML;
//}
//
//std::unique_ptr<Interpolator<float>> SoundSource::getInterpolator(const XmlElement& interpXML) const
//{
//    if (interpXML.getTagName().equalsIgnoreCase("INTERPOLATOR"))
//    {
//        //InterpolatorType type = (InterpolatorType)interpXML.getIntAttribute("pathType");
//        int type = interpXML.getIntAttribute("type");
//        // first child element should be the xml element containing all dataPts
//        XmlElement *xmlPoints = interpXML.getFirstChildElement();
//        int numPts = xmlPoints->getNumChildElements();
//        std::vector<std::vector<float>> points (numPts);
//        for (int i = 0; i < numPts; ++i)
//        {
//            int dim = xmlPoints->getChildElement(i)->getNumAttributes();
//            std::vector<float> pt (dim);
//            for (int j = 0; j < dim; ++j)
//                pt[j] = xmlPoints->getChildElement(i)->getDoubleAttribute(Identifier("pt" + std::to_string(j)));
//            points[i] = pt;
//        }
//        return InterpolatorFactory<float>((InterpolatorType)type, points);
//    }
//    else
//        return nullptr;
//}
//
//void SoundSource::boundsCheckRAE(std::array<float,3>& rae, float& eleDirection)
//{
//    // bounds checking for the setting the source's position
//    while (rae[2] < 0)
//        rae[2] += 2.0*M_PI;
//    while (rae[2] > 2.0*M_PI)
//        rae[2] -= 2.0*M_PI;
//    // now eleTemp must be between 0 and 2pi, just have to make sure it is btw 0 and pi now
//    if (rae[2] > M_PI)
//    {
//        rae[2] = M_PI-(rae[2]-M_PI);
//        rae[1] = M_PI+rae[1];
//        eleDirection *= -1;
//    }
//    while (rae[1] > 2.0*M_PI)
//        rae[1] -= 2.0*M_PI;
//    while (rae[1] < 0)
//        rae[1] += 2.0*M_PI;
//    // no moving source inside the head (rad = 0.1meters)
//    if (rae[0] < distanceBegin)
//        rae[0] = distanceBegin;
//}
//
//void SoundSource::boundsCheckRAE(float (&rae)[3], float& eleDirection)
//{
//    // bounds checking for the setting the source's position
//    while (rae[2] < 0)
//        rae[2] += 2.0*M_PI;
//    while (rae[2] > 2.0*M_PI)
//        rae[2] -= 2.0*M_PI;
//    // now eleTemp must be between 0 and 2pi, just have to make sure it is btw 0 and pi now
//    if (rae[2] > M_PI)
//    {
//        rae[2] = M_PI-(rae[2]-M_PI);
//        rae[1] = M_PI+rae[1];
//        eleDirection *= -1;
//    }
//    while (rae[1] > 2.0*M_PI)
//        rae[1] -= 2.0*M_PI;
//    while (rae[1] < 0)
//        rae[1] += 2.0*M_PI;
//    // no moving source inside the head (rad = 0.1meters)
//    if (rae[0] < distanceBegin)
//        rae[0] = distanceBegin;
//}
//
//void SoundSource::boundsCheckXYZ(std::array<float,3>& xyz)
//{
//    std::array<float,3> rae;
//    XYZtoRAE(&xyz[0], &rae[0]);
//    boundsCheckRAE(rae, eleDir);
//    RAEtoXYZ(&rae[0], &xyz[0]);
//}
//
//void SoundSource::setPosRAE(std::array<float,3>& rae)
//{
//    boundsCheckRAE(rae, eleDir);
//    posRAE = rae;
//}
//
//std::array<float,3> SoundSource::getPosRAE() const
//{
//    return posRAE;
//}
//
//void SoundSource::setEleDir(const float newEleDir)
//{
//    eleDir = newEleDir;
//}
//
//float SoundSource::getEleDir() const
//{
//    return eleDir;
//}
//
//void SoundSource::setPosXYZ(const std::array<float,3>& xyz)
//{
//    std::array<float,3> rae;
//    XYZtoRAE(&xyz[0], &rae[0]);
//    boundsCheckRAE(rae, eleDir);
//    posRAE = rae;
//}
//
//void SoundSource::setPosXYZ(const float* xyz)
//{
//    float rae[3];
//    XYZtoRAE(xyz, rae);
//    boundsCheckRAE(rae, eleDir);
//    posRAE[0] = rae[0];
//    posRAE[1] = rae[1];
//    posRAE[2] = rae[2];
//}
//
//std::array<float,3> SoundSource::getPosXYZ() const
//{
//    std::array<float,3> xyz;
//    RAEtoXYZ(&posRAE[0], &xyz[0]);
//    return xyz;
//}
//
//bool SoundSource::setParametricPosition(const float posSec, int& prevPathPosIndex)
//{
//    // only makes sense to set the sources position on its path when the source is moving(playing) and actually on a valid path with at least two points
//    bool setPosFromPath = false;
//    if (path.get() != nullptr && path->getNumPoints() > 1)
//    {
//        float y;
//        if (pathPos.pointAtSmart(posSec, &y, prevPathPosIndex))
//        {
//            sourceMuted = false;
//            float xyz[4]; // need 4, and not 3 b/c we stored the eleDir for each point in the 4th dim and we get stack corruption if we don't make room for it here
//            float range[2];
//            path->getInputRangeQuick(range);
//            // the 0.99999 scaling here is to prevent the case when a pt-pt interp would suddenly jump back to the begining of the path if the y value is exactly equal to 1.0
//            if (path->pointAt(y * range[1] * 0.99999, xyz))
//            {
//                setPosXYZ(xyz);
//                setPosFromPath = true;
//            }
//        }
//        else
//            sourceMuted = true;
//        //        return setPosFromPath;
//        
//        //        std::vector<float> y;
//        //        if (pathPos.pointAtSmart(posSec, y, prevPathPosIndex))
//        //        {
//        //            sourceMuted = false;
//        //            std::vector<float> xyz;
//        //            if (path->pointAt(y[0]*path->getInputRange()[1], xyz))
//        //                setPosXYZ({xyz[0], xyz[1], xyz[2]});
//        //        }
//        //        else
//        //            sourceMuted = true;
//    }
//    return setPosFromPath;
//}
//
//void SoundSource::setPositionUpdate(const std::array<float,3>& newPosRAE, const bool newMuted)
//{
//    posRAE = newPosRAE;
//    sourceMuted = newMuted;
//}
//
//void SoundSource::setSourceSelected(const bool newSourceSelected) noexcept
//{
//    sourceSelected = newSourceSelected;
//    if (newSourceSelected)
//        eleDir = 1; // reset ele dir so all selected sources move the same direction in ele
//        // was trying to do keep symetry in ele angle for the selected sources, but need to do that by considering all selected sources positions...
//        //    if (0 <= posRAE[1] && posRAE[1] < M_PI_2)
//        //        eleDir = 1.0;
//        //    else if (M_PI_2 <= posRAE[1] && posRAE[1] < M_PI)
//        //        eleDir = -1.0;
//        //    else if (M_PI <= posRAE[1] && posRAE[1] < 3*M_PI_2)
//        //        eleDir = 1.0;
//        //    else if (3*M_PI_2 <= posRAE[1] && posRAE[1] < 2*M_PI)
//        //        eleDir = -1.0;
//        }
//
//bool SoundSource::getSourceSelected() const noexcept
//{
//    return sourceSelected;
//}
//
//void SoundSource::setSourceMuted(const bool newMutedState)
//{
//    sourceMuted = newMutedState;
//}
//
//bool SoundSource::getSourceMuted() const
//{
//    return sourceMuted;
//}
//
//void SoundSource::addPathPoint()
//{
//    std::array<float,3> current = getPosXYZ();
//    if (path.get() == nullptr)
//    {
//        std::vector<std::vector<float>> points;
//        points.push_back({current[0], current[1], current[2], 1.0});
//        path = std::unique_ptr<ParametricInterpolator<float>>(new ClosedParametricInterpolator<float>(points));
//    }
//    else
//        path->addPoint({current[0], current[1], current[2], 1.0}); // last value is the elevation direction for the path point
//}
//
//void SoundSource::addPathPoint(std::array<float,3>& xyz)
//{
//    boundsCheckXYZ(xyz);
//    if (path.get() == nullptr)
//    {
//        std::vector<std::vector<float>> points;
//        points.push_back({xyz[0], xyz[1], xyz[2], 1.0});
//        path = std::unique_ptr<ParametricInterpolator<float>>(new ClosedParametricInterpolator<float>(points));
//    }
//    else
//        path->addPoint({xyz[0], xyz[1], xyz[2], 1.0}); // last value is the elevation direction for the path point
//}
//
//int SoundSource::deleteSelectedPathPoints()
//{
//    if (path.get() != nullptr)
//        return path->deleteSelectedPoints();
//    else
//        return 0;
//}
//
//void SoundSource::setPathType(const int pathType)
//{
//    if (path.get() != nullptr && path->type != pathType)
//    {
//        if (pathType == OPEN_PARAMETRIC_INTERPOLATOR)
//            path = std::unique_ptr<ParametricInterpolator<float>>(new OpenParametricInterpolator<float>(dynamic_cast<ClosedParametricInterpolator<float>&>(*path)));
//        else if (pathType == CLOSED_PARAMETRIC_INTERPOLATOR)
//            path = std::unique_ptr<ParametricInterpolator<float>>(new ClosedParametricInterpolator<float>(dynamic_cast<OpenParametricInterpolator<float>&>(*path)));
//    }
//}
//
//std::unique_ptr<ParametricInterpolator<float>> SoundSource::getPath() const
//{
//    if (path.get() != nullptr)
//        return path->clone();
//    else
//        return nullptr;
//}
//
////FunctionalInterpolator<float> SoundSource::getPathPos() const
////{
////    return pathPos;
////}
//
//FunctionalInterpolator<float>* SoundSource::getPathPosPtr()
//{
//    return &pathPos;
//}
//
//void SoundSource::setPathChangedState(const bool changed)
//{
//    if (path.get() != nullptr)
//        path->changed = changed;
//}
//
//void SoundSource::setPathPosChangedState(const bool changed)
//{
//    pathPos.changed = changed;
//}
//
//bool SoundSource::moveSelectedPathPointsXYZ(const float dX, const float dY, const float dZ)
//{
//    // NOTE: the bounds checking for not allowing a path point inside the head was removed
//    if (path.get() != nullptr)
//    {
//        int numMoved = path->moveSelectedPoints({dX,dY,dZ});
//        // return value signals weather or not the source(s) should be moved too
//        return (numMoved > 0 && numMoved != path->getNumPoints());
//    }
//    else
//        return false;
//}
//
//bool SoundSource::moveSelectedPathPointsRAE(const float dRad, const float dAzi, const float dEle)
//{
//    // normally we wouldn't move points of an interp like this, but the continuous elevation and RAE coordinate system is not supported by Interpolators
//    if (path.get() != nullptr)
//    {
//        std::vector<std::vector<float>> selPts = path->getSelectedPoints();
//        std::array<float,3> rae {0};
//        std::array<float,3> xyz {0};
//        std::vector<int> selPtsIndicies = path->getSelectedPointIndicies();
//        for (int i = 0; i < selPts.size(); ++i)
//        {
//            float selpt[3] = {selPts[i][0], selPts[i][1], selPts[i][2]};
//            XYZtoRAE(&selpt[0], &rae[0]);
//            rae[0] *= dRad;
//            rae[1] += dAzi;
//            rae[2] += selPts[i][3]*dEle;
//            boundsCheckRAE(rae, selPts[i][3]);
//            RAEtoXYZ(&rae[0], &xyz[0]);
//            path->setPointPosition({xyz[0], xyz[1], xyz[2], selPts[i][3]}, selPtsIndicies[i]);
//        }
//        if (selPts.size() > 0)
//            path->recalcSplines();
//        return (selPts.size() > 0 && selPts.size() != path->getNumPoints());
//    }
//    else
//        return false;
//}
//
//void SoundSource::setAllPathPointsSelected(const bool selectedState)
//{
//    if (path.get() != nullptr)
//    {
//        path->setAllPointsSelected(selectedState);
//        if (selectedState)
//        {// reset ele dir so all path points move same direction in ele
//            int i = 0;
//            for (auto& pt : path->getPoints())
//            {
//                pt[3] = 1.0;
//                path->setPointPosition(pt, i++);
//            }
//        }
//    }
//}
//
//void SoundSource::setPathPointSelected(const int ptIndex, const bool selectedState)
//{
//    if (path.get() != nullptr)
//    {
//        path->setPointSelected(ptIndex, selectedState);
//        if (selectedState)
//        {// reset ele dir so all path points move same direction in ele
//            std::vector<float> resetEleDirPt = path->getPoint(ptIndex);
//            resetEleDirPt[3] = 1.0;
//            path->setPointPosition(resetEleDirPt, ptIndex);
//        }
//    }
//}
//
//bool SoundSource::getPathPointSelected(const int ptIndex) const
//{
//    if (path.get() != nullptr)
//        return path->getPointSelected(ptIndex);
//    else
//        return false;
//}
//
//void SoundSource::setPathAutomationPointSelected(const int ptIndex, const bool selectedState)
//{
//    pathPos.setPointSelected(ptIndex, selectedState);
//}
//
//bool SoundSource::getPathAutomationPointSelected(const int ptIndex) const
//{
//    return pathPos.getPointSelected(ptIndex);
//}
//
//int SoundSource::moveSelectedPathAutomationPoints(const float dx, const float dy)
//{
//    return pathPos.moveSelectedPoints({dx, dy});
//}
//
////std::vector<int> SoundSource::moveSelectedPathAutomationPointsWithReorderingInfo(const float dx, const float dy)
////{
////    return pathPos.moveSelectedPointsWithReorderingInfo({dx, dy});
////}
//
//bool SoundSource::deleteSelectedAutomationPoints()
//{
//    return pathPos.deleteSelectedPoints() > 0;
//}
//
//void SoundSource::setAllPathAutomationPointsSelected(const bool selectedState)
//{
//    pathPos.setAllPointsSelected(selectedState);
//}
//
//void SoundSource::addPathAutomationPoint(const float x, const float y)
//{
//    pathPos.addPoint({x, y});
//}
//
//int SoundSource::copySelectedPathPoints()
//{
//    if (path.get() != nullptr)
//        return path->copySelectedPoints();
//    else
//        return 0;
//}
//
//int SoundSource::copySelectedPathAutomationPoints()
//{
//    return pathPos.copySelectedPoints();
//}
//
//std::vector<std::vector<float>> SoundSource::getPathPoints() const
//{
//    if (path.get() != nullptr)
//        return path->getPoints();
//    else
//        return std::vector<std::vector<float>>();
//}
//
//int SoundSource::getNumPathPoints() const
//{
//    if (path.get() != nullptr)
//        return path->getNumPoints();
//    else
//        return 0;
//}
//
//int SoundSource::getNumPathAutomationPoints() const
//{
//    return pathPos.getNumPoints();
//}
//
//std::vector<bool> SoundSource::getSelectedPathPoints() const
//{
//    if (path.get() != nullptr)
//        return path->getPointsSelected();
//    else
//        return std::vector<bool>();
//}
//
//std::vector<bool> SoundSource::getSelectedPathAutomationPoints() const
//{
//    return pathPos.getPointsSelected();
//}
//
//bool SoundSource::setSelectedPathAutomationPointsSegmentType(const int newSegType)
//{
//    return pathPos.setSelectedSplinesType((SplineShape)newSegType) > 0;
//}
//
///***** PlayableSoundSource *****/
//PlayableSoundSource::PlayableSoundSource()
//{
//    interpolateHRIR(&posRAE[0], &HRIR[0]);
//    HRIRScaling[0] = HRIRScaling[1] = 0;//= HRIRScaling[2] = HRIRScaling[3] = 0;
//    for (int n = 0; n < numTimeSteps; ++n) {
//        HRIRScaling[0] += std::abs(HRIR[             n]);
//        //HRIRScaling[2] += std::abs(HRIR[             n]);
//        HRIRScaling[1] += std::abs(HRIR[numTimeSteps+n]);
//        //HRIRScaling[3] += std::abs(HRIR[numTimeSteps+n]);
//    }
//    HRIRScaling[0] = 1.0/HRIRScaling[0];
//    HRIRScaling[1] = 1.0/HRIRScaling[1];
//    // hoping this (init of HRIRs at construction) might fix the random fuzz issue with moving sources, it did seem to work...
//    for (int n = 0; n < numTimeSteps; ++n)
//    {
//        HRIR[             n] *= HRIRScaling[0];
//        HRIR[numTimeSteps+n] *= HRIRScaling[1];
//        HRIRs[2*numTimeSteps+n] = HRIRs[             n] = HRIR[             n];// * HRIRScaling[0];
//        //HRIRs[2*numTimeSteps+n] = HRIR[             n];// * HRIRScaling[0];
//        HRIRs[3*numTimeSteps+n] = HRIRs[numTimeSteps+n] = HRIR[numTimeSteps+n];// * HRIRScaling[1];
//        //HRIRs[3*numTimeSteps+n] = HRIR[numTimeSteps+n];// * HRIRScaling[1];
//    }
//    HRIRScaling[2] = HRIRScaling[0] = 1.0/HRIRScaling[0];
//    HRIRScaling[3] = HRIRScaling[1] = 1.0/HRIRScaling[1];
//}
//
//PlayableSoundSource::~PlayableSoundSource()
//{
//    //inputs.clear();
//    //for (int i = 0; i < inputs.size(); ++i)
//    //   inputs[i].~_Input_();
//    delete[] hqHRIRs;
//    delete[] hqHRIRScaling;
//    delete[] temp;
//}
//
//void PlayableSoundSource::advancePosition() noexcept
//{
//    if (prevHRIRChange && dopplerOn)
//    {
//        HRIRChange = true;
//        float currentXYZ[3];
//        RAEtoXYZ(&posRAE[0], currentXYZ);
//        float prevXYZ[3];
//        RAEtoXYZ(&pprevRAE[0], prevXYZ);
//        float newXYZ[3] = { currentXYZ[0] + currentXYZ[0]-prevXYZ[0],
//            currentXYZ[1] + currentXYZ[1]-prevXYZ[1],
//            currentXYZ[2] + currentXYZ[2]-prevXYZ[2] };
//        prevRAE = posRAE; // need this so that pprevRAE can get properly updated in processAudio() if advancePosition() gets called two or more buffers in a row
//        XYZtoRAE(newXYZ, &posRAE[0]);
//    }
//}
//
//void PlayableSoundSource::updateFromSoundSource(const SoundSource& source) noexcept
//{
//    if (posRAE != source.posRAE)
//    {
//        HRIRChange = true;
//        posRAE = source.posRAE;
//    }
//    //    // SMOOTH TRANSITION
//    //    if (posRAE != source.posRAE && !HRIRChange)
//    //    {
//    //        HRIRChange = true;
//    //        prevRAE = posRAE;
//    //        posRAE = source.posRAE;
//    //        currentTransitionTime = nextTransitionTime;//std::min(nextTransitionTime, maxTransitionTime);
//    //        nextTransitionTime = 0;
//    //    }
//    sourceMuted = source.sourceMuted;
//}
//
//std::array<float,3> PlayableSoundSource::getPosRAE() const noexcept
//{
//    return posRAE;
//}
//
//void PlayableSoundSource::allocateForMaxBufferSize(const int N_max)
//{
//    Nmax = N_max;
//    inputs.resize(std::ceil((float)(numTimeSteps-1)/((float)Nmax)) + 1);
//    for (auto& input : inputs)
//        input.setSize(Nmax);
//    newInputIndex = 0;
//    if (dopplerOn)
//    {
//        //doppler[0].free();
//        //doppler[1].free();
//        doppler[0].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
//        doppler[1].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
//    }
//}
//
////void PlayableSoundSource::setRealTime(const bool isRealTime) noexcept
////{
////    realTime = isRealTime;
////}
////
////bool PlayableSoundSource::getRealTime() const noexcept
////{
////    return realTime;
////}
//
//void PlayableSoundSource::setDopplerOn(const bool newDopplerOn, const float newSpeedOfSound)
//{
//    const bool speedOfSoundChanged = (newSpeedOfSound != dopplerSpeedOfSound);
//    dopplerSpeedOfSound = newSpeedOfSound;
//    if (newDopplerOn != dopplerOn || speedOfSoundChanged)
//    {
//        if (newDopplerOn || speedOfSoundChanged)
//        {
//            doppler[0].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
//            doppler[1].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
//        }
//        else
//        {
//            doppler[0].free();
//            doppler[1].free();
//        }
//        // reset processing state of sound source
//        for (auto& i : inputs)
//            i.clear();
//        newInputIndex = 0;
//        HRIRChange = false;
//        prevRAE = posRAE;
//    }
//    dopplerOn = newDopplerOn;
//}
//
//void PlayableSoundSource::setDopplerSampleRate(const float sampleRate) noexcept
//{
//    doppler[0].setSampleRate(sampleRate);
//    doppler[1].setSampleRate(sampleRate);
//}
//
//void PlayableSoundSource::setSourceMuted(const bool newMutedState) noexcept
//{
//    sourceMuted = newMutedState;
//}
//
//bool PlayableSoundSource::getSourceMuted() const noexcept
//{
//    return sourceMuted;
//}
//
//void PlayableSoundSource::resetProcessingState() noexcept
//{
//    if (dopplerOn)
//    {
//        doppler[0].reset();
//        doppler[1].reset();
//    }
//    for (auto& i : inputs)
//        i.clear();
//        newInputIndex = 0;
//        HRIRChange = false;
//        prevRAE = posRAE;
//        }
//
//// PRE-SMOOTH TRANSITION
//void PlayableSoundSource::processAudio(const float* in, const int N, float* out, const bool realTime)
//{
//    float* whichHRIRs = nullptr;
//    float* whichHRIRScaling = nullptr;
//    // if we had an HRIRChange update we gotta interpolate that hrir data for the blended output
//    if (HRIRChange) {
//        if (realTime) {
//            // to process in realtime, need to limit the number of blending positions to something that can be reasonably computed
//            numHRIRs = 2; // must be 2 in order to get away with only making one new interpolateHRIR() call
//            whichHRIRs = &HRIRs[0];
//            whichHRIRScaling = &HRIRScaling[0];
//            // end of the positional interps (only one that needs computation for realtime)
//            interpolateHRIR(&posRAE[0], &HRIRs[2*numTimeSteps]);//&whichHRIRs[(numHRIRs-1)*2*numTimeSteps]);
//            // this pre-convolution normalization is required to get rid of the crackling in the quiet ear for close sources due to floating point addition inaccuracy
//            HRIRScaling[2] = HRIRScaling[3] = 0;
//            for (int n = 0; n < numTimeSteps; ++n) {
//                HRIRScaling[2] += std::abs(HRIRs[2*numTimeSteps+n]);
//                HRIRScaling[3] += std::abs(HRIRs[3*numTimeSteps+n]);
//            }
//            HRIRScaling[2] = 1.0/HRIRScaling[2];
//            HRIRScaling[3] = 1.0/HRIRScaling[3];
//            for (int n = 0; n < numTimeSteps; ++n) {
//                HRIRs[2*numTimeSteps+n] *= HRIRScaling[2];
//                HRIRs[3*numTimeSteps+n] *= HRIRScaling[3];
//            }
//            HRIRScaling[2] = 1.0/HRIRScaling[2];
//            HRIRScaling[3] = 1.0/HRIRScaling[3];
//            //            scaleCh1 = 0, scaleCh2 = 0;
//            //            float* const beginNewHRIR = &whichHRIRs[(numHRIRs-1)*2*numTimeSteps];
//            //            for (int n = 0; n < numTimeSteps; ++n) {
//            //                scaleCh1 += std::abs(beginNewHRIR[n]);
//            //                scaleCh2 += std::abs(beginNewHRIR[n+numTimeSteps]);
//            //            }
//            //            scaleCh1 = 1.0/scaleCh1;
//            //            scaleCh2 = 1.0/scaleCh2;
//            //            for (int n = 0; n < numTimeSteps; ++n) {
//            //                beginNewHRIR[n]              *= scaleCh1;
//            //                beginNewHRIR[n+numTimeSteps] *= scaleCh2;
//            //            }
//            //            scaleCh1 = 1.0/scaleCh1;
//            //            scaleCh2 = 1.0/scaleCh2;
//        } else {
//            // for non-realtime processing, we can go crazy and have each output sample be processed with a different blending position for nice smooth audio despite potentially fast moving source
//            // (note the N+1 instead of N because the blend widths are calculated L = 2N/(numHRIRs-1) and we want L = 2.0 in this case)
//            const int newNumHRIRs = (N>>2/*HRIRInterpQuality*/)+1; // new hrir position for each 4 samples seems more than sufficient...
//            if (newNumHRIRs != numHRIRs) {
//                numHRIRs = newNumHRIRs;
//                delete[] hqHRIRs;
//                hqHRIRs = new float[numHRIRs*2*numTimeSteps];
//                delete[] hqHRIRScaling;
//                hqHRIRScaling = new float[numHRIRs*2];
//            }
//            whichHRIRs = hqHRIRs;
//            whichHRIRScaling = hqHRIRScaling;
//            const int lastHRIR = numHRIRs-1;
//            // end of the positional interps (only one that needs computation for realtime)
//            interpolateHRIR(&posRAE[0], &hqHRIRs[lastHRIR*2*numTimeSteps]);
//            // pre-convolution normalization
//            hqHRIRScaling[lastHRIR*2] = hqHRIRScaling[lastHRIR*2+1] = 0;
//            for (int n = 0; n < numTimeSteps; ++n) {
//                hqHRIRScaling[lastHRIR*2  ] += std::abs(hqHRIRs[ lastHRIR*2   *numTimeSteps+n]);
//                hqHRIRScaling[lastHRIR*2+1] += std::abs(hqHRIRs[(lastHRIR*2+1)*numTimeSteps+n]);
//            }
//            hqHRIRScaling[lastHRIR*2  ] = 1.0/hqHRIRScaling[lastHRIR*2  ];
//            hqHRIRScaling[lastHRIR*2+1] = 1.0/hqHRIRScaling[lastHRIR*2+1];
//            for (int n = 0; n < numTimeSteps; ++n) {
//                hqHRIRs[ lastHRIR*2   *numTimeSteps+n] *= hqHRIRScaling[lastHRIR*2  ];
//                hqHRIRs[(lastHRIR*2+1)*numTimeSteps+n] *= hqHRIRScaling[lastHRIR*2+1];
//            }
//            hqHRIRScaling[lastHRIR*2  ] = 1.0/hqHRIRScaling[lastHRIR*2  ];
//            hqHRIRScaling[lastHRIR*2+1] = 1.0/hqHRIRScaling[lastHRIR*2+1];
//            hqHRIRScaling[0] = HRIRScaling[0]; // load the first hrir pos scaling factors
//            hqHRIRScaling[1] = HRIRScaling[1];
//            // number of interps minus the endpoints which have already been interped!
//            const int numInterps = numHRIRs-2;
//            // positional data for blending
//            //float posX, posY, posZ;//, posRad, posAzi, posEle; // intermediate positions
//            float posXYZ[3];
//            //std::array<float,3> pos_RAE;
//            float pos_RAE[3];
//            //float posXCurrent, posYCurrent, posZCurrent;
//            //std::array<float,3> xyzCurrent;
//            float xyzCurrent[3];
//            RAEtoXYZ(&prevRAE[0], &xyzCurrent[0]);
//            //        const float posXCurrent = radPrev*std::sin(elePrev)*cos(aziPrev);
//            //        const float posYCurrent = radPrev*std::cos(elePrev);
//            //        const float posZCurrent = radPrev*sin(elePrev)*sin(aziPrev);
//            //float posXNext, posYNext, posZNext;
//            //std::array<float,3> xyzNext;
//            float xyzNext[3];
//            RAEtoXYZ(&posRAE[0], &xyzNext[0]);
//            //        const float posXNext = rad*sin(ele)*cos(azi);
//            //        const float posYNext = rad*cos(ele);
//            //        const float posZNext = rad*sin(ele)*sin(azi);
//            const float oneOverNumInterpsP1 = 1.0/(numInterps+1);
//            const float factorX = oneOverNumInterpsP1 * (xyzNext[0]-xyzCurrent[0]);
//            const float factorY = oneOverNumInterpsP1 * (xyzNext[1]-xyzCurrent[1]);
//            const float factorZ = oneOverNumInterpsP1 * (xyzNext[2]-xyzCurrent[2]);
//            // interpolate positions and hrirs for those positions
//            for (int i = 1; i <= numInterps; ++i) {
//                // interpolate intermediate positions in xyz land
//                posXYZ[0] = i * factorX + xyzCurrent[0];
//                posXYZ[1] = i * factorY + xyzCurrent[1];
//                posXYZ[2] = i * factorZ + xyzCurrent[2];
//                // convert back to spherical
//                XYZtoRAE(&posXYZ[0], &pos_RAE[0]);
//                //            posRad = std::sqrt(posX*posX + posY*posY + posZ*posZ);
//                //            if (posX < 0)
//                //                posAzi = std::atan(posZ/posX) + M_PI;
//                //            else
//                //                posAzi = std::atan(posZ/posX);
//                //            while (posAzi > 2.0*M_PI)
//                //                posAzi -= 2.0*M_PI;
//                //            while (posAzi < 0)
//                //                posAzi += 2.0*M_PI;
//                //            posEle = std::acos(posY/posRad);
//                //            interpolateHRIR(posRad, posAzi, posEle, &whichHRIRs[i*2*numTimeSteps]);
//                interpolateHRIR(pos_RAE, &hqHRIRs[i*2*numTimeSteps]);
//                // pre-convolution normalization
//                hqHRIRScaling[i*2] = hqHRIRScaling[i*2+1] = 0;
//                for (int n = 0; n < numTimeSteps; ++n) {
//                    hqHRIRScaling[i*2  ] += std::abs(hqHRIRs[ i*2   *numTimeSteps+n]);
//                    hqHRIRScaling[i*2+1] += std::abs(hqHRIRs[(i*2+1)*numTimeSteps+n]);
//                }
//                hqHRIRScaling[i*2  ] = 1.0/hqHRIRScaling[i*2  ];
//                hqHRIRScaling[i*2+1] = 1.0/hqHRIRScaling[i*2+1];
//                for (int n = 0; n < numTimeSteps; ++n) {
//                    hqHRIRs[ i*2   *numTimeSteps+n] *= hqHRIRScaling[i*2  ];
//                    hqHRIRs[(i*2+1)*numTimeSteps+n] *= hqHRIRScaling[i*2+1];
//                }
//                hqHRIRScaling[i*2  ] = 1.0/hqHRIRScaling[i*2  ];
//                hqHRIRScaling[i*2+1] = 1.0/hqHRIRScaling[i*2+1];
//            }
//        }
//        // load the "current" hrir into the blended HRIRs and make "current" hrir the one for the next position, think that screwy stuff with the HRIR data is causing those rare fuzzes when the sources moves, still not sure what to do to fix it...
//        //        for (int ch = 0; ch < 2; ++ch) {
//        //            for (int n = 0; n < numTimeSteps; ++n) {
//        //                whichHRIRs[ch*numTimeSteps+n] = HRIR[ch*numTimeSteps+n];
//        //                HRIR[ch*numTimeSteps+n] = whichHRIRs[((numHRIRs-1)*2+ch)*numTimeSteps+n];
//        //            }
//        //        }
//        for (int n = 0; n < numTimeSteps; ++n) {
//            // ch 0
//            whichHRIRs[             n] = HRIR      [             n];
//            HRIR      [             n] = whichHRIRs[((numHRIRs-1)*2)  *numTimeSteps+n];
//            // ch 1
//            whichHRIRs[numTimeSteps+n] = HRIR      [numTimeSteps+n];
//            HRIR      [numTimeSteps+n] = whichHRIRs[((numHRIRs-1)*2+1)*numTimeSteps+n];
//        }
//        // advance positional state
//        pprevRAE = prevRAE;
//        prevRAE = posRAE;
//    } // end if HRIRChange
//    // load the current input
//    inputs[newInputIndex].load(in, N);
//    const int numInputs = inputs.size();
//    int buflengths = 0;//inputs[(newInputIndex+1) % numInputs].N;
//    const int begin = (newInputIndex+1) % numInputs;//(newInputIndex+2) % numInputs;
//    const int end = newInputIndex;
//    const int beginInputIndex = newInputIndex;
//    //int endInputIndex = begin;
//    int inputsToProcess = 1;
//    for (int i = begin; i != end; i = (i+1) % numInputs) {
//        ++inputsToProcess;
//        buflengths += inputs[i].N;
//        if (inputs[i].N+numTimeSteps-1 <= buflengths) {
//            //endInputIndex = i;
//            break;
//        }
//    }
//    const int numInputsToProcess = inputsToProcess;//std::abs(endInputIndex - beginInputIndex) + 1;
//    if (--newInputIndex < 0)
//        newInputIndex = numInputs - 1;
//    //    inputs.insert(0, new Input(in, N));
//    //    // remove any previous buffers that are no longer needed...
//    //    int buflengths = 0;
//    //    for (int i = 1; i < inputs.size(); ++i) {
//    //        buflengths += inputs[i]->N;
//    //        if (inputs[i]->N+numTimeSteps-1 <= buflengths) {
//    //            delete inputs[i];
//    //            inputs.remove(i);
//    //            --i;
//    //        }
//    //    }
//    // allocate final output array
//    float yfinal[N];
//    //    // temp buffer for post-doppler, pre-hrir audio
//    //    float in[N];
//    // process for each ear
//    for (int ch = 0; ch < 2; ++ch) {
//        // zero fill output array
//        for (int n = 0; n < N; ++n)
//            yfinal[n] = 0;
//        
//        // blending hrirs in this buffer
//        if (HRIRChange) {
//            // init array of outputs for each hrir chunk and each needed previous input's tail
//            const int N_max = Nmax;
//            float* ytemp = nullptr;
//            const int tempSize = numInputs*numHRIRs*(N_max+numTimeSteps-1);
//            float y [realTime ? tempSize : 0];
//            if (realTime) {
//                ytemp = y;
//            } else {  // using new here to allocate on the heap instead of the stack which will likely overflow for numHRIRs = big number in the hq hrir interpolated non-realtime case
//                if (prevTempSize != tempSize) {
//                    prevTempSize = tempSize;
//                    delete[] temp;
//                    temp = new float[tempSize];
//                }
//                ytemp = temp;
//                //ytemp = new float[tempSize];
//            }
//            //            // hoping this fixes that rare and random fuzzing issue when source is moving, nope...
//            //            for (int n = 0; n < tempSize; ++n)
//            //                ytemp[n] = 0;
//            //float y [numInputs][numHRIRs][N_max+numTimeSteps-1];
//            //float ytemp [numInputs*numHRIRs*(N_max+numTimeSteps-1)];
//            //            float *** y = new float**[inputs.size()];
//            //            for (int i = 0; i < inputs.size(); ++i) {
//            //                int L = inputs[i]->N+numTimeSteps-1;
//            //                y[i] = new float*[numHRIRs];
//            //                for (int j = 0; j < numHRIRs; ++j) {
//            //                    y[i][j] = new float[L];
//            //                    for (int n = 0; n < L; ++n) {
//            //                        y[i][j][n] = 0;
//            //                    }
//            //                }
//            //            }
//            // length of blending region (in samples) that blends between two consecutive interped hrirs
//            const float L = ((float)N) / ((float)(numHRIRs-1));
//            int beginIndex = 0, endIndex, thing, beginIndex2, endIndex2;
//            //for (int i = 0; i < inputs.size(); ++i) {
//            for (int k = 0, i = beginInputIndex; k < numInputsToProcess; i = (i+1) % numInputs, ++k) {
//                //if (i != 0)
//                //    beginIndex += inputs[i]->N;
//                if (i != beginInputIndex)
//                    beginIndex += inputs[i].N;
//                // if the tail for this previous input goes up to or past the end of this current buffer
//                thing = inputs[i].N+numTimeSteps-1 - (N+beginIndex);
//                if (thing >= 0)
//                    endIndex = beginIndex + N-1;
//                else // otherwise the tail for this previous input ends somewhere in this current buffer
//                    endIndex = beginIndex + N-1 + thing;
//                // loop through each hrir and do the positional convolution chunks
//                for (int j = 0; j < numHRIRs-1; ++j) {
//                    // indexing for the hrir section of interest in terms of the respective input's indexing
//                    beginIndex2 = std::floor( ((float)j)*L + beginIndex );
//                    endIndex2 = std::ceil( ((float)(j+1))*L + beginIndex );
//                    // make sure to not go past the end of the convolution
//                    if (endIndex2 > endIndex)
//                        endIndex2 = endIndex;
//                    // make sure we even need to do the convolution for this positional hrir chunk
//                    if (beginIndex2 <= endIndex2) {
//                        convolve(&inputs[i].input[0], inputs[i].N,
//                                 &whichHRIRs[ ( j  *2+ch)*numTimeSteps], numTimeSteps,
//                                 &ytemp[(i*numHRIRs+  j  )*(N_max+numTimeSteps-1)]/*y[i][j]*/  , beginIndex2, endIndex2);
//                        convolve(&inputs[i].input[0], inputs[i].N,
//                                 &whichHRIRs[((j+1)*2+ch)*numTimeSteps], numTimeSteps,
//                                 &ytemp[(i*numHRIRs+(j+1))*(N_max+numTimeSteps-1)]/*y[i][j+1]*/, beginIndex2, endIndex2);
//                    }
//                }
//            }
//            int j, c1, c2, offset;
//            for (int n = 0; n < N; ++n) {
//                // which hrir position chunk are we in
//                j = std::floor( ((float)n) / L );
//                // starts at 0.0 at multiples of L(frac) samples, goes up to ~< 1.0 at the sample before the next multiple
//                c2 = (((float)n) - ((float)j)*L) / L;
//                c1 = 1.0 - c2;
//                offset = 0;
//                //for (int i = 0; i < inputs.size(); ++i) {
//                for (int k = 0, i = beginInputIndex; k < numInputsToProcess; i = (i+1) % numInputs, ++k) {
//                    //if (i != 0)
//                    //    offset += inputs[i]->N;
//                    if (i != beginInputIndex)
//                        offset += inputs[i].N;
//                    // don't want to exceed the bounds of the y[][][N+numtimesteps-1] array
//                    if (n+offset < inputs[i].N+numTimeSteps-1) {
//                        yfinal[n] += c1 * whichHRIRScaling[  j  *2+ch]
//                        * ytemp[((i*numHRIRs+  j  )*(N_max+numTimeSteps-1))+(n+offset)]/*y[i][j][n+offset]*/
//                        + c2 * whichHRIRScaling[(j+1)*2+ch]
//                        * ytemp[((i*numHRIRs+(j+1))*(N_max+numTimeSteps-1))+(n+offset)]/*y[i][j+1][n+offset]*/;
//                    }
//                }
//            }
//            // advance the HRIR scaling stuff
//            HRIRScaling[0] = whichHRIRScaling[(numHRIRs-1)*2];
//            HRIRScaling[1] = whichHRIRScaling[(numHRIRs-1)*2+1];
//            // free temporary output holding array
//            //            if (!realTime) {
//            //                delete[] ytemp;
//            //            }
//            
//            //            for (int i = inputs.size()-1; i >= 0; --i) {
//            //                for (int j = numHRIRs-1; j >= 0; --j) {
//            //                    delete[] y[i][j];
//            //                }
//            //                delete[] y[i];
//            //            }
//            //            delete[] y;
//        } else { // no blending to do in this buffer as we are stationary
//            // do convolutions for all the inputs that are needed to render this buffers output
//            // note that begin and end indecies are refering to the previous buffer's indexing context, not the current buffer's
//            int beginIndex = 0, endIndex, thing;
//            //for (int i = 0; i < inputs.size(); ++i) {
//            for (int k = 0, i = beginInputIndex; k < numInputsToProcess; i = (i+1) % numInputs, ++k) {
//                // intermediate output for each input buffer
//                float y[inputs[i].N+numTimeSteps-1];
//                //if (i != 0)
//                //    beginIndex += inputs[i]->N;
//                if (i != beginInputIndex)
//                    beginIndex += inputs[i].N;
//                // if the tail for this previous input goes up to or past the end of this current buffer
//                thing = inputs[i].N+numTimeSteps-1 - (N+beginIndex);
//                if (thing >= 0)
//                    endIndex = beginIndex + N-1;
//                else // otherwise the tail for this previous input ends somewhere in this current buffer
//                    endIndex = beginIndex + N-1 + thing;
//                // do the convolution
//                convolve(&inputs[i].input[0], inputs[i].N, &HRIR[ch*numTimeSteps], numTimeSteps, y, beginIndex, endIndex);
//                for (int n = beginIndex; n <= endIndex; ++n)
//                    yfinal[n-beginIndex] += y[n] * HRIRScaling[ch];
//            }
//        }
//        
//        //        // scale signal back up after convolution is performed
//        //        if (ch == 0) {
//        //            for (int n = 0; n < N; ++n)
//        //                yfinal[n] *= scaleCh1;
//        //        } else {
//        //            for (int n = 0; n < N; ++n)
//        //                yfinal[n] *= scaleCh2;
//        //        }
//        
//        // apply doppler effect
//        if (dopplerOn) {
//            float yDoppler[N];
//            float sourceXYZ[3];
//            RAEtoXYZ(&posRAE[0], sourceXYZ);
//            float earXYZ[3];
//            if (ch == 0) {
//                float earRAE[3] = {sphereRad,earAzimuth,earElevation};
//                RAEtoXYZ(earRAE, earXYZ);
//            } else {
//                float earRAE[3] = {sphereRad,-earAzimuth,earElevation};
//                RAEtoXYZ(earRAE, earXYZ);
//            }
//            const float dx = sourceXYZ[0]-earXYZ[0];
//            const float dy = sourceXYZ[1]-earXYZ[1];
//            const float dz = sourceXYZ[2]-earXYZ[2];
//            const float earToSourceDistance = std::sqrt(dx*dx + dy*dy + dz*dz);
//            if (earToSourceDistance > dopplerMaxDistance) {
//                // shouldn't happen that often, so reallocing here when necessary shouldn't cause any big problems
//                dopplerMaxDistance = earToSourceDistance * 2.0;
//                doppler[0].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
//                doppler[1].allocate(dopplerMaxDistance, Nmax, dopplerSpeedOfSound);
//                //dopplerMaxDistanceChanged = true;
//            }
//            doppler[ch].process(earToSourceDistance, N, yfinal, yDoppler);
//            // package each channel's output into one dual-channel array
//            for (int n = 0; n < N; ++n)
//                out[ch*N+n] += yDoppler[n];
//        } else { // no doppler effect
//            // package each channel's output into one dual-channel array
//            for (int n = 0; n < N; ++n)
//                out[ch*N+n] += yfinal[n];
//        }
//    } // end for each channel
//    prevHRIRChange = HRIRChange;
//    // set the state of movement so that the next buffer is stationary, which may change if we get an updated position from the gl side
//    HRIRChange = false;
//}
//
//// the global hrir data that gets one instance across multiple plugin instances, this just references the one instance defined in PluginProcessor.cpp
//extern float***** HRIRdata;
//extern float**** HRIRdataPoles;
//
//// compacted (one azimuth side provided) with pole data version
//void PlayableSoundSource::interpolateHRIR(const float* rae, float* hrir) const noexcept
//{
//    // get the inner + outer rad,azi,ele indicies that define the 3d region bounded by the hrtf/dvf sampling resolution that the source is currently located in
//    const int innerRadiusIndex = std::max(0, std::min((int)((std::log(rae[0])-std::log(distanceBegin))/std::log(distanceEnd/distanceBegin)*(numDistanceSteps-1)), numDistanceSteps-2));//-1);
//    const int outerRadiusIndex = innerRadiusIndex+1;// std::min(innerRadiusIndex+1, numDistanceSteps-1);
//    
//    const int lowerElevationIndex = std::max(0, std::min((int)std::floor(rae[2]/M_PI*numElevationSteps), numElevationSteps-1));
//    const int upperElevationIndex = lowerElevationIndex+1;
//    
//    const float revAzi = std::fmod(4*M_PI-rae[1], 2*M_PI); // fix reversed azimuth indexing with hrir array's, this caused lowerAzimuthIndex = -1 without fmod
//    int lowerAzimuthIndex = std::min((int)std::floor(revAzi/(2*M_PI)*numAzimuthSteps), numAzimuthSteps-1);
//    int upperAzimuthIndex = (lowerAzimuthIndex+1) % numAzimuthSteps;
//    
//    // inner/outer surface radius values
//    const float rIn  = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)innerRadiusIndex)/(numDistanceSteps-1));
//    const float rOut = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)outerRadiusIndex)/(numDistanceSteps-1));
//    
//    // upper/lower azimuth values
//    const float aP = ((float)(upperAzimuthIndex))*(2*M_PI)/numAzimuthSteps;
//    const float aM = ((float)(lowerAzimuthIndex))*(2*M_PI)/numAzimuthSteps;
//    
//    // upper/lower elevation values
//    const float eM = ((float)lowerElevationIndex)*M_PI/numElevationSteps;
//    const float eP = ((float)upperElevationIndex)*M_PI/numElevationSteps;
//    
//    // for making close/far more loud/quiet
//    const float intensity_factor = 0.1 / std::pow(rae[0], 0.5);
//    
//    const float mu3 = 0.5*intensity_factor*std::min((rae[0]-rIn)/(rOut-rIn), 1.0f); // scaled by 1/2*intensity_factor here instead of for each sample below
//    
//    // interpolate along azimuth edges
//    const float mu1_01 = (rae[2]-eM)*numElevationSteps/M_PI; // should be btw 0 and 1
//    const float mu1 = mu1_01 + 2;                            // should be btw 2 and 3
//    const float nmu1 = std::abs(2.5-mu1); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
//    
//    // interpolate along elevation edges
//    const float mu2_01 = (revAzi-aM)*numAzimuthSteps/(2.0*M_PI); // should be btw 0 and 1
//    const float mu2 = mu2_01 + 2;                                // should be btw 2 and 3
//    const float nmu2 = std::abs(2.5-mu2); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
//    
//    // variables for bounds wrapping surrounding data
//    int uAzip1 = upperAzimuthIndex+1;
//    int lAzim1 = lowerAzimuthIndex-1;
//    int uElep1 = upperElevationIndex+1;
//    int lElem1 = lowerElevationIndex-1;
//    
//    if (uAzip1 > numAzimuthSteps-1)
//        uAzip1 -= numAzimuthSteps;
//        if (lAzim1 < 0)
//            lAzim1 += numAzimuthSteps;
//            
//            bool lElem1Flip = false, uElep1Flip = false; // ele's need bounds wrapping and if they wrap, the corresponding azi it is used with needs flipping
//            if (uElep1 > numElevationSteps) {
//                uElep1 = 2*numElevationSteps - uElep1;
//                uElep1Flip = true;
//            }
//    if (lElem1 < 0) {
//        lElem1 = -lElem1;
//        lElem1Flip = true;
//    }
//    
//    int nAzi2, nEle2;  // neighboring azi/ele indices
//    bool nAziUp, nEleUp;
//    if ((aP > aM ? aP-revAzi : 2.0*M_PI-revAzi) > revAzi-aM) {
//        nAzi2 = lowerAzimuthIndex-2;
//        nAziUp = false;
//    } else {
//        nAzi2 = upperAzimuthIndex+2;
//        nAziUp = true;
//    }
//    if (eP-rae[2] > rae[2]-eM) {
//        nEle2 = lowerElevationIndex-2;
//        nEleUp = false;
//    } else {
//        nEle2 = upperElevationIndex+2;
//        nEleUp = true;
//    }
//    
//    if (nAzi2 > numAzimuthSteps-1)
//        nAzi2 -= numAzimuthSteps;
//        if (nAzi2 < 0)
//            nAzi2 += numAzimuthSteps;
//            
//            bool nEleFlip = false; // ele's need bounds wrapping and if they wrap, the corresponding azi needs flipping
//            if (nEle2 > numElevationSteps) {
//                nEle2 = 2*numElevationSteps - nEle2;
//                nEleFlip = true;
//            }
//    if (nEle2 < 0) {
//        nEle2 = -nEle2;
//        nEleFlip = true;
//    }
//    
//    // for the compacted data with only one azimuth side provided, we gotta do some channel flipping
//    int lAziBaseCh = 0, uAziBaseCh = 0;
//    if (lowerAzimuthIndex > numAzimuthSteps/2) {
//        lowerAzimuthIndex = numAzimuthSteps - lowerAzimuthIndex;// numAzimuthSteps/2 - (lowerAzimuthIndex - numAzimuthSteps/2);
//        lAziBaseCh = 1;
//    }
//    if (upperAzimuthIndex > numAzimuthSteps/2) {
//        upperAzimuthIndex = numAzimuthSteps - upperAzimuthIndex;
//        uAziBaseCh = 1;
//    }
//    int lAzim1BaseCh = 0, uAzip1BaseCh = 0;
//    if (lAzim1 > numAzimuthSteps/2) {
//        lAzim1 = numAzimuthSteps - lAzim1;
//        lAzim1BaseCh = 1;
//    }
//    if (uAzip1 > numAzimuthSteps/2) {
//        uAzip1 = numAzimuthSteps - uAzip1;
//        uAzip1BaseCh = 1;
//    }
//    int nAziBaseCh = 0;
//    if (nAzi2 > numAzimuthSteps/2) {
//        nAzi2 = numAzimuthSteps - nAzi2;
//        nAziBaseCh = 1;
//    }
//    
//    // i like things that are difficult to understand (see below, they follow the same pattern as the original interpolateHRIR())
//    float **niRuAE1, **niRuAE2, **niRuAE3, **niRuAE4,
//    **iRuAE1,  **iRuAE2,  **iRuAE3,  **iRuAE4,
//    **niRlAE1, **niRlAE2, **niRlAE3, **niRlAE4,
//    **iRlAE1,  **iRlAE2,  **iRlAE3,  **iRlAE4,
//    **noRuAE1, **noRuAE2, **noRuAE3, **noRuAE4,
//    **oRuAE1,  **oRuAE2,  **oRuAE3,  **oRuAE4,
//    **noRlAE1, **noRlAE2, **noRlAE3, **noRlAE4,
//    **oRlAE1,  **oRlAE2,  **oRlAE3,  **oRlAE4,
//    **niRA1uE, **niRA2uE, **niRA3uE, **niRA4uE,
//    **iRA1uE,  **iRA2uE,  **iRA3uE,  **iRA4uE,
//    **niRA1lE, **niRA2lE, **niRA3lE, **niRA4lE,
//    **iRA1lE,  **iRA2lE,  **iRA3lE,  **iRA4lE,
//    **noRA1uE, **noRA2uE, **noRA3uE, **noRA4uE,
//    **oRA1uE,  **oRA2uE,  **oRA3uE,  **oRA4uE,
//    **noRA1lE, **noRA2lE, **noRA3lE, **noRA4lE,
//    **oRA1lE,  **oRA2lE,  **oRA3lE,  **oRA4lE;
//    
//    // need these cuz bounds wrapped ele indecies can flip their channels or at least the order of the 1234 matters depending on n(Ele/Azi)Up
//    int nuAE1BaseCh, nuAE2BaseCh, nuAE3BaseCh, nuAE4BaseCh,
//    nlAE1BaseCh, nlAE2BaseCh, nlAE3BaseCh, nlAE4BaseCh,
//    uAE1BaseCh,           /*uAE3BaseCh,*/  uAE4BaseCh,
//    lAE1BaseCh,           /*lAE3BaseCh,*/  lAE4BaseCh,
//    nA1uEBaseCh, nA2uEBaseCh, nA3uEBaseCh, nA4uEBaseCh,
//    nA1lEBaseCh, nA2lEBaseCh, nA3lEBaseCh, nA4lEBaseCh;
//    //         A1uEBaseCh,  A2uEBaseCh,  A3uEBaseCh,  A4uEBaseCh;
//    
//    //const int uAzip1EleFlipped        = numAzimuthSteps-(uAzip1           +numAzimuthSteps/2);
//    const int upperAziIndexEleFlipped = numAzimuthSteps-(upperAzimuthIndex+numAzimuthSteps/2);
//    const int lowerAziIndexEleFlipped = numAzimuthSteps-(lowerAzimuthIndex+numAzimuthSteps/2);
//    //const int lAzim1EleFlipped        = numAzimuthSteps-(lAzim1           +numAzimuthSteps/2);
//    //const int nAzi2EleFlipped         = numAzimuthSteps-(nAzi2            +numAzimuthSteps/2);
//    
//    float mu1n;
//    if (nEleUp) {
//        mu1n = mu1 - 1;
//        if (lowerElevationIndex == 0) {
//            niRuAE1 = niRlAE1 = HRIRdataPoles[innerRadiusIndex][0];
//            noRuAE1 = noRlAE1 = HRIRdataPoles[outerRadiusIndex][0];
//            nuAE1BaseCh = nlAE1BaseCh = 0;
//        } else {
//            niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            nuAE1BaseCh = uAziBaseCh;
//            nlAE1BaseCh = lAziBaseCh;
//        }
//        if (upperElevationIndex == numElevationSteps) {
//            niRuAE2 = niRlAE2 = HRIRdataPoles[innerRadiusIndex][1];
//            noRuAE2 = noRlAE2 = HRIRdataPoles[outerRadiusIndex][1];
//            nuAE2BaseCh = nlAE2BaseCh = 0;
//        } else {
//            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            nuAE2BaseCh = uAziBaseCh;
//            nlAE2BaseCh = lAziBaseCh;
//        }
//        if (uElep1Flip) {
//            niRuAE3 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
//            noRuAE3 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
//            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
//            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
//            nuAE3BaseCh = (uAziBaseCh + 1) % 2;
//            nlAE3BaseCh = (lAziBaseCh + 1) % 2;
//        } else if (uElep1 == numElevationSteps) {
//            niRuAE3 = niRlAE3 = HRIRdataPoles[innerRadiusIndex][1];
//            noRuAE3 = noRlAE3 = HRIRdataPoles[outerRadiusIndex][1];
//            nuAE3BaseCh = nlAE3BaseCh = 0;
//        } else {
//            niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1-1];
//            noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1-1];
//            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1-1];
//            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1-1];
//            nuAE3BaseCh = uAziBaseCh;
//            nlAE3BaseCh = lAziBaseCh;
//        }
//        if (nEleFlip) {
//            niRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
//            noRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
//            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
//            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
//            nuAE4BaseCh = (uAziBaseCh + 1) % 2;
//            nlAE4BaseCh = (lAziBaseCh + 1) % 2;
//        } else if (nEle2 == numElevationSteps) {
//            niRuAE4 = niRlAE4 = HRIRdataPoles[innerRadiusIndex][1];
//            noRuAE4 = noRlAE4 = HRIRdataPoles[outerRadiusIndex][1];
//            nuAE4BaseCh = nlAE4BaseCh = 0;
//        } else {
//            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2-1];
//            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2-1];
//            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2-1];
//            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2-1];
//            nuAE4BaseCh = uAziBaseCh;
//            nlAE4BaseCh = lAziBaseCh;
//        }
//    } else {
//        mu1n = mu1 + 1;
//        if (nEleFlip) {
//            niRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
//            noRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2-1];
//            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
//            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2-1];
//            nuAE1BaseCh = (uAziBaseCh + 1) % 2;
//            nlAE1BaseCh = (lAziBaseCh + 1) % 2;
//        } else if (nEle2 == 0) {
//            niRuAE1 = niRlAE1 = HRIRdataPoles[innerRadiusIndex][0];
//            noRuAE1 = noRlAE1 = HRIRdataPoles[outerRadiusIndex][0];
//            nuAE1BaseCh = nlAE1BaseCh = 0;
//        } else {
//            niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2-1];
//            noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2-1];
//            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2-1];
//            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2-1];
//            nuAE1BaseCh = uAziBaseCh;
//            nlAE1BaseCh = lAziBaseCh;
//        }
//        if (lElem1Flip) {
//            niRuAE2 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
//            noRuAE2 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
//            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
//            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
//            nuAE2BaseCh = (uAziBaseCh + 1) % 2;
//            nlAE2BaseCh = (lAziBaseCh + 1) % 2;
//        } else if (lElem1 == 0) {
//            niRuAE2 = niRlAE2 = HRIRdataPoles[innerRadiusIndex][0];
//            noRuAE2 = noRlAE2 = HRIRdataPoles[outerRadiusIndex][0];
//            nuAE2BaseCh = nlAE2BaseCh = 0;
//        } else {
//            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1-1];
//            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1-1];
//            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1-1];
//            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1-1];
//            nuAE2BaseCh = uAziBaseCh;
//            nlAE2BaseCh = lAziBaseCh;
//        }
//        if (lowerElevationIndex == 0) {
//            niRuAE3 = niRlAE3 = HRIRdataPoles[innerRadiusIndex][0];
//            noRuAE3 = noRlAE3 = HRIRdataPoles[outerRadiusIndex][0];
//            nuAE3BaseCh = nlAE3BaseCh = 0;
//        } else {
//            niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            nuAE3BaseCh = uAziBaseCh;
//            nlAE3BaseCh = lAziBaseCh;
//        }
//        if (upperElevationIndex == numElevationSteps) {
//            niRuAE4 = niRlAE4 = HRIRdataPoles[innerRadiusIndex][1];
//            noRuAE4 = noRlAE4 = HRIRdataPoles[outerRadiusIndex][1];
//            nuAE4BaseCh = nlAE4BaseCh = 0;
//        } else {
//            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            nuAE4BaseCh = uAziBaseCh;
//            nlAE4BaseCh = lAziBaseCh;
//        }
//    }
//    if (lElem1Flip) {
//        iRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
//        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
//        oRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1-1];
//        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1-1];
//        uAE1BaseCh = (uAziBaseCh + 1) % 2;
//        lAE1BaseCh = (lAziBaseCh + 1) % 2;
//    } else if (lElem1 == 0) {
//        iRuAE1 = iRlAE1 = HRIRdataPoles[innerRadiusIndex][0];
//        oRuAE1 = oRlAE1 = HRIRdataPoles[outerRadiusIndex][0];
//        uAE1BaseCh = lAE1BaseCh = 0;
//    } else {
//        iRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1-1];
//        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1-1];
//        oRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1-1];
//        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1-1];
//        uAE1BaseCh = uAziBaseCh;
//        lAE1BaseCh = lAziBaseCh;
//    }
//    if (lowerElevationIndex == 0) {
//        iRuAE2 = iRlAE2 = HRIRdataPoles[innerRadiusIndex][0];
//        oRuAE2 = oRlAE2 = HRIRdataPoles[outerRadiusIndex][0];
//    } else {
//        iRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//        iRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//        oRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//        oRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//    }
//    if (upperElevationIndex == numElevationSteps) {
//        iRuAE3 = iRlAE3 = HRIRdataPoles[innerRadiusIndex][1];
//        oRuAE3 = oRlAE3 = HRIRdataPoles[outerRadiusIndex][1];
//        //uAE3BaseCh = lAE3BaseCh = 0;
//    } else {
//        iRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//        iRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//        oRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//        oRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//        //uAE3BaseCh = uAziBaseCh;
//        //lAE3BaseCh = lAziBaseCh;
//    }
//    if (uElep1Flip) {
//        iRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
//        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
//        oRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1-1];
//        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1-1];
//        uAE4BaseCh = (uAziBaseCh + 1) % 2;
//        lAE4BaseCh = (lAziBaseCh + 1) % 2;
//    } else if (uElep1 == numElevationSteps) {
//        iRuAE4 = iRlAE4 = HRIRdataPoles[innerRadiusIndex][1];
//        oRuAE4 = oRlAE4 = HRIRdataPoles[outerRadiusIndex][1];
//        uAE4BaseCh = lAE4BaseCh = 0;
//    } else {
//        iRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1-1];
//        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1-1];
//        oRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1-1];
//        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1-1];
//        uAE4BaseCh = uAziBaseCh;
//        lAE4BaseCh = lAziBaseCh;
//    }
//    
//    float mu2n;
//    if (nAziUp) {
//        mu2n = mu2 - 1;
//        if (lowerElevationIndex == 0) {
//            niRA1lE = niRA2lE = niRA3lE = niRA4lE = HRIRdataPoles[innerRadiusIndex][0];
//            noRA1lE = noRA2lE = noRA3lE = noRA4lE = HRIRdataPoles[outerRadiusIndex][0];
//            nA1lEBaseCh = nA2lEBaseCh = nA3lEBaseCh = nA4lEBaseCh = 0;
//        } else {
//            niRA1lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            niRA2lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            niRA3lE = HRIRdata[innerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
//            niRA4lE = HRIRdata[innerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
//            noRA1lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            noRA2lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            noRA3lE = HRIRdata[outerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
//            noRA4lE = HRIRdata[outerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
//            nA1lEBaseCh = lAziBaseCh;
//            nA2lEBaseCh = uAziBaseCh;
//            nA3lEBaseCh = uAzip1BaseCh;
//            nA4lEBaseCh = nAziBaseCh;
//        }
//        if (upperElevationIndex == numElevationSteps) {
//            niRA1uE = niRA2uE = niRA3uE = niRA4uE = HRIRdataPoles[innerRadiusIndex][1];
//            noRA1uE = noRA2uE = noRA3uE = noRA4uE = HRIRdataPoles[outerRadiusIndex][1];
//            nA1uEBaseCh = nA2uEBaseCh = nA3uEBaseCh = nA4uEBaseCh = 0;
//        } else {
//            niRA1uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            niRA2uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            niRA3uE = HRIRdata[innerRadiusIndex][uAzip1]           [upperElevationIndex-1];
//            niRA4uE = HRIRdata[innerRadiusIndex][nAzi2]            [upperElevationIndex-1];
//            noRA1uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            noRA2uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            noRA3uE = HRIRdata[outerRadiusIndex][uAzip1]           [upperElevationIndex-1];
//            noRA4uE = HRIRdata[outerRadiusIndex][nAzi2]            [upperElevationIndex-1];
//            nA1uEBaseCh = lAziBaseCh;
//            nA2uEBaseCh = uAziBaseCh;
//            nA3uEBaseCh = uAzip1BaseCh;
//            nA4uEBaseCh = nAziBaseCh;
//        }
//    } else {
//        mu2n = mu2 + 1;
//        if (lowerElevationIndex == 0) { // NOTE; this is exact same as in nAziUp above
//            niRA1lE = niRA2lE = niRA3lE = niRA4lE = HRIRdataPoles[innerRadiusIndex][0];
//            noRA1lE = noRA2lE = noRA3lE = noRA4lE = HRIRdataPoles[outerRadiusIndex][0];
//            nA1lEBaseCh = nA2lEBaseCh = nA3lEBaseCh = nA4lEBaseCh = 0;
//        } else {
//            niRA1lE = HRIRdata[innerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
//            niRA2lE = HRIRdata[innerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
//            niRA3lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            niRA4lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            noRA1lE = HRIRdata[outerRadiusIndex][nAzi2]            [lowerElevationIndex-1];
//            noRA2lE = HRIRdata[outerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
//            noRA3lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//            noRA4lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//            nA1lEBaseCh = nAziBaseCh;
//            nA2lEBaseCh = lAzim1BaseCh;
//            nA3lEBaseCh = lAziBaseCh;
//            nA4lEBaseCh = uAziBaseCh;
//        }
//        if (upperElevationIndex == numElevationSteps) { // NOTE; this is exact same as in nAziUp above
//            niRA1uE = niRA2uE = niRA3uE = niRA4uE = HRIRdataPoles[innerRadiusIndex][1];
//            noRA1uE = noRA2uE = noRA3uE = noRA4uE = HRIRdataPoles[outerRadiusIndex][1];
//            nA1uEBaseCh = nA2uEBaseCh = nA3uEBaseCh = nA4uEBaseCh = 0;
//        } else {
//            niRA1uE = HRIRdata[innerRadiusIndex][nAzi2]            [upperElevationIndex-1];
//            niRA2uE = HRIRdata[innerRadiusIndex][lAzim1]           [upperElevationIndex-1];
//            niRA3uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            niRA4uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            noRA1uE = HRIRdata[outerRadiusIndex][nAzi2]            [upperElevationIndex-1];
//            noRA2uE = HRIRdata[outerRadiusIndex][lAzim1]           [upperElevationIndex-1];
//            noRA3uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//            noRA4uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//            nA1uEBaseCh = nAziBaseCh;
//            nA2uEBaseCh = lAzim1BaseCh;
//            nA3uEBaseCh = lAziBaseCh;
//            nA4uEBaseCh = uAziBaseCh;
//        }
//    }
//    if (lowerElevationIndex == 0) {
//        iRA1lE = iRA2lE = iRA3lE = iRA4lE = HRIRdataPoles[innerRadiusIndex][0];
//        oRA1lE = oRA2lE = oRA3lE = oRA4lE = HRIRdataPoles[outerRadiusIndex][0];
//    } else {
//        iRA1lE = HRIRdata[innerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
//        iRA2lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//        iRA3lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//        iRA4lE = HRIRdata[innerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
//        oRA1lE = HRIRdata[outerRadiusIndex][lAzim1]           [lowerElevationIndex-1];
//        oRA2lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex-1];
//        oRA3lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex-1];
//        oRA4lE = HRIRdata[outerRadiusIndex][uAzip1]           [lowerElevationIndex-1];
//    }
//    if (upperElevationIndex == numElevationSteps) {
//        iRA1uE = iRA2uE = iRA3uE = iRA4uE = HRIRdataPoles[innerRadiusIndex][1];
//        oRA1uE = oRA2uE = oRA3uE = oRA4uE = HRIRdataPoles[outerRadiusIndex][1];
//    } else {
//        iRA1uE = HRIRdata[innerRadiusIndex][lAzim1]           [upperElevationIndex-1];
//        iRA2uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//        iRA3uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//        iRA4uE = HRIRdata[innerRadiusIndex][uAzip1]           [upperElevationIndex-1];
//        oRA1uE = HRIRdata[outerRadiusIndex][lAzim1]           [upperElevationIndex-1];
//        oRA2uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex-1];
//        oRA3uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex-1];
//        oRA4uE = HRIRdata[outerRadiusIndex][uAzip1]           [upperElevationIndex-1];
//        //        A1uEBaseCh = lAzim1BaseCh;
//        //        A2uEBaseCh = lAziBaseCh;
//        //        A3uEBaseCh = uAziBaseCh;
//        //        A4uEBaseCh = uAzip1BaseCh;
//    }
//    
//    // mu1's are for azi interp
//    const float mu1_1 = mu1 - 1;
//    const float mu1_2 = mu1 - 2;
//    const float mu1_3 = mu1 - 3;
//    const float mu1_4 = mu1 - 4;
//    const float mu1n_1 = mu1n - 1;
//    const float mu1n_2 = mu1n - 2;
//    const float mu1n_3 = mu1n - 3;
//    const float mu1n_4 = mu1n - 4;
//    
//    // mu2's are for ele interp
//    const float mu2_1 = mu2 - 1;
//    const float mu2_2 = mu2 - 2;
//    const float mu2_3 = mu2 - 3;
//    const float mu2_4 = mu2 - 4;
//    const float mu2n_1 = mu2n - 1;
//    const float mu2n_2 = mu2n - 2;
//    const float mu2n_3 = mu2n - 3;
//    const float mu2n_4 = mu2n - 4;
//    
//    const float oneminus_nmu1 = 1.0 - nmu1;
//    const float oneminus_nmu2 = 1.0 - nmu2;
//    const float oneminus_mu3 = 0.5*intensity_factor - mu3;//1.0 - mu3; // scaled by 1/2*intensity_factor here instead of for each sample below
//    const float oneminus_mu1_01 = 1.0 - mu1_01;
//    const float oneminus_mu2_01 = 1.0 - mu2_01;
//    
//    const float a1 = mu1_2 * mu1_3 * mu1_4 * -0.1666666666666666667;
//    const float a2 = mu1_1 * mu1_3 * mu1_4 * 0.5;
//    const float a3 = mu1_1 * mu1_2 * mu1_4 * -0.5;
//    const float a4 = mu1_1 * mu1_2 * mu1_3 * 0.1666666666666666667;
//    const float na1 = mu1n_2 * mu1n_3 * mu1n_4 * -0.1666666666666666667;
//    const float na2 = mu1n_1 * mu1n_3 * mu1n_4 * 0.5;
//    const float na3 = mu1n_1 * mu1n_2 * mu1n_4 * -0.5;
//    const float na4 = mu1n_1 * mu1n_2 * mu1n_3 * 0.1666666666666666667;
//    
//    const float e1 = mu2_2 * mu2_3 * mu2_4 * -0.1666666666666666667;
//    const float e2 = mu2_1 * mu2_3 * mu2_4 * 0.5;
//    const float e3 = mu2_1 * mu2_2 * mu2_4 * -0.5;
//    const float e4 = mu2_1 * mu2_2 * mu2_3 * 0.1666666666666666667;
//    const float ne1 = mu2n_2 * mu2n_3 * mu2n_4 * -0.1666666666666666667;
//    const float ne2 = mu2n_1 * mu2n_3 * mu2n_4 * 0.5;
//    const float ne3 = mu2n_1 * mu2n_2 * mu2n_4 * -0.5;
//    const float ne4 = mu2n_1 * mu2n_2 * mu2n_3 * 0.1666666666666666667;
//    
//    // intermediate interpolation values (in-region/nearby)(inner/outer)(azimuth/elevation)(plus/minus)
//    float inAp, inAm, inEp, inEm, outAp, outAm, outEp, outEm, netIn, netOut;
//    
//    bool again = true;
//    int hrirCh = 0;
//AGAIN:
//    for (int n = 0; n < numTimeSteps; ++n)
//    {
//        inAp = nmu1*(na1*niRuAE1[nuAE1BaseCh][n] + na2*niRuAE2[nuAE2BaseCh][n] + na3*niRuAE3[nuAE3BaseCh][n] + na4*niRuAE4[nuAE4BaseCh][n])
//        + oneminus_nmu1*( a1* iRuAE1[ uAE1BaseCh][n] +  a2* iRuAE2[ uAziBaseCh][n] +  a3* iRuAE3[ uAziBaseCh][n] +  a4* iRuAE4[ uAE4BaseCh][n]);
//        
//        inAm = nmu1*(na1*niRlAE1[nlAE1BaseCh][n] + na2*niRlAE2[nlAE2BaseCh][n] + na3*niRlAE3[nlAE3BaseCh][n] + na4*niRlAE4[nlAE4BaseCh][n])
//        + oneminus_nmu1*( a1* iRlAE1[ lAE1BaseCh][n] +  a2* iRlAE2[ lAziBaseCh][n] +  a3* iRlAE3[ lAziBaseCh][n] +  a4* iRlAE4[ lAE4BaseCh][n]);
//        
//        outAp = nmu1*(na1*noRuAE1[nuAE1BaseCh][n] + na2*noRuAE2[nuAE2BaseCh][n] + na3*noRuAE3[nuAE3BaseCh][n] + na4*noRuAE4[nuAE4BaseCh][n])
//        + oneminus_nmu1*( a1* oRuAE1[ uAE1BaseCh][n] +  a2* oRuAE2[ uAziBaseCh][n] +  a3* oRuAE3[ uAziBaseCh][n] +  a4* oRuAE4[ uAE4BaseCh][n]);
//        
//        outAm = nmu1*(na1*noRlAE1[nlAE1BaseCh][n] + na2*noRlAE2[nlAE2BaseCh][n] + na3*noRlAE3[nlAE3BaseCh][n] + na4*noRlAE4[nlAE4BaseCh][n])
//        + oneminus_nmu1*( a1* oRlAE1[ lAE1BaseCh][n] +  a2* oRlAE2[ lAziBaseCh][n] +  a3* oRlAE3[ lAziBaseCh][n] +  a4* oRlAE4[ lAE4BaseCh][n]);
//        
//        
//        inEp = nmu2*(ne1*niRA1uE[ nA1uEBaseCh][n] + ne2*niRA2uE[nA2uEBaseCh][n] + ne3*niRA3uE[nA3uEBaseCh][n] + ne4*niRA4uE[ nA4uEBaseCh][n])
//        + oneminus_nmu2*( e1* iRA1uE[lAzim1BaseCh][n] +  e2* iRA2uE[ lAziBaseCh][n] +  e3* iRA3uE[ uAziBaseCh][n] +  e4* iRA4uE[uAzip1BaseCh][n]);
//        
//        inEm = nmu2*(ne1*niRA1lE[ nA1lEBaseCh][n] + ne2*niRA2lE[nA2lEBaseCh][n] + ne3*niRA3lE[nA3lEBaseCh][n] + ne4*niRA4lE[ nA4lEBaseCh][n])
//        + oneminus_nmu2*( e1* iRA1lE[lAzim1BaseCh][n] +  e2* iRA2lE[ lAziBaseCh][n] +  e3* iRA3lE[ uAziBaseCh][n] +  e4* iRA4lE[uAzip1BaseCh][n]);
//        
//        outEp = nmu2*(ne1*noRA1uE[ nA1uEBaseCh][n] + ne2*noRA2uE[nA2uEBaseCh][n] + ne3*noRA3uE[nA3uEBaseCh][n] + ne4*noRA4uE[ nA4uEBaseCh][n])
//        + oneminus_nmu2*( e1* oRA1uE[lAzim1BaseCh][n] +  e2* oRA2uE[ lAziBaseCh][n] +  e3* oRA3uE[ uAziBaseCh][n] +  e4* oRA4uE[uAzip1BaseCh][n]);
//        
//        outEm = nmu2*(ne1*noRA1lE[ nA1lEBaseCh][n] + ne2*noRA2lE[nA2lEBaseCh][n] + ne3*noRA3lE[nA3lEBaseCh][n] + ne4*noRA4lE[ nA4lEBaseCh][n])
//        + oneminus_nmu2*( e1* oRA1lE[lAzim1BaseCh][n] +  e2* oRA2lE[ lAziBaseCh][n] +  e3* oRA3lE[ uAziBaseCh][n] +  e4* oRA4lE[uAzip1BaseCh][n]);
//        
//        
//        netIn  = (oneminus_mu1_01*inEm  + mu1_01*inEp)
//        + (oneminus_mu2_01*inAm  + mu2_01*inAp);
//        netOut = (oneminus_mu1_01*outEm + mu1_01*outEp)
//        + (oneminus_mu2_01*outAm + mu2_01*outAp);
//        
//        hrir[hrirCh*numTimeSteps+n] = /*0.5*intensity_factor**/(mu3*netOut + oneminus_mu3*netIn);
//    }
//    if (again)
//    {
//        nuAE1BaseCh = (nuAE1BaseCh + 1) % 2;
//        nuAE2BaseCh = (nuAE2BaseCh + 1) % 2;
//        nuAE3BaseCh = (nuAE3BaseCh + 1) % 2;
//        nuAE4BaseCh = (nuAE4BaseCh + 1) % 2;
//        nlAE1BaseCh = (nlAE1BaseCh + 1) % 2;
//        nlAE2BaseCh = (nlAE2BaseCh + 1) % 2;
//        nlAE3BaseCh = (nlAE3BaseCh + 1) % 2;
//        nlAE4BaseCh = (nlAE4BaseCh + 1) % 2;
//        uAE1BaseCh = ( uAE1BaseCh + 1) % 2;
//        //uAE3BaseCh = ( uAE3BaseCh + 1) % 2;
//        uAE4BaseCh = ( uAE4BaseCh + 1) % 2;
//        lAE1BaseCh = ( lAE1BaseCh + 1) % 2;
//        //lAE3BaseCh = ( lAE3BaseCh + 1) % 2;
//        lAE4BaseCh = ( lAE4BaseCh + 1) % 2;
//        nA1uEBaseCh = (nA1uEBaseCh + 1) % 2;
//        nA2uEBaseCh = (nA2uEBaseCh + 1) % 2;
//        nA3uEBaseCh = (nA3uEBaseCh + 1) % 2;
//        nA4uEBaseCh = (nA4uEBaseCh + 1) % 2;
//        nA1lEBaseCh = (nA1lEBaseCh + 1) % 2;
//        nA2lEBaseCh = (nA2lEBaseCh + 1) % 2;
//        nA3lEBaseCh = (nA3lEBaseCh + 1) % 2;
//        nA4lEBaseCh = (nA4lEBaseCh + 1) % 2;
//        //         A1uEBaseCh = ( A1uEBaseCh + 1) % 2;
//        //         A2uEBaseCh = ( A2uEBaseCh + 1) % 2;
//        //         A3uEBaseCh = ( A3uEBaseCh + 1) % 2;
//        //         A4uEBaseCh = ( A4uEBaseCh + 1) % 2;
//        lAzim1BaseCh = (lAzim1BaseCh + 1) % 2;
//        lAziBaseCh = (  lAziBaseCh + 1) % 2;
//        uAziBaseCh = (  uAziBaseCh + 1) % 2;
//        uAzip1BaseCh = (uAzip1BaseCh + 1) % 2;
//        hrirCh = 1;
//        again = false;
//        goto AGAIN; // i love the goto
//    }
//}
//
////// full (both azimuth sides) and fast (lagrange interp coeffs precomputed) version
////void PlayableSoundSource::interpolateHRIR(const float* rae, float* hrir) const
////{
////    // get the inner + outer rad,azi,ele indicies that define the 3d region bounded by the hrtf/dvf sampling resolution that the source is currently located in
////    const int innerRadiusIndex = std::min((int)((std::log(rae[0])-std::log(distanceBegin))/std::log(distanceEnd/distanceBegin)*(numDistanceSteps-1)), numDistanceSteps-2);//-1);
////    const int outerRadiusIndex = innerRadiusIndex+1;// std::min(innerRadiusIndex+1, numDistanceSteps-1);
////
////    const int lowerElevationIndex = std::min((int)std::floor(rae[2]/M_PI*numElevationSteps), numElevationSteps-1);
////    const int upperElevationIndex = ((lowerElevationIndex != numElevationSteps-1) ? lowerElevationIndex+1 : numElevationSteps-1);
////    bool uEleFlip = false;
////    if (lowerElevationIndex == upperElevationIndex)
////        uEleFlip = true;
////
////    const float revAzi = 2*M_PI-rae[1];//std::fmod((2*M_PI-(rae[1]-2*M_PI/numAzimuthSteps))+2*M_PI , 2*M_PI); // fix reversed azimuth indexing with hrir array's
//////    if (revAzi < 0 || revAzi > 2*M_PI) {
//////        bool notgood = true;
//////    }
////    int lowerAzimuthIndex = std::min((int)std::floor(revAzi/(2*M_PI)*numAzimuthSteps), numAzimuthSteps-1);//numAzimuthSteps-2);
////    int upperAzimuthIndex = (lowerAzimuthIndex+1) % numAzimuthSteps;
////
////    // inner/outer surface radius values
////    const float rIn  = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)innerRadiusIndex)/(numDistanceSteps-1));
////    const float rOut = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)outerRadiusIndex)/(numDistanceSteps-1));
////
////    // upper/lower azimuth values
////    const float aP = ((float)(upperAzimuthIndex))*(2*M_PI)/numAzimuthSteps;//(numAzimuthSteps-1);
////    const float aM = ((float)(lowerAzimuthIndex))*(2*M_PI)/numAzimuthSteps;//(numAzimuthSteps-1);
////
////    // upper/lower elevation values
////    const float eM = ((float)lowerElevationIndex)*M_PI/numElevationSteps;
////    const float eP = ((float)upperElevationIndex)*M_PI/numElevationSteps;
////
////    // for making close/far more loud/quiet
////    const float intensity_factor = 0.1 / std::pow(rae[0], 0.5);
////
////    const float mu3 = 0.5*intensity_factor*std::min((rae[0]-rIn)/(rOut-rIn), 1.0f); // scaled by 1/2*intensity_factor here instead of for each sample below
////
////    // interpolate along azimuth edges
////    const float mu1_01 = (rae[2]-eM)*numElevationSteps/M_PI; // should be btw 0 and 1
////    const float mu1 = mu1_01 + 2;                            // should be btw 2 and 3
////    const float nmu1 = std::abs(2.5-mu1); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
////
////    // interpolate along elevation edges
////    const float mu2_01 = (revAzi-aM)*numAzimuthSteps/(2.0*M_PI); // should be btw 0 and 1
////    const float mu2 = mu2_01 + 2;                                // should be btw 2 and 3
////    const float nmu2 = std::abs(2.5-mu2); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
////
////    // variables for bounds wrapping surrounding data
////    int uAzip1 = upperAzimuthIndex+1;
////    int lAzim1 = lowerAzimuthIndex-1;
////    int uElep1 = lowerElevationIndex+2;//(uEleFlip ? upperElevationIndex-1 : upperElevationIndex+1);
////    int lElem1 = lowerElevationIndex-1;
////
////    if (uAzip1 > numAzimuthSteps-1)
////        uAzip1 -= numAzimuthSteps;
////    if (lAzim1 < 0)
////        lAzim1 += numAzimuthSteps;
////    bool uElep1Flip = false;//uEleFlip;
////    if (uElep1 > numElevationSteps-1) {
////        uElep1 = 2*numElevationSteps-1 - uElep1;
////        uElep1Flip = true;
////    }
////    bool lElem1Flip = false;
////    if (lElem1 < 0) {
////        lElem1 = -lElem1;
////        lElem1Flip = true;
////    }
////
////    int nAzi2, nEle2;  // neighboring azi/ele indices
////    bool nAziUp, nEleUp;
////    if ((aP > aM ? aP-revAzi : 2.0*M_PI-revAzi) > revAzi-aM) {
////        nAzi2 = lowerAzimuthIndex-2;
////        nAziUp = false;
////    } else {
////        nAzi2 = upperAzimuthIndex+2;
////        nAziUp = true;
////    }
////    if ((uEleFlip ? M_PI-rae[2] : eP-rae[2]) > rae[2]-eM) {
////        nEle2 = lowerElevationIndex-2;
////        nEleUp = false;
////    } else {
////        nEle2 = lowerElevationIndex+3;//(uEleFlip ? upperElevationIndex-2 : upperElevationIndex+2);
////        nEleUp = true;
////    }
////
////    if (nAzi2 > numAzimuthSteps-1)
////        nAzi2 -= numAzimuthSteps;
////    if (nAzi2 < 0)
////        nAzi2 += numAzimuthSteps;
////
////    bool nEle2Flip = false;
////    if (nEle2 > numElevationSteps-1) {
////        nEle2 = 2*numElevationSteps-1 - nEle2;
////        nEle2Flip = true;
////    }
////    if (nEle2 < 0) {
////        nEle2 = -nEle2;
////        nEle2Flip = true;
////    }
////
////    // i like things that are difficult to understand (see below, they follow the same pattern as the original interpolateHRIR())
////    float **niRuAE1, **niRuAE2, **niRuAE3, **niRuAE4,
////          **iRuAE1,  **iRuAE2,  **iRuAE3,  **iRuAE4,
////          **niRlAE1, **niRlAE2, **niRlAE3, **niRlAE4,
////          **iRlAE1,  **iRlAE2,  **iRlAE3,  **iRlAE4,
////          **noRuAE1, **noRuAE2, **noRuAE3, **noRuAE4,
////          **oRuAE1,  **oRuAE2,  **oRuAE3,  **oRuAE4,
////          **noRlAE1, **noRlAE2, **noRlAE3, **noRlAE4,
////          **oRlAE1,  **oRlAE2,  **oRlAE3,  **oRlAE4,
////          **niRA1uE, **niRA2uE, **niRA3uE, **niRA4uE,
////          **iRA1uE,  **iRA2uE,  **iRA3uE,  **iRA4uE,
////          **niRA1lE, **niRA2lE, **niRA3lE, **niRA4lE,
////          **iRA1lE,  **iRA2lE,  **iRA3lE,  **iRA4lE,
////          **noRA1uE, **noRA2uE, **noRA3uE, **noRA4uE,
////          **oRA1uE,  **oRA2uE,  **oRA3uE,  **oRA4uE,
////          **noRA1lE, **noRA2lE, **noRA3lE, **noRA4lE,
////          **oRA1lE,  **oRA2lE,  **oRA3lE,  **oRA4lE;
////
////    const int uAzip1EleFlipped        = (uAzip1           +numAzimuthSteps/2) % numAzimuthSteps;
////    const int upperAziIndexEleFlipped = (upperAzimuthIndex+numAzimuthSteps/2) % numAzimuthSteps;
////    const int lowerAziIndexEleFlipped = (lowerAzimuthIndex+numAzimuthSteps/2) % numAzimuthSteps;
////    const int lAzim1EleFlipped        = (lAzim1           +numAzimuthSteps/2) % numAzimuthSteps;
////    const int nAzi2EleFlipped         = (nAzi2            +numAzimuthSteps/2) % numAzimuthSteps;
////
////    float mu1n;
////    if (nEleUp) {
////        mu1n = mu1 - 1;
////        niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        if (uEleFlip) {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        } else {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        }
////        if (uElep1Flip) {
////            niRuAE3 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////            noRuAE3 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////        } else {
////            niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1];
////            noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1];
////            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1];
////            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1];
////        }
////        if (nEle2Flip) {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////        } else {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2];
////        }
////    } else {
////        mu1n = mu1 + 1;
////        if (nEle2Flip) {
////            niRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            noRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////        } else {
////            niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2];
////            noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2];
////            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2];
////            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2];
////        }
////        if (lElem1Flip) {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////        } else {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1];
////        }
////        niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        if (uEleFlip) {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        } else {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        }
////    }
////    if (lElem1Flip) {
////        iRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////        oRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////    } else {
////        iRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1];
////        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1];
////        oRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1];
////        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1];
////    }
////    iRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////    iRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////    oRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////    oRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////    if (uEleFlip) {
////        iRuAE3 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        iRlAE3 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        oRuAE3 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        oRlAE3 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////    } else {
////        iRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        iRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        oRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        oRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////    }
////    if (uElep1Flip) {
////        iRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////        oRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////    } else {
////        iRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1];
////        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1];
////        oRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1];
////        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1];
////    }
////
////    float mu2n;
////    if (nAziUp) {
////        mu2n = mu2 - 1;
////        niRA1lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        niRA2lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        niRA3lE = HRIRdata[innerRadiusIndex][uAzip1][lowerElevationIndex];
////        niRA4lE = HRIRdata[innerRadiusIndex][nAzi2][lowerElevationIndex];
////        noRA1lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        noRA2lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        noRA3lE = HRIRdata[outerRadiusIndex][uAzip1][lowerElevationIndex];
////        noRA4lE = HRIRdata[outerRadiusIndex][nAzi2][lowerElevationIndex];
////        if (uEleFlip) {
////            niRA1uE = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////        } else {
////            niRA1uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex];
////        }
////    } else {
////        mu2n = mu2 + 1;
////        niRA1lE = HRIRdata[innerRadiusIndex][nAzi2][lowerElevationIndex];
////        niRA2lE = HRIRdata[innerRadiusIndex][lAzim1][lowerElevationIndex];
////        niRA3lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        niRA4lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        noRA1lE = HRIRdata[outerRadiusIndex][nAzi2][lowerElevationIndex];
////        noRA2lE = HRIRdata[outerRadiusIndex][lAzim1][lowerElevationIndex];
////        noRA3lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        noRA4lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        if (uEleFlip) {
////            niRA1uE = HRIRdata[innerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        } else {
////            niRA1uE = HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        }
////    }
////    iRA1lE = HRIRdata[innerRadiusIndex][lAzim1][lowerElevationIndex];
////    iRA2lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////    iRA3lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////    iRA4lE = HRIRdata[innerRadiusIndex][uAzip1][lowerElevationIndex];
////    oRA1lE = HRIRdata[outerRadiusIndex][lAzim1][lowerElevationIndex];
////    oRA2lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////    oRA3lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////    oRA4lE = HRIRdata[outerRadiusIndex][uAzip1][lowerElevationIndex];
////    if (uEleFlip) {
////        iRA1uE = HRIRdata[innerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////        iRA2uE = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        iRA3uE = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        iRA4uE = HRIRdata[innerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////        oRA1uE = HRIRdata[outerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////        oRA2uE = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        oRA3uE = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        oRA4uE = HRIRdata[outerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////    } else {
////        iRA1uE = HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex];
////        iRA2uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        iRA3uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        iRA4uE = HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex];
////        oRA1uE = HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex];
////        oRA2uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        oRA3uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        oRA4uE = HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex];
////    }
////
////    // mu1's are for azi interp
////    const float mu1_1 = mu1 - 1;
////    const float mu1_2 = mu1 - 2;
////    const float mu1_3 = mu1 - 3;
////    const float mu1_4 = mu1 - 4;
////    const float mu1n_1 = mu1n - 1;
////    const float mu1n_2 = mu1n - 2;
////    const float mu1n_3 = mu1n - 3;
////    const float mu1n_4 = mu1n - 4;
////
////    // mu2's are for ele interp
////    const float mu2_1 = mu2 - 1;
////    const float mu2_2 = mu2 - 2;
////    const float mu2_3 = mu2 - 3;
////    const float mu2_4 = mu2 - 4;
////    const float mu2n_1 = mu2n - 1;
////    const float mu2n_2 = mu2n - 2;
////    const float mu2n_3 = mu2n - 3;
////    const float mu2n_4 = mu2n - 4;
////
////    const float oneminus_nmu1 = 1.0 - nmu1;
////    const float oneminus_nmu2 = 1.0 - nmu2;
////    const float oneminus_mu3 = 0.5*intensity_factor - mu3;//1.0 - mu3; // scaled by 1/2*intensity_factor here instead of for each sample below
////    const float oneminus_mu1_01 = 1.0 - mu1_01;
////    const float oneminus_mu2_01 = 1.0 - mu2_01;
////
////    const float a1 = mu1_2 * mu1_3 * mu1_4 * -0.1666666666666666667;
////    const float a2 = mu1_1 * mu1_3 * mu1_4 * 0.5;
////    const float a3 = mu1_1 * mu1_2 * mu1_4 * -0.5;
////    const float a4 = mu1_1 * mu1_2 * mu1_3 * 0.1666666666666666667;
////    const float na1 = mu1n_2 * mu1n_3 * mu1n_4 * -0.1666666666666666667;
////    const float na2 = mu1n_1 * mu1n_3 * mu1n_4 * 0.5;
////    const float na3 = mu1n_1 * mu1n_2 * mu1n_4 * -0.5;
////    const float na4 = mu1n_1 * mu1n_2 * mu1n_3 * 0.1666666666666666667;
////
////    const float e1 = mu2_2 * mu2_3 * mu2_4 * -0.1666666666666666667;
////    const float e2 = mu2_1 * mu2_3 * mu2_4 * 0.5;
////    const float e3 = mu2_1 * mu2_2 * mu2_4 * -0.5;
////    const float e4 = mu2_1 * mu2_2 * mu2_3 * 0.1666666666666666667;
////    const float ne1 = mu2n_2 * mu2n_3 * mu2n_4 * -0.1666666666666666667;
////    const float ne2 = mu2n_1 * mu2n_3 * mu2n_4 * 0.5;
////    const float ne3 = mu2n_1 * mu2n_2 * mu2n_4 * -0.5;
////    const float ne4 = mu2n_1 * mu2n_2 * mu2n_3 * 0.1666666666666666667;
////
////    // intermediate interpolation values (in-region/nearby)(inner/outer)(azimuth/elevation)(plus/minus)
////    float inAp, inAm, inEp, inEm, outAp, outAm, outEp, outEm, netIn, netOut;
////
////    for (int ch = 0; ch < 2; ++ch)
////    {
////        for (int n = 0; n < numTimeSteps; ++n)
////        {
////            inAp = nmu1*(na1*niRuAE1[ch][n] + na2*niRuAE2[ch][n] + na3*niRuAE3[ch][n] + na4*niRuAE4[ch][n])
////        + oneminus_nmu1*( a1* iRuAE1[ch][n] +  a2* iRuAE2[ch][n] +  a3* iRuAE3[ch][n] +  a4* iRuAE4[ch][n]);
////
////            inAm = nmu1*(na1*niRlAE1[ch][n] + na2*niRlAE2[ch][n] + na3*niRlAE3[ch][n] + na4*niRlAE4[ch][n])
////        + oneminus_nmu1*( a1* iRlAE1[ch][n] +  a2* iRlAE2[ch][n] +  a3* iRlAE3[ch][n] +  a4* iRlAE4[ch][n]);
////
////            outAp = nmu1*(na1*noRuAE1[ch][n] + na2*noRuAE2[ch][n] + na3*noRuAE3[ch][n] + na4*noRuAE4[ch][n])
////         + oneminus_nmu1*( a1* oRuAE1[ch][n] +  a2* oRuAE2[ch][n] +  a3* oRuAE3[ch][n] +  a4* oRuAE4[ch][n]);
////
////            outAm = nmu1*(na1*noRlAE1[ch][n] + na2*noRlAE2[ch][n] + na3*noRlAE3[ch][n] + na4*noRlAE4[ch][n])
////         + oneminus_nmu1*( a1* oRlAE1[ch][n] +  a2* oRlAE2[ch][n] +  a3* oRlAE3[ch][n] +  a4* oRlAE4[ch][n]);
////
////
////            inEp = nmu2*(ne1*niRA1uE[ch][n] + ne2*niRA2uE[ch][n] + ne3*niRA3uE[ch][n] + ne4*niRA4uE[ch][n])
////        + oneminus_nmu2*( e1* iRA1uE[ch][n] +  e2* iRA2uE[ch][n] +  e3* iRA3uE[ch][n] +  e4* iRA4uE[ch][n]);
////
////            inEm = nmu2*(ne1*niRA1lE[ch][n] + ne2*niRA2lE[ch][n] + ne3*niRA3lE[ch][n] + ne4*niRA4lE[ch][n])
////        + oneminus_nmu2*( e1* iRA1lE[ch][n] +  e2* iRA2lE[ch][n] +  e3* iRA3lE[ch][n] +  e4* iRA4lE[ch][n]);
////
////            outEp = nmu2*(ne1*noRA1uE[ch][n] + ne2*noRA2uE[ch][n] + ne3*noRA3uE[ch][n] + ne4*noRA4uE[ch][n])
////         + oneminus_nmu2*( e1* oRA1uE[ch][n] +  e2* oRA2uE[ch][n] +  e3* oRA3uE[ch][n] +  e4* oRA4uE[ch][n]);
////
////            outEm = nmu2*(ne1*noRA1lE[ch][n] + ne2*noRA2lE[ch][n] + ne3*noRA3lE[ch][n] + ne4*noRA4lE[ch][n])
////         + oneminus_nmu2*( e1* oRA1lE[ch][n] +  e2* oRA2lE[ch][n] +  e3* oRA3lE[ch][n] +  e4* oRA4lE[ch][n]);
////
////            netIn  = (oneminus_mu1_01*inEm  + mu1_01*inEp)
////                   + (oneminus_mu2_01*inAm  + mu2_01*inAp);
////            netOut = (oneminus_mu1_01*outEm + mu1_01*outEp)
////                   + (oneminus_mu2_01*outAm + mu2_01*outAp);
////
////            hrir[ch*numTimeSteps+n] = /*0.5*intensity_factor**/(mu3*netOut + oneminus_mu3*netIn);
////        }
////    }
////}
//
////// compacted (one azimuth side provided) version
////void PlayableSoundSource::interpolateHRIR(const float* rae, float* hrir) const
////{
////    // get the inner + outer rad,azi,ele indicies that define the 3d region bounded by the hrtf/dvf sampling resolution that the source is currently located in
////    const int innerRadiusIndex = std::min((int)((std::log(rae[0])-std::log(distanceBegin))/std::log(distanceEnd/distanceBegin)*(numDistanceSteps-1)), numDistanceSteps-2);//-1);
////    const int outerRadiusIndex = innerRadiusIndex+1;// std::min(innerRadiusIndex+1, numDistanceSteps-1);
////
////    const int lowerElevationIndex = std::min((int)std::floor(rae[2]/M_PI*numElevationSteps), numElevationSteps-1);
////    const int upperElevationIndex = ((lowerElevationIndex != numElevationSteps-1) ? lowerElevationIndex+1 : numElevationSteps-1);
////    bool uEleFlip = false; // upperElevationIndex can flip also
////    if (lowerElevationIndex == upperElevationIndex)
////        uEleFlip = true;
////
////    const float revAzi = 2*M_PI-rae[1]; // fix reversed azimuth indexing with hrir array's
////    int lowerAzimuthIndex = std::min((int)std::floor(revAzi/(2*M_PI)*numAzimuthSteps), numAzimuthSteps-1);//numAzimuthSteps-2);
////    int upperAzimuthIndex = (lowerAzimuthIndex+1) % numAzimuthSteps;
////
////    // inner/outer surface radius values
////    const float rIn  = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)innerRadiusIndex)/(numDistanceSteps-1));
////    const float rOut = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)outerRadiusIndex)/(numDistanceSteps-1));
////
////    // upper/lower azimuth values
////    const float aP = ((float)(upperAzimuthIndex))*(2*M_PI)/numAzimuthSteps;//(numAzimuthSteps-1);
////    const float aM = ((float)(lowerAzimuthIndex))*(2*M_PI)/numAzimuthSteps;//(numAzimuthSteps-1);
////
////    // upper/lower elevation values
////    const float eM = ((float)lowerElevationIndex)*M_PI/numElevationSteps;
////    const float eP = ((float)upperElevationIndex)*M_PI/numElevationSteps;
////
////    // for making close/far more loud/quiet
////    const float intensity_factor = 0.1 / std::pow(rae[0], 0.5);
////
////    const float mu3 = 0.5*intensity_factor*std::min((rae[0]-rIn)/(rOut-rIn), 1.0f); // scaled by 1/2*intensity_factor here instead of for each sample below
////
////    // interpolate along azimuth edges
////    const float mu1_01 = (rae[2]-eM)*numElevationSteps/M_PI; // should be btw 0 and 1
////    const float mu1 = mu1_01 + 2;                            // should be btw 2 and 3
////    const float nmu1 = std::abs(2.5-mu1); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
////
////    // interpolate along elevation edges
////    const float mu2_01 = (revAzi-aM)*numAzimuthSteps/(2.0*M_PI); // should be btw 0 and 1
////    const float mu2 = mu2_01 + 2;                                // should be btw 2 and 3
////    const float nmu2 = std::abs(2.5-mu2); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
////
////    // variables for bounds wrapping surrounding data
////    int uAzip1 = upperAzimuthIndex+1;
////    int lAzim1 = lowerAzimuthIndex-1;
////    int uElep1 = upperElevationIndex+1;
////    int lElem1 = lowerElevationIndex-1;
////
////    if (uAzip1 > numAzimuthSteps-1)
////        uAzip1 -= numAzimuthSteps;
////    if (lAzim1 < 0)
////        lAzim1 += numAzimuthSteps;
////
////    bool lElem1Flip = false, uElep1Flip = uEleFlip;//false; // ele's need bounds wrapping and if they wrap, the corresponding azi it is used with needs flipping
////    if (uElep1 > numElevationSteps-1) {
////        uElep1 = 2*numElevationSteps-1 - uElep1;
////        uElep1Flip = true;
////    }
////    if (lElem1 < 0) {
////        lElem1 = -lElem1;
////        lElem1Flip = true;
////    }
////
////    int nAzi2, nEle2;  // neighboring azi/ele indices
////    bool nAziUp, nEleUp;
////    //if (std::abs(aP > aM ? aP-revAzi : 2.0*M_PI-revAzi) > std::abs(revAzi-aM)) {
////    if ((aP > aM ? aP-revAzi : 2.0*M_PI-revAzi) > revAzi-aM) {
////        nAzi2 = lowerAzimuthIndex-2;
////        nAziUp = false;
////    } else {
////        nAzi2 = upperAzimuthIndex+2;
////        nAziUp = true;
////    }
////    if (uEleFlip ? true : (eP-rae[2] > rae[2]-eM)) {
//////    if (uEleFlip ? true : std::abs(eP-rae[2]) > std::abs(rae[2]-eM)) {
//////        if (revAzi < M_PI) {
//////            nEle2 = lowerElevationIndex-2;
//////            nEleUp = false;
//////        } else {
//////            nEle2 = upperElevationIndex+2;
//////            nEleUp = true;
//////        }
////        nEle2 = lowerElevationIndex-2;
////        nEleUp = false;
////    } else {
//////        if (revAzi < M_PI) {
//////            nEle2 = upperElevationIndex+2;
//////            nEleUp = true;
//////        } else {
//////            nEle2 = upperElevationIndex-2;
//////            nEleUp = false;
//////        }
////        nEle2 = upperElevationIndex+2;
////        nEleUp = true;
////    }
////
////    if (nAzi2 > numAzimuthSteps-1)
////        nAzi2 -= numAzimuthSteps;
////    if (nAzi2 < 0)
////        nAzi2 += numAzimuthSteps;
////
////    bool nEleFlip = false; // ele's need bounds wrapping and if they wrap, the corresponding azi needs flipping
////    if (nEle2 > numElevationSteps-1) {
////        nEle2 = 2*numElevationSteps-1 - nEle2;
////        nEleFlip = true;
////    }
////    if (nEle2 < 0) {
////        nEle2 = -nEle2;
////        nEleFlip = true;
////    }
////
////    // for the compacted data with only one azimuth side provided, we gotta do some channel flipping
////    int lAziBaseCh = 0, uAziBaseCh = 0;
////    if (lowerAzimuthIndex > numAzimuthSteps/2) {
////        lowerAzimuthIndex = numAzimuthSteps - lowerAzimuthIndex;// numAzimuthSteps/2 - (lowerAzimuthIndex - numAzimuthSteps/2);
////        lAziBaseCh = 1;
////    }
////    if (upperAzimuthIndex > numAzimuthSteps/2) {
////        upperAzimuthIndex = numAzimuthSteps - upperAzimuthIndex;
////        uAziBaseCh = 1;
////    }
////    int lAzim1BaseCh = 0, uAzip1BaseCh = 0;
////    if (lAzim1 > numAzimuthSteps/2) {
////        lAzim1 = numAzimuthSteps - lAzim1;
////        lAzim1BaseCh = 1;
////    }
////    if (uAzip1 > numAzimuthSteps/2) {
////        uAzip1 = numAzimuthSteps - uAzip1;
////        uAzip1BaseCh = 1;
////    }
////    int nAziBaseCh = 0;
////    if (nAzi2 > numAzimuthSteps/2) {
////        nAzi2 = numAzimuthSteps - nAzi2;
////        nAziBaseCh = 1;
////    }
////
////    // i like things that are difficult to understand (see below, they follow the same pattern as the original interpolateHRIR())
////    float **niRuAE1, **niRuAE2, **niRuAE3, **niRuAE4,
////           **iRuAE1,  **iRuAE2,  **iRuAE3,  **iRuAE4,
////          **niRlAE1, **niRlAE2, **niRlAE3, **niRlAE4,
////           **iRlAE1,  **iRlAE2,  **iRlAE3,  **iRlAE4,
////          **noRuAE1, **noRuAE2, **noRuAE3, **noRuAE4,
////           **oRuAE1,  **oRuAE2,  **oRuAE3,  **oRuAE4,
////          **noRlAE1, **noRlAE2, **noRlAE3, **noRlAE4,
////           **oRlAE1,  **oRlAE2,  **oRlAE3,  **oRlAE4,
////          **niRA1uE, **niRA2uE, **niRA3uE, **niRA4uE,
////           **iRA1uE,  **iRA2uE,  **iRA3uE,  **iRA4uE,
////          **niRA1lE, **niRA2lE, **niRA3lE, **niRA4lE,
////           **iRA1lE,  **iRA2lE,  **iRA3lE,  **iRA4lE,
////          **noRA1uE, **noRA2uE, **noRA3uE, **noRA4uE,
////           **oRA1uE,  **oRA2uE,  **oRA3uE,  **oRA4uE,
////          **noRA1lE, **noRA2lE, **noRA3lE, **noRA4lE,
////           **oRA1lE,  **oRA2lE,  **oRA3lE,  **oRA4lE;
////
////    // need these cuz bounds wrapped ele indecies can flip their channels or at least the order of the 1234 matters depending on n(Ele/Azi)Up
////    int nuAE1BaseCh, nuAE2BaseCh, nuAE3BaseCh, nuAE4BaseCh,
////        nlAE1BaseCh, nlAE2BaseCh, nlAE3BaseCh, nlAE4BaseCh,
////         uAE1BaseCh,               uAE3BaseCh,  uAE4BaseCh,
////         lAE1BaseCh,               lAE3BaseCh,  lAE4BaseCh,
////        nA1uEBaseCh, nA2uEBaseCh, nA3uEBaseCh, nA4uEBaseCh,
////        nA1lEBaseCh, nA2lEBaseCh, nA3lEBaseCh, nA4lEBaseCh,
////         A1uEBaseCh,  A2uEBaseCh,  A3uEBaseCh,  A4uEBaseCh;
////
////    const int uAzip1EleFlipped        = numAzimuthSteps-(uAzip1           +numAzimuthSteps/2);
////    const int upperAziIndexEleFlipped = numAzimuthSteps-(upperAzimuthIndex+numAzimuthSteps/2);
////    const int lowerAziIndexEleFlipped = numAzimuthSteps-(lowerAzimuthIndex+numAzimuthSteps/2);
////    const int lAzim1EleFlipped        = numAzimuthSteps-(lAzim1           +numAzimuthSteps/2);
////    const int nAzi2EleFlipped         = numAzimuthSteps-(nAzi2            +numAzimuthSteps/2);
//////    const int uAzip1EleFlipped        = numAzimuthSteps-(uAzip1           +numAzimuthSteps/2);
//////    const int upperAziIndexEleFlipped = numAzimuthSteps-(upperAzimuthIndex+numAzimuthSteps/2);
//////    const int lowerAziIndexEleFlipped = numAzimuthSteps-(lowerAzimuthIndex+numAzimuthSteps/2);
//////    const int lAzim1EleFlipped        = numAzimuthSteps-(lAzim1           +numAzimuthSteps/2);
//////    const int nAzi2EleFlipped         = numAzimuthSteps-(nAzi2            +numAzimuthSteps/2);
////
////    float mu1n;
////    if (nEleUp) {
////        mu1n = mu1 - 1;
////        niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
//////        niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
//////        noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
//////        niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
//////        noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        nuAE1BaseCh = uAziBaseCh;
//////        nuAE2BaseCh = uAziBaseCh;
////        nlAE1BaseCh = lAziBaseCh;
//////        nlAE2BaseCh = lAziBaseCh;
////        if (uEleFlip) {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            nuAE2BaseCh = (uAziBaseCh + 1) % 2;
////            nlAE2BaseCh = (lAziBaseCh + 1) % 2;
////        } else {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            nuAE2BaseCh = uAziBaseCh;
////            nlAE2BaseCh = lAziBaseCh;
////        }
////        if (uElep1Flip) {
////            niRuAE3 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////            noRuAE3 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////            nuAE3BaseCh = (uAziBaseCh + 1) % 2;
////            nlAE3BaseCh = (lAziBaseCh + 1) % 2;
////        } else {
////            niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1];
////            noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1];
////            niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1];
////            noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1];
////            nuAE3BaseCh = uAziBaseCh;
////            nlAE3BaseCh = lAziBaseCh;
////        }
////        if (nEleFlip) {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////            nuAE4BaseCh = (uAziBaseCh + 1) % 2;
////            nlAE4BaseCh = (lAziBaseCh + 1) % 2;
////        } else {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2];
////            nuAE4BaseCh = uAziBaseCh;
////            nlAE4BaseCh = lAziBaseCh;
////        }
////    } else {
////        mu1n = mu1 + 1;
////        niRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
//////        niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        noRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
//////        noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        niRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
//////        niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        noRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
//////        noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        nuAE3BaseCh = uAziBaseCh;
//////        nuAE4BaseCh = uAziBaseCh;
////        nlAE3BaseCh = lAziBaseCh;
//////        nlAE4BaseCh = lAziBaseCh;
////        if (uEleFlip) {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            nuAE4BaseCh = (uAziBaseCh + 1) % 2;
////            nlAE4BaseCh = (lAziBaseCh + 1) % 2;
////        } else {
////            niRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            niRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            nuAE4BaseCh = uAziBaseCh;
////            nlAE4BaseCh = lAziBaseCh;
////        }
////        if (nEleFlip) {
////            niRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            noRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][nEle2];
////            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][nEle2];
////            nuAE1BaseCh = (uAziBaseCh + 1) % 2;
////            nlAE1BaseCh = (lAziBaseCh + 1) % 2;
////        } else {
////            niRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2];
////            noRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2];
////            niRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2];
////            noRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2];
////            nuAE1BaseCh = uAziBaseCh;
////            nlAE1BaseCh = lAziBaseCh;
////        }
////        if (lElem1Flip) {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////            nuAE2BaseCh = (uAziBaseCh + 1) % 2;
////            nlAE2BaseCh = (lAziBaseCh + 1) % 2;
////        } else {
////            niRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1];
////            noRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1];
////            niRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1];
////            noRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1];
////            nuAE2BaseCh = uAziBaseCh;
////            nlAE2BaseCh = lAziBaseCh;
////        }
////    }
////    iRuAE2 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
//////    iRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////    iRlAE2 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
//////    iRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////    oRuAE2 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
//////    oRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////    oRlAE2 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
//////    oRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////    if (uEleFlip) {
////        iRuAE3 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        iRlAE3 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        oRuAE3 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        oRlAE3 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        uAE3BaseCh = (uAziBaseCh + 1) % 2;
////        lAE3BaseCh = (lAziBaseCh + 1) % 2;
////    } else {
////        iRuAE3 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        iRlAE3 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        oRuAE3 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        oRlAE3 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        uAE3BaseCh = uAziBaseCh;
////        lAE3BaseCh = lAziBaseCh;
////    }
////    if (lElem1Flip) {
////        iRuAE1 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////        oRuAE1 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][lElem1];
////        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][lElem1];
////        uAE1BaseCh = (uAziBaseCh + 1) % 2;
////        lAE1BaseCh = (lAziBaseCh + 1) % 2;
////    } else {
////        iRuAE1 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1];
////        iRlAE1 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1];
////        oRuAE1 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1];
////        oRlAE1 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1];
////        uAE1BaseCh = uAziBaseCh;
////        lAE1BaseCh = lAziBaseCh;
////    }
////    if (uElep1Flip) {
////        iRuAE4 = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////        oRuAE4 = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][uElep1];
////        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][uElep1];
////        uAE4BaseCh = (uAziBaseCh + 1) % 2;
////        lAE4BaseCh = (lAziBaseCh + 1) % 2;
////    } else {
////        iRuAE4 = HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1];
////        iRlAE4 = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1];
////        oRuAE4 = HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1];
////        oRlAE4 = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1];
////        uAE4BaseCh = uAziBaseCh;
////        lAE4BaseCh = lAziBaseCh;
////    }
////
////    float mu2n;
////    if (nAziUp) {
////        mu2n = mu2 - 1;
//////        niRA1uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
//////        niRA2uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
//////        niRA3uE = HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex];
//////        niRA4uE = HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex];
//////        noRA1uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
//////        noRA2uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
//////        noRA3uE = HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex];
//////        noRA4uE = HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex];
////        niRA1lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        niRA2lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        niRA3lE = HRIRdata[innerRadiusIndex][uAzip1][lowerElevationIndex];
////        niRA4lE = HRIRdata[innerRadiusIndex][nAzi2][lowerElevationIndex];
////        noRA1lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        noRA2lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        noRA3lE = HRIRdata[outerRadiusIndex][uAzip1][lowerElevationIndex];
////        noRA4lE = HRIRdata[outerRadiusIndex][nAzi2][lowerElevationIndex];
////        nA1lEBaseCh = lAziBaseCh;
////        nA2lEBaseCh = uAziBaseCh;
////        nA3lEBaseCh = uAzip1BaseCh;
////        nA4lEBaseCh = nAziBaseCh;
//////        nA1uEBaseCh = nA1lEBaseCh = lAziBaseCh;
//////        nA2uEBaseCh = nA2lEBaseCh = uAziBaseCh;
//////        nA3uEBaseCh = nA3lEBaseCh = uAzip1BaseCh;
//////        nA4uEBaseCh = nA4lEBaseCh = nAziBaseCh;
////        if (uEleFlip) {
////            niRA1uE = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            nA1uEBaseCh = (lAziBaseCh + 1) % 2;
////            nA2uEBaseCh = (uAziBaseCh + 1) % 2;
////            nA3uEBaseCh = (uAzip1BaseCh + 1) % 2;
////            nA4uEBaseCh = (nAziBaseCh + 1) % 2;
////        } else {
////            niRA1uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex];
////            nA1uEBaseCh = lAziBaseCh;
////            nA2uEBaseCh = uAziBaseCh;
////            nA3uEBaseCh = uAzip1BaseCh;
////            nA4uEBaseCh = nAziBaseCh;
////        }
////    } else {
////        mu2n = mu2 + 1;
//////        niRA1uE = HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex];
//////        niRA2uE = HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex];
//////        niRA3uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
//////        niRA4uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
//////        noRA1uE = HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex];
//////        noRA2uE = HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex];
//////        noRA3uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
//////        noRA4uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        niRA1lE = HRIRdata[innerRadiusIndex][nAzi2][lowerElevationIndex];
////        niRA2lE = HRIRdata[innerRadiusIndex][lAzim1][lowerElevationIndex];
////        niRA3lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        niRA4lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        noRA1lE = HRIRdata[outerRadiusIndex][nAzi2][lowerElevationIndex];
////        noRA2lE = HRIRdata[outerRadiusIndex][lAzim1][lowerElevationIndex];
////        noRA3lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////        noRA4lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////        nA1lEBaseCh = nAziBaseCh;
////        nA2lEBaseCh = lAzim1BaseCh;
////        nA3lEBaseCh = lAziBaseCh;
////        nA4lEBaseCh = uAziBaseCh;
//////        nA1uEBaseCh = nA1lEBaseCh = nAziBaseCh;
//////        nA2uEBaseCh = nA2lEBaseCh = lAzim1BaseCh;
//////        nA3uEBaseCh = nA3lEBaseCh = lAziBaseCh;
//////        nA4uEBaseCh = nA4lEBaseCh = uAziBaseCh;
////        if (uEleFlip) {
////            niRA1uE = HRIRdata[innerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][nAzi2EleFlipped][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////            nA1uEBaseCh = (nAziBaseCh + 1) % 2;
////            nA2uEBaseCh = (lAzim1BaseCh + 1) % 2;
////            nA3uEBaseCh = (lAziBaseCh + 1) % 2;
////            nA4uEBaseCh = (uAziBaseCh + 1) % 2;
////        } else {
////            niRA1uE = HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex];
////            niRA2uE = HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex];
////            niRA3uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            niRA4uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            noRA1uE = HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex];
////            noRA2uE = HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex];
////            noRA3uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////            noRA4uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////            nA1uEBaseCh = nAziBaseCh;
////            nA2uEBaseCh = lAzim1BaseCh;
////            nA3uEBaseCh = lAziBaseCh;
////            nA4uEBaseCh = uAziBaseCh;
////        }
////    }
//////    iRA1uE = HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex];
//////    iRA2uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
//////    iRA3uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
//////    iRA4uE = HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex];
////    iRA1lE = HRIRdata[innerRadiusIndex][lAzim1][lowerElevationIndex];
////    iRA2lE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////    iRA3lE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////    iRA4lE = HRIRdata[innerRadiusIndex][uAzip1][lowerElevationIndex];
//////    oRA1uE = HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex];
//////    oRA2uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
//////    oRA3uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
//////    oRA4uE = HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex];
////    oRA1lE = HRIRdata[outerRadiusIndex][lAzim1][lowerElevationIndex];
////    oRA2lE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex];
////    oRA3lE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex];
////    oRA4lE = HRIRdata[outerRadiusIndex][uAzip1][lowerElevationIndex];
////    if (uEleFlip) {
////        iRA1uE = HRIRdata[innerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////        iRA2uE = HRIRdata[innerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        iRA3uE = HRIRdata[innerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        iRA4uE = HRIRdata[innerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////        oRA1uE = HRIRdata[outerRadiusIndex][lAzim1EleFlipped][upperElevationIndex];
////        oRA2uE = HRIRdata[outerRadiusIndex][lowerAziIndexEleFlipped][upperElevationIndex];
////        oRA3uE = HRIRdata[outerRadiusIndex][upperAziIndexEleFlipped][upperElevationIndex];
////        oRA4uE = HRIRdata[outerRadiusIndex][uAzip1EleFlipped][upperElevationIndex];
////        A1uEBaseCh = (lAzim1BaseCh + 1) % 2;
////        A2uEBaseCh = (lAziBaseCh + 1) % 2;
////        A3uEBaseCh = (uAziBaseCh + 1) % 2;
////        A4uEBaseCh = (uAzip1BaseCh + 1) % 2;
////    } else {
////        iRA1uE = HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex];
////        iRA2uE = HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        iRA3uE = HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        iRA4uE = HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex];
////        oRA1uE = HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex];
////        oRA2uE = HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex];
////        oRA3uE = HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex];
////        oRA4uE = HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex];
////        A1uEBaseCh = lAzim1BaseCh;
////        A2uEBaseCh = lAziBaseCh;
////        A3uEBaseCh = uAziBaseCh;
////        A4uEBaseCh = uAzip1BaseCh;
////    }
////
////    // mu1's are for azi interp
////    const float mu1_1 = mu1 - 1;
////    const float mu1_2 = mu1 - 2;
////    const float mu1_3 = mu1 - 3;
////    const float mu1_4 = mu1 - 4;
////    const float mu1n_1 = mu1n - 1;
////    const float mu1n_2 = mu1n - 2;
////    const float mu1n_3 = mu1n - 3;
////    const float mu1n_4 = mu1n - 4;
////
////    // mu2's are for ele interp
////    const float mu2_1 = mu2 - 1;
////    const float mu2_2 = mu2 - 2;
////    const float mu2_3 = mu2 - 3;
////    const float mu2_4 = mu2 - 4;
////    const float mu2n_1 = mu2n - 1;
////    const float mu2n_2 = mu2n - 2;
////    const float mu2n_3 = mu2n - 3;
////    const float mu2n_4 = mu2n - 4;
////
////    //const float nmu1 = std::abs(2.5-mu1);
////    const float oneminus_nmu1 = 1.0 - nmu1;
////    //const float nmu2 = std::abs(2.5-mu2);
////    const float oneminus_nmu2 = 1.0 - nmu2;
////    const float oneminus_mu3 = 0.5*intensity_factor - mu3;//1.0 - mu3; // scaled by 1/2*intensity_factor here instead of for each sample below
////    const float oneminus_mu1_01 = 1.0 - mu1_01;
////    const float oneminus_mu2_01 = 1.0 - mu2_01;
////
////    const float a1 = mu1_2 * mu1_3 * mu1_4 * -0.1666666666666666667;
////    const float a2 = mu1_1 * mu1_3 * mu1_4 * 0.5;
////    const float a3 = mu1_1 * mu1_2 * mu1_4 * -0.5;
////    const float a4 = mu1_1 * mu1_2 * mu1_3 * 0.1666666666666666667;
////    const float na1 = mu1n_2 * mu1n_3 * mu1n_4 * -0.1666666666666666667;
////    const float na2 = mu1n_1 * mu1n_3 * mu1n_4 * 0.5;
////    const float na3 = mu1n_1 * mu1n_2 * mu1n_4 * -0.5;
////    const float na4 = mu1n_1 * mu1n_2 * mu1n_3 * 0.1666666666666666667;
////
////    const float e1 = mu2_2 * mu2_3 * mu2_4 * -0.1666666666666666667;
////    const float e2 = mu2_1 * mu2_3 * mu2_4 * 0.5;
////    const float e3 = mu2_1 * mu2_2 * mu2_4 * -0.5;
////    const float e4 = mu2_1 * mu2_2 * mu2_3 * 0.1666666666666666667;
////    const float ne1 = mu2n_2 * mu2n_3 * mu2n_4 * -0.1666666666666666667;
////    const float ne2 = mu2n_1 * mu2n_3 * mu2n_4 * 0.5;
////    const float ne3 = mu2n_1 * mu2n_2 * mu2n_4 * -0.5;
////    const float ne4 = mu2n_1 * mu2n_2 * mu2n_3 * 0.1666666666666666667;
////
////    // intermediate interpolation values (in-region/nearby)(inner/outer)(azimuth/elevation)(plus/minus)
////    float inAp, inAm, inEp, inEm, outAp, outAm, outEp, outEm, netIn, netOut;
////
////    bool again = true;
////    int hrirCh = 0;
////AGAIN:
////    for (int n = 0; n < numTimeSteps; ++n)
////    {
////        inAp = nmu1*(na1*niRuAE1[nuAE1BaseCh][n] + na2*niRuAE2[nuAE2BaseCh][n] + na3*niRuAE3[nuAE3BaseCh][n] + na4*niRuAE4[nuAE4BaseCh][n])
////    + oneminus_nmu1*( a1* iRuAE1[ uAE1BaseCh][n] +  a2* iRuAE2[ uAziBaseCh][n] +  a3* iRuAE3[ uAE3BaseCh][n] +  a4* iRuAE4[ uAE4BaseCh][n]);
////
////        inAm = nmu1*(na1*niRlAE1[nlAE1BaseCh][n] + na2*niRlAE2[nlAE2BaseCh][n] + na3*niRlAE3[nlAE3BaseCh][n] + na4*niRlAE4[nlAE4BaseCh][n])
////    + oneminus_nmu1*( a1* iRlAE1[ lAE1BaseCh][n] +  a2* iRlAE2[ lAziBaseCh][n] +  a3* iRlAE3[ lAE3BaseCh][n] +  a4* iRlAE4[ lAE4BaseCh][n]);
////
////        outAp = nmu1*(na1*noRuAE1[nuAE1BaseCh][n] + na2*noRuAE2[nuAE2BaseCh][n] + na3*noRuAE3[nuAE3BaseCh][n] + na4*noRuAE4[nuAE4BaseCh][n])
////     + oneminus_nmu1*( a1* oRuAE1[ uAE1BaseCh][n] +  a2* oRuAE2[ uAziBaseCh][n] +  a3* oRuAE3[ uAE3BaseCh][n] +  a4* oRuAE4[ uAE4BaseCh][n]);
////
////        outAm = nmu1*(na1*noRlAE1[nlAE1BaseCh][n] + na2*noRlAE2[nlAE2BaseCh][n] + na3*noRlAE3[nlAE3BaseCh][n] + na4*noRlAE4[nlAE4BaseCh][n])
////     + oneminus_nmu1*( a1* oRlAE1[ lAE1BaseCh][n] +  a2* oRlAE2[ lAziBaseCh][n] +  a3* oRlAE3[ lAE3BaseCh][n] +  a4* oRlAE4[ lAE4BaseCh][n]);
////
////
////        inEp = nmu2*(ne1*niRA1uE[ nA1uEBaseCh][n] + ne2*niRA2uE[nA2uEBaseCh][n] + ne3*niRA3uE[nA3uEBaseCh][n] + ne4*niRA4uE[ nA4uEBaseCh][n])
////    + oneminus_nmu2*( e1* iRA1uE[  A1uEBaseCh][n] +  e2* iRA2uE[ A2uEBaseCh][n] +  e3* iRA3uE[ A3uEBaseCh][n] +  e4* iRA4uE[  A4uEBaseCh][n]);
////
////        inEm = nmu2*(ne1*niRA1lE[ nA1lEBaseCh][n] + ne2*niRA2lE[nA2lEBaseCh][n] + ne3*niRA3lE[nA3lEBaseCh][n] + ne4*niRA4lE[ nA4lEBaseCh][n])
////    + oneminus_nmu2*( e1* iRA1lE[lAzim1BaseCh][n] +  e2* iRA2lE[ lAziBaseCh][n] +  e3* iRA3lE[ uAziBaseCh][n] +  e4* iRA4lE[uAzip1BaseCh][n]);
////
////        outEp = nmu2*(ne1*noRA1uE[ nA1uEBaseCh][n] + ne2*noRA2uE[nA2uEBaseCh][n] + ne3*noRA3uE[nA3uEBaseCh][n] + ne4*noRA4uE[ nA4uEBaseCh][n])
////     + oneminus_nmu2*( e1* oRA1uE[  A1uEBaseCh][n] +  e2* oRA2uE[ A2uEBaseCh][n] +  e3* oRA3uE[ A3uEBaseCh][n] +  e4* oRA4uE[  A4uEBaseCh][n]);
////
////        outEm = nmu2*(ne1*noRA1lE[ nA1lEBaseCh][n] + ne2*noRA2lE[nA2lEBaseCh][n] + ne3*noRA3lE[nA3lEBaseCh][n] + ne4*noRA4lE[ nA4lEBaseCh][n])
////     + oneminus_nmu2*( e1* oRA1lE[lAzim1BaseCh][n] +  e2* oRA2lE[ lAziBaseCh][n] +  e3* oRA3lE[ uAziBaseCh][n] +  e4* oRA4lE[uAzip1BaseCh][n]);
////
//////        inAp = nmu1*(na1*niRuAE1[nuAE1BaseCh][n] + na2*niRuAE2[nuAE2BaseCh][n] + na3*niRuAE3[nuAE3BaseCh][n] + na4*niRuAE4[nuAE4BaseCh][n])
//////        + oneminus_nmu1*( a1* iRuAE1[ uAE1BaseCh][n] +  a2* iRuAE2[ uAziBaseCh][n] +  a3* iRuAE3[ uAziBaseCh][n] +  a4* iRuAE4[ uAE4BaseCh][n]);
//////
//////        inAm = nmu1*(na1*niRlAE1[nlAE1BaseCh][n] + na2*niRlAE2[nlAE2BaseCh][n] + na3*niRlAE3[nlAE3BaseCh][n] + na4*niRlAE4[nlAE4BaseCh][n])
//////        + oneminus_nmu1*( a1* iRlAE1[ lAE1BaseCh][n] +  a2* iRlAE2[ lAziBaseCh][n] +  a3* iRlAE3[ lAziBaseCh][n] +  a4* iRlAE4[ lAE4BaseCh][n]);
//////
//////        outAp = nmu1*(na1*noRuAE1[nuAE1BaseCh][n] + na2*noRuAE2[nuAE2BaseCh][n] + na3*noRuAE3[nuAE3BaseCh][n] + na4*noRuAE4[nuAE4BaseCh][n])
//////        + oneminus_nmu1*( a1* oRuAE1[ uAE1BaseCh][n] +  a2* oRuAE2[ uAziBaseCh][n] +  a3* oRuAE3[ uAziBaseCh][n] +  a4* oRuAE4[ uAE4BaseCh][n]);
//////
//////        outAm = nmu1*(na1*noRlAE1[nlAE1BaseCh][n] + na2*noRlAE2[nlAE2BaseCh][n] + na3*noRlAE3[nlAE3BaseCh][n] + na4*noRlAE4[nlAE4BaseCh][n])
//////        + oneminus_nmu1*( a1* oRlAE1[ lAE1BaseCh][n] +  a2* oRlAE2[ lAziBaseCh][n] +  a3* oRlAE3[ lAziBaseCh][n] +  a4* oRlAE4[ lAE4BaseCh][n]);
//////
//////
//////        inEp = nmu2*(ne1*niRA1uE[ nA1uEBaseCh][n] + ne2*niRA2uE[nA2uEBaseCh][n] + ne3*niRA3uE[nA3uEBaseCh][n] + ne4*niRA4uE[ nA4uEBaseCh][n])
//////        + oneminus_nmu2*( e1* iRA1uE[lAzim1BaseCh][n] +  e2* iRA2uE[ lAziBaseCh][n] +  e3* iRA3uE[ uAziBaseCh][n] +  e4* iRA4uE[uAzip1BaseCh][n]);
//////
//////        inEm = nmu2*(ne1*niRA1lE[ nA1lEBaseCh][n] + ne2*niRA2lE[nA2lEBaseCh][n] + ne3*niRA3lE[nA3lEBaseCh][n] + ne4*niRA4lE[ nA4lEBaseCh][n])
//////        + oneminus_nmu2*( e1* iRA1lE[lAzim1BaseCh][n] +  e2* iRA2lE[ lAziBaseCh][n] +  e3* iRA3lE[ uAziBaseCh][n] +  e4* iRA4lE[uAzip1BaseCh][n]);
//////
//////        outEp = nmu2*(ne1*noRA1uE[ nA1uEBaseCh][n] + ne2*noRA2uE[nA2uEBaseCh][n] + ne3*noRA3uE[nA3uEBaseCh][n] + ne4*noRA4uE[ nA4uEBaseCh][n])
//////        + oneminus_nmu2*( e1* oRA1uE[lAzim1BaseCh][n] +  e2* oRA2uE[ lAziBaseCh][n] +  e3* oRA3uE[ uAziBaseCh][n] +  e4* oRA4uE[uAzip1BaseCh][n]);
//////
//////        outEm = nmu2*(ne1*noRA1lE[ nA1lEBaseCh][n] + ne2*noRA2lE[nA2lEBaseCh][n] + ne3*noRA3lE[nA3lEBaseCh][n] + ne4*noRA4lE[ nA4lEBaseCh][n])
//////        + oneminus_nmu2*( e1* oRA1lE[lAzim1BaseCh][n] +  e2* oRA2lE[ lAziBaseCh][n] +  e3* oRA3lE[ uAziBaseCh][n] +  e4* oRA4lE[uAzip1BaseCh][n]);
////
////        netIn  = (oneminus_mu1_01*inEm  + mu1_01*inEp)
////               + (oneminus_mu2_01*inAm  + mu2_01*inAp);
////        netOut = (oneminus_mu1_01*outEm + mu1_01*outEp)
////               + (oneminus_mu2_01*outAm + mu2_01*outAp);
////
////        hrir[hrirCh*numTimeSteps+n] = /*0.5*intensity_factor**/(mu3*netOut + oneminus_mu3*netIn);
////    }
////    if (again)
////    {
////        nuAE1BaseCh = (nuAE1BaseCh + 1) % 2;
////        nuAE2BaseCh = (nuAE2BaseCh + 1) % 2;
////        nuAE3BaseCh = (nuAE3BaseCh + 1) % 2;
////        nuAE4BaseCh = (nuAE4BaseCh + 1) % 2;
////        nlAE1BaseCh = (nlAE1BaseCh + 1) % 2;
////        nlAE2BaseCh = (nlAE2BaseCh + 1) % 2;
////        nlAE3BaseCh = (nlAE3BaseCh + 1) % 2;
////        nlAE4BaseCh = (nlAE4BaseCh + 1) % 2;
////         uAE1BaseCh = ( uAE1BaseCh + 1) % 2;
////         uAE3BaseCh = ( uAE3BaseCh + 1) % 2;
////         uAE4BaseCh = ( uAE4BaseCh + 1) % 2;
////         lAE1BaseCh = ( lAE1BaseCh + 1) % 2;
////         lAE3BaseCh = ( lAE3BaseCh + 1) % 2;
////         lAE4BaseCh = ( lAE4BaseCh + 1) % 2;
////        nA1uEBaseCh = (nA1uEBaseCh + 1) % 2;
////        nA2uEBaseCh = (nA2uEBaseCh + 1) % 2;
////        nA3uEBaseCh = (nA3uEBaseCh + 1) % 2;
////        nA4uEBaseCh = (nA4uEBaseCh + 1) % 2;
////        nA1lEBaseCh = (nA1lEBaseCh + 1) % 2;
////        nA2lEBaseCh = (nA2lEBaseCh + 1) % 2;
////        nA3lEBaseCh = (nA3lEBaseCh + 1) % 2;
////        nA4lEBaseCh = (nA4lEBaseCh + 1) % 2;
////         A1uEBaseCh = ( A1uEBaseCh + 1) % 2;
////         A2uEBaseCh = ( A2uEBaseCh + 1) % 2;
////         A3uEBaseCh = ( A3uEBaseCh + 1) % 2;
////         A4uEBaseCh = ( A4uEBaseCh + 1) % 2;
////        lAzim1BaseCh = (lAzim1BaseCh + 1) % 2;
////          lAziBaseCh = (  lAziBaseCh + 1) % 2;
////          uAziBaseCh = (  uAziBaseCh + 1) % 2;
////        uAzip1BaseCh = (uAzip1BaseCh + 1) % 2;
////        hrirCh = 1;
////        again = false;
////        goto AGAIN; // i love the goto
////    }
////}
//
////// non-compact version
////void PlayableSoundSource::interpolateHRIR(const float* rae, float* hrir) const
////{
////    // get the inner + outer rad,azi,ele indicies that define the 3d region bounded by the hrtf/dvf sampling resolution that the source is currently located in
////    int innerRadiusIndex, outerRadiusIndex;
////    if (rae[0] < distanceEnd) {
////        innerRadiusIndex = (std::log(rae[0]) - std::log(distanceBegin)) / std::log(distanceEnd/distanceBegin)
////                            * (numDistanceSteps - 1);
////        outerRadiusIndex = innerRadiusIndex + 1;
//////        float di = 0.0;
//////        for (int i = 0; (i < numDistanceSteps) && (di <= rae[0]); ++i) {
//////
//////            di = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)i)/(numDistanceSteps-1.0));
//////
//////            if (di > rae[0]) {
//////                innerRadiusIndex = i-1;
//////                outerRadiusIndex = i;
//////            }
//////        }
////    } else {
////        innerRadiusIndex = numDistanceSteps-1;
////        outerRadiusIndex = numDistanceSteps-1;
////    }
////
////    int lowerElevationIndex = (int) std::floor(rae[2]/M_PI * (numElevationSteps-1));
////    int upperElevationIndex = (int) std::ceil(rae[2]/M_PI * (numElevationSteps-1));
////
////    // fix reversed azimuth indexing with hrir array's
////    float revAzi = 2*M_PI-rae[1];
////    int lowerAzimuthIndex = (int) std::floor(revAzi/(2*M_PI) * (numAzimuthSteps-1));
////    int upperAzimuthIndex = (int) std::ceil(revAzi/(2*M_PI) * (numAzimuthSteps-1));
////
////    // bounds check indicies
////    if (innerRadiusIndex > numDistanceSteps-1)
////        innerRadiusIndex = numDistanceSteps-1;
////    if (innerRadiusIndex < 0)
////        innerRadiusIndex = 0;
////    if (outerRadiusIndex > numDistanceSteps-1)
////        outerRadiusIndex = numDistanceSteps-1;
////    if (outerRadiusIndex < 0)
////        outerRadiusIndex = 0;
////
////    if (lowerElevationIndex > numElevationSteps-2)
////        lowerElevationIndex = numElevationSteps-2;
////    if (lowerElevationIndex < 0)
////        lowerElevationIndex = 0;
////    if (upperElevationIndex > numElevationSteps-1)
////        upperElevationIndex = numElevationSteps-1;
////    if (upperElevationIndex < 1)
////        upperElevationIndex = 1;
////
////    if (lowerAzimuthIndex > numAzimuthSteps-2)
////        lowerAzimuthIndex = numAzimuthSteps-2;
////    if (lowerAzimuthIndex < 0)
////        lowerAzimuthIndex = 0;
////    if (upperAzimuthIndex > numAzimuthSteps-1)
////        upperAzimuthIndex = numAzimuthSteps-1;
////    if (upperAzimuthIndex < 1)
////        upperAzimuthIndex = 1;
////
////    float rIn, rOut, eP, eM, aP, aM;
////    float mu1, nmu1, mu2, nmu2, mu3;
////
////    // inner/outer surface radius values
////    rIn = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)innerRadiusIndex)/(numDistanceSteps-1.0));
////    rOut = distanceBegin*std::pow(distanceEnd/distanceBegin, ((float)outerRadiusIndex)/(numDistanceSteps-1.0));
////
////    // upper/lower azimuth values
////    aP = ((float)(upperAzimuthIndex))*(2.0*M_PI)/(numAzimuthSteps-1.0);
////    aM = ((float)(lowerAzimuthIndex))*(2.0*M_PI)/(numAzimuthSteps-1.0);
////
////    // upper/lower elevation values
////    eM = ((float)lowerElevationIndex)*M_PI/(numElevationSteps-1.0);
////    eP = ((float)upperElevationIndex)*M_PI/(numElevationSteps-1.0);
////
////    float intensity_factor;
////    if (rae[0] < distanceBegin)
////        intensity_factor = /*1.0/32.5 * 3.25*/0.1/std::pow(distanceBegin, 0.5);
////    else
////        intensity_factor = /*1.0/32.5 * 3.25*/0.1/std::pow(rae[0], 0.5);
////
////    // variables for bounds wrapping surrounding data
////    int uAzip1 = upperAzimuthIndex+1;
////    int lAzim1 = lowerAzimuthIndex-1;
////    int uElep1 = upperElevationIndex+1;
////    int lElem1 = lowerElevationIndex-1;
////
////    if (uAzip1 > numAzimuthSteps-1)
////        uAzip1 -= numAzimuthSteps;
////    if (lAzim1 < 0)
////        lAzim1 += numAzimuthSteps;
////
////    if (uElep1 > numElevationSteps-1)
////        uElep1 -= numElevationSteps;
////    if (lElem1 < 0)
////        lElem1 += numElevationSteps;
////
////    int nAzi2, nEle2;  // neighboring azi/ele indices
////    bool nAziUp, nEleUp;
////    if (std::abs(revAzi-aP) > std::abs(revAzi-aM)) {
////        nAzi2 = lowerAzimuthIndex-2;
////        nAziUp = false;
////    } else {
////        nAzi2 = upperAzimuthIndex+2;
////        nAziUp = true;
////    }
////    if (std::abs(rae[2]-eP) > std::abs(rae[2]-eM)) {
////        nEle2 = lowerElevationIndex-2;
////        nEleUp = false;
////    } else {
////        nEle2 = upperElevationIndex+2;
////        nEleUp = true;
////    }
////
////    if (nAzi2 > numAzimuthSteps-1)
////        nAzi2 -= numAzimuthSteps;
////    if (nAzi2 < 0)
////        nAzi2 += numAzimuthSteps;
////
////    if (nEle2 > numElevationSteps-1)
////        nEle2 -= numElevationSteps;
////    if (nEle2 < 0)
////        nEle2 += numElevationSteps;
////
////    for (int ch = 0; ch < 2; ++ch) {
////
////        // intermediate interpolation values (in-region/nearby)(inner/outer)(azimuth/elevation)(plus/minus)
////        float inAp, nInAp, inAm, nInAm, inEp, nInEp, inEm, nInEm;
////        float outAp, nOutAp, outAm, nOutAm, outEp, nOutEp, outEm, nOutEm;
////        float netIn, netOut, netDVF;
////
////        // mutes the ear closest to the source, mysteriously the opposite ear buzzing goes away... thus indicating a problem with floating point dynamic range between the two channels.
//////        if (revAzi > M_PI) {
//////            if (ch == 1) {
//////                for (int i = 0; i < numTimeSteps; ++i)
//////                    hrir[numTimeSteps+i] = 0;
//////                goto DONE;
//////            }
//////        } else {
//////            if (ch == 0) {
//////                for (int i = 0; i < numTimeSteps; ++i)
//////                    hrir[i] = 0;
//////                goto DONE;
//////            }
//////        }
////
////        for (int i = 0; i < numTimeSteps; ++i) {
////
////            // interpolate along azimuth edges
////            mu1 = 2.0+(rae[2] - eM)/(eP-eM); // should be btw 2 and 3
////            nmu1 = std::abs(2.5-mu1); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
////            // direction of interp matters
////            if (nEleUp) {
////                nInAp = lagrangeInterpolate(HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2][ch][i],                  mu1-1.0);
////                nOutAp = lagrangeInterpolate(HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2][ch][i],                  mu1-1.0);
////                nInAm = lagrangeInterpolate(HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2][ch][i],                  mu1-1.0);
////                nOutAm = lagrangeInterpolate(HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2][ch][i],                  mu1-1.0);
////            } else {
////
////                nInAp = lagrangeInterpolate(HRIRdata[innerRadiusIndex][upperAzimuthIndex][nEle2][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],    mu1+1.0);
////                nOutAp = lagrangeInterpolate(HRIRdata[outerRadiusIndex][upperAzimuthIndex][nEle2][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],    mu1+1.0);
////                nInAm = lagrangeInterpolate(HRIRdata[innerRadiusIndex][lowerAzimuthIndex][nEle2][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],    mu1+1.0);
////                nOutAm = lagrangeInterpolate(HRIRdata[outerRadiusIndex][lowerAzimuthIndex][nEle2][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],    mu1+1.0);
////            }
////
////            inAp = (1.0-nmu1)*lagrangeInterpolate(HRIRdata[innerRadiusIndex][upperAzimuthIndex][lElem1][ch][i],
////                                                  HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][upperAzimuthIndex][uElep1][ch][i],               mu1)
////            + nmu1*nInAp;
////
////            inAm = (1.0-nmu1)*lagrangeInterpolate(HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lElem1][ch][i],
////                                                  HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][lowerAzimuthIndex][uElep1][ch][i],               mu1)
////            + nmu1*nInAm;
////
////            outAp = (1.0-nmu1)*lagrangeInterpolate(HRIRdata[outerRadiusIndex][upperAzimuthIndex][lElem1][ch][i],
////                                                   HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][upperAzimuthIndex][uElep1][ch][i],               mu1)
////            + nmu1*nOutAp;
////
////            outAm = (1.0-nmu1)*lagrangeInterpolate(HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lElem1][ch][i],
////                                                   HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][lowerAzimuthIndex][uElep1][ch][i],               mu1)
////            + nmu1*nOutAm;
////
////            // interpolate along elevation edges
////            mu2 = 2.0+(revAzi - aM)/(aP-aM); // should be btw 2 and 3
////            nmu2 = std::abs(2.5-mu2); // should be 0 when source is dead center in interp region, 0.5 when source is on boarder
////            // direction of interp matters
////            if (nAziUp) {
////                nInEp = lagrangeInterpolate(HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex][ch][i],                  mu2-1.0);
////                nOutEp = lagrangeInterpolate(HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex][ch][i],                  mu2-1.0);
////                nInEm = lagrangeInterpolate(HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][uAzip1][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][nAzi2][lowerElevationIndex][ch][i],                  mu2-1.0);
////                nOutEm = lagrangeInterpolate(HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][uAzip1][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][nAzi2][lowerElevationIndex][ch][i],                  mu2-1.0);
////            } else {
////
////                nInEp = lagrangeInterpolate(HRIRdata[innerRadiusIndex][nAzi2][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],    mu2+1.0);
////                nOutEp = lagrangeInterpolate(HRIRdata[outerRadiusIndex][nAzi2][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],    mu2+1.0);
////                nInEm = lagrangeInterpolate(HRIRdata[innerRadiusIndex][nAzi2][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lAzim1][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                            HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],    mu2+1.0);
////                nOutEm = lagrangeInterpolate(HRIRdata[outerRadiusIndex][nAzi2][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lAzim1][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                             HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],    mu2+1.0);
////            }
////
////            inEp = (1.0-nmu2)*lagrangeInterpolate(HRIRdata[innerRadiusIndex][lAzim1][upperElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][uAzip1][upperElevationIndex][ch][i],               mu2)
////            + nmu2*nInEp;
////
////            inEm = (1.0-nmu2)*lagrangeInterpolate(HRIRdata[innerRadiusIndex][lAzim1][lowerElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                                  HRIRdata[innerRadiusIndex][uAzip1][lowerElevationIndex][ch][i],               mu2)
////            + nmu2*nInEm;
////
////            outEp = (1.0-nmu2)*lagrangeInterpolate(HRIRdata[outerRadiusIndex][lAzim1][upperElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][lowerAzimuthIndex][upperElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][upperAzimuthIndex][upperElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][uAzip1][upperElevationIndex][ch][i],               mu2)
////            + nmu2*nOutEp;
////
////            outEm = (1.0-nmu2)*lagrangeInterpolate(HRIRdata[outerRadiusIndex][lAzim1][lowerElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][lowerAzimuthIndex][lowerElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][upperAzimuthIndex][lowerElevationIndex][ch][i],
////                                                   HRIRdata[outerRadiusIndex][uAzip1][lowerElevationIndex][ch][i],               mu2)
////            + nmu2*nOutEm;
////
////
////            // bring mu's from btw 2 and 3 down to btw 0 and 1
////            mu1 -= 2.0;
////            mu2 -= 2.0;
////
////            // interpolate across inner/outer surfaces
////            netIn = 0.5*(((1.0-mu1)*inEm + mu1*inEp) + ((1.0-mu2)*inAm + mu2*inAp));
////            netOut = 0.5*(((1.0-mu1)*outEm + mu1*outEp) + ((1.0-mu2)*outAm + mu2*outAp));
////
////            // linear interpolation of radius, may want logarithmic
////            if (rOut-rIn != 0) {
////                mu3 = (rae[0]-rIn)/(rOut-rIn);
////                netDVF = intensity_factor*(mu3*netOut + (1.0-mu3)*netIn);
////
////            } else if (rae[0] >= distanceEnd) {
////                // source is past the outer radius of hrir/dvf data, just use outermost surface data
////                netDVF = intensity_factor*netOut;
////                
////            } else if (rae[0] <= distanceBegin) {
////                // source is inside the inner radius of hrir/dvf data, just use innermost surface data
////                netDVF = intensity_factor*netIn;
////                
////            } else {
////                netDVF = 0;
////                // shouldn't ever get here, but if so write over the potential junk in the output array
////            }
////            
////            // the interped result
////            hrir[ch*numTimeSteps+i] = netDVF;
////        }
//////    DONE:
//////        bool channelChanged = true;
////    }
////}

