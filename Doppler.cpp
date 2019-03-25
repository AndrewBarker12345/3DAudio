//
//  Doppler.cpp
//  ThreeDAudio
//
//  Created by Andrew Barker on 4/16/15.
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

#include "Doppler.h"

void Doppler::process(const float distance, const int bufferSize, const float* input, float* output) noexcept
{
	const auto delay = distance / speedOfSound * sampleRate;
	const auto cBufSize = buffer.size();
	if (delayPrev == -1) {
		delayPrev = delay;
		prevSampleDelayedIdx = delay;
	}
	const auto slope = (delay - delayPrev) / bufferSize;
	const auto a0 = delayPrev;
	const auto a1 = slopePrev;
	float _a2, _a3;
	if ((slope > 0 && slopePrev < 0) || (slope < 0 && slopePrev > 0)) { // use 3rd degree polynomial interpolation for sample delay
		_a2 = 2 * (slope - slopePrev) / bufferSize;
		_a3 = (slopePrev - slope) / (bufferSize * bufferSize);
	}
	else { // use 2nd degree polynomial interpolation for sample delay
		_a2 = (slope - slopePrev) / (2 * bufferSize);
		_a3 = 0;
	}
	const auto a2 = _a2, a3 = _a3;
    const auto denom = a1*bufferSize + a2*bufferSize*bufferSize + a3*bufferSize*bufferSize*bufferSize;
	const auto delayScale = denom == 0 ? 0 : (delay - delayPrev) / denom; // check for div by 0
	for (int n = 0; n < bufferSize; ++n) {
		const auto delayedIdx = a0 + (a1*n + a2*n*n + a3*n*n*n) * delayScale;
		auto fidx = bufferInIdx + delayedIdx;
		while (fidx >= cBufSize)
			fidx -= cBufSize;
		while (fidx < 0)
			fidx += cBufSize;
		const int idxP1 = int(fidx) + 1 == cBufSize ? 0 : int(fidx) + 1;
		const int prevIdxP1 = int(prevSampleDelayedIdx) + 1 == cBufSize ?
			0 : int(prevSampleDelayedIdx) + 1;
		bool forwards = true;
		if (idxP1 < prevIdxP1 && !(idxP1 + cBufSize - prevIdxP1 < prevIdxP1 - idxP1))
			forwards = false;
		if (forwards) {
			for (int i = prevIdxP1; i != idxP1; i = i + 1 == cBufSize ? 0 : i + 1) {
				const auto blend = (i - prevSampleDelayedIdx < 0 ?
					cBufSize - prevSampleDelayedIdx + i : i - prevSampleDelayedIdx)
					/
					(fidx - prevSampleDelayedIdx < 0 ?
						cBufSize - prevSampleDelayedIdx + fidx : fidx - prevSampleDelayedIdx);
				buffer[i] += prevSample + (input[n] - prevSample) * blend;
			}
		}
		else {
			for (int i = idxP1; i != prevIdxP1; i = i + 1 == cBufSize ? 0 : i + 1) {
				const auto blend = (i - fidx < 0 ?
					cBufSize - fidx + i : i - fidx)
					/
					(prevSampleDelayedIdx - fidx < 0 ?
						cBufSize + prevSampleDelayedIdx - fidx : prevSampleDelayedIdx - fidx);
				buffer[i] += prevSample + (input[n] - prevSample) * (1 - blend);
			}
		}
		prevSample = input[n];
		prevSampleDelayedIdx = fidx;
		bufferInIdx = bufferInIdx + 1 == cBufSize ? 0 : bufferInIdx + 1;
	}
	slopePrev = slope;
	delayPrev = delay;
	for (int n = 0; n < bufferSize; ++n) {
		output[n] = buffer[bufferOutIdx];
		buffer[bufferOutIdx] = 0;
		bufferOutIdx = bufferOutIdx + 1 == cBufSize ? 0 : bufferOutIdx + 1;
	}
}

void Doppler::allocate(const float maxDistance, const int maxBufferSize, const float speedOfSoundToPlanAllocationSize)
{
	const auto maxDelayInSamples = maxDistance / speedOfSoundToPlanAllocationSize * sampleRate;
	buffer.resize(maxBufferSize + maxDelayInSamples);
	reset();
}

void Doppler::free() noexcept
{
	buffer.clear();
}

void Doppler::reset() noexcept
{
	for (auto& x : buffer)
		x = 0;
	delayPrev = -1;
	slopePrev = 0;
	bufferInIdx = bufferOutIdx = 0;
	prevSample = 0;
}

void Doppler::setSampleRate(const float _sampleRate) noexcept
{
	sampleRate = _sampleRate;
}

void Doppler::setSpeedOfSound(const float _speedOfSound) noexcept
{
	speedOfSound = _speedOfSound;
}

