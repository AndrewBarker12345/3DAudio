//
//  Doppler.h
//  ThreeDAudio
//
//  Created by Andrew Barker on 4/2/18.
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

#ifndef __Doppler__
#define __Doppler__

#include <vector>

static constexpr float defaultSpeedOfSound = 343.0f; // in meters/sec

class Doppler
{
public:
	/*Doppler() noexcept;
	~Doppler();*/
    /** process an input audio buffer at certain distance from the listener such that the doppler effect is applied to output */
	void process(float distance, int bufferSize, const float* input, float* output) noexcept;
    /** allocate enough memory for the doppler effect given a maximum sound source distance (in meters), maximum buffer size, and minimum speed of sound (in m/s) */
	void allocate(float maxDistance, int maxBufferSize, float speedOfSoundToPlanAllocationSize);
    /** free all memory */
	void free()	noexcept;
    /** reset the doppler effect state */
	void reset() noexcept;
    /** specify the sample rate of the audio being processed */
	void setSampleRate(float sampleRate) noexcept;
    /** set the speed of sound for the doppler effect */
	void setSpeedOfSound(float speedOfSound) noexcept;
private:
	// circular buffer for holding delayed input
	std::vector<float> buffer;
	// next index to insert input in circular buffer
	float bufferInIdx = 0;
	// next index to output from circular buffer
	float bufferOutIdx = 0;
	// sample rate (in Hz)
	float sampleRate = 44100; 
	// in meters / sec
	float speedOfSound = defaultSpeedOfSound;
	// previous buffer's delay at end (in samples) 
	float delayPrev = -1; 
	// distance delay over input sample slope at begining of current input buffer / end of last input buffer
	float slopePrev = 0;
    // previous sample put into circular buffer
	float prevSample = 0;
    // previous sample's delay compensated index
	float prevSampleDelayedIdx = 0;
};





















