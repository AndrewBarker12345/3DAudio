/*
 MyImageEffectFilter.cpp
 
 Similar to Juce's ImageEffectFilter, but with value semantics as discussed by Sean Parent.

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

#include "ImageEffect.h"


MyGlowEffect::MyGlowEffect (Colour color,
                            float radius,
                            float originalAlpha) noexcept
    : color(color), radius(radius), originalAlpha(originalAlpha)
{}

void MyGlowEffect::applyEffect (Image& sourceImage,
                                Graphics& destContext,
                                const float scaleFactor,
                                const float alpha) const
{
    Image temp (sourceImage.getFormat(), sourceImage.getWidth(), sourceImage.getHeight(), true);
    
    ImageConvolutionKernel blurKernel (roundToInt (radius * scaleFactor * 2.0f));
    
    blurKernel.createGaussianBlur (radius);
    blurKernel.rescaleAllValues (radius);
    
    blurKernel.applyToImage (temp, sourceImage, sourceImage.getBounds());
    
    // need to duplicate source image in order to redraw it on top of the glow b/c the Graphics context might be set to draw onto the source image
    Image source = sourceImage;
    source.duplicateIfShared();
    
    destContext.setColour (color.withMultipliedAlpha (alpha));
    destContext.drawImageAt (temp, 0, 0, true);
    
    destContext.setOpacity (alpha * originalAlpha);
    destContext.drawImageAt (source, 0, 0);
}

std::unique_ptr<MyImageEffectFilter> MyGlowEffect::blendedTo (const MyImageEffectFilter* end,
                                                              const float alpha) const
{
    if (end == nullptr) {
        return std::make_unique<MyGlowEffect>
            (color.withMultipliedAlpha(1-alpha), radius * (1-alpha), originalAlpha);
    } else {
        auto temp = dynamic_cast<const MyGlowEffect*>(end);
        return std::make_unique<MyGlowEffect>
            (color.interpolatedWith(temp->color, alpha),
             radius + alpha * (temp->radius - radius),
             originalAlpha + alpha * (temp->originalAlpha - originalAlpha));
    }
}

bool MyGlowEffect::operator == (const MyImageEffectFilter& other) const noexcept
{
    if (getType() == (&other)->getType()) {
        auto x = dynamic_cast<const MyGlowEffect&>(other);
        return color == x.color && radius == x.radius && originalAlpha == x.originalAlpha;
    }
    return false;
}

bool MyGlowEffect::operator != (const MyImageEffectFilter& other) const noexcept
{
    if (getType() == (&other)->getType()) {
        auto x = dynamic_cast<const MyGlowEffect&>(other);
        return color != x.color || radius != x.radius || originalAlpha != x.originalAlpha;
    }
    return false;
}

MyImageEffectFilterType MyGlowEffect::getType() const noexcept
{
    return MyImageEffectFilterType::GLOW_EFFECT;
}



std::vector<ImageEffect> blend (const std::vector<ImageEffect>& begin,
                                const std::vector<ImageEffect>& end,
                                const float alpha)
{
    std::vector<ImageEffect> blendeds;// (std::max(begin.size(), end.size()));
    blendeds.reserve(std::max(begin.size(), end.size()));
    int i = 0;
    for (; i < std::min(begin.size(), end.size()); ++i)
        blendeds.emplace_back(begin[i].blendedTo(end[i], alpha));
    if (begin.size() > end.size()) {
        for (; i < begin.size(); ++i)
            blendeds.emplace_back(begin[i].blendedTo(ImageEffect(), alpha));
    } else {
        for (; i < end.size(); ++i)
            blendeds.emplace_back(end[i].blendedTo(ImageEffect(), 1-alpha));
    }
    return blendeds;
}