//#include <algorithm>
//
//void Doppler::process(const float dist, const int N, const float* x, float* y) noexcept
//{
//    // time delay in samples, measured from the first sample of the next buffer to the beginning of the next buffer's delayed waveform
////    float d; // the delay actually used for this buffer
////    const float dDesired = dist*ONE_OVER_SoW*Fs; // the desired delay (the one passed in this buffer)
////    // the following shit is just to smooth the distance changes when they are comming erraticly from the user input thread as opposed to the nice consistent changes that the interpolators generate on playback
////    const float deviance = maxChange*((float)N); // how much the delay changes from the previous buffer for this buffer
////    const float dif = std::abs(dDesired-dPrev); // how much distance change is there to still be made up
////    const float totalDif = std::abs(dDesired-dDesiredPrev); // total distance change between each "step"
////    if (dif > INIT_MAX_CHANGE*N)
////    {
////        if (dDesired < dPrev)
////            d = dPrev - deviance;
////        else
////            d = dPrev + deviance;
////        if (/*std::abs(dDesired-d)*/dif > totalDif*0.5) // less than half way there
////            maxChange *= 1.05;//2.0;
////        else // more than half way there
////            maxChange /= 1.05;//0.5;
////    }
////    else
////    {
////        dDesiredPrev = dDesired;
////        d = dDesired;
////        maxChange = INIT_MAX_CHANGE;
////    }
//    const float d = dist/speedOfSound*Fs;
//    //const float d = dist*ONE_OVER_SoW*Fs;
//    const int numInputs = inputs.size();
//    // load the new input
//    //if (inputs.size() == 0)
//    if (oldest == newest)
//    {   // the first input cannot be stretched yet since we have only been passed one delay value so far
//        //inputs.emplace_back(DelayedInput(x, N, d, d, (float)N, N));
//        inputs[newest].load(x, N, d, d, (float)N, N);
//        //inputs[newest].load(x, N, d, d, (float)N, N, N, N);
//    }
//    else // all following inputs can be stretched with potentially different begin + end delays
//    {    // for the 2nd buffer, N should equal NPrev, if not then the audio stretching will be slightly off for just that buffer
//        //int NPrev = inputs.back().input.size();
//        //float NstrPrev = inputs.back().dEnd - inputs.back().dBegin;
//        //inputs.emplace_back(DelayedInput(x, N, dPrev, d, NstrPrev, NPrev));
//        int prev = newest-1;
//        if (prev < 0)
//            prev = numInputs-1;
//        const int NPrev = inputs[prev].inputLength;
//        const float NstrPrev = inputs[prev].dEnd - inputs[prev].dBegin;
//        inputs[newest].load(x, N, dPrev, d, NstrPrev, NPrev);
//    }
//    newest = (newest+1) % numInputs;
//    // zero output array
//    for (int n = 0; n < N; ++n)
//        y[n] = 0;
//    // loop though inputs and compute the samples for this buffer
//    //for (int i = 0; i < inputs.size(); ++i)
//    int next, begin, end;
//    for (int i = oldest; i != newest;)
//    {
//        next = (i+1) % numInputs;
//        // if the input is needed for the current buffer
//        if (inputs[i].dBegin <= N-1)
//        {
//            begin = std::max(0, /*((int)inputs[i].dBegin+1)*/ (int)std::ceil(inputs[i].dBegin));
//            end   = std::min(N, /*((int)inputs[i].dEnd+1)*/ (int)std::ceil(inputs[i].dEnd));
////            if (i < inputs.size()-1)
////            {
//                for (int n = begin; n < (const int)end; ++n)
//                    y[n] += inputs[i].valueAt(n, inputs[next/*i+1*/].input[0]);
////            }
////            else
////            {
////                for (int n = begin; n < end; ++n)
////                    y[n] += inputs[i].valueAt(n, 0);
////            }
//        }
//        inputs[i].dBegin -= N;
//        inputs[i].dEnd -= N;
//        inputs[i].shift += N;
//        if (inputs[i].dEnd < 0)
//        {
//            //inputs.erase(inputs.begin()+i);
//            //--i;
//            oldest = next;
//        }
//        i = next;
//    }
//    dPrev = d;
//}
//
////void Doppler::process(const float* dists, int N, const float* x, float* y) noexcept
////{
////    
////}
//
//void Doppler::reset() noexcept
//{
//    //inputs.clear();
//    oldest = 0;
//    newest = 0;
//    dPrev = 0;
//}
//
//void Doppler::setSampleRate(const float sampleRate) noexcept
//{
//    Fs = sampleRate;
//}
//
////void Doppler::setSpeedOfSound(const float newSpeedOfSound)
////{
////    if (speedOfSound != newSpeedOfSound)
////    {
////        allocate();
////    }
////    speedOfSound = newSpeedOfSound;
////}
//
//float Doppler::getSpeedOfSound() const noexcept
//{
//    return speedOfSound;
//}
//
//void Doppler::allocate(const float max_Dist, const int max_N, const float newSpeedOfSound)
//{
//    // maximum time delay in samples that must be supported
//    maxDistance = max_Dist;
//    maxN = max_N;
//    speedOfSound = newSpeedOfSound;
//    allocate();
////    const float maxDelayInSamples = maxDistance/speedOfSound*Fs;
////    inputs.resize(std::ceil(maxDelayInSamples/maxN));
////    for (auto& input : inputs)
////        input.allocate(maxN);
////    reset();
//}
//
//void Doppler::allocate()
//{
//    const float maxDelayInSamples = maxDistance/speedOfSound*Fs;
//    inputs.resize(std::max(3, (int)std::ceil(maxDelayInSamples/maxN))); // need at least 3 inputs otherwise stuff gets weird...
//    for (auto& input : inputs)
//        input.allocate(maxN);
//    inputs.shrink_to_fit();
//    reset();
//}
//
//void Doppler::free()
//{
//    inputs.clear();
//    inputs.shrink_to_fit();
//}
//
//
//Doppler::Doppler() noexcept
//{
//}
//
//Doppler::~Doppler()
//{
//}
