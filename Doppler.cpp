/*
 Doppler.cpp
 
 Processes audio to simulate the doppler effect.
 
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

#include "Doppler.h"
#include <algorithm>

void Doppler::process(const float dist, const int N, const float* x, float* y) noexcept
{
    const float d = dist/speedOfSound*Fs;
    const int numInputs = inputs.size();
    // load the new input
    if (oldest == newest) // the first input cannot be stretched yet since we have only been passed one delay value so far
        inputs[newest].load(x, N, d, d, (float)N, N);
    else // all following inputs can be stretched with potentially different begin + end delays
    {    // for the 2nd buffer, N should equal NPrev, if not then the audio stretching will be slightly off for just that buffer
        int prev = newest-1;
        if (prev < 0)
            prev = numInputs-1;
        const int NPrev = inputs[prev].inputLength;
        const float NstrPrev = inputs[prev].dEnd - inputs[prev].dBegin;
        inputs[newest].load(x, N, dPrev, d, NstrPrev, NPrev);
    }
    newest = (newest+1) % numInputs;
    // zero output array
    for (int n = 0; n < N; ++n)
        y[n] = 0;
    // loop though inputs and compute the samples for this buffer
    int next, begin, end;
    for (int i = oldest; i != newest;)
    {
        next = (i+1) % numInputs;
        // if the input is needed for the current buffer
        if (inputs[i].dBegin <= N-1)
        {
            begin = std::max(0, (int)std::ceil(inputs[i].dBegin));
            end   = std::min(N, (int)std::ceil(inputs[i].dEnd));
            for (int n = begin; n < (const int)end; ++n)
                y[n] += inputs[i].valueAt(n, inputs[next].input[0]);
        }
        inputs[i].dBegin -= N;
        inputs[i].dEnd -= N;
        inputs[i].shift += N;
        if (inputs[i].dEnd < 0)
            oldest = next;
        i = next;
    }
    dPrev = d;
}

void Doppler::reset() noexcept
{
    oldest = 0;
    newest = 0;
    dPrev = 0;
}

void Doppler::setSampleRate(const float sampleRate) noexcept
{
    Fs = sampleRate;
}

float Doppler::getSpeedOfSound() const noexcept
{
    return speedOfSound;
}

void Doppler::allocate(const float max_Dist, const int max_N, const float newSpeedOfSound)
{
    // maximum time delay in samples that must be supported
    maxDistance = max_Dist;
    maxN = max_N;
    speedOfSound = newSpeedOfSound;
    allocate();
}

void Doppler::allocate()
{
    const float maxDelayInSamples = maxDistance/speedOfSound*Fs;
    inputs.resize(std::max(3, (int)std::ceil(maxDelayInSamples/maxN))); // need at least 3 inputs otherwise stuff gets weird...
    for (auto& input : inputs)
        input.allocate(maxN);
    inputs.shrink_to_fit();
    reset();
}

void Doppler::free()
{
    inputs.clear();
    inputs.shrink_to_fit();
}