// OLD DOPPLER EFFECT
//#include "PolynomialSpline.h"
//
//static constexpr float defaultSpeedOfSound = 343.0f; // in meters/sec
////#define SPEED_OF_WAVE  343 //60 //100 // in meters/sec
////#define ONE_OVER_SoW 0.16//0.00291545189// 0.016 // in sec/meters
////#define INIT_MAX_CHANGE 0.01
//
//// structure to hold previous input buffers with their relative delays and stretching
//typedef struct _DelayedInput
//{
//    std::vector<float> input; // an input buffer
//    int inputLength = 0; // the length of the buffer actually used
//    float dBegin = 0; // delay in samples from the current buffer's start to the beginning of input
//    float dEnd = 0; // delay in samples from the current buffer's start to the end of input (beginning sample of next input)
//    int shift = 0;
//    int prevSign = 1;
//    //CubicFunctionalSpline<float> spline;
//    LightweightCubicFunctionalSpline<float, 2> spline; // a 3rd order 2D polynomial curve for generating smoothly stretched output buffers
//    //_DelayedInput
//    void load(const float* buf, int N,
//              float delayBegin, float delayEnd,
//              float prevNstr,   int prevN) noexcept
//    {
//        //input = std::vector<float>(buf, buf+N);
//        for (int i = 0; i < N; ++i)
//            input[i] = buf[i];
//        const int inputSize = input.size();
//        for (int i = N; i < inputSize; ++i)
//            input[i] = 0;
//        inputLength = N;
//        shift = 0;
//        float yBegin, yEnd;
//        if (delayBegin < delayEnd+N)
//        {
//            dBegin = delayBegin;
//            dEnd = delayEnd+N;
//            yBegin = 0;
//            yEnd = N;
//            const float splinePts[4][2] =
//                { {dBegin-prevNstr,     yBegin-prevSign*prevN},
//                  {dBegin,              yBegin},
//                  {dEnd,                yEnd},
//                  {dEnd+(dEnd-dBegin),  yEnd+N}              }; // problem here is that if we switch sign the next buffer, the end of this spline's slope will not be perfectly continuous with the begining of the next spline, we're not set up to know the future here... however this seems preferable to forcing the audio to lag one buffer in order to compute a perfectly continuous spline segment, plus the discontinuity should be rather small if the sound barrier is approached gradually.
//            spline.calc(splinePts);
////            spline = CubicFunctionalSpline<float>
////            ({dBegin-prevNstr, yBegin-prevSign*prevN},
////             {dBegin,yBegin}, {dEnd,yEnd},
////             {dEnd+(dEnd-dBegin), yEnd+N});
//            prevSign = 1;
//        }
//        else // averaged faster than SPEED_OF_WAVE for change in distance btw buffers, so audio gets flipped backwards
//        {
//            dBegin = delayEnd+N;
//            dEnd = delayBegin;
//            yBegin = N;
//            yEnd = 0;
//            const float splinePts[4][2] =
//                { {dBegin-prevNstr,     yBegin-prevSign*prevN},
//                  {dBegin,              yBegin},
//                  {dEnd,                yEnd},
//                  {dEnd+(dEnd-dBegin),  yEnd-N}              };
//            spline.calc(splinePts);
////            spline = CubicFunctionalSpline<float>
////            ({dBegin-prevNstr, yBegin-prevSign*prevN},
////             {dBegin,yBegin}, {dEnd,yEnd},
////             {dEnd+(dEnd-dBegin), yEnd-N});
//            prevSign = -1;
//        }
////        spline = CubicFunctionalSpline<float>
////                 ({dBegin-prevNstr, yBegin-prevN},
////                  {dBegin,yBegin}, {dEnd,yEnd},
////                  {dEnd+(dEnd-dBegin), yEnd+N});
//    }
//    void allocate(int maxN)
//    {
//        input.resize(maxN, 0);
//        input.shrink_to_fit(); // not tested thoroughly
//        spline.allocate();
//    }
//    float valueAt(float s, float nextInputSample) noexcept
//    {
//        //float index = spline.pointAt(s+shift)[0];
//        float index;
//        spline.pointAt(s+shift, &index);
//        int low = index; //floor(index);
//        int high = low + 1;
//        float k = high - index;
//        float value = k*input[low];
//        if (high < inputLength/*input.size()*/)
//            value += (1-k)*input[high];
//        else
//            value += (1-k)*nextInputSample;
//        return value;
//        //return k*input[low] + (1-k)*input[high];
//    }
//} DelayedInput;
//
//class Doppler
//{
//public:
//    Doppler() {};
//    ~Doppler() {};
//    // output N processed samples in y given a current distance dist (in meters), and N input samples in x
//    void process(float dist, int N, const float* x, float* y) noexcept;
//    // processes with an array of N distances, one for each sample
//    //void process(const float* dists, int N, const float* x, float* y) noexcept;
//    // reset the processing state
//    void reset() noexcept;
//    void setSampleRate(float sampleRate) noexcept;
//    //void setSpeedOfSound(float newSpeedOfSound);
//    float getSpeedOfSound() const noexcept;
//    // pre-allocate all internal storage prior to realtime audio processing
//    void allocate(float maxDist, int max_N, float newSpeedOfSound);
//    void free();
//private:
//    void allocate();
//    std::vector<DelayedInput> inputs; // storage for delayed and stretched input audio
//    float Fs = 44100; // sample rate (in Hz)
//    float dPrev = 0; // previous buffer's delay (in samples)
//    //float maxChange = INIT_MAX_CHANGE; // in percent of input buffer samples
//    //float dDesiredPrev = 0;
//    // indecies for the range of inputs yet to be played back
//    int oldest = 0;
//    int newest = 0;
//    float maxN = 0;
//    float maxDistance = 0;
//    float speedOfSound = defaultSpeedOfSound; // in meters / sec
//};

#endif /* defined(__Doppler__) */
