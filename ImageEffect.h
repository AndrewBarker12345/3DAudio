/*
 MyImageEffectFilter.h
 
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

#ifndef ImageEffect_h
#define ImageEffect_h

#include "../JuceLibraryCode/JuceHeader.h"
#include <memory>

enum class MyImageEffectFilterType { GLOW_EFFECT };

class MyImageEffectFilter
{
public:
    virtual void applyEffect (Image& sourceImage,
                              Graphics& destContext,
                              float scaleFactor,
                              float alpha) const = 0;
    
    virtual std::unique_ptr<MyImageEffectFilter> blendedTo (const MyImageEffectFilter* end,
                                                            float alpha) const = 0;
    
    virtual ~MyImageEffectFilter() = default;
    
    virtual bool operator == (const MyImageEffectFilter& other) const noexcept = 0;
    virtual bool operator != (const MyImageEffectFilter& other) const noexcept = 0;
    virtual MyImageEffectFilterType getType() const noexcept = 0;
};

class MyGlowEffect : public MyImageEffectFilter
{
public:
    MyGlowEffect (Colour color = Colours::white,
                  float radius = 2,
                  float originalAlpha = 1) noexcept;
    
    void applyEffect (Image& sourceImage,
                      Graphics& destContext,
                      float scaleFactor,
                      float alpha) const override;
    
    std::unique_ptr<MyImageEffectFilter> blendedTo (const MyImageEffectFilter* end,
                                                    float alpha) const override;
    
    bool operator == (const MyImageEffectFilter& other) const noexcept override;
    bool operator != (const MyImageEffectFilter& other) const noexcept override;
    MyImageEffectFilterType getType() const noexcept override;

    Colour color;
    float radius;
    float originalAlpha;
};

class ImageEffect
{
public:
    ImageEffect() : self_(nullptr) {}
    
    template <class T>
    ImageEffect(T x) : self_(std::make_shared<model<T>>(std::move(x))) {}
    
    void apply(Image& source, Graphics& dest, float scale, float alpha) const
    { if (self_) self_->applyEffect(source, dest, scale, alpha); }

    ImageEffect blendedTo(const ImageEffect& end, float alpha) const
    { return self_ ? self_->blendedTo(end.self_.get(), alpha) : ImageEffect(); }
    
    bool operator == (const ImageEffect& other) const noexcept
    { return *self_ == *other.self_; }
    
    bool operator != (const ImageEffect& other) const noexcept
    { return *self_ != *other.self_; }
    
private:
    ImageEffect(std::unique_ptr<MyImageEffectFilter>&& x) : self_(std::move(x)) {}
    
    template <class T>
    struct model : MyImageEffectFilter {
        model(T x) : data_(std::move(x)) {}
        
        void applyEffect(Image& source, Graphics& dest, float scale, float alpha) const override
        { (&data_)->applyEffect(source, dest, scale, alpha); }
        
        std::unique_ptr<MyImageEffectFilter> blendedTo(const MyImageEffectFilter* end, float alpha) const override
        { return (&data_)->blendedTo(end ? &dynamic_cast<const model<T>*>(end)->data_ : nullptr, alpha); }
        
        bool operator == (const MyImageEffectFilter& other) const noexcept override
        { return (&data_)->operator==(other); }
        
        bool operator != (const MyImageEffectFilter& other) const noexcept override
        { return (&data_)->operator!=(other); }

        MyImageEffectFilterType getType() const noexcept override
        { return (&data_)->getType(); }
        
        T data_;
    };
    
    std::shared_ptr<const MyImageEffectFilter> self_;
};

// the two effects vectors better have the same ordering of ImageEffect subclasses for the effects that both share, otherwise not good stuff may happen
std::vector<ImageEffect> blend (const std::vector<ImageEffect>& begin,
                                const std::vector<ImageEffect>& end,
                                const float alpha);

#endif /* ImageEffect_h */
